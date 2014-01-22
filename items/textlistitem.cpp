#include "textlistitem.hpp"
#include "utility.hpp"

static QMetaMethod mmPolishAndUpdate;
static QMetaMethod mmTextChanged;

struct TextListItem::Data {
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
	if (!mmPolishAndUpdate.isValid())
		mmPolishAndUpdate = _FindMethod<TextListItem>("polishAndUpdate");
	d->font = Theme::font();
	d->textHeight = Theme::lineHeight() - 2*Theme::padding();
	setFixedItemLength(d->textHeight);
	setVerticalPadding(Theme::padding());
	setHorizontalPadding(Theme::padding());
	connect(this, &ItemListItem::listChanged, this, &TextListItem::itemsChanged);
}

TextListItem::~TextListItem() {
	qDeleteAll(d->textItems);
	delete d;
}

QStringList TextListItem::texts() const {
	return d->texts;
}

void TextListItem::setTexts(const QStringList &texts) {
	d->texts = texts;
	clear();
	if (d->textItems.size() < texts.size()) {
		d->textItems.reserve(texts.size());
		while (d->textItems.size() < texts.size()) {
			auto item = Utility::createItem("Text");
			item->setProperty("verticalAlignment", d->valign);
			item->setProperty("horizontalAlignment", d->haligh);
			item->setProperty("font", d->font);
			if (!mmTextChanged.isValid())
				mmTextChanged = _FindMethod(item, "textChanged");
			Q_ASSERT(mmTextChanged.isValid());
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

void TextListItem::setText(int index, const QString &text) {
	if (auto item = d->textItems.value(index))
		item->setProperty("text", text);
}
