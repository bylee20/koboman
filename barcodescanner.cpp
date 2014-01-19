#include "barcodescanner.hpp"
#include "utils.hpp"
#include "barcodescannerbackend.hpp"
#include "barcodescannerthread.hpp"
#include <QQuickWindow>
#include <QCameraExposureControl>
#include <QCameraFlashControl>
#include <QCameraFocusControl>
#include <QOpenGLContext>
#include <QSGGeometryNode>
#include <QCamera>
#include <QScreen>

enum CustomEventType {
	RequestImage = QEvent::User + 1,
	UpdateBarcodes,
};

class BarcodeScannerSurface : public QAbstractVideoSurface {
public:
	BarcodeScannerSurface(BarcodeScanner *item, QObject *parent = nullptr)
	: QAbstractVideoSurface(parent), m_item(item) {}
	~BarcodeScannerSurface() {}
	PixelFormatList supportedPixelFormats(VideoFrameHandleType handleType) const override {
		return BarcodeScannerBackend::supportedPixelFormats(handleType);
	}
	bool present(const QVideoFrame &given) override {
		return m_item->present(given);
	}
	bool start(const QVideoSurfaceFormat &format) override {
		if (!supportedPixelFormats(format.handleType()).contains(format.pixelFormat()))
			return false;
		if (!m_item->prepare(format))
			return false;
		return QAbstractVideoSurface::start(format);
	}
private:
	BarcodeScanner *m_item = nullptr;
};

struct BarcodeScanner::Data {
	BarcodeScanner *p = nullptr;
	QSGGeometryNode *node = nullptr;
	BarcodeScannerMaterial *material = nullptr;
	QVideoSurfaceFormat format;
	QVideoFrame frame;
	bool newFormat = true;
	bool newFrame = false;
	bool dirtyGeometry = false;
	bool screenOrientationChanged = false;
	bool scanning = false;
	bool crop = true;
	QOpenGLContext *gl = nullptr;
	QRectF texRect{0.0, 0.0, 1.0, 1.0}, vtxRect;

	QCamera *camera = nullptr;
	QString deviceDescription;
	BarcodeScannerSurface *surface = nullptr;
	QList<QByteArray> devices;
	int device = -1;
	bool completed = false, locked = false;
	QCamera::State cameraState = QCamera::LoadedState;

	bool cameraLoaded = false;
	bool cameraChanged = false;
	bool cameraConfigured = false;
	QMediaService *service = nullptr;
	QCameraExposureControl *exposure = nullptr;
	QCameraFlashControl *flash = nullptr;
	QCameraFocusControl *focus = nullptr;
	bool torch = false;

	BarcodeList barcodes;
	QVector<BarcodeObject*> barcodeObjects;
	QList<QMetaObject::Connection> onWindowChangedConnections;
	BarcodeScannerThread *thread = nullptr;
	QScreen *screen = nullptr;
	Qt::ScreenOrientation orientation = Qt::PrimaryOrientation;

	void updateVertices(const QRectF &rect) {
		QSizeF size = format.sizeHint();
		if (orientation == Qt::PortraitOrientation)
			qSwap(size.rwidth(), size.rheight());
		if (crop && !size.isEmpty()) {
			size.scale(rect.size(), Qt::KeepAspectRatioByExpanding);
			vtxRect = rect;
			texRect.setBottomRight({ rect.width()/size.width(), rect.height()/size.height()});
		} else {
			size.scale(rect.size(), Qt::KeepAspectRatio);
			vtxRect.setSize(size);
			vtxRect.moveCenter(rect.center());
			texRect.setBottomRight({1.0, 1.0});
		}
		vtxRect.translate(0, -rect.y());
		texRect.moveCenter({0.5, 0.5});
		dirtyGeometry = true;
	}

	void setCameraState(QCamera::State state) {
		cameraState = state;
		if (completed && camera && p->isVisible()) {
			switch (state) {
			case QCamera::ActiveState:
				camera->start();
				break;
			case QCamera::UnloadedState:
				camera->unload();
				break;
			case QCamera::LoadedState:
				camera->load();
				break;
			}
		}
	}

	void updateTorch() {
		if (cameraLoaded && flash)
			flash->setFlashMode(torch ? QCameraExposure::FlashVideoLight : QCameraExposure::FlashOff);
	}

	void releaseCamera() {
		if (service) {
			service->releaseControl(exposure);
			service->releaseControl(flash);
			service->releaseControl(focus);
			exposure = nullptr;
			flash = nullptr;
			focus = nullptr;
			service = nullptr;
		}
		_Delete(camera);
		cameraLoaded = cameraConfigured = false;
	}

	void updateCameraState() { setCameraState(cameraState); }

	void createCamera(int index) {
		releaseCamera();
		if (0 <= index && index < devices.size()) {
			cameraChanged = true;
			camera = new QCamera(devices[index]);

			service = camera->service();
			Q_ASSERT(service != nullptr);
			exposure = service->requestControl<QCameraExposureControl*>();
			flash = service->requestControl<QCameraFlashControl*>();
			focus = service->requestControl<QCameraFocusControl*>();

			connect(camera, &QCamera::statusChanged, [this] (QCamera::Status status) {
				if (!cameraLoaded && status == QCamera::LoadedStatus) {
					cameraLoaded = true;
					updateTorch();
					if (focus)
						focus->setFocusMode(QCameraFocus::ContinuousFocus);
				}
			});
			camera->setViewfinder(surface);
			deviceDescription = QCamera::deviceDescription(devices[index]);
			setCameraState(cameraState);
		}
	}
};

BarcodeScanner::BarcodeScanner(QQuickItem *parent)
: QQuickItem(parent), d(new Data) {
	d->p = this;
	d->surface = new BarcodeScannerSurface(this);
	d->device = -1;
	d->devices = QCamera::availableDevices();
	d->thread = new BarcodeScannerThread;
	d->thread->start();
	setFlag(ItemHasContents, true);

	connect(this, &QQuickItem::windowChanged, [this] (QQuickWindow *window) {
		for (auto &connection : d->onWindowChangedConnections)
			disconnect(connection);
		d->onWindowChangedConnections.clear();
		if (window) {
			d->screen = window->screen();
			d->screen->setOrientationUpdateMask(Qt::LandscapeOrientation | Qt::PortraitOrientation | Qt::InvertedLandscapeOrientation | Qt::InvertedPortraitOrientation);
			d->screenOrientationChanged = true;
			adjustOrientation(d->screen->orientation());
			d->onWindowChangedConnections
				<< connect(window, &QQuickWindow::sceneGraphInitialized, this, &BarcodeScanner::initializeGL, Qt::DirectConnection)
				<< connect(window, &QQuickWindow::sceneGraphInvalidated, this, &BarcodeScanner::finalizeGL, Qt::DirectConnection)
				<< connect(d->screen, &QScreen::orientationChanged, this, &BarcodeScanner::adjustOrientation);
		}
	});
	connect(d->thread, &BarcodeScannerThread::found, this, &BarcodeScanner::setFoundBarcodes, Qt::DirectConnection);
}

BarcodeScanner::~BarcodeScanner() {
	qDeleteAll(d->barcodeObjects);
	d->thread->stop();
	d->releaseCamera();
	delete d->surface;
	delete d;
}

QQmlListProperty<BarcodeObject> BarcodeScanner::barcodeObjects() const {
	static auto count = [] (QQmlListProperty<BarcodeObject> *list) -> int {
		return static_cast<BarcodeScanner*>(list->object)->d->barcodeObjects.size();
	};
	static auto at = [] (QQmlListProperty<BarcodeObject> *list, int index) -> BarcodeObject* {
		return static_cast<BarcodeScanner*>(list->object)->d->barcodeObjects.value(index);
	};
	return QQmlListProperty<BarcodeObject>(const_cast<BarcodeScanner*>(this), nullptr, count, at);
}

BarcodeList BarcodeScanner::barcodes() const {
	return d->barcodes;
}

void BarcodeScanner::setFoundBarcodes(const BarcodeList &barcodes) {
	_PostEvent(this, UpdateBarcodes, barcodes);
}

bool BarcodeScanner::torch() const {
	return d->torch;
}

void BarcodeScanner::setTorch(bool on) {
	if (d->torch != on) {
		d->torch = on;
		d->updateTorch();
		emit torchChanged();
	}
}

void BarcodeScanner::adjustOrientation(Qt::ScreenOrientation orientation) {
	d->screenOrientationChanged = true;
	d->orientation = orientation;
	d->updateVertices(rect());
	update();
}

void BarcodeScanner::itemChange(ItemChange change, const ItemChangeData &data) {
	QQuickItem::itemChange(change, data);
	if (change == ItemVisibleHasChanged)
		d->setCameraState(d->cameraState);
}

void BarcodeScanner::initializeGL() {
	if (d->gl == QOpenGLContext::currentContext())
		return;
	d->gl = QOpenGLContext::currentContext();
}

void BarcodeScanner::finalizeGL() {
	Q_ASSERT(QOpenGLContext::currentContext() != nullptr);
	delete d->material;
	delete d->node;
}

void BarcodeScanner::componentComplete() {
	QQuickItem::componentComplete();
	d->completed = true;
	if (d->device != -1)
		d->createCamera(d->device);
	d->updateCameraState();
	d->updateTorch();
}

int BarcodeScanner::device() const {
	return d->device;
}

QString BarcodeScanner::deviceDescription() const {
	return d->deviceDescription;
}

void BarcodeScanner::setDevice(int device) {
	if (d->device != device) {
		d->createCamera(d->device = device);
		emit deviceChanged();
		update();
	}
}

bool BarcodeScanner::isScanning() const {
	return d->scanning;
}

void BarcodeScanner::setScanning(bool scanning) {
	if (_Change(d->scanning, scanning)) {
		d->setCameraState(scanning ? QCamera::ActiveState : QCamera::LoadedState);
		emit scanningChanged();
	}
}

bool BarcodeScanner::prepare(const QVideoSurfaceFormat &format) {
	if (d->format != format) {
		d->newFormat = true;
		d->format = format;
		d->updateVertices(rect());
		auto size = format.sizeHint();
		setImplicitWidth(size.width());
		setImplicitHeight(size.height());
	}
	return true;
}

bool BarcodeScanner::present(const QVideoFrame &frame) {
	if (d->scanning) {
		d->frame = frame;
		d->newFrame = true;
		update();
	}
	return true;
}

QSGNode *BarcodeScanner::updatePaintNode(QSGNode *old, UpdatePaintNodeData */*data*/) {
#ifdef Q_OS_ANDROID
	if (d->cameraChanged) {
		d->cameraChanged = false;
		auto obj = d->surface->property("_q_GLThreadCallback").value<QObject*>();
		QEvent event(QEvent::User);
		obj->event(&event);
	}
#endif

	d->node = static_cast<QSGGeometryNode*>(old);
	if (!d->node) {
		auto node = new QSGGeometryNode;
		node->setFlags(QSGNode::OwnsGeometry);
		node->setGeometry(new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4));
		d->node = node;
	}

	if (d->newFormat) {
		_Renew(d->material, d->format);
		d->material->setScreenOrientation(d->orientation);
		d->node->setMaterial(d->material);
		d->node->markDirty(QSGNode::DirtyMaterial);
		d->newFormat = false;
	}
	Q_ASSERT(d->material != nullptr);
	if (d->screenOrientationChanged) {
		d->material->setScreenOrientation(d->orientation);
		d->node->markDirty(QSGNode::DirtyMaterial);
		d->screenOrientationChanged = false;
	}
	if (d->newFrame) {
		d->material->setFrame(d->frame);
		d->node->markDirty(QSGNode::DirtyMaterial);
		if (d->scanning && d->thread->isWaiting())
			d->thread->scan(d->material->backend()->toImage(d->frame));
		d->newFrame = false;
	}
	if (d->dirtyGeometry) {
		auto p = d->node->geometry()->vertexDataAsTexturedPoint2D();
		p[0].set(d->vtxRect.left(), d->vtxRect.top(), d->texRect.left(), d->texRect.top());
		p[1].set(d->vtxRect.right(), d->vtxRect.top(), d->texRect.right(), d->texRect.top());
		p[2].set(d->vtxRect.left(), d->vtxRect.bottom(), d->texRect.left(), d->texRect.bottom());
		p[3].set(d->vtxRect.right(), d->vtxRect.bottom(), d->texRect.right(), d->texRect.bottom());
		d->node->markDirty(QSGNode::DirtyGeometry);
		d->dirtyGeometry = false;
	}
	return d->node;
}

void BarcodeScanner::geometryChanged(const QRectF &new_, const QRectF &old) {
	QQuickItem::geometryChanged(new_, old);
	d->updateVertices(new_);
	update();
}

void BarcodeScanner::requestImage() {
	_PostEvent(this, RequestImage);
}

void BarcodeScanner::customEvent(QEvent *event) {
	QQuickItem::customEvent(event);
	switch ((int)event->type()) {
	case RequestImage:
		update();
		break;
	case UpdateBarcodes:
		setScanning(false);
		qDeleteAll(d->barcodeObjects);
		_GetAllEventData(event, d->barcodes);
		d->barcodeObjects.resize(d->barcodes.size());
		for (int i=0; i<d->barcodes.size(); ++i)
			d->barcodeObjects[i] = new BarcodeObject(d->barcodes[i], this);
		emit barcodesChanged();
		break;
	default:
		return;
	}
	event->accept();
}
