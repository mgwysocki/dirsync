#ifndef __PROFILE_DATA_H
#define __PROFILE_DATA_H

#include "SyncData.h"

class ProfileData
{
 public:
  ProfileData();
  ProfileData(const QString &filename);

  static std::pair<QString,QString> read_dirs_from_file(const QString &);

  void load_header_info();
  bool load_sync_data();
  bool save_header_info();
  bool save_sync_data();

  QString get_filename() {return filename_;}
  quint32 get_filenumber() {return QFileInfo(filename_).baseName().remove(".dat").toUInt();}

  const QList<SyncData>& get_sync_data() {return sync_data_;}
  void set_sync_data(QList<SyncData> sd) {sync_data_ = sd;}

  QString name;
  QString local_dir;
  QString remote_dir;

 private:
  QString filename_;
  QList<SyncData> sync_data_;
};

#endif // __PROFILE_DATA_H
