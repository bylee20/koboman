#include "itemlistitem.hpp"
#include "itemlistattached.hpp"
#include "utils.hpp"
#include "actionitem.hpp"

static inline qreal pick(qreal v, qreal fallback) { return v < 0 ? fallback : v; }
template<typename T>
static inline T *pick(T *v, T *fallback) { return v ? v : fallback; }

struct ItemPos {
	operator bool () const { return attached != nullptr; }
	ItemListAttached *operator -> () { return attached; }
	ItemListAttached *attached = nullptr;
	qreal pos = 0.0;
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
	bool interactive = false;
	SelectionMode selectionMode = NoSelection;
	QColor highlight = Theme::highlight(), selection;
	Qt::Orientation orientation = Qt::Vertical;
	QList<ItemListAttached*> list;
	QList<ItemListAttached*> selections;
	ItemPos pressed;
	ItemListAttached *headerAttached = nullptr, *footerAttached = nullptr;
	ItemListSeparator separator, *headerSeparator = nullptr, *footerSeparator = nullptr;
	QVector<ItemPos> rects;
	int rectCount = 0;
	qreal vpad = 0, hpad = 0, minimum = 0, length = -1;
	ItemListSeparator *getSeparator(ItemListSeparator *candidate) const {
		return pick(candidate, const_cast<ItemListSeparator*>(&separator));
	}
	ItemListSeparator *getHeaderSeparator() const { return getSeparator(headerSeparator); }
	ItemListSeparator *getFooterSeparator() const { return getSeparator(footerSeparator); }
	ItemListAttached *attach(QObject *object) {
		if (!object)
			return nullptr;
		ItemListAttached *attached = nullptr;
		if (auto item = qobject_cast<QQuickItem*>(object)) {
			attached = p->attached(item, false);
			if (!attached)
				attached = p->attached(item, true);
			item->setParentItem(p);
			connect(item, &QQuickItem::visibleChanged, p, &ItemListItem::polishAndUpdate);
			connect(item, &QQuickItem::enabledChanged, p, &ItemListItem::handleEnabledChanged);
		} else {
			attached = static_cast<ItemListSeparator*>(object);
			Q_ASSERT(attached->isSeparator());
		}
		Q_ASSERT(attached != nullptr);
		connectAttached(attached);
		p->polishAndUpdate();
		return attached;
	}
	void connectAttached(ItemListAttached *attached) {
		connect(attached, &ItemListAttached::thicknessChanged, p, &ItemListItem::polishAndUpdate);
		connect(attached, &ItemListAttached::verticalPaddingChanged, p, &ItemListItem::polishAndUpdate);
		connect(attached, &ItemListAttached::horizontalPaddingChanged, p, &ItemListItem::polishAndUpdate);
		connect(attached, &ItemListAttached::colorChanged, p, &ItemListItem::forceToUpdateGeometry);
		connect(attached, &ItemListAttached::selectedChanged, p, &ItemListItem::handleSelectionChanged);
		connect(p, &ItemListItem::orientationChanged, attached, &ItemListAttached::setOrientation);
	}
	template<typename Attached>
	bool changeAttached(Attached *&attached, QObject *object) {
		static_assert(std::is_same<Attached, ItemListAttached>::value
			|| std::is_same<Attached, ItemListSeparator>::value, "error!");
		if (attached) {
			if (attached->attachee() == object)
				return false;
			disconnect(attached->attachee(), nullptr, p, nullptr);
			disconnect(attached, nullptr, p, nullptr);
		} else if (!object)
			return false;
		attached = static_cast<Attached*>(attach(object));
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
		if (pressed) {
			pressed = ItemPos();
			p->forceToUpdateGeometry();
		}
	}
	bool isHeaderVisible() const { return headerAttached && headerAttached->asItem()->isVisible(); }
	bool isFooterVisible() const { return footerAttached && footerAttached->asItem()->isVisible(); }
	template<typename InLoop>
	void runLayoutLoop(InLoop inLoop) {
		if (isHeaderVisible()) {
			inLoop(headerAttached);
			inLoop(getHeaderSeparator());
		}
		bool prevItem = false;
		for (int i=0; i<list.size(); ++i) {
			auto attached = list[i];
			if (qFuzzyCompare(attached->thickness(), 0.0))
				continue;
			if (prevItem) { // prev is item
				if (attached->isQmlItem()) {
					if (attached->asItem()->isVisible()) {
						inLoop(&separator); // add sep
						inLoop(attached);   // and item
					}
				} else// attachee is separator
					inLoop(attached); // add sep
			} else { // prev is separator
				if (attached->isQmlItem()) {
					if (attached->asItem()->isVisible())
						inLoop(attached); // add item
				} // skip separator
			}
			prevItem = attached->isQmlItem();
		}
		if (isFooterVisible()) {
			inLoop(getFooterSeparator());
			inLoop(footerAttached);
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
		auto calculate = [&filled, &fills, this] (const ItemListAttached *attached) {
			filled += 2*O::padding(p, attached);
			if (attached->thickness() < 0) {
				if (length < 0)
					++fills;
				else
					filled += length;
			} else
				filled += attached->thickness();
		};
		runLayoutLoop(calculate);
		const auto fill = qMax((O::length(p) - filled)/fills, 0.0);
		auto it = rects.begin();
		qreal pos = 0;
		auto append = [&it, &pos, fill, this] (ItemListAttached *attached) {
			qreal total = 2*O::padding(p, attached);
			if (attached->thickness() < 0) {
				if (length < 0)
					total += fill;
				else
					total += length;
			} else
				total += attached->thickness();
			attached->fill(total);
			if (attached->isQmlItem()) {
				const auto vpad = pick(attached->verticalPadding(), this->vpad);
				const auto hpad = pick(attached->horizontalPadding(), this->hpad);
				const auto rect = O::itemRect(p, attached, pos, {hpad, vpad});
				auto qml = static_cast<QQuickItem*>(attached->attachee());
				qml->setPosition(rect.topLeft());
				qml->setSize(rect.size());
			}
			it->attached = attached;
			it->pos = pos;
			++it;
			pos += total;
		};
		runLayoutLoop(append);
		rectCount = std::distance(rects.begin(), it);
		if (_Change(minimum, filled))
			emit p->minimumLengthChanged();
		p->setGeometryDirty();
	}
	template<typename O>
	void updateVertex(QSGGeometry *geometry) {
		int size = 0;
		for (int i=0; i<rectCount; ++i) {
			auto attached = rects[i].attached;
			if (attached->filling() > 0 && (attached == pressed.attached || attached->isSelected() || attached->color().alpha()))
				size += 6;
		}
		geometry->allocate(size);
		if (size > 0) {
			auto v = geometry->vertexDataAsColoredPoint2D();
			QColor color;
			for (int i=0; i<rectCount; ++i) {
				const auto &item = rects[i]; const auto attached = item.attached;
				if (attached->filling() <= 0)
					continue;
				if (attached == pressed.attached)
					color = highlight;
				else if (attached->isSelected())
					color = selection;
				else if (!attached->color().alpha())
					continue;
				else
					color = attached->color();
				v = fillColoredPointAsTriangle(v, O::vertexRect(p, item), color);
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
	static auto listAppend(QQmlListProperty<QObject> *list, QObject *item) -> void {
		static_cast<ItemListItem*>(list->object)->append(item);
	}
	static auto listCount(QQmlListProperty<QObject> *list) -> int {
		return static_cast<ItemListItem*>(list->object)->d->list.size();
	}
	static auto listAt(QQmlListProperty<QObject> *list, int index) -> QObject* {
		return static_cast<ItemListItem*>(list->object)->d->list.at(index)->attachee();
	}
	static auto listClear(QQmlListProperty<QObject> *list) -> void {
		static_cast<ItemListItem*>(list->object)->clear();
	}
};

ItemListItem::ItemListItem(QQuickItem *parent)
: TextureItem(parent), d(new Data) {
	d->p = this;
	d->selection = d->highlight;
	d->selection.setAlphaF(0.6);
	setAcceptedMouseButtons(Qt::LeftButton);
	connect(this, &ItemListItem::horizontalPaddingChanged, this, &ItemListItem::polishAndUpdate);
	connect(this, &ItemListItem::verticalPaddingChanged, this, &ItemListItem::polishAndUpdate);
}

ItemListItem::~ItemListItem() {
	delete d;
}

void ItemListItem::handleSelectionChanged() {
	auto attached = static_cast<ItemListAttached*>(sender());
	Q_ASSERT(attached != nullptr);
	bool clear = false;
	if (d->selectionMode == SingleSelection) {
		if (attached->isSelected()) {
			Q_ASSERT(d->selections.size() < 2);
			if (d->selections.isEmpty())
				d->selections.append(attached);
			else {
				d->selections[0]->selection() = false;
				d->selections[0] = attached;
			}
		} else
			clear = true;
	} else if (d->selectionMode == MultiSelection) {
		if (attached->isSelected())
			d->selections.append(attached);
		else
			d->selections.removeOne(attached);
	} else
		clear = true;
	if (clear)
		clearSelections();
	else {
		emit selectionsChanged();
		forceToUpdateGeometry();
	}
}

QQmlListProperty<QQuickItem> ItemListItem::selections() const {
	static auto count = [] (QQmlListProperty<QQuickItem> *list) -> int {
		return static_cast<ItemListItem*>(list->object)->d->selections.size();
	};
	static auto at = [] (QQmlListProperty<QQuickItem> *list, int index) -> QQuickItem * {
		return static_cast<ItemListItem*>(list->object)->d->selections.at(index)->asItem();
	};
	return QQmlListProperty<QQuickItem>(const_cast<ItemListItem*>(this), nullptr, count, at);
}

ItemListAttached *ItemListItem::attached(QQuickItem *item, bool create) {
	return static_cast<ItemListAttached*>(qmlAttachedPropertiesObject<ItemListItem>(item, create));
}

ItemListAttached *ItemListItem::qmlAttachedProperties(QObject *parent) {
	return new ItemListAttached(parent);
}

QQmlListProperty<QObject> ItemListItem::list() const {
	return QQmlListProperty<QObject>(const_cast<ItemListItem*>(this), nullptr
		, Data::listAppend, Data::listCount, Data::listAt, Data::listClear);
}

QQmlListProperty<QObject> ItemListItem::readOnlyList() const {
	return QQmlListProperty<QObject>(const_cast<ItemListItem*>(this), nullptr
		, Data::listCount, Data::listAt);
}

void ItemListItem::append(QObject *obj) {
	auto attached = d->attach(obj);
	attached->setIndex(d->list.size());
	attached->setOrientation(d->orientation);
	d->list.append(attached);
}

void ItemListItem::clear() {
	for (int i=0; i<d->list.size(); ++i)
		disconnect(d->list[i], nullptr, this, nullptr);
	d->list.clear();
}

void ItemListItem::forceToUpdateGeometry() {
	setGeometryDirty();
	update();
}

void ItemListItem::handleEnabledChanged() {
	auto item = static_cast<QQuickItem*>(sender());
	Q_ASSERT(item != nullptr);
	item->setOpacity(item->isEnabled() ? 1.0 : 0.3);
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
	return &d->separator;
}

void ItemListItem::mousePressEvent(QMouseEvent *event) {
	TextureItem::mousePressEvent(event);
	event->accept();
	if (!d->interactive)
		return;
	ItemPos pressed;
	if (d->orientation == Qt::Horizontal)
		pressed = d->contains<HorizontalOrientation>(event->localPos());
	else
		pressed = d->contains<VerticalOrientation>(event->localPos());
	if (pressed && !pressed->canInteract())
		pressed = ItemPos();
	if (d->pressed.attached != pressed.attached) {
		d->pressed = pressed;
		forceToUpdateGeometry();
	}
}

void ItemListItem::mouseReleaseEvent(QMouseEvent *event) {
	TextureItem::mouseReleaseEvent(event);
	event->accept();
	if (!d->interactive)
		return;
	if (d->pressed) {
		QRectF rect;
		if (d->orientation == Qt::Horizontal)
			rect = HorizontalOrientation::vertexRect(this, d->pressed);
		else
			rect = VerticalOrientation::vertexRect(this, d->pressed);
		if (rect.contains(event->localPos())) {
			emit d->pressed->clicked();
			emit clicked(static_cast<QQuickItem*>(d->pressed->attachee()));
			if (d->selectionMode)
				d->pressed->setSelected(true);
		}
		d->releaseMouse();
	}
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
		emit orientationChanged(o);
}

ItemListSeparator *ItemListItem::headerSeparator() const {
	return d->headerSeparator;
}

ItemListSeparator *ItemListItem::footerSeparator() const {
	return d->footerSeparator;
}

QQuickItem *ItemListItem::headerItem() const {
	return d->headerAttached ? static_cast<QQuickItem*>(d->headerAttached->attachee()) : nullptr;
}

QQuickItem *ItemListItem::footerItem() const {
	return d->footerAttached ? static_cast<QQuickItem*>(d->footerAttached->attachee()) : nullptr;
}

void ItemListItem::setHeaderItem(QQuickItem *item) {
	if (d->changeAttached(d->headerAttached, item)) {
		d->headerAttached->setIndex(HeaderItem);
		emit headerItemChanged();
	}
}

void ItemListItem::setFooterItem(QQuickItem *item) {
	if (d->changeAttached(d->footerAttached, item)) {
		d->footerAttached->setIndex(FooterItem);
		emit footerItemChanged();
	}
}

void ItemListItem::setHeaderSeparator(ItemListSeparator *sep) {
	if (d->changeAttached(d->headerSeparator, sep))
		emit headerSeparatorChanged();
}

void ItemListItem::setFooterSeparator(ItemListSeparator *sep) {
	if (d->changeAttached(d->footerSeparator, sep))
		emit footerSeparatorChanged();
}

qreal ItemListItem::minimumLength() const {
	return d->minimum;
}

const QList<ItemListAttached*> &ItemListItem::attachedList() const {
	return d->list;
}

QColor ItemListItem::highlight() const {
	return d->highlight;
}

void ItemListItem::setHighlight(const QColor &color) {
	if (_Change(d->highlight, color)) {
		if (d->pressed.attached && d->pressed.attached->canInteract())
			d->pressed.attached->setColor(d->highlight);
		emit highlightChanged();
	}
}

qreal ItemListItem::fixedItemLength() const {
	return d->length;
}

void ItemListItem::setFixedItemLength(qreal width) {
	if (_Change(d->length, width)) {
		polishAndUpdate();
		emit fixedItemLengthChanged();
	}
}

bool ItemListItem::isInteractive() const {
	return d->interactive;
}

void ItemListItem::setInteractive(bool interactive) {
	if (_Change(d->interactive, interactive))
		emit interactiveChanged();
}

ItemListItem::SelectionMode ItemListItem::selectionMode() const {
	return d->selectionMode;
}

void ItemListItem::setSelectionMode(SelectionMode mode) {
	if (_Change(d->selectionMode, mode)) {
		clearSelections();
		emit selectionModeChanged();
	}
}

int ItemListItem::indexOf(QQuickItem *item) const {
	auto attached = this->attached(item, false);
	return attached ? attached->index() : -1;
}

QQuickItem *ItemListItem::itemAt(int index) const {
	if (index == HeaderItem)
		return headerItem();
	if (index == FooterItem)
		return footerItem();
	auto attached = d->list.value(index);
	return attached && attached->isQmlItem() ? attached->asItem() : nullptr;
}

void ItemListItem::clearSelections() {
	if (!d->selections.isEmpty()) {
		for (auto one : d->selections)
			one->selection() = false;
		d->selections.clear();
		emit selectionsChanged();
		forceToUpdateGeometry();
	}
}
