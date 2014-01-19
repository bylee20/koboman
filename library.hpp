#ifndef LIBRARY_HPP
#define LIBRARY_HPP

#include <QThread>
#include <QVector>

class Book {
public:
	enum Field {
		RegNo, Title, Author, Publisher, Year, CallNo, State, RegDate, FieldCount
	};
	Book() { m_data.resize(FieldCount); }
	void set(Field field, const QString &data) { m_data[field] = data; }
	QString get(Field field) const { return m_data[field]; }
	QString registrationNumber() const { return m_data[RegNo]; }
	QString title() const { return m_data[Title]; }
	QString author() const { return m_data[Author]; }
	QString publisher() const { return m_data[Publisher]; }
	QString year() const { return m_data[Year]; }
	QString callNumber() const { return m_data[CallNo]; }
	QString state() const { return m_data[State]; }
	QString registrationDate() const { return m_data[RegDate]; }
private:
	QVector<QString> m_data;
	friend class Library;
};

class BookColumn {
public:
	BookColumn() {}
	BookColumn(Book::Field field, const char *display, const char *name, const char *type)
	: m_field(field), m_name(QString::fromUtf8(name)), m_display(QString::fromUtf8(display)), m_type(QString::fromLatin1(type)) {}
	static const QList<BookColumn> &list() { return m_info; }
	static QString display(Book::Field column) { return m_info[column].m_display; }
	QString name() const { return m_name; }
	QString display() const { return m_display; }
	QString type() const { return m_type; }
	Book::Field field() const { return (Book::Field)m_field; }
	static const BookColumn &get(Book::Field field) { return m_info[field]; }
//	QStringList names()
private:
	int m_field = -1;
	QString m_name, m_display, m_type;
	static const QList<BookColumn> m_info;
};

class Library : public QObject {
	Q_OBJECT
	Q_PROPERTY(bool importing READ isImporting NOTIFY importingChanged)
public:
	~Library();
	Book find(const QString &registrationNumber) const;
	QStringList find(const QString &registrationNumber, const QList<BookColumn> &columns) const;
	Q_INVOKABLE void import_();
	static Library &get();
	bool isImporting() const;
signals:
	void importStarted();
	void importFinished();
	void importingChanged();
private slots:
	void onImportThreadFinished();
private:
	Library(QObject *parent = nullptr);
	struct Data;
	Data *d;
};

class QSqlDatabase;

class ImportThread : public QThread {
	Q_OBJECT
public:
	ImportThread(const QSqlDatabase &db, QObject *parent = nullptr);
	~ImportThread();
	void run() override;
private:
	struct Data;
	Data *d;
};

#endif // LIBRARY_HPP
