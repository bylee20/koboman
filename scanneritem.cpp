#include "scanneritem.hpp"
#include "scanneritemshader.hpp"
#include <QSGMaterialShader>
#include <QVideoRendererControl>
#include <QSGGeometryNode>
#include <QVideoFrame>
#include <QCamera>
#include <QAbstractVideoSurface>

class ScannerItemSurface : public QAbstractVideoSurface {
public:
	ScannerItemSurface(ScannerItem *item): m_item(item) {}
	~ScannerItemSurface() {}
	QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const override {
		Q_ASSERT(false);
		if (handleType == QAbstractVideoBuffer::NoHandle) {
			QList<QVideoFrame::PixelFormat> fmts;
			for (int i=1; i<QVideoFrame::Format_AdobeDng; ++i)
				fmts << (QVideoFrame::PixelFormat)i;
			return fmts;
//			return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_YUV420P << QVideoFrame::Format_BGR32;
		} else
			return QList<QVideoFrame::PixelFormat>();
	}
	bool present(const QVideoFrame &given) override { return m_item->present(given); }
	bool start(const QVideoSurfaceFormat &format) override { return QAbstractVideoSurface::start(format) && m_item->prepare(format); }
	void stop() override { QAbstractVideoSurface::stop(); }
private:
	ScannerItem *m_item = nullptr;
};

struct ScannerItem::Data {
	QSGGeometryNode *node = nullptr;
	ScannerItemMaterial *material = nullptr;
	QVideoSurfaceFormat format;
	QVideoFrame frame;
	bool newFormat = true;
	bool newFrame = false;
	bool dirtyGeometry = false;
	void updateVertices(const QRectF &rect) {
//		/* Based on fill mode and our size, figure out the source/dest rects */
//		void QDeclarativeVideoOutput::_q_updateGeometry()
//		{
//			const QRectF rect(0, 0, width(), height());
//			const QRectF absoluteRect(x(), y(), width(), height());

//			if (!m_geometryDirty && m_lastRect == absoluteRect)
//				return;

//			QRectF oldContentRect(m_contentRect);

//			m_geometryDirty = false;
//			m_lastRect = absoluteRect;

//			if (m_nativeSize.isEmpty()) {
//				//this is necessary for item to receive the
//				//first paint event and configure video surface.
//				m_contentRect = rect;
//			} else if (m_fillMode == Stretch) {
//				m_contentRect = rect;
//			} else if (m_fillMode == PreserveAspectFit || m_fillMode == PreserveAspectCrop) {
//				QSizeF scaled = m_nativeSize;
//				scaled.scale(rect.size(), m_fillMode == PreserveAspectFit ?
//								 Qt::KeepAspectRatio : Qt::KeepAspectRatioByExpanding);

//				m_contentRect = QRectF(QPointF(), scaled);
//				m_contentRect.moveCenter(rect.center());
//			}

//			if (m_backend)
//				m_backend->updateGeometry();

//			if (m_contentRect != oldContentRect)
//				emit contentRectChanged();
//		}


		QSizeF size = format.sizeHint();
		size.scale(rect.size(), Qt::KeepAspectRatio);
		const qreal x = (rect.width() - size.width())*0.5f;
		const qreal y = (rect.height() - size.height())*0.5f;
		vtxRect = {QPointF{x, y}+rect.topLeft(), size};
		dirtyGeometry = true;
	}
	static void set(QSGGeometry::TexturedPoint2D *tp, const QRectF &vtx, const QRectF &txt) {
		auto set = [&tp] (const QPointF &vtx, const QPointF &txt) {
			tp++->set(vtx.x(), vtx.y(), txt.x(), txt.y());
		};
		set(vtx.topLeft(), txt.topLeft());
		set(vtx.bottomLeft(), txt.bottomLeft());
		set(vtx.topRight(), txt.topRight());
		set(vtx.bottomRight(), txt.bottomRight());
	}
	QRectF vtxRect, texRect;
	QCamera *camera = nullptr;
	QString deviceDescription;
	ScannerItemSurface *surface = nullptr;
	QList<QByteArray> devices;
	int device = -1;
	bool completed = false;
	QCamera::State pending = QCamera::ActiveState;
	void setCameraState(QCamera::State state) {
		pending = state;
		if (completed && camera) {
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

	#define QVideoRendererControl_iid "org.qt-project.qt.videorenderercontrol/5.0"

	QVideoRendererControl *ctrl = nullptr;
	QMediaService *service = nullptr;
	void createCamera(int index) {
//		if (ctrl && service) {
//			ctrl->setSurface(nullptr);
//			service->releaseControl(ctrl);
//		}
		delete camera;

//		service = nullptr;
//		ctrl = nullptr;
		camera = nullptr;

		if (0 <= index && index < devices.size()) {
//			camera = new QCamera(devices[index]);
//			camera->setViewfinder(surface);
//			camera->setCaptureMode(QCamera::CaptureStillImage);
//			deviceDescription = QCamera::deviceDescription(devices[index]);
//			service = camera->service();
//			ctrl = qobject_cast<QVideoRendererControl*>(service->requestControl(QVideoRendererControl_iid));
//			ctrl->setSurface(surface);
//			setCameraState(QCamera::ActiveState);
			//			if (QMediaControl *control = service->requestControl(QVideoRendererControl_iid)) {
			//		        if ((m_rendererControl = qobject_cast<QVideoRendererControl *>(control))) {
			//		            m_rendererControl->setSurface(m_surface);
			//		            m_service = service;
			//		            return true;
			//		        }
		}
	}

	//	QByteArray device;
	//	QComboBox *devices;
	//	ScannerSurface *surface = nullptr;
	//	VideoWidget *view = nullptr;
	//	QList<Barcode> barcodes;
};

ScannerItem::ScannerItem(QQuickItem *parent)
: QQuickItem(parent), d(new Data) {
	setFlag(ItemHasContents, true);
	d->surface = new ScannerItemSurface(this);
	d->texRect = {0.0, 0.0, 1.0, 1.0};
	d->device = -1;
	d->devices = QCamera::availableDevices();

	connect(d->surface, &QAbstractVideoSurface::surfaceFormatChanged, [this] (const QVideoSurfaceFormat &format) {
		qDebug() << format.pixelFormat() << format.sizeHint();
	});
//	QObject::connect(m_surface, SIGNAL(surfaceFormatChanged(QVideoSurfaceFormat)),
//                     q, SLOT(_q_updateNativeSize()), Qt::QueuedConnection);
}

ScannerItem::~ScannerItem() {
	d->completed = false;
	delete d->camera;
	delete d->surface;
	delete d;
}

void ScannerItem::componentComplete() {
	QQuickItem::componentComplete();
	d->completed = true;
	if (d->device != -1)
		d->createCamera(d->device);
	d->setCameraState(d->pending);
}

int ScannerItem::device() const {
	return d->device;
}

QString ScannerItem::deviceDescription() const {
	return d->deviceDescription;
}

void ScannerItem::setDevice(int device) {
	if (d->device != device) {
		d->createCamera(d->device = device);
		emit deviceChanged();
		update();
		qDebug() << "visible" << isVisible() << position() <<width() <<height();
	}
}

bool ScannerItem::prepare(const QVideoSurfaceFormat &format) {
	if (!d->material || d->material->format() != format) {
		d->newFormat = true;
		d->format = format;
		d->updateVertices({position(), QSizeF{width(), height()}});
		qDebug() << format.pixelFormat() << format.sizeHint();
	}
	return true;
}

bool ScannerItem::present(const QVideoFrame &frame) {
	d->frame = frame;
	d->newFrame = true;
	update();
	qDebug() << "new frame" << frame.pixelFormat();
	return true;
}

QSGNode *ScannerItem::updatePaintNode(QSGNode *old, UpdatePaintNodeData */*data*/) {
	if (d->camera) {

	}
	d->node = static_cast<QSGGeometryNode*>(old);
	if (!d->node) {
		auto node = new QSGGeometryNode;
		node->setFlags(QSGNode::OwnsGeometry);
		node->setGeometry(new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4));
		d->node = node;
	}
	if (d->newFormat) {
		delete d->material;
		d->material = new ScannerItemMaterial(d->format);
		d->node->setMaterial(d->material);
		d->node->markDirty(QSGNode::DirtyMaterial);
		d->newFormat = false;
	}
	Q_ASSERT(d->material != nullptr);
	if (d->newFrame) {
		d->material->setFrame(d->frame);
		d->node->markDirty(QSGNode::DirtyMaterial);
	}
	if (d->dirtyGeometry) {
		d->set(d->node->geometry()->vertexDataAsTexturedPoint2D(), d->vtxRect, d->texRect);
		d->node->markDirty(QSGNode::DirtyGeometry);
		d->dirtyGeometry = false;
	}
	return d->node;
}

void ScannerItem::geometryChanged(const QRectF &new_, const QRectF &old) {
	QQuickItem::geometryChanged(new_, old);
	d->updateVertices(new_);
	update();
}
