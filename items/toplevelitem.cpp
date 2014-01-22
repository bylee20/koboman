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
	QTransform trans;
	int loc_shade = -1, loc_tex_box = -1, loc_trans = -1;
	qreal shade = 0.5, boundary = Theme::spacing();
	GLuint texBox = GL_NONE, transparent = GL_NONE;
	bool redraw = false, upload = false, autohide = true, animate = true;
	QSize textureSize{0, 0};
	QRectF boxRect{0.0, 0.0, 0.0, 0.0};
	TopLevelContainer *container = nullptr;
	TopLevelShadow *shadow = nullptr;
	QParallelAnimationGroup animation;
	QPropertyAnimation scaler, dimmer;
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
//	connect(this, &TopLevelItem::focusChanged, this, &TopLevelItem::updateFocusState);
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
		if (_Change(d->boxRect, d->container->paintingArea()))
			updateContainerRect();
		d->upload = true;
		d->update();
	});
	connect(d->container, &TopLevelContainer::paintingAreaChanged, [this] () {
		if (_Change(d->boxRect, d->container->paintingArea()))
			updateContainerRect();
		d->update();
	});
}

QByteArray TopLevelItem::fragmentShader() const {
	static const char *shader = R"(
		uniform sampler2D tex_box;
		varying highp vec2 texCoord;
		uniform lowp float shade, opacity;
		void main() {
			highp vec4 color = texture2D(tex_box, texCoord).bgra;
			gl_FragColor = mix(vec4(0.0, 0.0, 0.0, shade), vec4(color.rgb, 1.0), color.a);
			gl_FragColor.a *= opacity;
		}
	)";
	return shader;
}

QByteArray TopLevelItem::vertexShader() const {
	static const char *shader = R"(
		uniform highp mat4 vMatrix;
		uniform highp mat3 trans;
		attribute highp vec4 vPosition;
		attribute highp vec2 vCoord;
		varying highp vec2 texCoord;
		void main() {
			texCoord = (trans*vec3(vCoord, 1.0)).xy;
			gl_Position = vMatrix*vPosition;
		}
	)";
	return shader;
}

void TopLevelItem::initializeGL() {
	TextureItem::initializeGL();
	glGenTextures(1, &d->texBox);
	glGenTextures(1, &d->transparent);
	newTexure(d->transparent, 1, 1, GL_RGBA, GL_RGBA);
	static const quint32 zero = 0x0;
	uploadTexture(d->transparent, 1, 1, GL_RGBA, &zero);
}

void TopLevelItem::finalizeGL() {
	glDeleteTextures(1, &d->texBox);
	d->texBox = GL_NONE;
	TextureItem::finalizeGL();
}

void TopLevelItem::bind(QOpenGLShaderProgram *prog, const QSGMaterialShader::RenderState &state) {
	TextureItem::bind(prog, state);
	prog->setUniformValue(d->loc_shade, (GLfloat)d->shade);
	prog->setUniformValue(d->loc_tex_box, 0);
	prog->setUniformValue(d->loc_trans, d->trans);
	activateTexture(d->textureSize.isEmpty() ? d->transparent : d->texBox, 0);
}

void TopLevelItem::link(QOpenGLShaderProgram *prog) {
	TextureItem::link(prog);
	d->loc_shade = prog->uniformLocation("shade");
	d->loc_tex_box = prog->uniformLocation("tex_box");
	d->loc_trans = prog->uniformLocation("trans");
}

bool TopLevelItem::prepareToRender() {
	if (d->upload) {
		auto &image = d->container->image();
		if (_Change(d->textureSize, image.size()))
			newTexure(d->texBox, image.width(), image.height(), GL_RGBA, GL_RGBA, GL_NEAREST);
		uploadTexture(d->texBox, image.width(), image.height(), GL_RGBA, image.bits());
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
	qDebug() << "key";
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
	d->trans.reset();
	d->trans.scale(width()/d->boxRect.width(), height()/d->boxRect.height());
	d->trans.translate(-d->boxRect.x()/width(), -d->boxRect.y()/height());
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
