#include "actionitem.hpp"
#include "themeapi.hpp"
#include "utils.hpp"
#include "utility.hpp"
#include "toplevelitem.hpp"
#include "toplevelcontainer.hpp"
#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlComponent>

ActionItem::ActionItem(QQuickItem *parent)
: QQuickItem(parent) {
	m_iconSize = Theme::iconSize();
	m_textHeight = Theme::lineHeight() - 2*Theme::padding();
	layout();
	connect(this, &ActionItem::triggered, this, &ActionItem::showMenu);
}

void ActionItem::setMenu(TopLevelItem *menu) {
	if (_Change(m_menu, menu)) {
		m_menu->container()->setAttach(this);
		emit menuChanged();
	}
}

void ActionItem::showMenu() {
	if (m_menu)
		m_menu->show();
}

void ActionItem::setOrientation(Qt::Orientation o) {
	if (_Change(m_orientation, o)) {
		layout();
		emit orientationChanged();
	}
}

void ActionItem::layout() {
	if (m_orientation == Qt::Horizontal) {
		qreal thickness = Theme::padding()*2;
		qreal imw = Theme::padding()*2;
		if (m_imageItem)
			thickness += m_iconSize;
		if (m_textItem) {
			m_font.setPixelSize(Theme::textSizeMicro());
			thickness += m_font.pixelSize();
			m_textItem->setProperty("horizontalAlignment", Qt::AlignHCenter);
			m_textItem->setProperty("verticalAlignment", Qt::AlignTop);
			m_textItem->setProperty("font", m_font);
		}
		if (m_imageItem && m_textItem) {
			thickness += Theme::spacing();
			imw += qMax(m_textItem->implicitWidth(), m_iconSize);
		} else if (m_imageItem)
			imw += m_iconSize;
		else if (m_textItem)
			imw += m_textItem->implicitWidth();
		setImplicitWidth(imw);
		setImplicitHeight(thickness);
	} else {
		if (m_textItem) {
			m_textItem->setProperty("horizontalAlignment", Qt::AlignLeft);
			m_textItem->setProperty("verticalAlignment", Qt::AlignVCenter);
		}
		setImplicitHeight(Theme::lineHeight());
	}
	polish();
}

void ActionItem::setFont(const QFont &font) {
	if (_Change(m_font, font)) {
		if (m_textItem) {
			m_textItem->setProperty("text", m_font);
			layout();
		}
		emit fontChanged();
	}
}

void ActionItem::setText(const QString &text) {
	if (_Change(m_text, text)) {
		if (m_text.isEmpty())
			_Delete(m_textItem);
		else {
			if (!m_textItem) {
				m_textItem = Utility::createItem("Text", this);
				m_textItem->setProperty("verticalAlignment", Qt::AlignVCenter);
				m_textItem->setHeight(Theme::lineHeight());
				m_textItem->setProperty("font", m_font);
			}
			m_textItem->setProperty("text", m_text);
		}
		layout();
		emit textChanged();
	}
}

void ActionItem::setIcon(const QUrl &url) {
	if (_Change(m_icon, url)) {
		if (m_icon.isEmpty()) {
			_Delete(m_imageItem);
		} else {
			if (!m_imageItem) {
				m_imageItem = Utility::createItem("Image", this);
				m_imageItem->setWidth(m_iconSize);
				m_imageItem->setHeight(m_iconSize);
			}
			m_imageItem->setProperty("source", m_icon);
		}
		layout();
		emit iconChanged();
	}
}

void ActionItem::setIconSize(qreal size) {
	if (_Change(m_iconSize, size)) {
		if (m_imageItem) {
			m_imageItem->setWidth(size);
			m_imageItem->setHeight(size);
			layout();
		}
		emit iconSizeChanged();
	}
}

void ActionItem::updatePolish() {
	if (m_orientation == Qt::Horizontal) {
		qreal y = Theme::padding();
		if (m_imageItem) {
			const qreal x = (width() - m_imageItem->width())*0.5;
			m_imageItem->setPosition({x, y});
			y += m_imageItem->height() + Theme::spacing();
		}
		if (m_textItem) {
			const qreal x = (width() - m_textItem->width())*0.5;
			m_textItem->setPosition({x, y});
		}
	} else {
		qreal x = Theme::padding();
		if (m_imageItem) {
			const qreal gap = (height() - m_imageItem->height())*0.5;
			m_imageItem->setPosition({gap, gap});
			x = gap + m_imageItem->width() + Theme::spacing();
		}
		if (m_textItem)
			m_textItem->setX(x);
	}
}
