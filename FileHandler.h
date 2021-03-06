#ifndef FILEHANDLER_H
#define FILEHANDLER_H

//-----------------------------------------------------------------
// The FileHandler class is the interface for reading & writing files
// to disks and calculating MD5 sums.
// -----------------------------------------------------------------

#include <QObject>
#include <string>

#include "FileData.h"
#include "md5/md5.h"

class QTcpSocket;

class FileHandler : QObject
{
Q_OBJECT

 public:
  FileHandler();
  FileHandler(const QString &);
  FileHandler(const FileData &);
  ~FileHandler();

  static QString get_temp_filename();
  static bool get_override_permissions() {return _override_perms;}
  static void set_override_permissions(bool b) {_override_perms = b;}

  bool begin_file_write();
  void write_to_file(const QByteArray &);
  void end_file_write(const QString);

  quint16 get_checksum();
  FileData& get_fd() {return _fd;}


 signals:
  void error(QString);

 private:
  void _load_file_data(const QString &);

  static bool _override_perms;

  FileData _fd;
  QFile _file;
  bool _isopen;
  md5 _md5;
};

#endif
