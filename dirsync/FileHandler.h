#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QObject>
#include "../FileData.h"

class QTcpSocket;

class FileHandler : QObject
{
Q_OBJECT

 public:
  FileHandler();
  FileHandler(const QString &);
  FileHandler(const FileData &);

  static FileData get_fd_from_socket(QTcpSocket*);
  static void send_fd_to_socket(const FileData&, QTcpSocket*);

  bool begin_file_write();
  void write_to_file(const QByteArray &);
  void end_file_write();

  quint16 get_checksum();
  FileData& get_fd() {return _fd;}

 signals:
  void error(QString);

 private:
  void _load_file_data(const QString &);

  FileData _fd;
  QFile _file;
  bool _isopen;
};

#endif
