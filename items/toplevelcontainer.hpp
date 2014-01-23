#ifndef TOPLEVELCONTAINER_HPP
#define TOPLEVELCONTAINER_HPP

#include "toplevelitem.hpp"
#include "utils.hpp"
#include "utility.hpp"

struct ContainerImage {
	static const ContainerImage &retain();
	static void release();
	int outer() const { return m_outer; }
	int inner() const { return m_inner; }
	QSize size() const { return m_size; }
	int width() const { return m_size.width(); }
	int height() const { return m_size.height(); }
	const QByteArray &box() const { return m_box; }
	const QByteArray &shadow() const { return m_shadow; }
private:
	ContainerImage() {}
	void setup();
	static ContainerImage *m_object;
	static int m_ref;
	int m_outer = 0, m_inner = 0;
	QSize m_size;
	QByteArray m_box, m_shadow;
};

class TopLevelShadow : public QObject {
	Q_OBJECT
	Q_PROPERTY(QColor color MEMBER m_color NOTIFY colorChanged FINAL)
	Q_PROPERTY(bool visible MEMBER m_visible NOTIFY visibleChanged FINAL)
	Q_PROPERTY(QPointF offset MEMBER m_offset NOTIFY offsetChanged FINAL)
public:
	TopLevelShadow(QObject *parent = nullptr): QObject(parent) { }
	QColor m_color{0, 0, 0, 255};
	bool m_visible = true;
	QPointF m_offset{0, Utility::dpToPx(1)};
signals:
	void colorChanged();
	void visibleChanged();
	void offsetChanged();
};

class TopLevelContainer : public QObject {
	Q_OBJECT
	Q_PROPERTY(QPointF position MEMBER m_position NOTIFY positionChanged FINAL)
	Q_PROPERTY(QSizeF size READ size NOTIFY sizeChanged FINAL)
	Q_PROPERTY(QColor color MEMBER m_color NOTIFY colorChanged FINAL)
	Q_PROPERTY(QQuickItem *item READ item WRITE setItem NOTIFY itemChanged FINAL)
	Q_PROPERTY(QQuickItem *attach READ attach WRITE setAttach NOTIFY attachChanged FINAL)
	Q_PROPERTY(qreal padding MEMBER m_padding NOTIFY paddingChanged FINAL)
	Q_PROPERTY(TopLevelShadow shadow READ shadow CONSTANT FINAL)
public:
	TopLevelContainer(TopLevelItem *item);
	~TopLevelContainer();
	QPointF position() const { return m_position; }
	void setPosition(const QPointF &p) { if (_Change(m_position, p)) emit positionChanged(); }
	QQuickItem *item() const { return m_item; }
	void setItem(QQuickItem *item);
	QQuickItem *attach() const { return m_attach; }
	void setAttach(QQuickItem *attach);
	QRectF rect() const { return QRectF(m_pos, m_size); }
	TopLevelShadow *shadow() const { return m_shadow; }
	void complete();
	QSizeF size() const { return m_size; }
	QColor color() const { return m_color; }
	bool isEmpty() const { return m_size.isEmpty(); }
public slots:
	void reattach();
signals:
	void rectChanged();
	void positionChanged();
	void colorChanged();
	void itemChanged();
	void attachChanged();
	void paddingChanged();
	void shadowChanged();
	void sizeChanged();
private slots:
	void reposition();
	void resize();
private:
	void updateTransformOrigin();
	void connectAttach();
	void connectItem();
	bool m_completed = false;
	TopLevelItem *m_top = nullptr;
	TopLevelShadow *m_shadow;
	QSizeF m_size;
	QPointF m_position, m_pos;
	QColor m_color{255, 255, 255, 255};
	QQuickItem *m_item = nullptr, *m_attach = nullptr;
	Connections m_attachConnections, m_itemConnections;
	qreal m_padding = 0.0;
};

#endif // TOPLEVELCONTAINER_HPP
