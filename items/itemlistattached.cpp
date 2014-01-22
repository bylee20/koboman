#include "itemlistattached.hpp"
#include "actionitem.hpp"
#include "themeapi.hpp"

ItemListAttached::ItemListAttached(QObject *parent)
: QObject(parent) {
	m_attachee = parent;
	if (auto action = qobject_cast<ActionItem*>(m_attachee))
		connect(this, &ItemListAttached::clicked, action, &ActionItem::triggered);
}

ItemListAttached::ItemListAttached(bool separator, QObject *parent)
: QObject(parent), m_separator(separator) {
	Q_ASSERT(m_separator);
	m_vpad = m_hpad = 0.0;
	m_thickness = Theme::separator();
	m_color = Theme::separatorColor();
	m_attachee = this;
}
