#include "toplevelitem.hpp"
#include "toplevelcontainer.hpp"
#include <QDebug>
#include <QQuickWindow>
#include "utility.hpp"
#include "themeapi.hpp"
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

struct TopLevelItem::Data {
	TopLevelItem *p = nullptr;
	QQuickItem *root = Utility::root();
	int loc_shade = -1, loc_tex_box = -1, loc_tex_shadow = -1, loc_color_box = -1, loc_color_shadow = -1;
	int loc_shadow_offset = -1;
	qreal shade = 0.5, boundary = Theme::spacing();
	GLuint textures[3] = { 0 };
	GLuint &texBox = textures[0], &texShadow = textures[1], &transparent = textures[2];
	bool redraw = false, upload = false, autohide = true, animate = true;
	QSize textureSize{0, 0};
	QRectF boxRect{0.0, 0.0, 0.0, 0.0};
	TopLevelContainer *container = nullptr;
	TopLevelShadow *shadow = nullptr;
	QParallelAnimationGroup animation;
	QPropertyAnimation scaler, dimmer;
	QPointF shadowOffset;
	const ContainerImage *image = nullptr;
	void update() { redraw = true; p->update(); }
	void checkAnimation() {
		const int count = animation.animationCount();
		if (scaler.targetObject()) {
			if (count != 2)
				animation.addAnimation(&scaler);
		} else if (count == 2)
			animation.removeAnimation(&scaler);
	}
};

TopLevelItem::TopLevelItem(QQuickItem *parent)
: TextureItem(parent), d(new Data) {
	Q_ASSERT(d->root != nullptr);
	d->p = this;
	d->shadow = new TopLevelShadow(this);
	d->container = new TopLevelContainer(this);
	d->scaler.setDuration(200);
	d->scaler.setPropertyName("scale");
	d->scaler.setStartValue(0.0);
	d->scaler.setEndValue(1.0);
	d->scaler.setEasingCurve(QEasingCurve::OutCubic);
	d->dimmer.setDuration(d->scaler.duration());
	d->dimmer.setPropertyName("opacity");
	d->dimmer.setStartValue(0.0);
	d->dimmer.setTargetObject(this);
	d->dimmer.setEndValue(1.0);
	d->dimmer.setEasingCurve(QEasingCurve::OutCubic);
	d->animation.addAnimation(&d->dimmer);

	setParentItem(d->root);
	connect(d->root->window(), &QQuickWindow::widthChanged, this, &QQuickItem::setWidth);
	connect(d->root->window(), &QQuickWindow::heightChanged, this, &QQuickItem::setHeight);
	setVisible(false);

	setZ(1e200);
	setFlags(ItemHasContents | ItemIsFocusScope);
	setAcceptedMouseButtons(Qt::AllButtons);
	connect(this, &TopLevelItem::parentChanged, this, &TopLevelItem::updateParentItem);
	connect(this, &TopLevelItem::visibleChanged, this, &TopLevelItem::updateFocusState);
	connect(&d->animation, &QPropertyAnimation::finished, this, &TopLevelItem::handleAnimationFinished);
	connect(d->container, &TopLevelContainer::itemChanged, [this] () {
		d->scaler.setTargetObject(d->container->item());
	});
}

TopLevelItem::~TopLevelItem() {
	if (d->animation.animationCount() == 2)
		d->animation.removeAnimation(&d->scaler);
	d->animation.removeAnimation(&d->dimmer);
	disconnect(this, nullptr, this, nullptr);
	disconnect(d->root->window(), nullptr, this, nullptr);
	delete d;
}

void TopLevelItem::componentComplete() {
	TextureItem::componentComplete();
	d->container->complete();
	d->upload = true;
	d->scaler.setTargetObject(d->container->item());
	updateContainerRect();
	d->update();
	connect(d->container, &TopLevelContainer::repainted, [this] () {
		updateContainerRect();
		d->upload = true;
		d->update();
	});
	connect(d->container, &TopLevelContainer::rectChanged, [this] () {
		updateContainerRect();
		d->update();
	});
}

QByteArray TopLevelItem::fragmentShader() const {
	static const char *shader = R"(
		uniform sampler2D tex_box, tex_shadow;
		uniform highp vec4 color_box, color_shadow;
		varying highp vec2 texCoord;
		varying highp vec2 shadowCoord;
		uniform lowp float shade, opacity;
		void main() {
			lowp vec4 color;

			lowp float shadowAlpha = color_shadow.a*texture2D(tex_shadow, shadowCoord).x;
			color.rgb = color_shadow.rgb*shadowAlpha;
			color.a = mix(shade, 1.0, shadowAlpha);

			lowp float boxAlpha = color_box.a*texture2D(tex_box, texCoord).x;
			color.rgb = mix(color.rgb, color_box.rgb, boxAlpha);
			color.a = mix(color.a, 1.0, boxAlpha)*opacity;
			gl_FragColor = color;
		}
	)";
	return shader;
}

QByteArray TopLevelItem::vertexShader() const {
	static const char *shader = R"(
		uniform highp mat4 vMatrix;
		uniform highp mat3 trans;
		uniform highp vec2 shadowOffset;
		attribute highp vec4 vPosition;
		attribute highp vec2 vCoord;
		varying highp vec2 texCoord;
		varying highp vec2 shadowCoord;
		void main() {
			texCoord = vCoord;
			shadowCoord = texCoord - shadowOffset;
			gl_Position = vMatrix*vPosition;
		}
	)";
	return shader;
}

void TopLevelItem::initializeGL() {
	TextureItem::initializeGL();
	glGenTextures(1, &d->texBox);
	glGenTextures(1, &d->texShadow);
	glGenTextures(1, &d->transparent);
	newTexure(d->transparent, 4, 1, GL_LUMINANCE, GL_LUMINANCE);
	static const quint32 zero = 0x0;
	uploadTexture(d->transparent, 4, 1, GL_LUMINANCE, &zero);
	d->image = &ContainerImage::retain();
	const int w = d->image->width(), h = d->image->height();
	newTexure(d->texBox, w, h, GL_LUMINANCE, GL_LUMINANCE, GL_LINEAR);
	newTexure(d->texShadow, w, h, GL_LUMINANCE, GL_LUMINANCE, GL_LINEAR);
	uploadTexture(d->texBox, w, h, GL_LUMINANCE, d->image->box().constData());
	uploadTexture(d->texShadow, w, h, GL_LUMINANCE, d->image->shadow().constData());
}

void TopLevelItem::finalizeGL() {
	d->image = nullptr;
	ContainerImage::release();
	glDeleteTextures(1, &d->texBox);
	d->texBox = GL_NONE;
	TextureItem::finalizeGL();
}

void TopLevelItem::bind(QOpenGLShaderProgram *prog, const QSGMaterialShader::RenderState &state) {
	TextureItem::bind(prog, state);
	prog->setUniformValue(d->loc_shade, (GLfloat)d->shade);
	prog->setUniformValue(d->loc_tex_box, 0);
	prog->setUniformValue(d->loc_tex_shadow, 1);
	prog->setUniformValue(d->loc_color_box, d->container->color());
	prog->setUniformValue(d->loc_color_shadow, d->container->shadow()->m_color);
	prog->setUniformValue(d->loc_shadow_offset, d->shadowOffset);
	activateTexture(d->textureSize.isEmpty() ? d->transparent : d->texBox, 0);
	activateTexture(d->textureSize.isEmpty() ? d->transparent : d->texShadow, 1);
	func()->glActiveTexture(GL_TEXTURE0);
}

void TopLevelItem::link(QOpenGLShaderProgram *prog) {
	TextureItem::link(prog);
	d->loc_shade = prog->uniformLocation("shade");
	d->loc_tex_box = prog->uniformLocation("tex_box");
	d->loc_tex_shadow = prog->uniformLocation("tex_shadow");
	d->loc_color_box = prog->uniformLocation("color_box");
	d->loc_color_shadow = prog->uniformLocation("color_shadow");
	d->loc_shadow_offset = prog->uniformLocation("shadow_offset");
}

QSGGeometry *TopLevelItem::createSGGeometry() {
	auto geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 6*9);//6*(4+7));
	geometry->setDrawingMode(GL_TRIANGLES);
	return geometry;
}

void TopLevelItem::updateSGGeometry(QSGGeometry *geometry) {
	auto adjusted = [] (const QRectF &r, qreal a, qreal dx = 1.0, qreal dy = 1.0) {
		return r.adjusted(a*dx, a*dy, -a*dx, -a*dy);
	};
	const auto container = d->boxRect;
	const auto v_out = adjusted(container, -d->image->outer()+2);
	const auto v_in = adjusted(container, d->image->inner());

	const qreal dtx = 1./d->image->width();
	const qreal dty = 1./d->image->height();
	const qreal w = width(), h = height();
	const QRectF texture{.0, .0, 1., 1.};
	const auto t_out = adjusted(texture, 2.0, dtx, dty);
	const auto t_in = adjusted(texture, d->image->outer() + d->image->inner(), dtx, dty);

	auto p = geometry->vertexDataAsTexturedPoint2D();

	p = fillTexturedPointAsTriangle(p, {.0, .0}, {w, v_out.top()},  {0., 0.}, {dtx, dty});
	p = fillTexturedPointAsTriangle(p, {.0, v_out.top()}, v_out.bottomLeft(),  {0., 0.}, {dtx, dty});
	p = fillTexturedPointAsTriangle(p, v_out.topRight(), {w, v_out.bottom()},  {0., 0.}, {dtx, dty});
	p = fillTexturedPointAsTriangle(p, {.0, v_out.bottom()}, {w, h},  {0., 0.}, {dtx, dty});

	p = fillTexturedPointAsTriangle(p, v_out.topLeft(), v_in.topLeft(), t_out.topLeft(), t_in.topLeft());
	p = fillTexturedPointAsTriangle(p, {v_in.left(), v_out.top()}, v_in.topRight(), {t_in.left(), t_out.top()}, t_in.topRight());
	p = fillTexturedPointAsTriangle(p, {v_in.right(), v_out.top()}, {v_out.right(), v_in.top()}, {t_in.right(), t_out.top()}, {t_out.right(), t_in.top()});

	p = fillTexturedPointAsTriangle(p, {v_out.left(), v_in.top()}, v_in.bottomLeft(), {t_out.left(), t_in.top()}, t_in.bottomLeft());
	p = fillTexturedPointAsTriangle(p, v_in, t_in);
	p = fillTexturedPointAsTriangle(p, v_in.topRight(), {v_out.right(), v_in.bottom()}, t_in.topRight(), {t_out.right(), t_in.bottom()});

	p = fillTexturedPointAsTriangle(p, {v_out.left(), v_in.bottom()}, {v_in.left(), v_out.bottom()}, {t_out.left(), t_in.bottom()}, {t_in.left(), t_out.bottom()});
	p = fillTexturedPointAsTriangle(p, v_in.bottomLeft(), {v_in.right(), v_out.bottom()}, t_in.bottomLeft(), {t_in.right(), t_out.bottom()});
	p = fillTexturedPointAsTriangle(p, v_in.bottomRight(), v_out.bottomRight(), t_in.bottomRight(), t_out.bottomRight());

	d->shadowOffset = d->container->shadow()->m_offset;
	d->shadowOffset.rx() *= dtx;
	d->shadowOffset.ry() *= dty;
}

bool TopLevelItem::prepareToRender() {
	if (d->upload) {

		d->redraw = true;
		d->upload = false;
	}
	auto ret = d->redraw;
	d->redraw = false;
	return ret;
}

void TopLevelItem::updateFocusState() {
	if (isVisible() != hasFocus())
		setFocus(isVisible());
}

void TopLevelItem::updateParentItem() {
	setParentItem(d->root);
	updateFocusState();
}

void TopLevelItem::mousePressEvent(QMouseEvent *event) {
	QQuickItem::mousePressEvent(event);
	event->accept();
	if (d->autohide && !d->container->rect().contains(event->localPos()))
		hide();
}

void TopLevelItem::mouseReleaseEvent(QMouseEvent *event) {
	QQuickItem::mouseReleaseEvent(event);
}

void TopLevelItem::keyPressEvent(QKeyEvent *event) {
	QQuickItem::keyReleaseEvent(event);
}

void TopLevelItem::keyReleaseEvent(QKeyEvent *event) {
	QQuickItem::keyReleaseEvent(event);
	event->accept();
	if (d->autohide) {
		switch (event->key()) {
		case Qt::Key_Escape:
		case Qt::Key_Back:
			hide();
			break;
		default:
			break;
		}
	}
}

void TopLevelItem::geometryChanged(const QRectF &new_, const QRectF &old) {
	QQuickItem::geometryChanged(new_, old);
	if (isComponentComplete())
		updateContainerRect();
	setVertexRect(QRectF(x(), y(), width(), height()));
}

qreal TopLevelItem::shade() const {
	return d->shade;
}

void TopLevelItem::setShade(qreal shade) {
	if (_Change(d->shade, shade)) {
		emit shadeChanged();
		d->update();
	}
}

TopLevelContainer *TopLevelItem::container() const {
	return d->container;
}

TopLevelShadow *TopLevelItem::shadow() const {
	return d->shadow;
}

qreal TopLevelItem::boundary() const {
	return d->boundary;
}

void TopLevelItem::setBoundary(qreal boundary) {
	if (_Change(d->boundary, boundary))
		emit boundaryChanged();
}

void TopLevelItem::updateContainerRect() {
	if (_Change(d->boxRect, d->container->rect())) {
		setGeometryDirty();
		d->redraw = true;
		update();
	}
//	d->trans.reset();
//	d->trans.scale(width()/d->boxRect.width(), height()/d->boxRect.height());
//	d->trans.translate(-d->boxRect.x()/width(), -d->boxRect.y()/height());
}

bool TopLevelItem::autohide() const {
	return d->autohide;
}

void TopLevelItem::setAutohide(bool autohide) {
	if (_Change(d->autohide, autohide))
		emit autohideChanged();
}

void TopLevelItem::show() {
	if (!isVisible() && d->animate) {
		if (d->container->item())
			d->container->item()->setScale(0.0);
		d->checkAnimation();
		setOpacity(0.0);
		setVisible(true);
		d->animation.setDirection(QPropertyAnimation::Forward);
		d->animation.start();
	}
}

void TopLevelItem::hide() {
	if (isVisible() && d->animate) {
		d->checkAnimation();
		d->animation.setDirection(QPropertyAnimation::Backward);
		d->animation.start();
	}
}

void TopLevelItem::handleAnimationFinished() {
	if (d->animation.direction() == QPropertyAnimation::Backward)
		setVisible(false);
}
