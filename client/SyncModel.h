#ifndef __SYNCMODEL_H
#define __SYNCMODEL_H

#include <QAbstractTableModel>
#include <QItemSelection>

#include "../FileAgent.h"
#include "../FileHandler.h"
#include "../FileData.h"
#include "DirData.h"
#include "ProfileData.h"

class SyncModel : public QAbstractTableModel
{
Q_OBJECT

 public:
  SyncModel();
  ~SyncModel() {if(profile_data_) delete profile_data_;}

  int rowCount(const QModelIndex &) const {return _diff_only ? diff_list.size() : sync_list.size();}
  int columnCount(const QModelIndex &) const {return 4;}

  QVariant data(const QModelIndex &, int) const;
  QVariant headerData(int, Qt::Orientation, int ) const;
  QModelIndex parent(const QModelIndex &) const {return QModelIndex();}

  void set_action(const QModelIndexList &, quint32);
  void set_sync_to_client(const QModelIndexList &);
  void set_sync_to_server(const QModelIndexList &);

  QList<FileData> get_files_to_send();
  QList<FileData> get_files_to_get();
  QList<FileData> get_remote_files_to_delete();
  QList<FileData> get_local_files_to_delete();

  void set_profile_data(ProfileData pd) {
    if(profile_data_) delete profile_data_;
    profile_data_ = new ProfileData(pd);
  }

  QString get_local_dir() {return _local_dir;}
  void set_local_dir(QString d) {_local_dir = d; _local_dirdata.set_dir(d);}
  void make_local_list();
  void reset();

  bool get_diff_only() const {return _diff_only;}

  qint64 get_size_to_send() {
    qint64 total(0);
    for(int i=0; i<_files_to_send.size(); i++)
      total += _files_to_send[i].size;
    return total;
  }

  qint64 get_size_to_get() {
    qint64 total(0);
    for(int i=0; i<_files_to_get.size(); i++)
      total += _files_to_get[i].size;
    return total;
  }

 signals:
  void set_info(QString, QString);
  
 public slots:
  void set_remote_filelist(QList<FileData>);
  void save_sync_file();
  //void selection_changed(QItemSelection, QItemSelection);
  void selection_changed(const QModelIndex &, const QModelIndex);
  void set_diff_only(const bool);

 private:
  void make_changes_list();
  void _read_dir(QString, const QDir &);
  void _compile_changes();
  bool read_sync_file();
  void _make_fresh_sync_list();
  void _generate_diff_list();

  bool _loaded_previous_state;
  QString _local_dir;
  DirData _local_dirdata;
  DirData _remote_dirdata;

  ProfileData* profile_data_;

  QList<SyncData> sync_list;
  QList<SyncData*> diff_list;

  bool _diff_only;
  bool changes_compiled;
  QList<FileData> _files_to_send;
  QList<FileData> _files_to_get;
  QList<FileData> _remote_files_to_delete;
  QList<FileData> _local_files_to_delete;
};

#endif
