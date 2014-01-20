#ifndef BUTTONBOXITEM_HPP
#define BUTTONBOXITEM_HPP

#include "textlistitem.hpp"

class ButtonBoxItem : public TextListItem {
	Q_OBJECT
	Q_ENUMS(Button)
	Q_PROPERTY(QList<int> buttons READ buttons WRITE setButtons NOTIFY buttonsChanged)
	Q_PROPERTY(QString customButtonText READ customButtonText WRITE setCustomButtonText NOTIFY customButtonTextChanged)
public:
	enum Button { Ok, Cancel, Yes, No, Custom, None };
	ButtonBoxItem(QQuickItem *parent = nullptr);
	~ButtonBoxItem();
	void setButtons(const QList<int> &buttons);
	QList<int> buttons() const;
	Q_INVOKABLE static QString buttonText(Button button);
	QString customButtonText() const;
	void setCustomButtonText(const QString &text);
signals:
	void customButtonTextChanged();
	void buttonsChanged();
	void triggered(int button);
private:
	void handleItemClicked(QQuickItem *item);
	struct Data;
	Data *d;
};

#endif // BUTTONBOXITEM_HPP
