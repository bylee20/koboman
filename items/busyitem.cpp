#include "busyitem.hpp"
#include "themeapi.hpp"
#include "utils.hpp"
#include <qmath.h>
#include <QPropertyAnimation>

struct ColorVertex {
	QPointF outer, inner;
	uchar r, g, b, a;
};

struct BusyItem::Data {
	BusyItem *p = nullptr;
	qreal thickness = Theme::padding();
	int segments = 100;
	QVector<ColorVertex> vertex;
	QColor start{255, 0, 0, 255}, end{255, 0, 0, 0};
	QPropertyAnimation rotator;
	bool running = true;
	static uchar interpolate(uchar v1, uchar v2, float r) {
		static const float r0 = 0.95;
		if (r < r0)
			return v1 + (v2-v1)*r/r0;
		return v2 + (v1-v2)*(r-r0)/(1.0-r0);
	}
	void updateAnimation() {
		if (running && p->isVisible())
			rotator.start();
		else
			rotator.stop();
	}
};

BusyItem::BusyItem(QQuickItem *parent)
: TextureItem(parent), d(new Data) {
	d->p = this;
	d->start = Theme::highlightDark();
	d->end = d->start; d->end.setAlpha(0);
	d->rotator.setDuration(1000);
	d->rotator.setLoopCount(-1);
	d->rotator.setTargetObject(this);
	d->rotator.setPropertyName("rotation");
	d->rotator.setStartValue(0.0);
	d->rotator.setEndValue(360.0);
}

BusyItem::~BusyItem() {
	delete d;
}

QSGGeometry *BusyItem::createSGGeometry() {
	auto geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), 0);
	geometry->setDrawingMode(GL_TRIANGLE_STRIP);
	return geometry;
}

void BusyItem::updateSGGeometry(QSGGeometry *geometry) {
	geometry->allocate(d->vertex.size()*2);
	auto p = geometry->vertexDataAsColoredPoint2D();
	for (int i=0; i<d->vertex.size(); ++i) {
		auto &v = d->vertex[i];
		p++->set(v.outer.x(), v.outer.y(), v.r, v.g, v.b, v.a);
		p++->set(v.inner.x(), v.inner.y(), v.r, v.g, v.b, v.a);
	}
}

QByteArray BusyItem::fragmentShader() const {
	static const char *shader = R"(
		varying highp vec4 color;
		void main() {
			gl_FragColor = color;
		}
	)";
	return shader;
}

QByteArray BusyItem::vertexShader() const {
	static const char *shader = R"(
		uniform highp mat4 vMatrix;
		uniform lowp float opacity;
		attribute highp vec4 vPosition;
		attribute highp vec4 vColor;
		varying highp vec4 color;
		void main() {
			color = vec4(vColor.rgb, 1.0)*(vColor.a*opacity);
			gl_Position = vMatrix*vPosition;
		}
	)";
	return shader;
}

const char *const *BusyItem::attributeNames() const {
	static const char *const names[] = {"vPosition", "vColor", 0};
	return names;
}

void BusyItem::updatePolish() {
	const int points = d->segments+1;
	constexpr auto pi2 = 2.0*M_PI;
	const double dt = pi2/d->segments;
	d->vertex.resize(points);
	const QPointF center(width()*0.5, height()*0.5);
	auto setPoint = [center] (QPointF &p, qreal radius, qreal radian) {
		p.rx() = radius*qCos(radian) + center.x();
		p.ry() = radius*qSin(radian) + center.y();
	};
	const auto inner = qMax(0.0, center.x()-d->thickness);
	const auto outer = qMin(center.x(), center.y());
	for (int i=0; i<points; ++i) {
		const double radian = dt*i;
		auto &v = d->vertex[i];
		setPoint(v.inner, inner, radian);
		setPoint(v.outer, outer, radian);
		const float r = radian/pi2;
		v.r = d->interpolate(d->start.red(), d->end.red(), r);
		v.g = d->interpolate(d->start.green(), d->end.green(), r);
		v.b = d->interpolate(d->start.blue(), d->end.blue(), r);
		v.a = d->interpolate(d->start.alpha(), d->end.alpha(), r);
	}
}

qreal BusyItem::thickness() const {
	return d->thickness;
}

void BusyItem::setThickness(qreal t) {
	if (_Change(d->thickness, t)) {
		setGeometryDirty();
		polishAndUpdate();
		emit thicknessChanged();
	}
}

int BusyItem::segments() const {
	return d->segments;
}

void BusyItem::setSegments(int seg) {
	if (_Change(d->segments, seg)) {
		setGeometryDirty();
		polishAndUpdate();
		emit segmentsChanged();
	}
}

void BusyItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) {
	TextureItem::geometryChanged(newGeometry, oldGeometry);
	setTransformOriginPoint({newGeometry.width()*0.5, newGeometry.height()*0.5});
	setGeometryDirty();
	polishAndUpdate();
}

void BusyItem::setRunning(bool running) {
	if (_Change(d->running, running)) {
		d->updateAnimation();
		emit runningChanged();
	}
}

void BusyItem::componentComplete() {
	TextureItem::componentComplete();
	d->updateAnimation();
}

void BusyItem::itemChange(ItemChange change, const ItemChangeData &data) {
	TextureItem::itemChange(change, data);
	if (change == ItemVisibleHasChanged)
		d->updateAnimation();
}

bool BusyItem::isRunning() const {
	return d->running;
}

int BusyItem::duration() const {
	return d->rotator.duration();
}

void BusyItem::setDuration(int duration) {
	if (d->rotator.duration() != duration) {
		d->rotator.setDuration(duration);
		emit durationChanged();
	}
}
