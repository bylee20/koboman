#ifndef ITEMLISTATTACHED_HPP
#define ITEMLISTATTACHED_HPP

#include <QQuickItem>
#include "utils.hpp"

class ItemListAttached : public QObject {
	Q_OBJECT
	Q_PROPERTY(qreal thickness READ thickness WRITE setThickness NOTIFY thicknessChanged FINAL)
	Q_PROPERTY(bool isSeparator READ isSeparator CONSTANT FINAL)
	Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
	Q_PROPERTY(qreal verticalPadding READ verticalPadding WRITE setVerticalPadding NOTIFY verticalPaddingChanged)
	Q_PROPERTY(qreal horizontalPadding READ horizontalPadding WRITE setHorizontalPadding NOTIFY horizontalPaddingChanged)
	Q_PROPERTY(int index READ index CONSTANT FINAL)
	Q_PROPERTY(Qt::Orientation orientation READ orientation NOTIFY orientationChanged)
	Q_PROPERTY(bool selected READ isSelected WRITE setSelected NOTIFY selectedChanged)
public:
	ItemListAttached(QObject *parent = nullptr);
	ItemListAttached(bool separator, QObject *parent = nullptr);
	QObject *attachee() const { return m_attachee; }
	QQuickItem *asItem() const { Q_ASSERT(isQmlItem()); return static_cast<QQuickItem*>(m_attachee); }
	qreal thickness() const { return m_thickness; }
	bool isSeparator() const { return m_separator; }
	bool isQmlItem() const { return !m_separator; }
	bool canInteract() const { return !m_separator && asItem()->isEnabled(); }
	void setThickness(qreal h) { if (_Change(m_thickness, h)) emit thicknessChanged(); }
	void setColor(const QColor &color) { if (_Change(m_color, color)) emit colorChanged(); }
	QColor color() const { return m_color; }
	qreal filling() const { return m_filling; }
	void fill(qreal h) { m_filling = h; }
	qreal verticalPadding() const { return m_vpad; }
	qreal horizontalPadding() const { return m_hpad; }
	void setVerticalPadding(qreal pad) { if (_Change(m_vpad, pad)) emit verticalPaddingChanged(); }
	void setHorizontalPadding(qreal pad) { if (_Change(m_hpad, pad)) emit horizontalPaddingChanged(); }
	void setIndex(int index) { m_index = index; }
	int index() const { return m_index; }
	bool needsVertex() const { return m_filling > 0.0 && (m_selected || m_color.alpha()); }
	void setOrientation(Qt::Orientation o) { if (_Change(m_orientation, o)) emit orientationChanged(); }
	Qt::Orientation orientation() const { return m_orientation; }
	bool isSelected() const { return m_selected; }
	void setSelected(bool s) { if (_Change(m_selected, s)) emit selectedChanged(); }
	bool &selection() { return m_selected; }
signals:
	void selectedChanged();
	void thicknessChanged();
	void separatorChanged();
	void colorChanged();
	void verticalPaddingChanged();
	void horizontalPaddingChanged();
	void clicked();
	void interactiveChanged();
	void orientationChanged();
private:
	qreal m_thickness = -1, m_filling = -1, m_vpad = -1, m_hpad = -1;
	bool m_separator = false;
	QColor m_color = Qt::transparent;
	bool m_selected = false;
	int m_index = -1;
	QObject *m_attachee = nullptr;
	Qt::Orientation m_orientation = Qt::Vertical;
};

class ItemListSeparator : public ItemListAttached {
	Q_OBJECT
public:
	ItemListSeparator(QObject *parent = nullptr): ItemListAttached(true, parent) { }
};

#endif // ITEMLISTATTACHED_HPP
