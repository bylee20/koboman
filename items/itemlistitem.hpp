#ifndef ITEMLISTITEM_HPP
#define ITEMLISTITEM_HPP

#include "textureitem.hpp"
#include "utils.hpp"
#include "utility.hpp"
#include "themeapi.hpp"
#include <QQmlEngine>
#include <QQmlContext>

class ItemListAttached : public QObject {
	Q_OBJECT
	Q_PROPERTY(qreal thickness READ thickness WRITE setThickness NOTIFY thicknessChanged FINAL)
	Q_PROPERTY(bool isSeparator READ isSeparator CONSTANT FINAL)
	Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
	Q_PROPERTY(qreal verticalPadding READ verticalPadding WRITE setVerticalPadding NOTIFY verticalPaddingChanged)
	Q_PROPERTY(qreal horizontalPadding READ horizontalPadding WRITE setHorizontalPadding NOTIFY horizontalPaddingChanged)
	Q_PROPERTY(bool interactive READ isInteractive WRITE setInteractive NOTIFY interactiveChanged)
	Q_PROPERTY(int index READ index CONSTANT FINAL)
	Q_PROPERTY(Qt::Orientation orientation READ orientation NOTIFY orientationChanged)
public:
	ItemListAttached(QObject *parent = nullptr);
	ItemListAttached(bool separator, QObject *parent = nullptr);
	QObject *attachee() const { return m_attachee; }
	QQuickItem *asItem() const { Q_ASSERT(isQmlItem()); return static_cast<QQuickItem*>(m_attachee); }
	qreal thickness() const { return m_thickness; }
	bool isSeparator() const { return m_separator; }
	bool isQmlItem() const { return !m_separator; }
	bool canInteract() const { return m_interactive && !m_separator && asItem()->isEnabled(); }
	void setThickness(qreal h) { if (_Change(m_thickness, h)) emit thicknessChanged(); }
	void setColor(const QColor &color) { if (_Change(m_color, color)) emit colorChanged(); }
	QColor color() const { return m_color; }
	qreal filling() const { return m_filling; }
	void fill(qreal h) { m_filling = h; }
	qreal verticalPadding() const { return m_vpad; }
	qreal horizontalPadding() const { return m_hpad; }
	void setVerticalPadding(qreal pad) { if (_Change(m_vpad, pad)) emit verticalPaddingChanged(); }
	void setHorizontalPadding(qreal pad) { if (_Change(m_hpad, pad)) emit horizontalPaddingChanged(); }
	bool isInteractive() const { return m_interactive; }
	void setInteractive(bool on) { if (_Change(m_interactive, on)) emit interactiveChanged(); }
	void setIndex(int index) { m_index = index; }
	int index() const { return m_index; }
	bool needsVertex() const { return m_color.alpha() && m_filling > 0.0; }
	void setOrientation(Qt::Orientation o) { if (_Change(m_orientation, o)) emit orientationChanged(); }
	Qt::Orientation orientation() const { return m_orientation; }
signals:
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
	bool m_interactive = false;
	int m_index = -1;
	QObject *m_attachee = nullptr;
	Qt::Orientation m_orientation = Qt::Vertical;
};

class ItemListSeparator : public ItemListAttached {
	Q_OBJECT
	Q_PROPERTY(bool interactive READ isInteractive CONSTANT)
public:
	ItemListSeparator(QObject *parent = nullptr): ItemListAttached(true, parent) { }
};

class ItemListItem : public TextureItem {
	Q_OBJECT
	Q_ENUMS(SpecialItem)
	Q_PROPERTY(QQmlListProperty<QObject> items READ list NOTIFY listChanged)
	Q_PROPERTY(ItemListSeparator *separator READ separator CONSTANT FINAL)
	Q_PROPERTY(qreal verticalPadding READ verticalPadding WRITE setVerticalPadding NOTIFY verticalPaddingChanged FINAL)
	Q_PROPERTY(qreal horizontalPadding READ horizontalPadding WRITE setHorizontalPadding NOTIFY horizontalPaddingChanged FINAL)
	Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
	Q_PROPERTY(QQuickItem *headerItem READ headerItem WRITE setHeaderItem NOTIFY headerItemChanged)
	Q_PROPERTY(QQuickItem *footerItem READ footerItem WRITE setFooterItem NOTIFY footerItemChanged)
	Q_PROPERTY(ItemListSeparator *headerSeparator READ headerSeparator WRITE setHeaderSeparator NOTIFY headerSeparatorChanged)
	Q_PROPERTY(ItemListSeparator *footerSeparator READ footerSeparator WRITE setFooterSeparator NOTIFY footerSeparatorChanged)
	Q_PROPERTY(qreal minimumLength READ minimumLength NOTIFY minimumLengthChanged FINAL)
	Q_PROPERTY(QColor highlight READ highlight WRITE setHighlight NOTIFY highlightChanged FINAL)
	Q_PROPERTY(qreal fixedItemLength READ fixedItemLength WRITE setFixedItemLength NOTIFY fixedItemLengthChanged)
public:
	enum SpecialItem { HeaderItem = -2, FooterItem = -3 };
	ItemListItem(QQuickItem *parent = nullptr);
	~ItemListItem();
	QQmlListProperty<QObject> list() const;
	QQmlListProperty<QObject> readOnlyList() const;
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
	qreal minimumLength() const;
	QColor highlight() const;
	void setHighlight(const QColor &color);
	static ItemListAttached *attached(QQuickItem *item, bool create) {
		return static_cast<ItemListAttached*>(qmlAttachedPropertiesObject<ItemListItem>(item, create));
	}
	void setFixedItemLength(qreal width);
	qreal fixedItemLength() const;
public slots:
	void polishAndUpdate() { polish(); update(); }
signals:
	void fixedItemLengthChanged();
	void highlightChanged();
	void clicked(QQuickItem *item);
	void minimumLengthChanged();
	void listChanged();
	void verticalPaddingChanged();
	void horizontalPaddingChanged();
	void orientationChanged(Qt::Orientation orientation);
	void headerItemChanged();
	void footerItemChanged();
	void headerSeparatorChanged();
	void footerSeparatorChanged();
protected:
	const QList<ItemListAttached*> &attachedList() const;
private:
	void handleItemColorChanged();
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
