#include "scanneritemshader.hpp"
#include <QOpenGLContext>

//static void fillArray(QVector<GLfloat> &vec, const QPointF &p1, const QPointF &p2) {
//	auto p = vec.data();
//	*p++ = p1.x(); *p++ = p1.y();
//	*p++ = p1.x(); *p++ = p2.y();
//	*p++ = p2.x(); *p++ = p1.y();
//	*p++ = p2.x(); *p++ = p2.y();
//}

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

inline static QMatrix3x3 matRgbToYCbCr(double kb, double kr, const YCbCrRange &range) {
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


QSGMaterialType *ScannerItemMaterial::type() const {
	static QSGMaterialType types[QVideoFrame::Format_User];
	return &types[m_format.pixelFormat()];
}

struct ScannerItemShader::Data {
	GLuint textures[3];
	QSize textureSize, sizeHint;
	int loc_mul_mat = -1, loc_add_vec = -1, loc_tex0 = -1, loc_tex1 = -1, loc_tex2 = -1, loc_vMatrix = -1;
	QMatrix4x4 vMatrix;
	QMatrix3x3 mul;
	QVector3D add;
	QVideoFrame::PixelFormat pixfmt;
	QByteArray frag, vtx;
	QVideoSurfaceFormat format;
	bool good = false;
};

ScannerItemShader::ScannerItemShader(const QVideoSurfaceFormat &format)
: d(new Data) {
	Q_ASSERT(QOpenGLContext::currentContext() != nullptr);
	glGenTextures(3, d->textures);

	d->format = format;
	d->pixfmt = format.pixelFormat();
	d->sizeHint = format.sizeHint();

	d->mul.setToIdentity();
	d->add = {0.f, 0.f, 0.f};
	if (d->pixfmt > QVideoFrame::Format_BGRA32_Premultiplied) { // ycbcr
		auto ycbcr = format.yCbCrColorSpace();
		if (ycbcr == QVideoSurfaceFormat::YCbCr_Undefined)
			ycbcr = format.frameHeight() < 720 ? QVideoSurfaceFormat::YCbCr_BT601 : QVideoSurfaceFormat::YCbCr_BT709;
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
			break;
		}
		range *= 1.0/255.f;
		const auto rgbFromYCbCr = matYCbCrToRgb(kb, kr, range);
		d->mul = rgbFromYCbCr;

		auto vec = make3x1(range.y1, (range.c1 + range.c2)*0.5);
		vec = -(rgbFromYCbCr*vec);
		d->add = {vec(0, 0), vec(1, 0), vec(2, 0)};

		d->frag = R"(
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
//				tex = vec3(1.0, 0.0, 0.0);
				gl_FragColor = tex.rgbr*one.xxxy + one.yyyx;
			}
		)";
	} else { // rgb
		d->frag = R"(
			uniform sampler2D tex0, tex1, tex2;
			uniform highp mat3 mul_mat;
			uniform highp vec3 add_vec;
			varying highp vec2 texCoord;
			void main() {
				const highp vec2 one = vec2(1.0, 0.0);
				highp vec3 tex = texture2D(tex0, texCoord).bgr;
				gl_FragColor = tex.rgbr*one.xxxy + one.yyyx;
			}
		)";
	}
	d->vtx = R"(
		uniform highp mat4 vMatrix;
		attribute highp vec4 vPosition;
		attribute highp vec2 vCoord;
		varying highp vec2 texCoord;
		void main() {
			texCoord = vCoord;
			gl_Position = vMatrix*vPosition;
		}
	)";
	d->good = true;
}

ScannerItemShader::~ScannerItemShader() {
	glDeleteTextures(3, d->textures);
	delete d;
}

const char *ScannerItemShader::fragmentShader() const {
	return d->frag.constData();
}

const char *ScannerItemShader::vertexShader() const {
	return d->vtx.constData();
}

const char *const *ScannerItemShader::attributeNames() const {
	static const char *const names[] = {"vPosition", "vCoord", nullptr};
	return names;
}

void ScannerItemShader::updateState(const RenderState &state, QSGMaterial *new_, QSGMaterial *old) {
	QSGMaterialShader::updateState(state, new_, old);
	auto material = static_cast<ScannerItemMaterial*>(new_);
	auto &frame = material->frame();
	if (!frame.isValid())
		return;
	Q_ASSERT(frame.pixelFormat() == d->pixfmt);
	Q_ASSERT(frame.isReadable());

	if (material->hasNewFrame()) {
		if (frame.pixelFormat() == QVideoFrame::Format_YUV420P) {
			const int ws[3] = {frame.bytesPerLine(), frame.bytesPerLine()/2, frame.bytesPerLine()/2};
			const int hs[3] = {frame.height(), frame.height()/2, frame.height()/2};

			if (d->textureSize != QSize(ws[0], hs[0])) {
				for (int i=0; i<3; ++i) {
					glBindTexture(GL_TEXTURE_2D, d->textures[i]);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, ws[i], hs[i], 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				}
				d->textureSize = QSize(ws[0], hs[0]);
			}
			auto p = frame.bits();
			for (int i=0; i<3; ++i) {
				glBindTexture(GL_TEXTURE_2D, d->textures[i]);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ws[i], hs[i], GL_LUMINANCE, GL_UNSIGNED_BYTE, p);
				p += ws[i]*hs[i];
			}
		} else if (frame.pixelFormat() == QVideoFrame::Format_BGR32) {
			const int w = frame.bytesPerLine()/4;
			const int h = frame.height();
			if (d->textureSize != QSize(w, h)) {
				glBindTexture(GL_TEXTURE_2D, d->textures[0]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, 0);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				d->textureSize = QSize(w, h);
			}
			glBindTexture(GL_TEXTURE_2D, d->textures[0]);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGRA_EXT, GL_UNSIGNED_BYTE, frame.bits());
		}
	}
	for (int i=0; i<3; ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, d->textures[i]);
	}

	auto prog = program();
	prog->setUniformValue(d->loc_tex0, 0);
	prog->setUniformValue(d->loc_tex1, 1);
	prog->setUniformValue(d->loc_tex2, 2);
	prog->setUniformValue(d->loc_mul_mat, d->mul);
	prog->setUniformValue(d->loc_add_vec, d->add);
	prog->setUniformValue(d->loc_vMatrix, state.combinedMatrix());
	glActiveTexture(GL_TEXTURE0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void ScannerItemShader::initialize() {
	QSGMaterialShader::initialize();
	auto prog = program();
	d->loc_vMatrix = prog->uniformLocation("vMatrix");
	d->loc_add_vec = prog->uniformLocation("add_vec");
	d->loc_mul_mat = prog->uniformLocation("mul_mat");
	d->loc_tex0 = prog->uniformLocation("tex0");
	d->loc_tex1 = prog->uniformLocation("tex1");
	d->loc_tex2 = prog->uniformLocation("tex2");
}


