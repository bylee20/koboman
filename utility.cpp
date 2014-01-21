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

static QQmlEngine *m_engine = nullptr;

Utility::Utility(QObject *parent)
: QObject(parent) { }

void Utility::initialize(QQuickView *window) {
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
