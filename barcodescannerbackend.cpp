#include "barcodescannerbackend.hpp"
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>

decltype(BarcodeScannerBackend::glActiveTexture) BarcodeScannerBackend::glActiveTexture = nullptr;

class BarcodeScannerDummyBackend : public BarcodeScannerBackend {
public:
	BarcodeScannerDummyBackend(const QVideoSurfaceFormat &format)
	: BarcodeScannerBackend(format) {
		m_vtxCode = simpleVertexShader();
		m_fragCode = R"(
			varying highp vec2 texCoord;
			void main() {
				gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
			}
		)";
	}
	void link(QOpenGLShaderProgram *prog) override {
		loc_vMatrix = prog->uniformLocation("vMatrix");
	}
	void update(QOpenGLShaderProgram *prog, const RenderState &state) override {
		prog->setUniformValue(loc_vMatrix, state.combinedMatrix());
	}
	void upload(const QVideoFrame &frame) override { Q_UNUSED(frame); }
	BarcodeScannerImagePtr toImage(const QVideoFrame &frame) const override {
		Q_UNUSED(frame); return BarcodeScannerImagePtr();
	}
};

class BarcodeScannerYCbCrBackend : public BarcodeScannerBackend {
	struct YCbCrRange {
		YCbCrRange &operator *= (float rhs) { y1 *= rhs; y2 *= rhs;  c1 *= rhs; c2 *= rhs; return *this; }
		YCbCrRange operator * (float rhs) const { return YCbCrRange(*this) *= rhs; }
		float y1, y2, c1, c2;
	};
	static QMatrix3x3 matYCbCrToRgb(double kb, double kr, const YCbCrRange &range) {
		const double dy = 1.0/(range.y2-range.y1);
		const double dc = 2.0/(range.c2-range.c1);
		const double kg = 1.0 - kb - kr;
		QMatrix3x3 mat;
		mat(0, 0) = dy; mat(0, 1) = 0.0;                mat(0, 2) = (1.0 - kr)*dc;
		mat(1, 0) = dy; mat(1, 1) = -dc*(1.0-kb)*kb/kg; mat(1, 2) = -dc*(1.0-kr)*kr/kg;
		mat(2, 0) = dy; mat(2, 1) = dc*(1-kb);          mat(2, 2) = 0.0;
		return mat;
	}
	static QMatrix3x3 matRgbToYCbCr(double kb, double kr, const YCbCrRange &range) {
		const double dy = (range.y2-range.y1);
		const double dc = (range.c2-range.c1)/2.0;
		const double kg = 1.0 - kb - kr;
		QMatrix3x3 mat;
		mat(0, 0) = dy*kr;              mat(0, 1) = dy*kg;              mat(0, 2) = dy*kb;
		mat(1, 0) = -dc*kr/(1.0 - kb);  mat(1, 1) = -dc*kg/(1.0-kb);    mat(1, 2) = dc;
		mat(2, 0) = dc;                 mat(2, 1) = -dc*kg/(1.0-kr);    mat(2, 2) = -dc*kb/(1.0 - kr);
		return mat;
	}
	using ColumnVector3 = QGenericMatrix<1, 3, float>;
	static ColumnVector3 make3x1(float v1, float v2, float v3) {
		ColumnVector3 vec;
		vec(0, 0) = v1; vec(1, 0) = v2; vec(2, 0) = v3;
		return vec;
	}
	static ColumnVector3 make3x1(float v1, float v23) { return make3x1(v1, v23, v23); }

	GLuint m_textures[3] = {GL_NONE, GL_NONE, GL_NONE};
	int m_texIndex[3] = {0, 1, 2};
	int loc_mul_mat = -1, loc_add_vec = -1, loc_tex[3] = {-1, -1, -1};
	QVector3D m_add_vec = {0.f, 0.f, 0.f};
	QMatrix3x3 m_mul_mat;
	QSize m_textureSize;
public:
	BarcodeScannerYCbCrBackend(const QVideoSurfaceFormat &format)
	: BarcodeScannerBackend(format) {
		Q_ASSERT(format.handleType() == QAbstractVideoBuffer::NoHandle);
		switch (format.pixelFormat()) {
		case QVideoFrame::Format_YV12:
			qSwap(m_texIndex[1], m_texIndex[2]);
		case QVideoFrame::Format_YUV420P:
			break;
		default:
			Q_ASSERT_X(false, "BarcodeScannerYCbCrBackend::BarcodeScannerYCbCrBackend()"
				, "Wrong Pixel Format!");
			return;
		}
		glGenTextures(3, m_textures);

		auto ycbcr = format.yCbCrColorSpace();
		if (ycbcr == QVideoSurfaceFormat::YCbCr_Undefined)
			ycbcr = format.frameHeight() < 720 ?
						QVideoSurfaceFormat::YCbCr_BT601 : QVideoSurfaceFormat::YCbCr_BT709;

		float kb, kr;
		switch (ycbcr) {
		case QVideoSurfaceFormat::YCbCr_BT601:
		case QVideoSurfaceFormat::YCbCr_xvYCC601:
			kb = 0.1140; kr = 0.2990; break;
		case QVideoSurfaceFormat::YCbCr_BT709:
		case QVideoSurfaceFormat::YCbCr_xvYCC709:
			kb = 0.0722; kr = 0.2126; break;
		default:
			return;
		}

		YCbCrRange range;
		switch (ycbcr) {
		case QVideoSurfaceFormat::YCbCr_BT601:
		case QVideoSurfaceFormat::YCbCr_BT709:
			range = { 16.f, 235.f, 16.f, 240.f}; break;
		case QVideoSurfaceFormat::YCbCr_xvYCC601:
		case QVideoSurfaceFormat::YCbCr_xvYCC709:
			range = {  0.f, 255.f,  0.f, 255.f}; break;
		default:
			return;
		}
		range *= 1.0/255.f;

		const auto matRgbFromYCbCr = matYCbCrToRgb(kb, kr, range);
		const auto vecRgbFromYCbCr = -(matRgbFromYCbCr*make3x1(range.y1, (range.c1 + range.c2)*0.5));
		m_mul_mat = matRgbFromYCbCr;
		m_add_vec = {vecRgbFromYCbCr(0, 0), vecRgbFromYCbCr(1, 0), vecRgbFromYCbCr(2, 0)};

		m_fragCode = R"(
			uniform sampler2D tex0, tex1, tex2;
			uniform highp mat3 mul_mat;
			uniform highp vec3 add_vec;
			varying highp vec2 texCoord;
			void main() {
				const highp vec2 one = vec2(1.0, 0.0);
				highp vec3 tex;
				tex.x = texture2D(tex0, texCoord).x;
				tex.y = texture2D(tex1, texCoord).x;
				tex.z = texture2D(tex2, texCoord).x;
				tex = mul_mat*tex + add_vec;
				tex = clamp(tex, 0.0, 1.0);
				gl_FragColor = tex.rgbr*one.xxxy + one.yyyx;
			}
		)";
		m_vtxCode = simpleVertexShader();
	}
	~BarcodeScannerYCbCrBackend() { glDeleteTextures(3, m_textures); }
	void link(QOpenGLShaderProgram *prog) override {
		loc_vMatrix = prog->uniformLocation("vMatrix");
		loc_add_vec = prog->uniformLocation("add_vec");
		loc_mul_mat = prog->uniformLocation("mul_mat");
		loc_tex[0] = prog->uniformLocation("tex0");
		loc_tex[1] = prog->uniformLocation("tex1");
		loc_tex[2] = prog->uniformLocation("tex2");
	}
	void upload(const QVideoFrame &frame) {
		Q_ASSERT(frame.isReadable());
		const int ws[3] = {frame.bytesPerLine(), frame.bytesPerLine()/2, frame.bytesPerLine()/2};
		const int hs[3] = {frame.height(), frame.height()/2, frame.height()/2};
		const QSize size(ws[0], hs[0]);
		if (m_textureSize != size) {
			m_textureSize = size;
			for (int i=0; i<3; ++i)
				newTex(m_textures[i], ws[i], hs[i], GL_LUMINANCE, GL_LUMINANCE);
		}
		auto p = frame.bits();
		for (int i=0; i<3; ++i) {
			uploadTex(m_textures[m_texIndex[i]], ws[i], hs[i], GL_LUMINANCE, p);
			p += ws[i]*hs[i];
		}
	}
	void update(QOpenGLShaderProgram *prog, const RenderState &state) override {
		for (int i=0; i<3; ++i) {
			activeTex(m_textures[m_texIndex[i]], i);
			prog->setUniformValue(loc_tex[i], i);
		}
		prog->setUniformValue(loc_mul_mat, m_mul_mat);
		prog->setUniformValue(loc_add_vec, m_add_vec);
		prog->setUniformValue(loc_vMatrix, state.combinedMatrix());
		glActiveTexture(GL_TEXTURE0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	BarcodeScannerImagePtr toImage(const QVideoFrame &frame) const override {
		Q_ASSERT(frame.isReadable());
		const QSize size{frame.bytesPerLine(), frame.height()};
		QByteArray data; data.resize(size.width()*size.height());
		memcpy(data.data(), frame.bits(), data.size());
		return BarcodeScannerImage::allocate(data, size, BarcodeScannerImage::Y8);
	}
};

#ifdef Q_OS_ANDROID
#define ExternalGLTextureHandle (QAbstractVideoBuffer::UserHandle + 1)

class BarcodeScannerAndroidBackend : public BarcodeScannerBackend {
	int loc_tex = -1;

	static const char *vertexShaderCode() {
		return R"(
			uniform highp mat4 vMatrix;
			uniform highp mat4 tMatrix;
			attribute highp vec4 vPosition;
			attribute highp vec2 vCoord;
			varying highp vec2 texCoord;
			void main() {
				texCoord = (tMatrix*vec4(vCoord, 0.0, 1.0)).xy;
				gl_Position = vMatrix*vPosition;
			}
		)";
	}

	static QOpenGLFunctions *func() { return QOpenGLContext::currentContext()->functions(); }

	class OpenGLFramebufferObject {
	public:
		OpenGLFramebufferObject(const QSize &size): m_size(size) {
			auto f = func();
			f->glGenFramebuffers(1, &m_id);
			f->glBindFramebuffer(GL_FRAMEBUFFER, m_id);
			glGenTextures(1, &m_texture);
			newTex(m_texture, size.width(), size.height(), GL_RGBA, GL_RGBA);
			bindTex(GL_NONE);
			f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
			release();
		}
		virtual ~OpenGLFramebufferObject() {
			glDeleteTextures(1, &m_texture);
			func()->glDeleteFramebuffers(1, &m_id);
		}
		QRect rect() const { return {0, 0, width(), height()}; }
		int width() const { return m_size.width(); }
		int height() const { return m_size.height(); }
		QSize size() const { return m_size; }
		void bind() const { func()->glBindFramebuffer(GL_FRAMEBUFFER, m_id); }
		void release() const { QOpenGLFramebufferObject::bindDefault(); }
		QByteArray toImage() const {
			QByteArray data; data.resize(m_size.width()*m_size.height()*4);
			glReadPixels(0, 0, width(), height(), GL_RGBA, GL_UNSIGNED_BYTE, data.data());
			return data;
		}
		GLuint texture() const { return m_texture; }
	private:
		GLuint m_id = GL_NONE, m_texture = GL_NONE;
		QSize m_size;
		QOpenGLShaderProgram m_prog;
	};

	class ExternalTextureRenderer {
		enum {vPosition, vCoord};
	public:
		ExternalTextureRenderer() {
			m_vPositions = vertices({QPointF{-1.0, -1.0}, QPointF{1.0, 1.0}}, Qt::PrimaryOrientation);
			m_vCoords = { 0.0, 1.0, /**/ 1.0, 1.0, /**/ 0.0, 0.0, /**/ 1.0, 0.0 };

			m_prog.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderCode());
			m_prog.addShaderFromSourceCode(QOpenGLShader::Fragment, R"(
				#extension GL_OES_EGL_image_external : require
				uniform samplerExternalOES tex;
				varying highp vec2 texCoord;
				void main() {
//					const lowp vec3 coef = vec3(0.2126, 0.7152, 0.0722);
//					lowp float luma = dot(texture2D(tex, texCoord).rgb, coef);
//					gl_FragColor = vec4(luma, luma, luma, 1.0);
					gl_FragColor = texture2D(tex, texCoord);
				}
			)");

			m_prog.bindAttributeLocation("vPosition", vPosition);
			m_prog.bindAttributeLocation("vCoord", vCoord);

			m_prog.link();
			Q_ASSERT(m_prog.isLinked());

			loc_tex = m_prog.uniformLocation("tex");
			loc_vMatrix = m_prog.uniformLocation("vMatrix");
			loc_tMatrix = m_prog.uniformLocation("tMatrix");
		}
		void render(const QVideoFrame &frame, Qt::ScreenOrientation orientation) {
			Q_ASSERT(frame.handleType() == ExternalGLTextureHandle);
			Q_ASSERT(frame.pixelFormat() == QVideoFrame::Format_BGR32);
			if (!frame.isValid())
				return;
			if (m_orientation != orientation) {
				m_orientation = orientation;
				m_vPositions = vertices({QPointF{-1.0, -1.0}, QPointF{1.0, 1.0}}, orientation);
			}

			m_prog.bind();

			const auto list = frame.handle().toList();
			activeTex(list[0].toUInt(), 0, GL_TEXTURE_EXTERNAL_OES);

			m_prog.setUniformValue(loc_tex, 0);
			m_prog.setUniformValue(loc_vMatrix, m_vMatrix);
			m_prog.setUniformValue(loc_tMatrix, list[1].value<QMatrix4x4>());

			m_prog.enableAttributeArray(vCoord);
			m_prog.enableAttributeArray(vPosition);

			m_prog.setAttributeArray(vCoord, m_vCoords.data(), 2);
			m_prog.setAttributeArray(vPosition, m_vPositions.data(), 2);

			glActiveTexture(GL_TEXTURE0);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			m_prog.disableAttributeArray(vPosition);
			m_prog.disableAttributeArray(vCoord);
			m_prog.release();
		}
	private:
		QOpenGLShaderProgram m_prog;
		QVector<GLfloat> m_vCoords, m_vPositions;
		Qt::ScreenOrientation m_orientation = Qt::PrimaryOrientation;
		int loc_tex = -1, loc_vMatrix = -1, loc_tMatrix = -1;
		QMatrix4x4 m_vMatrix;
		QRectF m_coords, m_positions;
	};

	mutable OpenGLFramebufferObject *m_fbo = nullptr;
	mutable ExternalTextureRenderer m_renderer;
public:
	BarcodeScannerAndroidBackend(const QVideoSurfaceFormat &format)
	: BarcodeScannerBackend(format) {
		Q_ASSERT(format.handleType() == ExternalGLTextureHandle && format.pixelFormat() == QVideoFrame::Format_BGR32);
		m_vtxCode = simpleVertexShader();
		m_fragCode = R"(
			uniform sampler2D tex;
			varying highp vec2 texCoord;
			void main() {
				gl_FragColor = texture2D(tex, texCoord);
			}
		)";
	}
	void link(QOpenGLShaderProgram *prog) override {
		loc_vMatrix = prog->uniformLocation("vMatrix");
		loc_tex = prog->uniformLocation("tex");
	}
	void upload(const QVideoFrame &frame) {
		if (!frame.isValid() || frame.size().isEmpty())
			return;
		Q_ASSERT(frame.pixelFormat() == QVideoFrame::Format_BGR32);
		QSize size = frame.size();
		if ((Qt::PortraitOrientation | Qt::InvertedPortraitOrientation) & screenOrientation())
			qSwap(size.rwidth(), size.rheight());
		if (!m_fbo || m_fbo->size() != size) {
			delete m_fbo;
			m_fbo = new OpenGLFramebufferObject(size);
		}
		m_fbo->bind();
		glViewport(0, 0, size.width(), size.height());
		m_renderer.render(frame, screenOrientation());
		m_fbo->release();
	}
	void update(QOpenGLShaderProgram *prog, const RenderState &state) override {
		if (!m_fbo)
			return;
		prog->setUniformValue(loc_tex, 0);
		prog->setUniformValue(loc_vMatrix, state.combinedMatrix());
		activeTex(m_fbo->texture(), 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	BarcodeScannerImagePtr toImage(const QVideoFrame &/*frame*/) const override {
		if (!m_fbo || m_fbo->size().isEmpty())
			return BarcodeScannerImagePtr();
		m_fbo->bind();
		auto data = m_fbo->toImage();
		m_fbo->release();
		return BarcodeScannerImage::allocate(data, m_fbo->size(), BarcodeScannerImage::Rgb32);
	}
};
#endif

PixelFormatList BarcodeScannerBackend::supportedPixelFormats(VideoFrameHandleType handleType) {
	switch ((int)handleType) {
	case QAbstractVideoBuffer::NoHandle:
		return QList<QVideoFrame::PixelFormat>()
			<< QVideoFrame::Format_YUV420P
			<< QVideoFrame::Format_YV12;
#ifdef Q_OS_ANDROID
	case ExternalGLTextureHandle:
		return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_BGR32;
#endif
	default:
		return QList<QVideoFrame::PixelFormat>();
	}
}

QSGMaterialType *BarcodeScannerBackend::materialType(const QVideoSurfaceFormat &format) {
	static QSGMaterialType types[2][QVideoFrame::Format_AdobeDng];
	return &types[format.handleType() == QAbstractVideoBuffer::NoHandle][format.pixelFormat()];
}

BarcodeScannerBackend *BarcodeScannerBackend::create(const QVideoSurfaceFormat &format) {
	switch ((int)format.handleType()) {
	case QAbstractVideoBuffer::NoHandle:
		switch (format.pixelFormat()) {
		case QVideoFrame::Format_YUV420P:
		case QVideoFrame::Format_YV12:
			return new BarcodeScannerYCbCrBackend(format);
		case QVideoFrame::Format_BGR32:
			break;
		default:
			break;
		}
#ifdef Q_OS_ANDROID
	case ExternalGLTextureHandle:
		if (format.isValid()) {
			if (format.pixelFormat() != QVideoFrame::Format_BGR32)
				qDebug() << format.pixelFormat();
			Q_ASSERT(format.pixelFormat() == QVideoFrame::Format_BGR32);
			return new BarcodeScannerAndroidBackend(format);
		} else
			break;
#endif
	default:
		break;
	}
	return new BarcodeScannerDummyBackend(format);
}

const char *const *BarcodeScannerBackend::attributeNames() const {
	static const char *const names[] = {"vPosition", "vCoord", nullptr};
	return names;
}

QVector<GLfloat> BarcodeScannerBackend::vertices(const QRectF &rect, Qt::ScreenOrientation orientation) {
	QVector<GLfloat> array(8);
	auto p = array.data();
	auto set = [&p] (const QPointF &pos) {
		*p++ = pos.x();
		*p++ = pos.y();
	};
	switch (orientation) {
	case Qt::PortraitOrientation:
		set(rect.topRight());
		set(rect.bottomRight());
		set(rect.topLeft());
		set(rect.bottomLeft());
		break;
	case Qt::InvertedPortraitOrientation:
		set(rect.bottomLeft());
		set(rect.topLeft());
		set(rect.bottomRight());
		set(rect.topRight());
		break;
	case Qt::InvertedLandscapeOrientation:
		set(rect.bottomRight());
		set(rect.bottomLeft());
		set(rect.topRight());
		set(rect.topLeft());
		break;
	default:
		set(rect.topLeft());
		set(rect.topRight());
		set(rect.bottomLeft());
		set(rect.bottomRight());
		break;
	}
	return array;
}

QSGMaterialShader *BarcodeScannerMaterial::createShader() const {
	return new BarcodeScannerShader(m_backend);
}
