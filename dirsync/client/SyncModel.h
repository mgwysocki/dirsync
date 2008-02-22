#ifndef __SYNCMODEL_H
#define __SYNCMODEL_H

#include <QAbstractTableModel>

#include "../FileData.h"
#include "../FileHandler.h"

namespace Action {
  const int Unset = 0;
  const int SendToServer = 1;
  const int GetFromServer = 2;
  const int DeleteFromServer = 3;
  const int DeleteFromClient = 4;
  const int Unclear = 5;
  const int None = 6;

  const QString strings[7] = {QString("None"),
			      QString("Copy to B"),
			      QString("Copy to A"),
			      QString("Delete from B"),
			      QString("Delete from A"),
			      QString("Unclear"),
			      QString("None")};
};


class SyncData
{
 public:
  SyncData();
  SyncData(const FileData&, const FileData&, const FileData&, const int &);
  SyncData(const SyncData &);
  ~SyncData() {}

  FileData local;
  FileData remote;

  FileData prev_local;
  FileData prev_remote;

  FileData lastsync;

  QString situ;
  int action;
};


class SyncModel : public QAbstractTableModel
{
Q_OBJECT

 public:
  SyncModel();
  int rowCount(const QModelIndex &) const {return sync_list.size();}
  int columnCount(const QModelIndex &) const {return 3;}

  QVariant data(const QModelIndex &, int) const;
  QVariant headerData(int, Qt::Orientation, int ) const;

  QList<FileData> get_files_to_send();
  QList<FileData> get_files_to_get();
  QList<FileData> get_remote_files_to_delete();
  QList<FileData> get_local_files_to_delete();
  void set_local_dir(QString d) {_local_dir = d;};
  QString get_local_dir() {return _local_dir;}
  void set_action(const QModelIndexList &, quint32);

 public slots:
  void construct(QList<FileData>);
  void save_sync_file();

 private:
  void make_changes_list();
  void make_local_list();
  void _read_dir(QString, const QDir &);
  void _compile_changes();
  bool _read_last_sync_file();
  void _make_fresh_sync_list();

  QString _local_dir;
  QList<FileData> _local_files;
  QList<FileData> _remote_files;
  QList<FileData> _sync_files;

  QList<SyncData> sync_list;

  bool changes_compiled;
  QList<FileData> _files_to_send;
  QList<FileData> _files_to_get;
  QList<FileData> _remote_files_to_delete;
  QList<FileData> _local_files_to_delete;
};

#endif
