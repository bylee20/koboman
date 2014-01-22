#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlContext>
#include "qtquick2applicationviewer.h"
#include "barcodescanner.hpp"
#include "booklistmodel.hpp"
#include "library.hpp"
#include "items/utility.hpp"
#include "items/themeapi.hpp"

int main(int argc, char *argv[]) {
	QGuiApplication app(argc, argv);

	Utility::registerTypes();
	qmlRegisterType<BarcodeObject>("KoboMan", 1, 0, "Barcode");
	qmlRegisterType<BarcodeScanner>("KoboMan", 1, 0, "BarcodeScanner");

	QtQuick2ApplicationViewer viewer;
#if defined(Q_OS_ANDROID) && !defined(Q_OS_ANDROID_NO_SDK)
	viewer.addImportPath(QStringLiteral("assets:/imports"));
#else
	viewer.addImportPath(QStringLiteral("imports"));
#endif
	qDebug() << viewer.engine()->importPathList();
	Utility::initialize(&viewer);
	Theme::initialize(Theme::androidHoloLight());
	BookListModel bookList;
	viewer.rootContext()->setContextProperty("BookList", &bookList);
	viewer.rootContext()->setContextProperty("Library", &Library::get());
	viewer.setMainQmlFile(QStringLiteral("qml/koboman/main.qml"));
	viewer.showExpanded();
	return app.exec();
}
