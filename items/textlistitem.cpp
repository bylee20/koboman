#include "textlistitem.hpp"
#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlComponent>

struct TextListItem::Data {
	QQmlComponent *component = nullptr;
	QList<QQuickItem*> textItems;
	QStringList texts;
	QFont font;
	Qt::AlignmentFlag valign = Qt::AlignVCenter, haligh = Qt::AlignLeft;
	bool interactive = false;
	template<typename T>
	void setTextProperty(const char *name, const T &t) {
		for (auto item : textItems)
			item->setProperty(name, t);
	}
};

TextListItem::TextListItem(QQuickItem *parent)
: ItemListItem(parent), d(new Data) {
	d->font = Utility::font();
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
			attached(item, true)->setInteractive(true);
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
