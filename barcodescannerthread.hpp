#ifndef BARCODESCANNERTHREAD_HPP
#define BARCODESCANNERTHREAD_HPP

#include <QThread>
#include "barcode.hpp"

class BarcodeScannerThread : public QThread {
	Q_OBJECT
public:
	enum State {
		Waiting, Scanning
	};

	BarcodeScannerThread(QObject *parent = nullptr);
	~BarcodeScannerThread();
	void stop();
	bool isScanning() const;
	bool isWaiting() const;
	void lock();
	void unlock();
	State state() const;
signals:
	void found(BarcodeList barcodes);
public slots:
	void scan(const BarcodeScannerImagePtr &image);
private:
	void run();
	struct Data;
	Data *d;
};

#endif // BARCODESCANNERTHREAD_HPP
