#ifndef ACTIONITEM_HPP
#define ACTIONITEM_HPP

#include <QQuickItem>
#include <QFont>

class TopLevelItem;

class ActionItem : public QQuickItem {
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
	Q_PROPERTY(QUrl icon READ icon WRITE setIcon NOTIFY iconChanged)
	Q_PROPERTY(qreal iconSize READ iconSize WRITE setIconSize NOTIFY iconSizeChanged)
	Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
	Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
	Q_PROPERTY(TopLevelItem *menu READ menu WRITE setMenu NOTIFY menuChanged)
public:
	ActionItem(QQuickItem *parent = nullptr);
	QString text() const { return m_text; }
	QUrl icon() const { return m_icon; }
	QQuickItem *textItem() const { return m_textItem; }
	QQuickItem *imageItem() const { return m_imageItem; }
	qreal iconSize() const { return m_iconSize; }
	QFont font() const { return m_font; }
	void setIconSize(qreal size);
	void setIcon(const QUrl &url);
	void setText(const QString &text);
	void setFont(const QFont &font);
	Qt::Orientation orientation() const { return m_orientation; }
	void setOrientation(Qt::Orientation o);
	TopLevelItem *menu() const { return m_menu; }
	void setMenu(TopLevelItem *menu);
signals:
	void menuChanged();
	void textChanged();
	void iconChanged();
	void iconSizeChanged();
	void fontChanged();
	void orientationChanged();
	void triggered();
private slots:
	void layout();
private:
	QQuickItem *createImageItem();
	void updateOrientation();
	void updatePolish();
	void showMenu();
	void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) {
		QQuickItem::geometryChanged(newGeometry, oldGeometry); layout();
	}
	QFont m_font;
	QString m_text;
	QUrl m_icon;
	QQuickItem *m_textItem = nullptr, *m_imageItem = nullptr;
	qreal m_iconSize = -1;
	QMetaMethod m_layout;
	qreal m_textHeight = -1;
	Qt::Orientation m_orientation = Qt::Vertical;
	TopLevelItem *m_menu = nullptr;
};

#endif // ACTIONLISTITEM_HPP
