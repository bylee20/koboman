#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlContext>
#include "qtquick2applicationviewer.h"
#include "barcodescanner.hpp"
#include "booklistmodel.hpp"
#include "library.hpp"
#include "utility.hpp"
#include "items/toplevelitem.hpp"
#include "items/toplevelcontainer.hpp"
#include "items/itemlistitem.hpp"
#include "items/textlistitem.hpp"
#include "items/buttonboxitem.hpp"

template<typename T>
static QObject *singletonProvider(QQmlEngine*, QJSEngine*) { return new T; }

int main(int argc, char *argv[]) {
	QGuiApplication app(argc, argv);

	qmlRegisterType<ItemListAttached>();
	qmlRegisterType<TopLevelItem>("net.xylosper.Mobile", 1, 0, "TopLevel");
	qmlRegisterType<ItemListItem>("net.xylosper.Mobile", 1, 0, "ItemList");
	qmlRegisterType<TextListItem>("net.xylosper.Mobile", 1, 0, "TextList");
	qmlRegisterType<ButtonBoxItem>("net.xylosper.Mobile", 1, 0, "ButtonBox");
	qmlRegisterType<ItemListSeparator>("net.xylosper.Mobile", 1, 0, "ItemListSeparator");
	qmlRegisterUncreatableType<TopLevelContainer>("net.xylosper.Mobile", 1, 0, "TopLevelContainer", "wrong");
	qmlRegisterUncreatableType<TopLevelShadow>("net.xylosper.Mobile", 1, 0, "TopLevelContainerShadow", "wrong");
	qmlRegisterSingletonType<Utility>("KoboMan", 1, 0, "Utility", singletonProvider<Utility>);
	qmlRegisterSingletonType<Utility>("net.xylosper.Mobile", 1, 0, "Utility", singletonProvider<Utility>);

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
	BookListModel bookList;
	viewer.rootContext()->setContextProperty("BookList", &bookList);
	viewer.rootContext()->setContextProperty("Library", &Library::get());
	viewer.setMainQmlFile(QStringLiteral("qml/koboman/main.qml"));
	viewer.showExpanded();
	return app.exec();
}
