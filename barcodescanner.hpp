#ifndef BARCODESCANNER_HPP
#define BARCODESCANNER_HPP

#include <QQuickItem>
#include <QAbstractVideoSurface>
#include "barcode.hpp"

class QVideoFrame;
class QVideoSurfaceFormat;
class BarcodeScannerImage;

class BarcodeScanner : public QQuickItem {
	Q_OBJECT
	Q_PROPERTY(int device READ device WRITE setDevice NOTIFY deviceChanged)
	Q_PROPERTY(QString deviceDescription READ deviceDescription NOTIFY deviceChanged)
	Q_PROPERTY(bool torch READ torch WRITE setTorch NOTIFY torchChanged)
	Q_PROPERTY(bool scanning READ isScanning WRITE setScanning NOTIFY scanningChanged)
	Q_PROPERTY(QQmlListProperty<BarcodeObject> barcodes READ barcodeObjects NOTIFY barcodesChanged)
public:
	BarcodeScanner(QQuickItem *parent = nullptr);
	~BarcodeScanner();
	void setDevice(int device);
	int device() const;
	QString deviceDescription() const;
	QRectF rect() const { return { x(), y(), width(), height() }; }
	bool torch() const;
	void setTorch(bool on);
	QQmlListProperty<BarcodeObject> barcodeObjects() const;
	BarcodeList barcodes() const;
	bool isScanning() const;
	void setScanning(bool scanning);
signals:
	void barcodesChanged();
	void deviceChanged();
	void torchChanged();
	void imageObtained(const BarcodeScannerImage &image);
	void scanningChanged();
private slots:
	void initializeGL();
	void finalizeGL();
	void adjustOrientation(Qt::ScreenOrientation orientation);
	void requestImage();
	void setFoundBarcodes(const BarcodeList &barcodes);
private:
	void itemChange(ItemChange change, const ItemChangeData &data) override;
	void componentComplete() override;
	void customEvent(QEvent *event) override;
	bool present(const QVideoFrame &frame);
	bool prepare(const QVideoSurfaceFormat &format);
	QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData */*data*/) override;
	void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
	struct Data;
	Data *d;
	friend class BarcodeScannerSurface;
};

#endif // BARCODESCANNER_HPP
