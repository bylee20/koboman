#ifndef ITEMLISTITEM_HPP
#define ITEMLISTITEM_HPP

#include "textureitem.hpp"
#include "utils.hpp"
#include "utility.hpp"
#include "themeapi.hpp"
#include <QQmlEngine>
#include <QQmlContext>

class ItemListSeparator;
class ItemListAttached;

class ItemListItem : public TextureItem {
	Q_OBJECT
	Q_ENUMS(SpecialItem)
	Q_ENUMS(SelectionMode)
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
	Q_PROPERTY(SelectionMode selectionMode READ selectionMode WRITE setSelectionMode NOTIFY selectionModeChanged)
	Q_PROPERTY(bool interactive READ isInteractive WRITE setInteractive NOTIFY interactiveChanged)
	Q_PROPERTY(QQmlListProperty<QQuickItem> selections READ selections NOTIFY selectionsChanged)
public:
	enum SelectionMode { NoSelection, SingleSelection, MultiSelection };
	enum SpecialItem { HeaderItem = -2, FooterItem = -3 };
	ItemListItem(QQuickItem *parent = nullptr);
	~ItemListItem();
	QQmlListProperty<QObject> list() const;
	QQmlListProperty<QObject> readOnlyList() const;
	QQmlListProperty<QQuickItem> selections() const;
	void append(QObject *item);
	void clear();
	ItemListSeparator *separator() const;
	static ItemListAttached *qmlAttachedProperties(QObject *parent);
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
	void setFixedItemLength(qreal width);
	qreal fixedItemLength() const;
	SelectionMode selectionMode() const;
	void setSelectionMode(SelectionMode mode);
	bool isInteractive() const;
	void setInteractive(bool interactive);
	Q_INVOKABLE int indexOf(QQuickItem *item) const;
	Q_INVOKABLE QQuickItem *itemAt(int index) const;
	void clearSelections();
public slots:
	void polishAndUpdate() { polish(); update(); }
signals:
	void selectionsChanged();
	void interactiveChanged();
	void selectionModeChanged();
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
private:
	const QList<ItemListAttached*> &attachedList() const;
	static ItemListAttached *attached(QQuickItem *item, bool create);
	void forceToUpdateGeometry();
	void handleEnabledChanged();
	void handleSelectionChanged();
	void updatePolish();
	void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
	QByteArray fragmentShader() const;
	QByteArray vertexShader() const;
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

#endif // ITEMLISTITEM_HPP
