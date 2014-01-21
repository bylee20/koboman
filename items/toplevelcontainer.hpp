#ifndef TOPLEVELCONTAINER_HPP
#define TOPLEVELCONTAINER_HPP

#include "toplevelitem.hpp"
#include "utils.hpp"
#include "utility.hpp"
#include <QImage>

class TopLevelShadow : public QObject {
	Q_OBJECT
	Q_PROPERTY(QColor color MEMBER m_color NOTIFY colorChanged FINAL)
	Q_PROPERTY(bool visible MEMBER m_visible NOTIFY visibleChanged FINAL)
	Q_PROPERTY(QPointF offset MEMBER m_offset NOTIFY offsetChanged FINAL)
	Q_PROPERTY(qreal radius MEMBER m_radius NOTIFY radiusChanged FINAL)
public:
	TopLevelShadow(QObject *parent = nullptr): QObject(parent) { }
	QColor m_color{0, 0, 0, 255};
	bool m_visible = true;
	QPointF m_offset{0, Utility::dpToPx(1)};
	qreal m_radius = Utility::dpToPx(7);
signals:
	void colorChanged();
	void visibleChanged();
	void offsetChanged();
	void radiusChanged();
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
	Q_PROPERTY(qreal radius MEMBER m_radius NOTIFY radiusChanged FINAL)
public:
	TopLevelContainer(TopLevelItem *item);
	~TopLevelContainer();
	QPointF position() const { return m_position; }
	void setPosition(const QPointF &pos) {
		if (_Change(m_position, pos))
			emit positionChanged();
	}
	QQuickItem *item() const { return m_item; }
	void setItem(QQuickItem *item);
	QQuickItem *attach() const { return m_attach; }
	void setAttach(QQuickItem *attach);
	QRectF paintingArea() const;
	QRectF rect() const { return QRectF(m_pos, m_size); }
	const QImage &image() const { return m_image; }
	TopLevelShadow *shadow() const { return m_shadow; }
	void complete();
	QSizeF size() const { return m_size; }
public slots:
	void repaint();
	void reattach();
signals:
	void paintingAreaChanged();
	void repainted();
	void positionChanged();
	void colorChanged();
	void itemChanged();
	void attachChanged();
	void paddingChanged();
	void shadowChanged();
	void sizeChanged();
	void radiusChanged();
private slots:
	void reposition();
	void resize();
private:
	void connectAttach();
	void connectItem();
	bool m_completed = false;
	TopLevelItem *m_top = nullptr;
	TopLevelShadow *m_shadow;
	QSizeF m_size;
	QPointF m_position{0.0, 0.0}, m_pos{0.0, 0.0}, m_dxy{0.0, 0.0}, m_shadowOffset{0.0, 0.0};
	QColor m_color{255, 255, 255, 255};
	QQuickItem *m_item = nullptr, *m_attach = nullptr;
	Connections m_attachConnections, m_itemConnections;
	QImage m_image;
	qreal m_padding = 0.0, m_radius = Utility::dpToPx(2);
	QByteArray m_imageData;
};

#endif // TOPLEVELCONTAINER_HPP
