#ifndef SCANNERITEM_HPP
#define SCANNERITEM_HPP

#include <QQuickItem>

class QVideoFrame;
class QVideoSurfaceFormat;

class ScannerItem : public QQuickItem {
	Q_OBJECT
	Q_PROPERTY(int device READ device WRITE setDevice NOTIFY deviceChanged)
	Q_PROPERTY(QString deviceDescription READ deviceDescription NOTIFY deviceChanged)
public:
	ScannerItem(QQuickItem *parent = nullptr);
	~ScannerItem();
	void setDevice(int device);
	int device() const;
	QString deviceDescription() const;
	QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData */*data*/) override;
	void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
signals:
	void deviceChanged();
private:
	void componentComplete() override;
	bool present(const QVideoFrame &frame);
	bool prepare(const QVideoSurfaceFormat &format);
	struct Data;
	Data *d;
	friend class ScannerItemSurface;
};





//class TextureRendererItem : public GeometryItem {
//	Q_OBJECT
//public:
//	TextureRendererItem(QQuickItem *parent = 0);
//	~TextureRendererItem();
//protected:
//	virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
//	virtual QSGMaterialType *shaderId() const { return &m_types[m_interpolator->category()][m_dithering > 0]; }
//	virtual TextureRendererShader *createShader() const;
//private slots:
//	void tryInitGL() { if (!m_init && QOpenGLContext::currentContext()) { initializeGL(); m_init = true; } }
//private:
//	const OpenGLTexture &texture() const { return m_texture; }
//	const OpenGLTexture &ditheringTexture() const { return m_ditheringTex; }
//	const Interpolator::Texture &lutInterpolatorTexture(int i) const { return m_lutInt[i]; }
//	QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData *data) final override;
//	friend class TextureRendererShader;
//	struct Material;	struct Node;	struct Data; Data *d;
//	bool m_dirtyGeomerty = true, m_init = false;
//	const Interpolator *m_interpolator = Interpolator::get(InterpolatorType::Bilinear);
//	InterpolatorType m_newInt = InterpolatorType::Bilinear;
//	mutable QSGMaterialType m_types[Interpolator::CategoryMax][2];
//	Interpolator::Texture m_lutInt[2];
//	OpenGLTexture m_texture, m_ditheringTex;
//	QQuickWindow *m_win = nullptr;
//	Dithering m_dithering = Dithering::None, m_newDithering = Dithering::None;
//};

//class TextureRendererShader : public QSGMaterialShader {
//public:
//	TextureRendererShader(const TextureRendererItem *item, Interpolator::Category category = Interpolator::None, bool dithering = false);
//	static QOpenGLFunctions *func() { return QOpenGLContext::currentContext()->functions(); }
//	const char *fragmentShader() const override { return m_fragCode.constData(); }
//	const char *vertexShader() const override { return m_vertexCode.constData(); }
//	const char *const *attributeNames() const override;
//	virtual void link(QOpenGLShaderProgram *prog);
//	virtual void bind(QOpenGLShaderProgram *prog);
//	const TextureRendererItem *item() const { return m_item; }
//private:
//	void updateState(const RenderState &state, QSGMaterial */*new_*/, QSGMaterial */*old*/) final override;
//	void initialize() final override;
//	const TextureRendererItem *m_item = nullptr;
//	Interpolator::Category m_category = Interpolator::None;
//	bool m_dithering = false;
//	int m_lutCount = 0;
//	int loc_lut_int[2] = {-1, -1}, loc_lut_int_mul[2] = {-1, -1};
//	int loc_tex = -1, loc_vMatrix = -1, loc_dxy = -1;
//	int loc_tex_size = -1;
//	int loc_dithering = -1, loc_dithering_quantization = -1, loc_dithering_center = -1, loc_dithering_size = -1;
//	QByteArray m_fragCode, m_vertexCode;
//};

#endif // SCANNERITEM_HPP
