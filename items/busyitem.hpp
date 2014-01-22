#ifndef BUSYITEM_HPP
#define BUSYITEM_HPP

#include "items/textureitem.hpp"

class BusyItem : public TextureItem {
	Q_OBJECT
	Q_PROPERTY(qreal thickness READ thickness WRITE setThickness NOTIFY thicknessChanged)
	Q_PROPERTY(int segments READ segments WRITE setSegments NOTIFY segmentsChanged)
	Q_PROPERTY(bool running READ isRunning WRITE setRunning NOTIFY runningChanged)
	Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
public:
	BusyItem(QQuickItem *parent = nullptr);
	~BusyItem();
	qreal thickness() const;
	void setThickness(qreal t);
	int segments() const;
	void setSegments(int seg);
	bool isRunning() const;
	void setRunning(bool running);
	int duration() const;
	void setDuration(int duration);
signals:
	void durationChanged();
	void runningChanged();
	void thicknessChanged();
	void segmentsChanged();
private:
	void componentComplete();
	void itemChange(ItemChange change, const ItemChangeData &data);
	void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
	QByteArray fragmentShader() const;
	QByteArray vertexShader() const;
	const char *const *attributeNames() const;
	QSGGeometry *createSGGeometry();
	void updateSGGeometry(QSGGeometry *geometry);
	void updatePolish();
	struct Data;
	Data *d;
};

#endif // BUSYITEM_HPP
