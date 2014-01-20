#include "textlistitem.hpp"
#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlComponent>

static QMetaMethod mmPolishAndUpdate;
static QMetaMethod mmTextChanged;

struct TextListItem::Data {
	QQmlComponent *component = nullptr;
	QList<QQuickItem*> textItems;
	QStringList texts;
	QFont font;
	Qt::AlignmentFlag valign = Qt::AlignVCenter, haligh = Qt::AlignLeft;
	qreal textHeight = -1.0;
	bool interactive = false;
	template<typename T>
	void setTextProperty(const char *name, const T &t) {
		for (auto item : textItems)
			item->setProperty(name, t);
	}
};

TextListItem::TextListItem(QQuickItem *parent)
: ItemListItem(parent), d(new Data) {
	d->font = Theme::font();
	d->textHeight = Theme::lineHeight() - 2*Theme::padding();
	setVerticalPadding(Theme::padding());
	setHorizontalPadding(Theme::padding());
	connect(this, &ItemListItem::listChanged, this, &TextListItem::itemsChanged);
}

TextListItem::~TextListItem() {
	qDeleteAll(d->textItems);
	delete d->component;
	delete d;
}

QStringList TextListItem::texts() const {
	return d->texts;
}

QByteArray TextListItem::sourceCode() const {
	return "import QtQuick 2.2\nText { }";
}

void TextListItem::setTexts(const QStringList &texts) {
	d->texts = texts;
	if (!d->component) {
		d->component = new QQmlComponent(QQmlEngine::contextForObject(this)->engine());
		d->component->setData(sourceCode(), QUrl());
	}
	clear();
	if (d->textItems.size() < texts.size()) {
		d->textItems.reserve(texts.size());
		while (d->textItems.size() < texts.size()) {
			auto item = static_cast<QQuickItem*>(d->component->create());
			item->setProperty("verticalAlignment", d->valign);
			item->setProperty("horizontalAlignment", d->haligh);
			item->setProperty("font", d->font);
			auto attached = this->attached(item, true);
			attached->setInteractive(true);
			attached->setThickness(d->textHeight);
			if (!mmPolishAndUpdate.isValid()) {
				auto find = [] (QObject *object, const char *name) {
					auto *mm = object->metaObject();
					const int size = mm->methodCount();
					for (int i=0; i<size; ++i) {
						if (!qstrcmp(mm->method(i).name(), name))
							return mm->method(i);
					}
					return QMetaMethod();
				};
				mmPolishAndUpdate = find(this, "polishAndUpdate");
				mmTextChanged = find(item, "textChanged");
			}
			connect(item, mmTextChanged, this, mmPolishAndUpdate);
			d->textItems.append(item);
		}
	}
	for (int i=0; i<texts.size(); ++i) {
		d->textItems[i]->setProperty("text", texts[i]);
		append(d->textItems[i]);
	}
	emit textsChanged();
}

QFont TextListItem::font() const {
	return d->font;
}

void TextListItem::setFont(const QFont &font) {
	if (_Change(d->font, font)) {
		d->setTextProperty("font", d->font);
		polishAndUpdate();
		emit fontChanged();
	}
}

Qt::AlignmentFlag TextListItem::verticalAlignment() const {
	return d->valign;
}

Qt::AlignmentFlag TextListItem::horizontalAlignment() const {
	return d->haligh;
}

void TextListItem::setVerticalAlignment(Qt::AlignmentFlag alignment) {
	if (_Change(d->valign, alignment)) {
		d->setTextProperty("verticalAlignment", alignment);
		emit verticalAlignmentChanged();
	}
}

void TextListItem::setHorizontalAlignment(Qt::AlignmentFlag alignment) {
	if (_Change(d->haligh, alignment)) {
		d->setTextProperty("horizontalAlignment", alignment);
		emit horizontalAlignmentChanged();
	}
}

bool TextListItem::isInteractive() const {
	return d->interactive;
}

void TextListItem::setInteractive(bool interactive) {
	if (_Change(d->interactive, interactive)) {
		for (auto item : d->textItems)
			attached(item, false)->setInteractive(interactive);
		emit interactiveChanged();
	}
}

void TextListItem::setText(int index, const QString &text) {
	if (auto item = d->textItems.value(index))
		item->setProperty("text", text);
}

qreal TextListItem::textHeight() const {
	return d->textHeight;
}

void TextListItem::setTextHeight(qreal height) {
	if (_Change(d->textHeight, height)) {
		for (auto item : d->textItems)
			attached(item, false)->setThickness(d->textHeight);
		emit textHeightChanged();
	}
}
