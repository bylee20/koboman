#ifndef ACTIONLISTITEM_HPP
#define ACTIONLISTITEM_HPP

#include "itemlistitem.hpp"

class Action : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString text MEMBER m_text NOTIFY textChanged)
	Q_PROPERTY(QUrl icon MEMBER m_icon NOTIFY iconChanged)
public:
	Action(QObject *parent = nullptr): QObject(parent) {}
	QString text() const { return m_text; }
	QUrl icon() const { return m_icon; }
signals:
	void textChanged();
	void iconChanged();
private:
	QString m_text;
	QUrl m_icon;
};

class ActionListItem : public ItemListItem {
	Q_OBJECT
	Q_PROPERTY(QQmlListProperty<QObject> items READ readOnlyList NOTIFY itemsChanged)
	Q_PROPERTY(QStringList texts READ texts WRITE setTexts NOTIFY textsChanged FINAL)
	Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged FINAL)
	Q_PROPERTY(Qt::AlignmentFlag verticalAlignment READ verticalAlignment WRITE setVerticalAlignment NOTIFY verticalAlignmentChanged FINAL)
	Q_PROPERTY(Qt::AlignmentFlag horizontalAlignment READ horizontalAlignment WRITE setHorizontalAlignment NOTIFY horizontalAlignmentChanged FINAL)
	Q_PROPERTY(bool interactive READ isInteractive WRITE setInteractive  NOTIFY interactiveChanged)
	Q_PROPERTY(qreal textHeight READ textHeight WRITE setTextHeight NOTIFY textHeightChanged)
public:
	ActionListItem(QQuickItem *parent = nullptr);
	~ActionListItem();
	QStringList texts() const;
	QQmlListProperty<Action> actions() const;
	void append(Action *action);
	void setTexts(const QStringList &texts);
	QFont font() const;
	void setFont(const QFont &font);
	Qt::AlignmentFlag verticalAlignment() const;
	Qt::AlignmentFlag horizontalAlignment() const;
	void setVerticalAlignment(Qt::AlignmentFlag alignment);
	void setHorizontalAlignment(Qt::AlignmentFlag alignment);
	bool isInteractive() const;
	void setInteractive(bool interactive);
	qreal textHeight() const;
	void setTextHeight(qreal height);
signals:
	void textHeightChanged();
	void interactiveChanged();
	void horizontalAlignmentChanged();
	void verticalAlignmentChanged();
	void fontChanged();
	void textsChanged();
	void itemsChanged();
protected:
	virtual QByteArray sourceCode() const;
	void setText(int index, const QString &text);
private:
	struct Data;
	Data *d;
};

#endif // ACTIONLISTITEM_HPP
