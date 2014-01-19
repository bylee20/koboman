#include "toplevelcontainer.hpp"
#include <QPainter>
#include <QImage>

// Stack Blur v1.0
//
// Author: Mario Klingemann <mario@quasimondo.com>
// http://incubator.quasimondo.com
// created Feburary 29, 2004


// This is a compromise between Gaussian Blur and Box blur
// It creates much better looking blurs than Box Blur, but is
// 7x faster than my Gaussian Blur implementation.
//
// I called it Stack Blur because this describes best how this
// filter works internally: it creates a kind of moving stack
// of colors whilst scanning through the image. Thereby it
// just has to add one new block of color to the right side
// of the stack and remove the leftmost color. The remaining
// colors on the topmost layer of the stack are either added on
// or reduced by one, depending on if they are on the right or
// on the left side of the stack.
//
// If you are using this algorithm in your code please add
// the following line:
//
// Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>

static void fastblur(QImage &image, int radius, const QColor &color, const QPoint &offset) {
	if (image.isNull() || radius < 1 || !color.alpha())
		return;
	quint32 *const pix = (quint32*)image.bits();
	const int r1 = radius + 1;
	const int h = image.height(), w = image.width(), last = w*h-1;
	const int xmax = w - 1, ymax = h - 1;
	const int div = 2*radius + 1;
	const int divsum = (radius + 1)*(radius + 1);

	QScopedArrayPointer<int> vmin(new int[qMax(w, h)]);
	QScopedArrayPointer<uchar> dv(new uchar[256*divsum]), stack(new uchar[div]), a(new uchar[w*h]);
	for (int i=0; i<256*divsum; ++i)
		dv[i] = (i/divsum);
	for (int i=0; i<div; ++i)
		stack[i] = 0;

	const int off = -(offset.x() + offset.y()*w);
	auto getAlpha = [off, pix, last] (int pos) { return qAlpha(pix[qBound(0, pos + off, last)]); };
	for (int y=0, yw=0, yi=0; y<h; ++y, yw += w) {
		int asum = 0, ainsum = 0, aoutsum = 0;
		for(int i=-radius; i<=radius; ++i){
			Q_ASSERT(yi == y*w && yi == yw);
			const int alpha = stack[i + radius] = getAlpha(yw + qBound(0, i, xmax));
			const int rbs = r1 - abs(i);
			asum += alpha*rbs;
			(int&)(i > 0 ? ainsum : aoutsum) += alpha;
		}
		for (int x=0, stackpointer = radius; x<w; ++x, ++yi) {
			a[yi] = dv[asum];

			auto &sir = stack[(stackpointer - radius + div)%div];
			asum -= aoutsum;
			aoutsum -= sir;

			if (!y)
				vmin[x] = qMin(x + radius + 1, xmax);
			sir = getAlpha(yw + vmin[x]);
			ainsum += sir;
			asum += ainsum;

			stackpointer = (stackpointer + 1)%div;
			const auto value = stack[stackpointer];
			aoutsum += value;
			ainsum -= value;
		}
	}
	for (int x=0; x<w; ++x) {
		int asum = 0, ainsum = 0, aoutsum = 0, yp = -radius*w;
		for(int i=-radius; i<=radius; ++i) {
			const int alpha = stack[i + radius] = a[qMax(0, yp) + x];
			const int rbs=r1-abs(i);
			asum += alpha*rbs;
			(int&)(i > 0 ? ainsum : aoutsum) += alpha;
			if (i < ymax)
				yp += w;
		}
		for (int y=0, stackpointer = radius, yi=x; y<h; ++y, yi += w){
			if (qAlpha(pix[yi]) == 0)
				pix[yi] = qRgba(color.red(), color.green(), color.blue(), dv[asum]);

			auto &sir = stack[(stackpointer - radius + div)%div];
			asum -= aoutsum;
			aoutsum -= sir;

			if (!x)
				vmin[y] = qMin(y+r1, ymax)*w;
			sir = a[x+vmin[y]];
			ainsum += sir;
			asum += ainsum;

			stackpointer = (stackpointer + 1)%div;
			const int alpha = stack[stackpointer];
			aoutsum += alpha;
			ainsum -= alpha;
		}
	}
}

TopLevelContainer::TopLevelContainer(TopLevelItem *item)
: QObject(item), m_top(item) {
	m_shadow = m_top->shadow();
	Q_ASSERT(m_top != nullptr);
}

TopLevelContainer::~TopLevelContainer() {
}

void TopLevelContainer::complete() {
	m_completed = true;
	resize();
	connect(m_top, &TopLevelItem::boundaryChanged, this, &TopLevelContainer::reattach);
	connect(m_top, &TopLevelItem::widthChanged, this, &TopLevelContainer::reattach);
	connect(m_top, &TopLevelItem::heightChanged, this, &TopLevelContainer::reattach);
	connect(this, &TopLevelContainer::attachChanged, this, &TopLevelContainer::reattach);
	connect(this, &TopLevelContainer::positionChanged, this, &TopLevelContainer::reattach);
	connect(this, &TopLevelContainer::itemChanged, this, &TopLevelContainer::reattach);
	connect(this, &TopLevelContainer::paddingChanged, this, &TopLevelContainer::resize);
	connect(m_shadow, &TopLevelShadow::colorChanged, this, &TopLevelContainer::repaint);
	connect(m_shadow, &TopLevelShadow::visibleChanged, this, &TopLevelContainer::repaint);
	connect(m_shadow, &TopLevelShadow::offsetChanged, this, &TopLevelContainer::repaint);
	connect(m_shadow, &TopLevelShadow::radiusChanged, this, &TopLevelContainer::repaint);
	connectItem();
	connectAttach();
}

void TopLevelContainer::setItem(QQuickItem *item) {
	if (_Change(m_item, item)) {
		if (m_item) {
			m_item->setParentItem(m_top);
			m_item->setTransformOrigin(QQuickItem::Top);
		}
		if (m_completed) {
			connectItem();
			resize();
		}
		emit itemChanged();
	}
}

void TopLevelContainer::reposition() {
	if (m_item)
		m_item->setPosition(m_pos + QPointF{m_padding, m_padding});
}

void TopLevelContainer::repaint() {
	m_dxy = {3.0, 3.0};
	QSizeF imageSize = m_size;
	if (m_shadow->m_visible) {
		imageSize.rwidth() += qAbs(m_shadow->m_offset.x()) + 2*m_shadow->m_radius;
		imageSize.rheight() += qAbs(m_shadow->m_offset.y()) + 2*m_shadow->m_radius;
		m_dxy += {m_shadow->m_radius + 2, m_shadow->m_radius + 2};
		if (m_shadow->m_offset.x() < 0)
			m_dxy.rx() -= m_shadow->m_offset.x();
		if (m_shadow->m_offset.y() < 0)
			m_dxy.ry() -= m_shadow->m_offset.y();
	}
	const int w = imageSize.width() + 0.5 + 6;
	const int h = imageSize.height() + 0.5 + 6;

	_Expand(m_imageData, w*h*4);
	m_image = QImage((uchar*)m_imageData.data(), w, h, QImage::Format_ARGB32);
	m_image.fill(0x0);
	QPainter painter(&m_image);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setBrush(m_color);
	painter.setPen(Qt::NoPen);
	const QRectF rect{m_dxy, m_size};
	painter.drawRoundedRect(rect, m_radius, m_radius);
	if (m_shadow->m_visible)
		fastblur(m_image, m_shadow->m_radius, m_shadow->m_color, m_shadow->m_offset.toPoint());
	painter.end();
	emit paintingAreaChanged();
	emit repainted();
}

QRectF TopLevelContainer::paintingArea() const {
	QPointF pos(m_pos - m_dxy);
	QSizeF size(m_image.size());
	if (m_item && !qFuzzyCompare(m_item->scale(), 1.0)) {
		const auto s = m_item->scale();
		const auto o = m_item->mapToItem(m_top, m_item->transformOriginPoint());
		pos = o + (pos - o)*s;
		size *= s;
	}
	return QRectF(pos, size);
}

void TopLevelContainer::resize() {
	QSizeF size{2*m_padding, 2*m_padding};
	if (m_item) {
		size.rwidth() += m_item->width();
		size.rheight() += m_item->height();
	}
	if (_Change(m_size, size)) {
		repaint();
		reattach();
		emit sizeChanged();
	}
}

void TopLevelContainer::connectAttach() {
	m_attachConnections.clear();
	if (m_attach)
		m_attachConnections << connect(m_attach, &QQuickItem::xChanged, this, &TopLevelContainer::reattach)
		<< connect(m_attach, &QQuickItem::yChanged, this, &TopLevelContainer::reattach)
		<< connect(m_attach, &QQuickItem::widthChanged, this, &TopLevelContainer::reattach)
		<< connect(m_attach, &QQuickItem::heightChanged, this, &TopLevelContainer::reattach);
}

void TopLevelContainer::connectItem() {
	m_itemConnections.clear();
	if (m_item) {
		m_itemConnections << connect(m_item, &QQuickItem::xChanged, this, &TopLevelContainer::reposition)
		<< connect(m_item, &QQuickItem::yChanged, this, &TopLevelContainer::reposition)
		<< connect(m_item, &QQuickItem::widthChanged, this, &TopLevelContainer::resize)
		<< connect(m_item, &QQuickItem::heightChanged, this, &TopLevelContainer::resize)
		<< connect(m_item, &QQuickItem::scaleChanged, this, &TopLevelContainer::paintingAreaChanged);
	}
}

void TopLevelContainer::setAttach(QQuickItem *attach) {
	if (_Change(m_attach, attach)) {
		if (m_completed)
			connectAttach();
		emit attachChanged();
	}
}

void TopLevelContainer::reattach() {
	auto pos = m_attach ? m_attach->mapToItem(m_top, {m_attach->x(), m_attach->y() + m_attach->height()}) : m_position;
	const auto boundary = m_top->boundary();
	const auto right = pos.x() + m_size.width() + boundary;
	const auto bottom = pos.y() + m_size.height() + boundary;
	if (right > m_top->width())
		pos.rx() -= right - m_top->width();
	if (bottom > m_top->height())
		pos.ry() -= bottom - m_top->height();
	if (_Change(m_pos, pos)) {
		emit paintingAreaChanged();
		reposition();
	}
}
