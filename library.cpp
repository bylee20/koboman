#include "library.hpp"
#include "utility.hpp"
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QRegularExpression>
#include <QDebug>
#include <QStringBuilder>
#include <QTextStream>

static QList<BookColumn> makeFieldInfo() {
	QList<BookColumn> fields;
	fields << BookColumn{Book::RegNo, "등록번호", "reg_no", "TEXT UNIQUE"};
	fields << BookColumn{Book::Title, "서명", "title", "TEXT"};
	fields << BookColumn{Book::Author, "저자", "author", "TEXT"};
	fields << BookColumn{Book::Publisher, "출판사", "publisher", "TEXT"};
	fields << BookColumn{Book::Year, "출판년도", "year", "TEXT"};
	fields << BookColumn{Book::CallNo, "청구기호", "call_no", "TEXT"};
	fields << BookColumn{Book::State, "도서상태", "state", "TEXT"};
	fields << BookColumn{Book::RegDate, "등록일", "reg_date", "TEXT"};
	return fields;
}

const QList<BookColumn> BookColumn::m_info = makeFieldInfo();

static QString toSql(const QString &text) {
	return '\'' % QString(text).replace('\'', "''") % '\'';
}

template<typename T> static T getter(const T &t) { return t; }

template<typename List, typename Get = decltype(getter<typename List::value_type>)>
static QString toVector(const List &list, Get get = getter<typename List::value_type>) {
	QString vector;
	for (auto &one : list)
		vector += get(one) % QLatin1String(", ");
	vector.chop(2);
	return vector;
}

struct Library::Data {
	QSqlDatabase db;
	QSqlQuery query;
	bool importing = false;
};


Library::Library(QObject *parent): QObject(parent), d(new Data) {
	d->db = QSqlDatabase::addDatabase("QSQLITE");
	d->db.setDatabaseName(Utility::storage() % "/books.db");
	d->db.open();
	Q_ASSERT(d->db.isOpen());
	d->query = QSqlQuery(d->db);
}

Library::~Library() {
	delete d;
}

Library &Library::get() {
	static Library obj;
	return obj;
}

bool Library::isImporting() const {
	return d->importing;
}

QStringList Library::find(const QString &registrationNumber, const QList<BookColumn> &columns) const {
	QStringList ret; ret.reserve(columns.size());
	d->query.exec("SELECT " % toVector(columns, [] (const BookColumn &c) { return c.name(); })
				  % " FROM books where reg_no = " % toSql(registrationNumber));
	if (d->query.next()) {
		for (int i=0; i<columns.size(); ++i)
			ret.append(d->query.value(i).toString());
	} else {
		for (int i=0; i<columns.size(); ++i)
			ret.append(QString());
	}
	return ret;
}

void Library::import_() {
	if (d->importing)
		return;
	d->importing = true;
	emit importStarted();
	emit importingChanged();
	auto thread = new ImportThread(d->db);
	connect(thread, &QThread::finished, this, &Library::onImportThreadFinished, Qt::QueuedConnection);
	connect(thread, &QThread::finished, thread, &QObject::deleteLater);
	thread->start();
}

void Library::onImportThreadFinished() {
	d->importing = false;
	emit importFinished();
	emit importingChanged();
}

struct ImportThread::Data {
	QSqlDatabase db;
};

ImportThread::ImportThread(const QSqlDatabase &db, QObject *parent)
: QThread(parent), d(new Data) {
	d->db = db;
}

ImportThread::~ImportThread() {
	delete d;
}

void ImportThread::run() {
	QFile file(Utility::storage() % "/books.csv");
	if (!file.open(QFile::ReadOnly))
		return;

	QTextStream in(&file);
	in.setCodec("CP949");
	QStringList header;

	auto read = [&in, &header] () {
		QStringList list;
		list.reserve(header.size());
		list.append(QString());
		bool qq = false;
		while (!in.atEnd()) {
			auto line = in.readLine();
			for (int i=0; i<line.size(); ++i) {
				auto ch = line[i];
				switch (ch.unicode()) {
				case '"':
					if (!qq) {
						qq = true;
					} else {
						if (i+1 < line.size() && line[i+1].unicode() == '"') {
							list.last() += ch;
							++i;
						} else
							qq = false;
					}
					break;
				case ',':
					if (!qq)
						list.append(QString());
					else
						list.last() += ch;
					break;
				default:
					list.last() += ch;
				}
			}
			if (!qq)
				break;
		}
		return list;
	};

	while (!in.atEnd()) {
		header = read();
		if (header.contains(BookColumn::display(Book::RegNo)))
			break;
	}

	auto fields = BookColumn::list();
	QVector<int> imports; imports.resize(fields.size());
	for (int i=0; i<fields.size(); ++i) {
		auto &field = fields[i];
		imports[i] = header.indexOf(field.display());
		Q_ASSERT(imports[i] != -1);
		Q_ASSERT(field.field() == i);
	}

	QSqlQuery query(d->db);

	d->db.transaction();
	query.exec("DROP TABLE IF EXISTS books");
	query.exec("CREATE TABLE books (" % toVector(fields, [] (const BookColumn &c) { return QString(c.name() % ' ' % c.type()); }) % ')');

	while (!in.atEnd()) {
		const auto row = read();
		Q_ASSERT(row.size() == header.size());
		if (row[imports[Book::RegNo]].isEmpty())
			continue;
		query.exec("INSERT INTO books VALUES (" % toVector(imports, [&row] (int i) { return toSql(row[i]); }) % ')');
	}
	d->db.commit();
}
