#include "utility.hpp"
#include <QDir>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QQuickItem>
#include <QScreen>
#include <QFontDatabase>

qreal Utility::m_p2d = 1.0;
qreal Utility::m_d2p = 1.0;
qreal Utility::m_dpi = 160;
QQuickItem *Utility::m_root = nullptr;
QString Utility::m_storage;

Utility::Utility(QObject *parent)
: QObject(parent) { }

void Utility::initialize(QQuickWindow *window) {
	m_root = window->contentItem();
	connect(window, &QQuickWindow::widthChanged, m_root, &QQuickItem::setWidth);
	connect(window, &QQuickWindow::heightChanged, m_root, &QQuickItem::setHeight);

	m_dpi = window->screen()->physicalDotsPerInch();
	m_d2p  = m_dpi/160.0;
	m_p2d  = 1.0/m_d2p;

#ifdef Q_OS_ANDROID
	m_storage = QString::fromLocal8Bit(qgetenv("EXTERNAL_STORAGE")) + "/koboman";
#else
	m_storage = QDir::homePath() + "/koboman";
#endif
	QDir().mkpath(m_storage);
}
