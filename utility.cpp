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
QFont Utility::m_font;
QQuickItem *Utility::m_root = nullptr;
QString Utility::m_storage;

Utility::Utility(QObject *parent)
: QObject(parent) { }

void Utility::initialize(QQuickWindow *window) {
	m_root = window->contentItem();

	m_dpi = window->screen()->physicalDotsPerInch();
	m_d2p  = m_dpi/160.0;
	m_p2d  = 1.0/m_d2p;

	m_font = qApp->font();
	m_font.setPixelSize(qRound(dpToPx(19)));
#ifdef Q_OS_ANDROID
	const auto families = QFontDatabase().families(QFontDatabase::Korean);
	if (!families.isEmpty())
		m_font.setFamily(families.last());
#endif
	qApp->setFont(m_font);

#ifdef Q_OS_ANDROID
	m_storage = QString::fromLocal8Bit(qgetenv("EXTERNAL_STORAGE")) + "/koboman";
#else
	m_storage = QDir::homePath() + "/koboman";
#endif
	QDir().mkpath(m_storage);
}
