#include "buttonboxitem.hpp"

static QVector<QString> buttonTexts;

struct ButtonBoxItem::Data {
	QList<int> buttons;
	QString customButtonText;
};

ButtonBoxItem::ButtonBoxItem(QQuickItem *parent)
: TextListItem(parent), d(new Data) {
	if (buttonTexts.isEmpty()) {
		buttonTexts.resize(None+1);
		buttonTexts[Ok] = trUtf8("확인");
		buttonTexts[Cancel] = trUtf8("취소");
		buttonTexts[Yes] = trUtf8("예");
		buttonTexts[No] = trUtf8("아니오");
		buttonTexts[Open] = trUtf8("열기");
	}
	connect(this, &ItemListItem::clicked, this, &ButtonBoxItem::handleItemClicked);
	setVerticalAlignment(Qt::AlignVCenter);
	setHorizontalAlignment(Qt::AlignHCenter);
}

ButtonBoxItem::~ButtonBoxItem() {
	delete d;
}

void ButtonBoxItem::handleItemClicked(QQuickItem *item) {
	const int index = this->indexOf(item);
	if (_InRange(0, index, d->buttons.size()-1))
		emit triggered(d->buttons[index]);
}

void ButtonBoxItem::setButtons(const QList<int> &buttons) {
	if (_Change(d->buttons, buttons)) {
		QStringList texts; texts.reserve(buttons.size());
		for (auto button : buttons)
			texts.append(buttonTexts[button]);
		setTexts(texts);
		emit buttonsChanged();
	}
}

QString ButtonBoxItem::buttonText(Button button) const {
	return button == Custom ? d->customButtonText : buttonTexts[button];
}

QList<int> ButtonBoxItem::buttons() const {
	return d->buttons;
}

QString ButtonBoxItem::customButtonText() const {
	return d->customButtonText;
}

void ButtonBoxItem::setCustomButtonText(const QString &text) {
	if (_Change(d->customButtonText, text)) {
		const int index = d->buttons.indexOf(Custom);
		if (index != -1)
			setText(index, d->customButtonText);
		emit customButtonTextChanged();
	}
}

QQuickItem *ButtonBoxItem::button(Button button) const {
	return itemAt(d->buttons.indexOf(button));
}
