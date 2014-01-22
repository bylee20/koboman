#include "utility.hpp"
#include <QDir>
#include <QQmlEngine>
#include <QGuiApplication>
#include <QQuickView>
#include <QQuickItem>
#include <QScreen>
#include <QFontDatabase>

qreal Utility::m_p2d = 1.0;
qreal Utility::m_d2p = 1.0;
qreal Utility::m_dpi = 160;
QQuickItem *Utility::m_root = nullptr;
QString Utility::m_storage;

static QMap<QByteArray, QSharedPointer<QQmlComponent>> components;
static QQuickView *view = nullptr;
static QQmlEngine *m_engine = nullptr;

Utility::Utility(QObject *parent)
: QObject(parent) {
	Q_ASSERT(view != nullptr);
	view->installEventFilter(this);
}

bool Utility::eventFilter(QObject *object, QEvent *ev) {
	if (object != view || m_sending)
		return false;
	switch (ev->type()) {
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
		m_sending = true;
		qApp->sendEvent(object, ev);
		m_sending = false;
		if (!ev->isAccepted())
			view->sendEvent(view->rootObject(), ev);
		return true;
	default:
		return false;
	}
}

void Utility::initialize(QQuickView *window) {
	view = window;
	m_root = window->contentItem();
	connect(window, &QWindow::widthChanged, m_root, &QQuickItem::setWidth);
	connect(window, &QWindow::heightChanged, m_root, &QQuickItem::setHeight);
	m_engine = window->engine();

#ifndef Q_OS_WIN
	m_dpi = window->screen()->physicalDotsPerInch();
#endif
	m_d2p  = m_dpi/160.0;
	m_p2d  = 1.0/m_d2p;

#ifdef Q_OS_ANDROID
	m_storage = QString::fromLocal8Bit(qgetenv("EXTERNAL_STORAGE")) + "/koboman";
#else
	m_storage = QDir::homePath() + "/koboman";
#endif
	QDir().mkpath(m_storage);
}

QQuickItem *Utility::createItem(const QByteArray &name, QQuickItem *parent) {
	Q_ASSERT(m_engine != nullptr);
	auto &comp = components[name];
	if (comp.isNull()) {
		comp.reset(new QQmlComponent(m_engine));
		comp->setData("import QtQuick 2.2\n" + name + " {}\n", QUrl());
	}
	auto item = static_cast<QQuickItem*>(comp->create());
	item->setParent(parent);
	item->setParentItem(parent);
	return item;
}

QStringList Utility::files(const QStringList &nameFilter, const QString &folder) {
	QDir dir(folder.isEmpty() ? m_storage : folder);
	if (!dir.exists())
		return QStringList();
	return dir.entryList(nameFilter, QDir::Files, QDir::Name);
}

#include "items/themeapi.hpp"
#include "items/toplevelitem.hpp"
#include "items/toplevelcontainer.hpp"
#include "items/itemlistitem.hpp"
#include "items/itemlistattached.hpp"
#include "items/textlistitem.hpp"
#include "items/buttonboxitem.hpp"
#include "items/actionitem.hpp"
#include "items/busyitem.hpp"

QML_DECLARE_TYPEINFO(ItemListItem, QML_HAS_ATTACHED_PROPERTIES)

void Utility::registerTypes() {
	qmlRegisterType<ItemListAttached>();
	qmlRegisterType<TopLevelContainer>();
	qmlRegisterType<TopLevelShadow>();
	qmlRegisterType<BusyItem>("net.xylosper.Mobile", 1, 0, "BusyIndicator");
	qmlRegisterType<ActionItem>("net.xylosper.Mobile", 1, 0, "Action");
	qmlRegisterType<TopLevelItem>("net.xylosper.Mobile", 1, 0, "TopLevel");
	qmlRegisterType<ItemListItem>("net.xylosper.Mobile", 1, 0, "ItemList");
	qmlRegisterType<TextListItem>("net.xylosper.Mobile", 1, 0, "TextList");
	qmlRegisterType<ButtonBoxItem>("net.xylosper.Mobile", 1, 0, "ButtonBox");
	qmlRegisterType<ItemListSeparator>("net.xylosper.Mobile", 1, 0, "ItemListSeparator");
	qmlRegisterSingletonType<Theme>("net.xylosper.Mobile", 1, 0, "Theme", singletonProvider<Theme>);
	qmlRegisterSingletonType<Utility>("net.xylosper.Mobile", 1, 0, "Utility", singletonProvider<Utility>);
}
