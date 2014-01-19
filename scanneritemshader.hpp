#ifndef SCANNERITEMSHADER_HPP
#define SCANNERITEMSHADER_HPP

#include <QSGMaterialShader>
#include <QSGMaterialType>
#include <QVideoSurfaceFormat>
#include <QVideoFrame>

class ScannerItemShader : public QSGMaterialShader {
public:
	ScannerItemShader(const QVideoSurfaceFormat &format);
	~ScannerItemShader();
private:
	const char *fragmentShader() const override;
	const char *vertexShader() const override;
	const char *const *attributeNames() const override;
	void updateState(const RenderState &state, QSGMaterial *new_, QSGMaterial */*old*/) final override; // render
	void initialize() final override; // link
	struct Data;
	Data *d;
};

class ScannerItemMaterial : public QSGMaterial {
public:
	ScannerItemMaterial(const QVideoSurfaceFormat &format) : m_format(format) {setFlag(Blending);}
	~ScannerItemMaterial() {}
	QSGMaterialType *type() const override;
	QSGMaterialShader *createShader() const override { return new ScannerItemShader(m_format); }
	const QVideoSurfaceFormat &format() const { return m_format; }
	bool hasNewFrame() const { return m_new; }
	const QVideoFrame &frame() {
		if (!m_frame.isMapped())
			m_frame.map(QAbstractVideoBuffer::ReadOnly);
		return m_frame;
	}
	void setFrame(const QVideoFrame &frame) { m_frame = frame; m_new = true; }
private:
	QVideoSurfaceFormat m_format;
	QVideoFrame m_frame;
	bool m_new = false;
};

#endif // SCANNERITEMSHADER_HPP
