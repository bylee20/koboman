#include "barcodescannerthread.hpp"
#include "zbar.h"
#include "utility.hpp"
#include <QMutex>
#include <QDebug>
#include <QImage>

struct BarcodeScannerThread::Data {
	BarcodeScannerImagePtr image;
	zbar::ImageScanner scanner;
	zbar::Image zImage;
	QMutex mutex;
	bool stop = false;
	bool newImage = false;
	State state = Waiting;

	static Barcode symbolToBarcode(const zbar::Symbol &symbol) {
		Barcode barcode;
		switch (symbol.get_type()) {
		case zbar::ZBAR_EAN8:
			barcode.type = Barcode::EAN_8;
			break;
		case zbar::ZBAR_UPCE:
			barcode.type = Barcode::UPCE;
			break;
		case zbar::ZBAR_ISBN10:
			barcode.type = Barcode::ISBN_10;
			break;
		case zbar::ZBAR_UPCA:
			barcode.type = Barcode::UPCA;
			break;
		case zbar::ZBAR_ISBN13:
			barcode.type = Barcode::ISBN_13;
			break;
		case zbar::ZBAR_EAN13:
			barcode.type = Barcode::EAN_13;
			break;
		case zbar::ZBAR_CODE39:
			barcode.type = Barcode::CODE_39;
			break;
		case zbar::ZBAR_CODE128:
			barcode.type = Barcode::CODE_128;
			break;
		default:
			barcode.type = Barcode::Unknown;
			break;
		}
		barcode.text = QString::fromStdString(symbol.get_data());
		return barcode;
	}
};

BarcodeScannerThread::BarcodeScannerThread(QObject *parent)
: QThread(parent), d(new Data) {
	d->scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 0);
	d->scanner.set_config(zbar::ZBAR_ISBN13, zbar::ZBAR_CFG_ENABLE, 1);
	d->scanner.set_config(zbar::ZBAR_CODE39, zbar::ZBAR_CFG_ENABLE, 1);
	d->zImage.set_format("Y800");
	connect(this, &QThread::finished, this, &QObject::deleteLater);
}

BarcodeScannerThread::~BarcodeScannerThread() {
	delete d;
}

void BarcodeScannerThread::scan(const BarcodeScannerImagePtr &image) {
	d->mutex.lock();
	d->image = image;
	d->newImage = true;
	d->mutex.unlock();
}

void BarcodeScannerThread::lock() {
	d->mutex.lock();
}

void BarcodeScannerThread::unlock() {
	d->mutex.unlock();
}

void BarcodeScannerThread::stop() {
	d->stop = true;
}

bool BarcodeScannerThread::isWaiting() const {
	return d->state == Waiting;
}

bool BarcodeScannerThread::isScanning() const {
	return d->state == Scanning;
}

static inline void saveTest(BarcodeScannerImagePtr y8) {
	QImage image(y8->size(), QImage::Format_ARGB32);
	Q_ASSERT(image.width()*image.height() == y8->bytes());
	Q_ASSERT(image.format() == QImage::Format_ARGB32);
	auto p = y8->data();
	for (int i=0; i<image.width(); ++i) {
		for (int j=0; j<image.height(); ++j) {
			auto y = p[j*image.width() + i];
			image.setPixel(i, j, qRgba(y, y, y, 255));
		}
	}
	static int i=0;	++i;
	qDebug() << i <<image.save(Utility::storage() + QString::number(i) + ".jpg");
}

void BarcodeScannerThread::run() {
	BarcodeScannerImagePtr y8;
	d->state = Waiting;
	while (!d->stop) {
		d->mutex.lock();
		if (!d->newImage)
			d->state = Waiting;
		else {
			y8 = d->image;
			if (y8 && y8->convertToY8() && !y8->isEmpty())
				d->state = Scanning;
			else
				d->state = Waiting;
			d->newImage = false;
		}
		d->mutex.unlock();
		if (d->state == Scanning) {
			d->zImage.set_size(y8->width(), y8->height());
			d->zImage.set_data(y8->data(), y8->bytes());
			const bool ok = d->scanner.scan(d->zImage);
			if (ok) {
				QList<Barcode> barcodes;
				for (auto &symbol : d->zImage)
					barcodes << d->symbolToBarcode(symbol);
				emit found(barcodes);
			}
			d->scanner.recycle_image(d->zImage);
			d->zImage.set_data(nullptr, 0);
		}
		msleep(5);
	}
}
