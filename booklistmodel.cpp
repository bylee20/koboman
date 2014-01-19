#include "booklistmodel.hpp"
#include "utility.hpp"
#include "library.hpp"
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QSettings>

struct BookListModel::Row {
	QString number;
	Status status = Good;
	QStringList data;
	QString get(int role) const {
		const int index = role - CallNumber;
		Q_ASSERT(0 <= index && index < data.size());
		return data[index];
	}
};

struct BookListModel::Data {
	~Data() { close(); }

	const QList<BookColumn> columns = {
		BookColumn::get(Book::CallNo),
		BookColumn::get(Book::Title),
		BookColumn::get(Book::Author)
	};

	QList<Row> rows;

	bool loading = false;

	Row makeRow(const QString &number) const {
		Row row;
		row.number = number;
		row.data = Library::get().find(number, columns);
		return row;
	}

	void updateStatus(int r) {
		auto &row = rows[r];
		if (row.data.first().isEmpty())
			row.status = Unregistered;
		else if (!rows.isEmpty() && r > 0 && row.number <= rows[r-1].number)
			row.status = OutOfOrder;
		else
			row.status = Good;
	}

	bool checkFormat(const Barcode &barcode) const {
		if (barcode.type != Barcode::CODE_39 || barcode.text.size() < 6)
			return false;
		bool digit = false;
		auto check = [] (ushort c, ushort min, ushort max) {
			return min <= c && c <= max;
		};
		for (auto &qchar : barcode.text) {
			const auto c = qchar.unicode();
			if (check(c, '0', '9')) {
				if (!digit)
					digit = true;
			} else if (check(c, 'A', 'Z')) {
				if (digit)
					return false;
			} else
				return false;
		}
		return true;
	}
	void append(const QString &number) {
		auto row = makeRow(number);
		rows.append(row);
		updateStatus(rows.size()-1);
	}

	bool open(const QString &fileName) {
		if (file.isOpen() || fileName.isEmpty())
			return false;
		file.setFileName(Utility::storage() % "/" % fileName);
		if (!file.open(QFile::ReadWrite)) {
			qDebug() << "Cannot open file:" << file.fileName();
			return false;
		}
		stream.setDevice(&file);
		rows.clear();
		while (!stream.atEnd()) {
			auto number = stream.readLine();
			if (!number.isEmpty())
				append(number);
		}
		modified = false;
		qDebug() << file.fileName() << "opened!";
		lastWorkingFile = fileName;
		settings->setValue("lastWorkingFile", lastWorkingFile);
		settings->sync();
		return true;
	}
	void close() {
		if (file.isOpen()) {
			stream.setDevice(nullptr);
			file.close();
		}
	}

	QFile file;
	QTextStream stream;
	QString lastWorkingFile;
	bool modified = true;
	QVector<QString> statusText;
	QSettings *settings;
};

BookListModel::BookListModel(QObject *parent)
: QAbstractListModel(parent), m_data(new Data), d(m_data.data()) {
	d->statusText.resize(StatusCount);
	d->statusText[Good] = trUtf8("");
	d->statusText[Unregistered] = trUtf8("등록안됨");
	d->statusText[OutOfOrder] = trUtf8("오배가");
	d->settings = new QSettings(Utility::storage() % "/config.ini", QSettings::IniFormat);

	d->lastWorkingFile = d->settings->value("lastWorkingFile").toString();
	QDir dir(Utility::storage());
	Q_ASSERT(dir.exists());
	if (!d->lastWorkingFile.isEmpty() && !dir.exists(d->lastWorkingFile))
		d->lastWorkingFile.clear();
	connect(this, &BookListModel::modelReset, this, &BookListModel::emitCountChanged);
	connect(this, &BookListModel::rowsInserted, this, &BookListModel::emitCountChanged);
	connect(this, &BookListModel::rowsRemoved, this, &BookListModel::emitCountChanged);
}

BookListModel::~BookListModel() { }

QString BookListModel::lastWorkingFile() const {
	return d->lastWorkingFile;
}

QString BookListModel::registrationNumber(int row) const {
	return d->rows.value(row).number;
}

void BookListModel::load(const QString &fileName) {
	if (d->loading)
		return;
	d->loading = true;
	emit loadingStarted();
	emit loadingChanged();
	beginResetModel();
	auto thread = new LoadThread(fileName, this);
	connect(thread, &QThread::finished, this, &BookListModel::onLoadThreadFinished, Qt::QueuedConnection);
	connect(thread, &QThread::finished, thread, &QObject::deleteLater);
	thread->start();
}

bool BookListModel::isLoading() const {
	return d->loading;
}

void BookListModel::onLoadThreadFinished() {
	d->loading = false;
	emit loadingChanged();
	emit loadingFinished();
	endResetModel();
}

void BookListModel::open() {
	d->close();
	beginResetModel();
	d->open(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss-zzz") % ".txt");
	endResetModel();
}

int BookListModel::rowCount(const QModelIndex &parent) const {
	return parent.isValid() ? 0 : d->rows.size();
}

QVariant BookListModel::data(const QModelIndex &index, int role) const {
	const int i = index.row();
	const int column = index.column();
	if (!(0 <= i && i < d->rows.size() && column == 0))
		return QVariant();
	const auto &row = d->rows[i];
	switch (role) {
	case Qt::DisplayRole:
	case RegistrationNumberRole:
		return row.number;
	case StatusRole:
		return d->statusText[row.status];
	case CallNumber:
	case Title:
	case Author:
		return row.get(role);
	default:
		return QVariant();
	}
}

QHash<int, QByteArray> BookListModel::roleNames() const {
	QHash<int, QByteArray> hash;
	hash.insert(RegistrationNumberRole, "registrationNumber");
	hash.insert(CallNumber, "callNumber");
	hash.insert(Title, "title");
	hash.insert(Author, "author");
	hash.insert(StatusRole, "status");
	return hash;
}

bool BookListModel::append(BarcodeObject *obj) {
	const auto barcode = obj->barcode();
	if (!d->checkFormat(barcode)) {
		notify(barcode.text, trUtf8("잘못된 형식입니다"));
		return false;
	} else
		return append(barcode.text);
}

bool BookListModel::append(const QString &number) {
	if (!d->rows.isEmpty() && d->rows.last().number == number) {
//		notify(number, trUtf8("이미 존재합니다."));
		return true;
	} else {
		beginInsertRows(QModelIndex(), d->rows.size(), d->rows.size() + (1) - 1);
		d->append(number);
		endInsertRows();
		notify(number, trUtf8("추가되었습니다."));
		if (!d->modified) {
			if (!d->file.isOpen())
				open();
			while (!d->stream.atEnd())
				d->stream.readAll();
			d->stream << number << endl;
		} else
			save();
		return true;
	}
}

void BookListModel::remove(int row) {
	if (0 <= row && row < d->rows.size()) {
		beginRemoveRows(QModelIndex(), row, row);
		const auto r = d->rows.takeAt(row);
		endRemoveRows();
		notify(r.number, trUtf8("삭제되었습니다."));
		d->modified = true;
		save();
	}
}

void BookListModel::modify(int row, const QString &number) {
	if (0 <= row && row < d->rows.size() && d->rows[row].number != number) {
		const auto old = d->rows[row].number;
		d->rows[row] = d->makeRow(number);
		d->updateStatus(row);
		emit dataChanged(index(row), index(row));
		notify(old, trUtf8("%1로 수정되었습니다.").arg(number));
		d->modified = true;
		save();
	}
}

int BookListModel::count() const {
	return d->rows.size();
}

bool BookListModel::save() const {
	if (!d->modified)
		return true;
	d->close();
	if (!d->file.open(QFile::ReadWrite | QFile::Truncate))
		return false;
	d->stream.setDevice(&d->file);
	for (auto &row : d->rows)
		d->stream << row.number << '\n';
	d->stream.flush();
	return true;
}

QStringList BookListModel::getFileList() const {
	QDir dir(Utility::storage());
	Q_ASSERT(dir.exists());
	return dir.entryList(QStringList() << "*.txt", QDir::Files, QDir::Name);
}

LoadThread::LoadThread(const QString &fileName, BookListModel *model)
: QThread(model), m_fileName(fileName), m_model(model), m_data(model->m_data) {

}

void LoadThread::run() {
	m_data->open(m_fileName);
}
