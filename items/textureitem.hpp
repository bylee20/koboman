#ifndef TEXTUREITEM_HPP
#define TEXTUREITEM_HPP

#include <QQuickItem>
#include <QSGMaterialShader>
#include <QOpenGLFunctions>
#include <QSGGeometry>
#include <QOpenGLShaderProgram>

class TextureItem : public QQuickItem {
	Q_OBJECT
public:
	using ShaderId = QSGMaterialType;
	using RenderState = QSGMaterialShader::RenderState;
	TextureItem(QQuickItem *parent = nullptr);
	~TextureItem();
public slots:
	void polishAndUpdate() { polish(); update(); }
protected:
	virtual QByteArray fragmentShader() const = 0;
	virtual QByteArray vertexShader() const = 0;
	virtual ShaderId *shaderId() const final { return &m_type; }
	virtual const char *const *attributeNames() const;
	virtual void link(QOpenGLShaderProgram *prog);
	virtual void bind(QOpenGLShaderProgram *prog, const RenderState &state);
	virtual bool prepareToRender() { return false; }
	virtual QSGGeometry *createSGGeometry();
	virtual void updateSGGeometry(QSGGeometry *geometry);
	void setVertexRect(const QRectF &rect);
	void setTextureRect(const QRectF &rect);
	void setGeometryDirty();

	static void fillTexturedPointAsTriangleStrip(QSGGeometry::TexturedPoint2D *p, const QRectF &vertex, const QRectF &texture) {
		p[0].set(vertex.left(), vertex.top(), texture.left(), texture.top());
		p[1].set(vertex.right(), vertex.top(), texture.right(), texture.top());
		p[2].set(vertex.left(), vertex.bottom(), texture.left(), texture.bottom());
		p[3].set(vertex.right(), vertex.bottom(), texture.right(), texture.bottom());
	}
	static void setColoredPoint(QSGGeometry::ColoredPoint2D *p, const QPointF &pos, const QColor &color) {
		p->set(pos.x(), pos.y(), color.red(), color.green(), color.blue(), color.alpha());
	}
	static void setTexturedPoint(QSGGeometry::TexturedPoint2D *p, const QPointF &vpos, const QPointF &tpos) {
		p->set(vpos.x(), vpos.y(), tpos.x(), tpos.y());
	}
	static QSGGeometry::TexturedPoint2D *fillTexturedPointAsTriangle(QSGGeometry::TexturedPoint2D *p, const QPointF &v1, const QPointF &v2, const QPointF &t1, const QPointF &t2) {
		p++->set(v1.x(), v1.y(), t1.x(), t1.y());
		p++->set(v2.x(), v1.y(), t2.x(), t1.y());
		p++->set(v1.x(), v2.y(), t1.x(), t2.y());
		p++->set(v1.x(), v2.y(), t1.x(), t2.y());
		p++->set(v2.x(), v2.y(), t2.x(), t2.y());
		p++->set(v2.x(), v1.y(), t2.x(), t1.y());
		return p;
	}
	static QSGGeometry::TexturedPoint2D *fillTexturedPointAsTriangle(QSGGeometry::TexturedPoint2D *p, const QRectF &vertex, const QRectF &texture) {
		setTexturedPoint(p++, vertex.topLeft(), texture.topLeft());
		setTexturedPoint(p++, vertex.topRight(), texture.topRight());
		setTexturedPoint(p++, vertex.bottomLeft(), texture.bottomLeft());

		setTexturedPoint(p++, vertex.bottomLeft(), texture.bottomLeft());
		setTexturedPoint(p++, vertex.bottomRight(), texture.bottomRight());
		setTexturedPoint(p++, vertex.topRight(), texture.topRight());
		return p;
	}

	static QSGGeometry::ColoredPoint2D *fillColoredPointAsTriangle(QSGGeometry::ColoredPoint2D *p, const QRectF &vertex, const QColor &color) {
		setColoredPoint(p++, vertex.topLeft(), color);
		setColoredPoint(p++, vertex.topRight(), color);
		setColoredPoint(p++, vertex.bottomLeft(), color);

		setColoredPoint(p++, vertex.bottomLeft(), color);
		setColoredPoint(p++, vertex.bottomRight(), color);
		setColoredPoint(p++, vertex.topRight(), color);
		return p;
	}

	static inline void bindTexture(GLuint tex, GLenum target = GL_TEXTURE_2D) { glBindTexture(target, tex); }
	static inline void newTexure(GLuint tex, int w, int h, GLint internalFormat, GLenum format, GLenum filter = GL_LINEAR, GLenum clamp = GL_CLAMP_TO_EDGE) {
		const auto target = GL_TEXTURE_2D;
		bindTexture(tex, target);
		glTexImage2D(target, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, nullptr);
		glTexParameterf(target, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameterf(target, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameterf(target, GL_TEXTURE_WRAP_T, clamp);
		glTexParameterf(target, GL_TEXTURE_WRAP_S, clamp);
	}
	static inline void uploadTexture(GLuint tex, int w, int h, GLenum format, const void *data) {
		const auto target = GL_TEXTURE_2D;
		bindTexture(tex, target);
		glTexSubImage2D(target, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, data);
	}
	inline void activateTexture(GLuint tex, int num, GLenum target = GL_TEXTURE_2D) const {
		func()->glActiveTexture(GL_TEXTURE0 + num);
		bindTexture(tex, target);
	}
	QOpenGLFunctions *func() const { return m_func; }
protected slots:
	virtual void initializeGL();
	virtual void finalizeGL();
private:
	friend class TextureShader;
	friend class TextureMaterial;
	QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *) final;
	struct Data;
	Data *d;
	mutable ShaderId m_type;
	QOpenGLFunctions *m_func = nullptr;
};

#endif // TEXTUREITEM_HPP
