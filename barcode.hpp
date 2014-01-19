#ifndef BARCODE_HPP
#define BARCODE_HPP

#include <QString>
#include <QSharedPointer>
#include <QSize>
#include <QList>
#include <QImage>
#include <QDebug>

class Barcode {
public:
	enum Type { Unknown, EAN_8, UPCE, ISBN_10, UPCA, ISBN_13, EAN_13, CODE_39, CODE_128 };
	Type type = Unknown;
	QString text;
};

using BarcodeList = QList<Barcode>;

class BarcodeObject : public QObject {
	Q_OBJECT
	Q_ENUMS(BarcodeType)
	Q_PROPERTY(BarcodeType type READ type CONSTANT FINAL)
	Q_PROPERTY(QString text READ text CONSTANT FINAL)
public:
	enum BarcodeType {
		Unknown = Barcode::Unknown,
		EAN_8   = Barcode::EAN_8,
		UPCE    = Barcode::UPCE,
		ISBN_10 = Barcode::ISBN_10,
		UPCA    = Barcode::UPCA,
		ISBN_13 = Barcode::ISBN_13,
		EAN_13  = Barcode::EAN_13,
		CODE_39 = Barcode::CODE_39,
		CODE_128= Barcode::CODE_128
	};
	BarcodeObject(QObject *parent = nullptr): QObject(parent) {}
	BarcodeObject(const Barcode &barcode, QObject *parent = nullptr)
	: QObject(parent), m_barcode(barcode) {}
	BarcodeType type() const { return BarcodeType(m_barcode.type); }
	QString text() const { return m_barcode.text; }
	const Barcode &barcode() const { return m_barcode; }
private:
	Barcode m_barcode;
};

class BarcodeScannerImage {
public:
	enum Format {Invalid, Y8, Gray32, Rgb32};
	bool getY8(BarcodeScannerImage &image) const;
	bool isEmpty() const { return m_size.isEmpty(); }
	Format format() const { return m_format; }
	const uchar *data() const { return (const uchar*)m_data.data(); }
	int bytes() const { return m_data.size(); }
	int width() const { return m_size.width(); }
	int height() const { return m_size.height(); }
	int area() const { return m_size.width()*m_size.height(); }
	const QSize &size() const { return m_size; }
	static QSharedPointer<BarcodeScannerImage> allocate(const QByteArray &data, const QSize &size, Format format) {
		return QSharedPointer<BarcodeScannerImage>(new BarcodeScannerImage(data, size, format));
	}
	bool convertToY8();
private:
	BarcodeScannerImage(const QByteArray &data, const QSize &size, Format format)
	: m_data(data), m_size(size), m_format(format), m_bytes(data.size()) { }
	QByteArray m_data;
	QSize m_size;
	Format m_format = Invalid;
	int m_bytes = 0;
};

using BarcodeScannerImagePtr = QSharedPointer<BarcodeScannerImage>;

#endif // BARCODE_HPP
