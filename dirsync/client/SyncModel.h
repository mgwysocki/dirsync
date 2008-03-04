#ifndef __SYNCMODEL_H
#define __SYNCMODEL_H

#include <QAbstractTableModel>
#include <QItemSelection>

#include "../FileAgent.h"
#include "../FileHandler.h"
#include "../FileData.h"
#include "DirData.h"

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
  SyncData(const FileAgent&, const FileAgent&, bool);
  SyncData(const SyncData &);
  ~SyncData() {}

  void determine_situation_first_time();
  void determine_situation();

  QString relative_filename;

  FileAgent local;
  FileAgent remote;

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

  void set_action(const QModelIndexList &, quint32);

  QList<FileData> get_files_to_send();
  QList<FileData> get_files_to_get();
  QList<FileData> get_remote_files_to_delete();
  QList<FileData> get_local_files_to_delete();

  QString get_local_dir() {return _local_dir;}
  void set_local_dir(QString d) {_local_dir = d; _local_dirdata.set_dir(d);}
  void make_local_list();
  void reset();

 signals:
  void set_info(QString, QString);
  
 public slots:
  void set_remote_filelist(QList<FileData>);
  void save_sync_file();
  void selection_changed(QItemSelection, QItemSelection);

 private:
  void make_changes_list();
  void _read_dir(QString, const QDir &);
  void _compile_changes();
  bool _read_last_sync_file();
  void _make_fresh_sync_list();

  bool _loaded_previous_state;
  QString _local_dir;
  DirData _local_dirdata;
  DirData _remote_dirdata;

  QList<SyncData> sync_list;

  bool changes_compiled;
  QList<FileData> _files_to_send;
  QList<FileData> _files_to_get;
  QList<FileData> _remote_files_to_delete;
  QList<FileData> _local_files_to_delete;
};

#endif
