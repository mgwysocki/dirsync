#ifndef __SAVEFILE_H
#define __SAVEFILE_H

#include "../FileData.h"

class SaveFile
{
 public:
  SaveFile();
  SaveFile(const QString &filename);

  static std::pair<QString,QString> read_dirs_from_file(const QString &);

  void load_header_info();
  bool load_from_file(QString filename = QObject::tr(""));
  bool save_to_file(QString filename = QObject::tr(""));

  QString get_filename() {return _filename;}
  quint32 get_filenumber() {return QFileInfo(_filename).baseName().remove(".dat").toUInt();}

  const QList<FileData>& get_local_filedata() {return _local_files;}
  const QList<FileData>& get_remote_filedata() {return _remote_files;}

  QString name;
  QString local_dir;
  QString remote_dir;

 private:
  QString _filename;
  QList<FileData> _local_files;
  QList<FileData> _remote_files;
};

#endif
