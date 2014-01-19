#include "textureitem.hpp"
#include "utils.hpp"
#include <QSGGeometryNode>
#include <QQuickWindow>

class TextureShader : public QSGMaterialShader {
public:
	TextureShader(TextureItem *item): m_item(item) {
		m_frag = m_item->fragmentShader();
		m_vtx = m_item->vertexShader();
		for (auto p = m_item->attributeNames(); p && *p; ++p) {
			m_names << *p;
			m_attr << m_names.last().data();
		}
		m_attr << nullptr;
	}
	char const *const *attributeNames() const { return m_attr.data(); }
	const char *vertexShader() const { return m_vtx.data(); }
	const char *fragmentShader() const { return m_frag.data(); }
	void initialize() {
		QSGMaterialShader::initialize();
		m_item->link(program());
	}
	void updateState(const RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial) {
		QSGMaterialShader::updateState(state, newMaterial, oldMaterial);
		m_item->bind(program(), state);
	}
private:
	TextureItem *m_item = nullptr;
	QByteArray m_frag, m_vtx;
	QVector<const char *> m_attr;
	QVector<QByteArray> m_names;
};

class TextureMaterial : public QSGMaterial {
public:
	TextureMaterial(TextureItem *item): m_item(item) { setFlag(Blending); }
	QSGMaterialType *type() const { return m_item->shaderId(); }
	QSGMaterialShader *createShader() const { return new TextureShader(m_item); }
private:
	TextureItem *m_item = nullptr;
};

struct TextureItem::Data {
	TextureItem *p = nullptr;
	QSGGeometryNode *node = nullptr;
	QQuickWindow *window = nullptr;
	QRectF vtxRect, txtRect = {0.0, 0.0, 1.0, 1.0};
	bool dirtyGeometry = false, initialized = false;
	int loc_vMatrix = -1, loc_opacity = -1;
	void setWindow(QQuickWindow *w) {
		if ((window = w)) {
			connect(w, &QQuickWindow::sceneGraphInitialized, p, &TextureItem::initializeGL);
			connect(w, &QQuickWindow::sceneGraphInvalidated, p, &TextureItem::finalizeGL);
		}
	}
};

TextureItem::TextureItem(QQuickItem *parent)
: QQuickItem(parent), d(new Data) {
	d->p = this;
	d->setWindow(window());
	connect(this, &QQuickItem::windowChanged, [this] (QQuickWindow *w) { d->setWindow(w); });
}

TextureItem::~TextureItem() {
	delete d;
}

void TextureItem::initializeGL() {
	if (d->initialized)
		return;
	m_func = QOpenGLContext::currentContext()->functions();
	d->initialized = true;
}

void TextureItem::finalizeGL() {
	if (!d->initialized)
		return;
	m_func = nullptr;
	d->initialized = false;
}

const char *const *TextureItem::attributeNames() const {
	static const char *const names[] = {"vPosition", "vCoord", nullptr};
	return names;
}

void TextureItem::setGeometryDirty() {
	d->dirtyGeometry = true;
}

QSGGeometry *TextureItem::createSGGeometry() {
	return new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);
}

void TextureItem::updateSGGeometry(QSGGeometry *geometry) {
	fillTexturedPointAsTriangleStrip(geometry->vertexDataAsTexturedPoint2D(), d->vtxRect, d->txtRect);
}

QSGNode *TextureItem::updatePaintNode(QSGNode *node, UpdatePaintNodeData *) {
	if (!d->initialized)
		initializeGL();
	d->node = static_cast<QSGGeometryNode*>(node);
	if (!d->node) {
		d->node = new QSGGeometryNode;
		d->node->setGeometry(createSGGeometry());
		d->node->setMaterial(new TextureMaterial(this));
		d->node->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial);
	}
	if (prepareToRender())
		d->node->markDirty(QSGNode::DirtyMaterial);
	if (d->dirtyGeometry) {
		updateSGGeometry(d->node->geometry());
		d->node->markDirty(QSGNode::DirtyGeometry);
		d->dirtyGeometry = false;
	}
	return d->node;
}

void TextureItem::setVertexRect(const QRectF &rect) {
	if (_Change(d->vtxRect, rect)) {
		d->dirtyGeometry = true;
		update();
	}
}

void TextureItem::setTextureRect(const QRectF &rect) {
	if (_Change(d->txtRect, rect)) {
		d->dirtyGeometry = true;
		update();
	}
}

void TextureItem::link(QOpenGLShaderProgram *prog) {
	d->loc_vMatrix = prog->uniformLocation("vMatrix");
	d->loc_opacity = prog->uniformLocation("opacity");
}

void TextureItem::bind(QOpenGLShaderProgram *prog, const RenderState &state) {
	if (state.isMatrixDirty())
		prog->setUniformValue(d->loc_vMatrix, state.combinedMatrix());
	if (state.isOpacityDirty())
		prog->setUniformValue(d->loc_opacity, state.opacity());
}
