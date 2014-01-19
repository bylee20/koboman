#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <QObject>
#include <QFont>

class QQuickItem;
class QQuickWindow;

class Utility : public QObject {
	Q_OBJECT
	Q_PROPERTY(qreal dpi READ dpi CONSTANT FINAL)
	Q_PROPERTY(QQuickItem *root READ root CONSTANT FINAL)
	Q_PROPERTY(QFont font READ font CONSTANT FINAL)
public:
	Utility(QObject *parent = nullptr);
	Q_INVOKABLE static qreal pxToDp(qreal px) { return px*m_p2d; }
	Q_INVOKABLE static qreal dpToPx(qreal dp) { return dp*m_d2p; }
	Q_INVOKABLE static QString storage() { return m_storage; }
	qreal dpi() const { return m_dpi; }
	QQuickItem *root() const { return m_root; }
	static QFont font() { return m_font; }
	static void initialize(QQuickWindow *window);
private:
	static qreal m_p2d, m_d2p, m_dpi;
	static QFont m_font;
	static QString m_storage;
	static QQuickItem *m_root;
};

#endif // UTILITY_HPP
