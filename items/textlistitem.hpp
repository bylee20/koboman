#ifndef TEXTLISTITEM_HPP
#define TEXTLISTITEM_HPP

#include "itemlistitem.hpp"

class TextListItem : public ItemListItem {
	Q_OBJECT
	Q_PROPERTY(QQmlListProperty<QObject> items READ readOnlyList NOTIFY itemsChanged)
	Q_PROPERTY(QStringList texts READ texts WRITE setTexts NOTIFY textsChanged FINAL)
	Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged FINAL)
	Q_PROPERTY(Qt::AlignmentFlag verticalAlignment READ verticalAlignment WRITE setVerticalAlignment NOTIFY verticalAlignmentChanged FINAL)
	Q_PROPERTY(Qt::AlignmentFlag horizontalAlignment READ horizontalAlignment WRITE setHorizontalAlignment NOTIFY horizontalAlignmentChanged FINAL)
	Q_PROPERTY(bool interactive READ isInteractive WRITE setInteractive  NOTIFY interactiveChanged)
public:
	TextListItem(QQuickItem *parent = nullptr);
	~TextListItem();
	QStringList texts() const;
	void setTexts(const QStringList &texts);
	QFont font() const;
	void setFont(const QFont &font);
	Qt::AlignmentFlag verticalAlignment() const;
	Qt::AlignmentFlag horizontalAlignment() const;
	void setVerticalAlignment(Qt::AlignmentFlag alignment);
	void setHorizontalAlignment(Qt::AlignmentFlag alignment);
	bool isInteractive() const;
	void setInteractive(bool interactive);
signals:
	void interactiveChanged();
	void horizontalAlignmentChanged();
	void verticalAlignmentChanged();
	void fontChanged();
	void textsChanged();
	void itemsChanged();
protected:
	virtual QByteArray sourceCode() const;
private:
	struct Data;
	Data *d;
};

#endif // TEXTLISTITEM_HPP
