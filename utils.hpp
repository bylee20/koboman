#ifndef UTILS_HPP
#define UTILS_HPP

#include <tuple>
#include <QString>
#include <QObject>
#include <QPointF>
#include <QSizeF>
#include <QRectF>

template<typename... Args>
class DataEvent : public QEvent {
public:
	using Data = std::tuple<Args...>;
	template<const int i>
	using DataType = typename std::tuple_element<i, Data>::type;
	static constexpr int size = sizeof...(Args);
	DataEvent(int type, const Args&... args): QEvent(static_cast<Type>(type)), m_data(args...) {}
	void get(Args&... args) { std::tie(args...) = m_data; }
	template<int i>
	const DataType<i> &data() const { return std::get<i>(m_data); }
private:
	Data m_data;
};

template<typename... Args>
static inline void _PostEvent(QObject *obj, int type, const Args&... args) {
	qApp->postEvent(obj, new DataEvent<Args...>(type, args...));
}

template<typename... Args>
static inline void _GetAllEventData(QEvent *event, Args&... args) {
	static_cast<DataEvent<Args...>*>(event)->get(args...);
}

template<typename... Args, const int i = 0>
static inline const typename DataEvent<Args...>::template DataType<i> &_GetEventData(QEvent *event) {
	return static_cast<DataEvent<Args...>*>(event)->template data<i>();
}


template<typename T>
bool _Change(T &value, const T &new_) { if (value == new_) return false; value = new_; return true; }

template<typename T, typename... Args> T *_New(T *&t, Args... args) { return t ? t : (t = new T(args...)); }
template<typename T, typename... Args> T *_Renew(T *&t, Args... args) {delete t; return (t = new T(args...)); }
template<typename T>                 void _Delete(T *&t) {delete t; t = nullptr; }

static inline QLatin1String _L1(const char *str) { return QLatin1String(str); }
static inline QLatin1Char _L1(char c) { return QLatin1Char(c); }

static inline void _Cut(QList<QMetaObject::Connection> &conns) {
	for (auto &c : conns) {
		if (c)
			QObject::disconnect(c);
	}
	conns.clear();
}

class Connections {
public:
	~Connections() { clear(); }
	inline Connections &operator << (const QMetaObject::Connection &connection) {
		m_connections.append(connection); return *this;
	}
	void clear() { _Cut(m_connections); }
	int size() const { return m_connections.size(); }
private:
	QList<QMetaObject::Connection> m_connections;
};

template<typename List>
void _Expand(List &list, int size, qreal s = 1.2) {
	if (list.size() < size)
		list.resize(size*s);
}

static inline bool qFuzzyCompare(const QPointF &p1, const QPointF &p2) {
	return qFuzzyCompare(p1.x(), p2.x()) && qFuzzyCompare(p1.y(), p2.y());
}

static inline bool qFuzzyCompare(const QSizeF &s1, const QSizeF &s2) {
	return qFuzzyCompare(s1.width(), s2.width()) && qFuzzyCompare(s1.height(), s2.height());
}

static inline bool _Change(QPointF &value, const QPointF &new_) {
	if (qFuzzyCompare(value, new_)) return false; value = new_; return true;
}

static inline bool _Change(QSizeF &value, const QSizeF &new_) {
	if (qFuzzyCompare(value, new_)) return false; value = new_; return true;
}

template<typename T>
static inline bool _InRange(const T &min, const T &v, const T &max) { return min <= v && v <= max; }

#endif // UTILS_HPP
