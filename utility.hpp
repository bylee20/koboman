#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <QObject>
#include <QFont>

class QQuickItem;
class QQuickView;

class Utility : public QObject {
	Q_OBJECT
	Q_PROPERTY(qreal dpi READ dpi CONSTANT FINAL)
	Q_PROPERTY(QQuickItem *root READ root CONSTANT FINAL)
	Q_PROPERTY(qreal dp1 READ dp1 CONSTANT FINAL)
	Q_PROPERTY(qreal dp2 READ dp2 CONSTANT FINAL)
	Q_PROPERTY(qreal dp3 READ dp3 CONSTANT FINAL)
	Q_PROPERTY(qreal dp4 READ dp4 CONSTANT FINAL)
	Q_PROPERTY(qreal dp5 READ dp5 CONSTANT FINAL)
	Q_PROPERTY(qreal dp6 READ dp6 CONSTANT FINAL)
	Q_PROPERTY(qreal dp7 READ dp7 CONSTANT FINAL)
	Q_PROPERTY(qreal dp8 READ dp8 CONSTANT FINAL)
	Q_PROPERTY(qreal dp9 READ dp9 CONSTANT FINAL)
	Q_PROPERTY(qreal dp10 READ dp10 CONSTANT FINAL)
	Q_PROPERTY(qreal dp20 READ dp20 CONSTANT FINAL)
	Q_PROPERTY(qreal dp30 READ dp30 CONSTANT FINAL)
	Q_PROPERTY(qreal dp40 READ dp40 CONSTANT FINAL)
	Q_PROPERTY(qreal dp50 READ dp50 CONSTANT FINAL)
	Q_PROPERTY(qreal dp60 READ dp60 CONSTANT FINAL)
	Q_PROPERTY(qreal dp70 READ dp70 CONSTANT FINAL)
	Q_PROPERTY(qreal dp80 READ dp80 CONSTANT FINAL)
	Q_PROPERTY(qreal dp90 READ dp90 CONSTANT FINAL)
	Q_PROPERTY(qreal dp100 READ dp100 CONSTANT FINAL)
	Q_PROPERTY(qreal dp200 READ dp200 CONSTANT FINAL)
	Q_PROPERTY(qreal dp300 READ dp300 CONSTANT FINAL)
	Q_PROPERTY(qreal dp400 READ dp400 CONSTANT FINAL)
	Q_PROPERTY(qreal dp500 READ dp500 CONSTANT FINAL)
	Q_PROPERTY(qreal dp600 READ dp600 CONSTANT FINAL)
	Q_PROPERTY(qreal dp700 READ dp700 CONSTANT FINAL)
	Q_PROPERTY(qreal dp800 READ dp800 CONSTANT FINAL)
	Q_PROPERTY(qreal dp900 READ dp900 CONSTANT FINAL)
public:
	Utility(QObject *parent = nullptr);
	Q_INVOKABLE static qreal pxToDp(qreal px) { return px*m_p2d; }
	Q_INVOKABLE static qreal dpToPx(qreal dp) { return dp*m_d2p; }
	Q_INVOKABLE static QString storage() { return m_storage; }
	qreal dpi() const { return m_dpi; }
	static QQuickItem *createItem(const QByteArray &name, QQuickItem *parent = nullptr);
	static QQuickItem *root() { return m_root; }
	static void initialize(QQuickView *window);
#define DP(i) static qreal dp##i() { return dpToPx(i); }
	DP(  1)DP(  2)DP(  3)DP(  4)DP(  5)DP(  6)DP(  7)DP(  8)DP(  9)
	DP( 10)DP( 20)DP( 30)DP( 40)DP( 50)DP( 60)DP( 70)DP( 80)DP( 90)
	DP(100)DP(200)DP(300)DP(400)DP(500)DP(600)DP(700)DP(800)DP(900)
#undef DP
private:
	static qreal m_p2d, m_d2p, m_dpi;
	static QString m_storage;
	static QQuickItem *m_root;
};

#endif // UTILITY_HPP
