#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QObject>
#include <string>

#include "../FileData.h"
#include "../md5/md5.h"

class QTcpSocket;

class FileHandler : QObject
{
Q_OBJECT

 public:
  FileHandler();
  FileHandler(const QString &);
  FileHandler(const FileData &);
  ~FileHandler();

  static FileData get_fd_from_socket(QTcpSocket*);
  static void send_fd_to_socket(const FileData&, QTcpSocket*);
  static QString get_temp_filename();
  static bool get_override_permissions() {return _override_perms;}
  static void set_override_permissions(bool b) {_override_perms = b;}

  bool begin_file_write();
  void write_to_file(const QByteArray &);
  void end_file_write();

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
