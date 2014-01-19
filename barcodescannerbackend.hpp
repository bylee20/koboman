#ifndef BARCODESCANNERBACKEND_HPP
#define BARCODESCANNERBACKEND_HPP

#include <QVideoSurfaceFormat>
#include <QOpenGLFunctions>
#include <QSGMaterialType>
#include <QSharedPointer>
#include "barcode.hpp"

class BarcodeScannerMaterial;

using VideoFrameHandleType = QAbstractVideoBuffer::HandleType;
using PixelFormatList = QList<QVideoFrame::PixelFormat>;

class BarcodeScannerBackend {
public:
	using RenderState = QSGMaterialShader::RenderState;
	BarcodeScannerBackend(const QVideoSurfaceFormat &format): m_format(format) {
		if (!glActiveTexture)
			glActiveTexture = decltype(glActiveTexture)(QOpenGLContext::currentContext()->getProcAddress("glActiveTexture"));
	}
	virtual ~BarcodeScannerBackend() {}
	QSGMaterialType *materialType() const { return materialType(m_format); }
	static QSGMaterialType *materialType(const QVideoSurfaceFormat &format);
	static BarcodeScannerBackend *create(const QVideoSurfaceFormat &format);
	static PixelFormatList supportedPixelFormats(VideoFrameHandleType handleType);
	const char *fragmentShader() const { return m_fragCode.data(); }
	const char *vertexShader() const { return m_vtxCode.data(); }
	const char *const *attributeNames() const;
	const QVideoSurfaceFormat &format() const { return m_format; }
	static QVector<GLfloat> vertices(const QRectF &rect, Qt::ScreenOrientation orientation);
	virtual void link(QOpenGLShaderProgram *prog) = 0;
	virtual void update(QOpenGLShaderProgram *prog, const RenderState &state) = 0;
	virtual void upload(const QVideoFrame &frame) = 0;
	virtual BarcodeScannerImagePtr toImage(const QVideoFrame &frame) const = 0;
	void setScreenOrientation(Qt::ScreenOrientation orientation) { Q_ASSERT(orientation != Qt::PrimaryOrientation); m_orientation = orientation; }
	Qt::ScreenOrientation screenOrientation() const { return m_orientation; }
protected:
	static const char *simpleVertexShader() {
		return R"(
			uniform highp mat4 vMatrix;
			attribute highp vec4 vPosition;
			attribute highp vec2 vCoord;
			varying highp vec2 texCoord;
			void main() {
				texCoord = vCoord;
				gl_Position = vMatrix*vPosition;
			}
		)";
	}
	static inline void bindTex(GLuint tex, GLenum target = GL_TEXTURE_2D) { glBindTexture(target, tex); }
	static inline void newTex(GLuint tex, int w, int h, GLint internalFormat, GLenum format) {
		const auto target = GL_TEXTURE_2D;
		bindTex(tex, target);
		glTexImage2D(target, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, nullptr);
		glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	}
	static inline void uploadTex(GLuint tex, int w, int h, GLenum format, const void *data) {
		const auto target = GL_TEXTURE_2D;
		bindTex(tex, target);
		glTexSubImage2D(target, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, data);
	}
	static inline void activeTex(GLuint tex, int num, GLenum target = GL_TEXTURE_2D) {
		glActiveTexture(GL_TEXTURE0 + num);
		bindTex(tex, target);
	}

	static void(*glActiveTexture)(GLenum texture);
	QByteArray m_fragCode, m_vtxCode;
	int loc_vMatrix = -1;
	QMatrix4x4 m_vMatrix;
private:
	mutable QSGMaterialType m_type;
	QVideoSurfaceFormat m_format;
	Qt::ScreenOrientation m_orientation = Qt::PrimaryOrientation;
	QOpenGLFunctions *m_func = nullptr;
};

class BarcodeScannerMaterial : public QSGMaterial {
public:
	BarcodeScannerMaterial(const QVideoSurfaceFormat &format)
	: m_backend(BarcodeScannerBackend::create(format)) { /*setFlag(Blending);*/ }
	QSGMaterialType *type() const override { return m_backend->materialType(); }
	QSGMaterialShader *createShader() const override;
	bool hasNewFrame() const { return m_new; }
	const QVideoFrame &frame() const { return m_frame; }
	void mapFrame() { if (!m_frame.isMapped()) m_frame.map(QAbstractVideoBuffer::ReadOnly); }
	void setFrame(const QVideoFrame &frame) {
		m_frame = frame;
		m_new = true;
		m_frame.map(QAbstractVideoBuffer::ReadOnly);
		m_backend->upload(m_frame);
	}
	const BarcodeScannerBackend *backend() const { return m_backend.data(); }
	const QVideoSurfaceFormat &format() const { return m_backend->format(); }
	void setScreenOrientation(Qt::ScreenOrientation o) { m_backend->setScreenOrientation(o); }
private:
	QSharedPointer<BarcodeScannerBackend> m_backend;
	QVideoFrame m_frame;
	bool m_new = false;
};

class BarcodeScannerShader : public QSGMaterialShader {
public:
	BarcodeScannerShader(const QSharedPointer<BarcodeScannerBackend> &backend)
	: m_backend(backend) {}
private:
	const char *fragmentShader() const final override { return m_backend->fragmentShader(); }
	const char *vertexShader() const final override { return m_backend->vertexShader(); }
	const char *const *attributeNames() const final override { return m_backend->attributeNames(); }
	void updateState(const RenderState &state, QSGMaterial *new_, QSGMaterial *old) final override {
		QSGMaterialShader::updateState(state, new_, old);
		m_backend->update(program(), state);
	}
	void initialize() final override { QSGMaterialShader::initialize(); Q_ASSERT(program()->isLinked()); m_backend->link(program()); }
	QSharedPointer<BarcodeScannerBackend> m_backend;
};






#endif // BARCODESCANNERBACKEND_HPP
