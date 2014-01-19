//#include "mainwindow.hpp"
//#include "ui_mainwindow.h"
//#include "scannerwidget.hpp"
//#include <QCamera>
//#include <QVideoProbe>
//#include <QCameraViewfinder>
//#include <QNetworkAccessManager>
//#include <QNetworkRequest>
//#include <QNetworkReply>
//#include <QXmlStreamReader>

//struct Book {
//	QString title, edition, cartographic, publish, form, series, note, standard, classfy, subject, cover, isbn, price;
//	bool contents, abstracts, original, copyright;
//	int key;
//	struct HoldInfo {
//		QString library, call, start;
//		bool original;
//	};
//	QList<HoldInfo> holds;
//};

//struct MainWindow::Data {
//	Ui::MainWindow ui;
//	ScannerDialog *scanner = nullptr;
//	QNetworkAccessManager network;
//	void scan() {
//		if (scanner->exec()) {
//			auto barcodes = scanner->barcodes();
//			for (auto barcode : barcodes) {
//				if (barcode.type == Barcode::ISBN_13) {
//					ui.isbn->setText(barcode.data);
//					search();
//					break;
//				}
//			}
//		}
//	}

//	void getDetail(const Book &book) {
//		auto reply = network.get(QNetworkRequest(QUrl("http://nl.go.kr/kolisnet/openApi/open.php?rec_key=" + QString::number(book.key))));
//		connect(reply, &QNetworkReply::finished, [this, reply] () {
//			qDebug() <<reply->readAll();
//		});
//	}

//	void search() {
////		QUrl::per
//		auto reply = network.get(QNetworkRequest(QUrl("http://nl.go.kr/kolisnet/openApi/open.php?page=1&gubun1=ISBN&code1=9791155240038")));
//		connect(reply, &QNetworkReply::finished, [this, reply] () {
//			QXmlStreamReader xml(reply->readAll());
//			auto name = [&xml] (const char *key) { return xml.name() == QLatin1String(key); };
//			auto read = [&xml] () { return xml.readElementText(); };
//			QList<Book> list;
//			if (!xml.readNextStartElement() || !name("METADATA"))
//				return;
//			while (xml.readNextStartElement()) {
//				if (name("RECORD")) {
//					list.append(Book());
//					auto &book = list.last();
//					while (xml.readNextStartElement()) {
//						if (name("REC_KEY"))
//							book.key = read().toInt();
//						else
//							xml.skipCurrentElement();
//					}
//				} else
//					xml.skipCurrentElement();
//			}
//			reply->deleteLater();
//			for (int i=0; i<list.size(); ++i) {
//				getDetail(list[i]);
//			}
//		});
//	}

////	QList<Book> parseList(const QByteArray &data) {
////		QXmlStreamReader xml(data);
////		auto name = [&xml] (const char *key) { return xml.name() == QLatin1String(key); };
////		auto read = [&xml] () { return xml.readElementText(); };
////		QList<Book> list;
////		while (xml.readNextStartElement()) {
////			qDebug() << xml.name();
////			if (name("RECORD"))
////				list.append(Book());
////			else if (list.isEmpty())
////				continue;
////			else if (name("TITLE"))
////				book.title = read();
////			else if (name("AUTHOR"))
////				book.author = read();
////			else if (name("PUBLISHER"))
////				book.publisher = read();
////			else if (name("PUBYEAR"))
////				book.year = read();
////			else if (name("TYPE"))
////				book.type = read();
////			else if (name("CONTENTS"))
////				book.hasTOC = read() == "Y";
////			else if (name("COVER_YN"))
////				book.hasCover = read() == "Y";
////			else if (name("LIB_NAME"))
////				book.libraryName = read();
////			else if (name("REC_KEY"))
////				book.key = read().toInt();
////			else
////				xml.skipCurrentElement();
////		}
////		qDebug() << list.size();
////		return list;
////	}
//};

//MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), d(new Data) {
//	d->ui.setupUi(this);
//	connect(d->ui.scan, &QPushButton::clicked, [this] () {
//		if (!d->scanner)
//			d->scanner = new ScannerDialog(this);
//		d->scan();
//	});
//	connect(d->ui.search, &QPushButton::clicked, [this] () { d->search(); });
////	connect(&d->network, &QNetworkAccessManager::finished, [this] (QNetworkReply *reply) {
////		qDebug() << reply->readAll();
//////		QTemporaryFile temp_file;
//////		temp_file.write(reply->readAll());

////	});
////9791155240038

////	http://nl.go.kr/kolisnet/openApi/open.php?page=1&gubun1=ISBN&code1=9791155240038

////	http://nl.go.kr/kolisnet/openApi/open.php?page=1&search_field1=total_field&value1=%EC%8B%A0%EB%8D%B0%EB%A0%90%EB%9D%BC&per_page=10
//}

//MainWindow::~MainWindow() {
//	delete d;
//}

