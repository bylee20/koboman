#ifndef BOOKLISTMODEL_HPP
#define BOOKLISTMODEL_HPP

#include <QThread>
#include <QAbstractListModel>
#include <QStringBuilder>
#include <QQmlListProperty>
#include "barcode.hpp"

class BookListModel : public QAbstractListModel {
	Q_OBJECT
	Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
	Q_PROPERTY(int count READ count NOTIFY countChanged)
	Q_PROPERTY(QString lastWorkingFile READ lastWorkingFile)
public:
	enum Status { Good, Unregistered, OutOfOrder, StatusCount };
	enum Role { RegistrationNumberRole = Qt::UserRole + 1, StatusRole, CallNumber, Title, Author };
	BookListModel(QObject *parent = nullptr);
	~BookListModel();
	Q_INVOKABLE bool append(const QString &registrationNumber);
	Q_INVOKABLE void remove(int row);
	Q_INVOKABLE void modify(int row, const QString &registrationNumber);
	Q_INVOKABLE QString registrationNumber(int row) const;
	Q_INVOKABLE void load(const QString &fileName);
	Q_INVOKABLE bool append(BarcodeObject *barcode);
	Q_INVOKABLE QStringList getFileList() const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QHash<int, QByteArray> roleNames() const override;
	bool save() const;
	int count() const;
	QString lastWorkingFile() const;
	bool isLoading() const;
signals:
	void messageNotified(const QString &message);
	void countChanged(int count);
	void loadingChanged();
	void loadingStarted();
	void loadingFinished();
private slots:
	void onLoadThreadFinished();
	void emitCountChanged() { emit countChanged(count()); }
private:
	friend class LoadThread;
	void open();
	void notify(const QString &number, const QString &message) {
		emit messageNotified(number % ": " % message);
	}
	struct Data;
	struct Row;
	QSharedPointer<BookListModel::Data> m_data;
	Data *d;
};

class LoadThread : public QThread {
	Q_OBJECT
public:
	LoadThread(const QString &fileName, BookListModel *model);

private:
	void run();
	QString m_fileName;
	BookListModel *m_model;
	QSharedPointer<BookListModel::Data> m_data;
};

#endif // BOOKLISTMODEL_HPP
