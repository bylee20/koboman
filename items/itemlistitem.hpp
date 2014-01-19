#ifndef ITEMLISTITEM_HPP
#define ITEMLISTITEM_HPP

#include "textureitem.hpp"
#include "utils.hpp"
#include "utility.hpp"

class ItemListAttached : public QObject {
	Q_OBJECT
	Q_PROPERTY(qreal thickness READ thickness WRITE setThickness NOTIFY thicknessChanged FINAL)
	Q_PROPERTY(bool isSeparator READ isSeparator CONSTANT FINAL)
	Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
	Q_PROPERTY(qreal verticalPadding READ verticalPadding WRITE setVerticalPadding NOTIFY verticalPaddingChanged)
	Q_PROPERTY(qreal horizontalPadding READ horizontalPadding WRITE setHorizontalPadding NOTIFY horizontalPaddingChanged)
public:
	ItemListAttached(QObject *parent = nullptr): QObject(parent) {}
	ItemListAttached(bool separator, QObject *parent = nullptr)
	: QObject(parent), m_separator(separator) {
		Q_ASSERT(m_separator);
		m_vpad = m_hpad = 0.0;
		m_thickness = Utility::dpToPx(2);
		m_color = Qt::gray;
	}
	qreal thickness() const { return m_thickness; }
	bool isSeparator() const { return m_separator; }
	bool isQmlItem() const { return !m_separator; }
	void setThickness(qreal h) { if (_Change(m_thickness, h)) emit thicknessChanged(); }
	void setColor(const QColor &color) { if (_Change(m_color, color)) emit colorChanged(); }
	QColor color() const { return m_color; }
	qreal filling() const { return m_filling; }
	void fill(qreal h) { m_filling = h; }
	qreal verticalPadding() const { return m_vpad; }
	qreal horizontalPadding() const { return m_hpad; }
	void setVerticalPadding(qreal pad) { if (_Change(m_vpad, pad)) emit verticalPaddingChanged(); }
	void setHorizontalPadding(qreal pad) { if (_Change(m_hpad, pad)) emit horizontalPaddingChanged(); }
signals:
	void thicknessChanged();
	void separatorChanged();
	void colorChanged();
	void verticalPaddingChanged();
	void horizontalPaddingChanged();
private:
	qreal m_thickness = -1, m_filling = -1, m_vpad = -1, m_hpad = -1;
	bool m_separator = false;
	QColor m_color = Qt::transparent;
};

class ItemListSeparator : public ItemListAttached {
	Q_OBJECT
public:
	ItemListSeparator(QObject *parent = nullptr): ItemListAttached(true, parent) {}
};

class ItemListItem : public TextureItem {
	Q_OBJECT
	Q_PROPERTY(QQmlListProperty<QObject> list READ list NOTIFY listChanged FINAL)
	Q_PROPERTY(ItemListSeparator *separator READ separator CONSTANT FINAL)
	Q_PROPERTY(qreal verticalPadding READ verticalPadding WRITE setVerticalPadding NOTIFY verticalPaddingChanged FINAL)
	Q_PROPERTY(qreal horizontalPadding READ horizontalPadding WRITE setHorizontalPadding NOTIFY horizontalPaddingChanged FINAL)
	Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged FINAL)
	Q_PROPERTY(QQuickItem *headerItem READ headerItem WRITE setHeaderItem NOTIFY headerItemChanged FINAL)
	Q_PROPERTY(QQuickItem *footerItem READ footerItem WRITE setFooterItem NOTIFY footerItemChanged FINAL)
	Q_PROPERTY(ItemListSeparator *headerSeparator READ headerSeparator WRITE setHeaderSeparator NOTIFY headerSeparatorChanged FINAL)
	Q_PROPERTY(ItemListSeparator *footerSeparator READ footerSeparator WRITE setFooterSeparator NOTIFY footerSeparatorChanged FINAL)
	Q_PROPERTY(qreal minimumLength READ minimumLength NOTIFY minimumLengthChanged FINAL)
public:
	ItemListItem(QQuickItem *parent = nullptr);
	~ItemListItem();
	QQmlListProperty<QObject> list() const;
	void append(QObject *item);
	void clear();
	ItemListSeparator *separator() const;
	static ItemListAttached *qmlAttachedProperties(QObject *parent) { return new ItemListAttached(parent); }
	qreal verticalPadding() const;
	qreal horizontalPadding() const;
	void setVerticalPadding(qreal pad);
	void setHorizontalPadding(qreal pad);
	Qt::Orientation orientation() const;
	void setOrientation(Qt::Orientation o);
	ItemListSeparator *headerSeparator() const;
	ItemListSeparator *footerSeparator() const;
	QQuickItem *headerItem() const;
	QQuickItem *footerItem() const;
	void setHeaderItem(QQuickItem *item);
	void setFooterItem(QQuickItem *item);
	void setHeaderSeparator(ItemListSeparator *sep);
	void setFooterSeparator(ItemListSeparator *sep);
	void polishAndUpdate() { polish(); update(); }
	qreal minimumLength() const;
signals:
	void minimumLengthChanged();
	void listChanged();
	void verticalPaddingChanged();
	void horizontalPaddingChanged();
	void orientationChanged();
	void headerItemChanged();
	void footerItemChanged();
	void headerSeparatorChanged();
	void footerSeparatorChanged();
private slots:
	void handleItemColorChanged();
private:
	void updatePolish();
	void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
	QByteArray fragmentShader() const;
	QByteArray vertexShader() const;
	ShaderId *shaderId() const { static ShaderId type; return &type; }
	const char *const *attributeNames() const;
	QSGGeometry *createSGGeometry();
	void updateSGGeometry(QSGGeometry *geometry);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseUngrabEvent();
	void touchEvent(QTouchEvent *event);
	void touchUngrabEvent();
	struct Data;
	Data *d;
};

QML_DECLARE_TYPEINFO(ItemListItem, QML_HAS_ATTACHED_PROPERTIES)

#endif // ITEMLISTITEM_HPP
