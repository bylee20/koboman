#include "itemlistitem.hpp"
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWindow>
#include "utils.hpp"

static inline qreal pick(qreal v, qreal fallback) { return v < 0 ? fallback : v; }

struct ItemPos {
	ItemListAttached *attached = nullptr;
	qreal pos = 0.0;
	bool transparent = false;
};

struct VerticalOrientation {
	static qreal padding(const ItemListItem *p, const ItemListAttached *attached) {
		return pick(attached->verticalPadding(), p->verticalPadding());
	}
	static qreal length(const ItemListItem *p) { return p->height(); }
	static QRectF itemRect(const ItemListItem *p, const ItemListAttached *attached, qreal pos, const QPointF &pad) {
		return QRectF(pad.x(), pad.y() + pos, p->width() - pad.x()*2, attached->filling() - 2*pad.y());
	}
	static QRectF vertexRect(const ItemListItem *p, const ItemPos &item) {
		return QRectF(0.0, item.pos, p->width(), item.attached->filling());
	}
};

struct HorizontalOrientation {
	static qreal padding(const ItemListItem *p, const ItemListAttached *attached) {
		return pick(attached->horizontalPadding(), p->horizontalPadding());
	}
	static qreal length(const ItemListItem *p) { return p->width(); }
	static QRectF itemRect(const ItemListItem *p, const ItemListAttached *attached, qreal pos, const QPointF &pad) {
		return QRectF(pad.x() + pos, pad.y(), attached->filling() - pad.x()*2, p->height() - 2*pad.y());
	}
	static QRectF vertexRect(const ItemListItem *p, const ItemPos &item) {
		return QRectF(item.pos, 0.0, item.attached->filling(), p->height());
	}
};

struct ItemListItem::Data {
	ItemListItem *p = nullptr;
	Qt::Orientation orientation = Qt::Vertical;
	QList<ListItem> list;
	ItemPos pressed;
	ListItem headerItem, footerItem, headerSeparator, footerSeparator, separator;
	QVector<ItemPos> rects;
	int rectCount = 0;
	QColor originalColor;
	qreal vpad = 0, hpad = 0, minimum = 0;
	void connectAttached(ItemListAttached *attached) {
		connect(attached, &ItemListAttached::thicknessChanged, p, &ItemListItem::polishAndUpdate);
		connect(attached, &ItemListAttached::verticalPaddingChanged, p, &ItemListItem::polishAndUpdate);
		connect(attached, &ItemListAttached::horizontalPaddingChanged, p, &ItemListItem::polishAndUpdate);
		connect(attached, &ItemListAttached::colorChanged, p, &ItemListItem::handleItemColorChanged);
	}
	bool changeListItem(ListItem &item, QObject *object) {
		if (item.object() == object)
			return false;
		if (item.object())
			disconnect(item.object(), nullptr, p, nullptr);
		item = ListItem{object};
		if (item.attached()) {
			if (!item.attached()->isSeparator())
				static_cast<QQuickItem*>(object)->setParentItem(p);
			connectAttached(item.attached());
		}
		p->polishAndUpdate();
		return true;
	}
	template<typename T>
	bool changeAndPolish(T &t, const T &new_) {
		if (!_Change(t, new_))
			return false;
		p->polishAndUpdate();
		return true;
	}

	void releaseMouse() {
		if (pressed.attached && pressed.attached->isInteractive()) {
			pressed.attached->setColor(originalColor);
			pressed = ItemPos();
		}
	}
	template<typename O>
	void layout() {
		rectCount = 0;
		minimum = 0;
		if (list.isEmpty())
			return;
		_Expand(rects, list.size()+4, 2.1);

		int fills = 0;
		qreal filled = 0;
		bool prevItem = false;
		auto acc = [&filled, &fills, this] (const ListItem &item) {
			filled += 2*O::padding(p, item.attached());
			if (item.attached()->thickness() < 0)
				++fills;
			else
				filled += item.attached()->thickness();
		};
		if (headerItem.object()) {
			acc(headerItem);
			acc(headerSeparator.object() ? headerSeparator : separator);
		}
		for (int i=0; i<list.size(); ++i) {
			auto &item = list[i];
			if (qFuzzyCompare(item.attached()->thickness(), 0.0))
				continue;
			if (prevItem && item.attached()->isQmlItem())
				acc(separator);
			acc(item);
			prevItem = item.attached()->isQmlItem();
		}
		if (footerItem.object()) {
			acc(footerSeparator.object() ? footerSeparator : separator);
			acc(footerItem);
		}
		const auto fill = qMax((O::length(p) - filled)/fills, 0.0);
		prevItem = false;
		auto it = rects.begin();
		qreal pos = 0;
		auto append = [&it, &pos, fill, this] (const ListItem &item) {
			auto attached = item.attached();
			const qreal total = pick(attached->thickness(), fill) + 2*O::padding(p, attached);
			attached->fill(total);
			const bool isItem = attached->isQmlItem();
			if (isItem) {
				const auto vpad = pick(attached->verticalPadding(), this->vpad);
				const auto hpad = pick(attached->horizontalPadding(), this->hpad);
				const auto rect = O::itemRect(p, attached, pos, {hpad, vpad});
				auto qml = static_cast<QQuickItem*>(item.object());
				qml->setPosition(rect.topLeft());
				qml->setSize(rect.size());
			}
			it->attached = attached;
			it->pos = pos;
			++it;
			pos += total;
			return isItem;
		};
		if (headerItem.object()) {
			append(headerItem);
			append(headerSeparator.object() ? headerSeparator : separator);
		}
		for (int i=0; i<list.size(); ++i) {
			auto &item = list[i];
			if (qFuzzyCompare(item.attached()->thickness(), 0.0))
				continue;
			if (prevItem && item.attached()->isQmlItem())
				append(separator);
			prevItem = append(item);
		}
		rectCount = std::distance(rects.begin(), it);
		if (_Change(minimum, filled))
			emit p->minimumLengthChanged();
		p->setGeometryDirty();
	}
	template<typename O>
	void updateVertex(QSGGeometry *geometry) {
		int size = 0;
		for (int i=0; i<rectCount; ++i) {
			if (rects[i].attached->color().alpha())
				size += 6;
		}
		geometry->allocate(size);
		if (size > 0) {
			auto v = geometry->vertexDataAsColoredPoint2D();
			for (int i=0; i<rectCount; ++i) {
				const auto &item = rects[i];
				if (item.attached->color().alpha()) {
					v = fillColoredPointAsTriangle(v, O::vertexRect(p, item), item.attached->color());
				}
			}
		}
	}
	template<typename O>
	ItemPos contains(const QPointF &pos) {
		for (int i=0; i<rectCount; ++i) {
			if (O::vertexRect(p, rects[i]).contains(pos))
				return rects[i];
		}
		return ItemPos();
	}
};

ItemListItem::ItemListItem(QQuickItem *parent)
: TextureItem(parent), d(new Data) {
	d->p = this;
	d->separator = ListItem(new ItemListSeparator);
	Q_ASSERT(d->separator.attached()->isSeparator());
	setFlag(ItemHasContents);
	setAcceptedMouseButtons(Qt::LeftButton);
	connect(this, &ItemListItem::horizontalPaddingChanged, this, &ItemListItem::polish);
	connect(this, &ItemListItem::verticalPaddingChanged, this, &ItemListItem::polish);
}

ItemListItem::~ItemListItem() {
	delete d->separator.object();
	delete d;
}

QQmlListProperty<QObject> ItemListItem::list() const {
	static auto append = [] (QQmlListProperty<QObject> *list, QObject *item) {
		static_cast<ItemListItem*>(list->object)->append(item);
	};
	static auto count = [] (QQmlListProperty<QObject> *list) {
		return static_cast<ItemListItem*>(list->object)->d->list.size();
	};
	static auto at = [] (QQmlListProperty<QObject> *list, int index) -> QObject* {
		return static_cast<ItemListItem*>(list->object)->d->list.at(index).object();
	};
	static auto clear = [] (QQmlListProperty<QObject> *list) {
		static_cast<ItemListItem*>(list->object)->clear();
	};
	return QQmlListProperty<QObject>(const_cast<ItemListItem*>(this), nullptr, append, count, at, clear);
}

void ItemListItem::append(QObject *obj) {
	ListItem item(obj);
	if (!item.attached()->isSeparator())
		static_cast<QQuickItem*>(obj)->setParentItem(this);
	d->connectAttached(item.attached());
	d->list.append(item);
	polishAndUpdate();
}

void ItemListItem::clear() {
	for (int i=0; i<d->list.size(); ++i)
		disconnect(d->list[i].attached(), nullptr, this, nullptr);
	d->list.clear();
}

void ItemListItem::handleItemColorChanged() {
	setGeometryDirty();
	update();
}

QByteArray ItemListItem::fragmentShader() const {
	static const char *shader = R"(
		varying highp vec4 color;
		void main() {
			gl_FragColor = color;
		}
	)";
	return shader;
}

QByteArray ItemListItem::vertexShader() const {
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

const char *const *ItemListItem::attributeNames() const {
	static const char *const names[] = {"vPosition", "vColor", 0};
	return names;
}

void ItemListItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) {
	TextureItem::geometryChanged(newGeometry, oldGeometry);
	polishAndUpdate();
}

void ItemListItem::updatePolish() {
	if (d->orientation == Qt::Horizontal)
		d->layout<HorizontalOrientation>();
	else
		d->layout<VerticalOrientation>();
}

QSGGeometry *ItemListItem::createSGGeometry() {
	auto geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), 0);
	geometry->setDrawingMode(GL_TRIANGLES);
	return geometry;
}

void ItemListItem::updateSGGeometry(QSGGeometry *geometry) {
	if (d->orientation == Qt::Horizontal)
		d->updateVertex<HorizontalOrientation>(geometry);
	else
		d->updateVertex<VerticalOrientation>(geometry);
}

ItemListSeparator *ItemListItem::separator() const {
	return static_cast<ItemListSeparator*>(d->separator.object());
}

void ItemListItem::mousePressEvent(QMouseEvent *event) {
	TextureItem::mousePressEvent(event);
	event->accept();
	d->releaseMouse();
	if (d->orientation == Qt::Horizontal)
		d->pressed = d->contains<HorizontalOrientation>(event->localPos());
	else
		d->pressed = d->contains<VerticalOrientation>(event->localPos());
	if (d->pressed.attached && d->pressed.attached->isInteractive()) {
		d->originalColor = d->pressed.attached->color();
		d->pressed.attached->setColor(Qt::blue);
	}
}

void ItemListItem::mouseReleaseEvent(QMouseEvent *event) {
	TextureItem::mouseReleaseEvent(event);
	if (d->pressed.attached && d->pressed.attached->isInteractive()) {
		QRectF rect;
		if (d->orientation == Qt::Horizontal)
			rect = HorizontalOrientation::vertexRect(this, d->pressed);
		else
			rect = VerticalOrientation::vertexRect(this, d->pressed);
		if (rect.contains(event->localPos()))
			emit d->pressed.attached->clicked();
	}
	d->releaseMouse();
}

void ItemListItem::mouseUngrabEvent() {
	TextureItem::mouseUngrabEvent();
	d->releaseMouse();
}

void ItemListItem::touchEvent(QTouchEvent *event) {
	TextureItem::touchEvent(event);
}

void ItemListItem::touchUngrabEvent() {
	TextureItem::touchUngrabEvent();
	d->releaseMouse();
}

qreal ItemListItem::verticalPadding() const {
	return d->vpad;

}
qreal ItemListItem::horizontalPadding() const {
	return d->hpad;
}

void ItemListItem::setVerticalPadding(qreal pad) {
	if (_Change(d->vpad, pad))
		emit verticalPaddingChanged();
}

void ItemListItem::setHorizontalPadding(qreal pad) {
	if (_Change(d->hpad, pad))
		emit horizontalPaddingChanged();
}

Qt::Orientation ItemListItem::orientation() const {
	return d->orientation;
}

void ItemListItem::setOrientation(Qt::Orientation o) {
	if (d->changeAndPolish(d->orientation, o))
		emit orientationChanged();
}

ItemListSeparator *ItemListItem::headerSeparator() const {
	return static_cast<ItemListSeparator*>(d->headerSeparator.object());
}

ItemListSeparator *ItemListItem::footerSeparator() const {
	return static_cast<ItemListSeparator*>(d->footerSeparator.object());
}

QQuickItem *ItemListItem::headerItem() const {
	return static_cast<QQuickItem*>(d->headerItem.object());
}

QQuickItem *ItemListItem::footerItem() const {
	return static_cast<QQuickItem*>(d->footerItem.object());
}

void ItemListItem::setHeaderItem(QQuickItem *item) {
	if (d->changeListItem(d->headerItem, item))
		emit headerItemChanged();
}

void ItemListItem::setFooterItem(QQuickItem *item) {
	if (d->changeListItem(d->footerItem, item))
		emit footerItemChanged();
}

void ItemListItem::setHeaderSeparator(ItemListSeparator *sep) {
	if (d->changeListItem(d->headerSeparator, sep))
		emit headerSeparatorChanged();
}

void ItemListItem::setFooterSeparator(ItemListSeparator *sep) {
	if (d->changeListItem(d->footerSeparator, sep))
		emit footerSeparatorChanged();
}

qreal ItemListItem::minimumLength() const {
	return d->minimum;
}

const QList<ItemListItem::ListItem> &ItemListItem::itemList() const {
	return d->list;
}
