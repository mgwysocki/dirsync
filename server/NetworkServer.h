#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include <QMutex>
#include <QTcpServer>
#include <QTcpSocket>
#include "../FileData.h"
#include "../FileHandler.h"

class NetworkServer : public QObject
{
Q_OBJECT

 public:
  NetworkServer(int port, QObject* parent = 0);
  ~NetworkServer();

 signals:
  void done();
  void error(const QString &);

 public slots:
  void new_connection();
  void read_incoming();
  void socket_disconnected();

 private:
  void _disconnect_socket();
  void _make_local_file_list();
  void _read_dir(QString, const QDir &);
  void _send_list( const QList<FileData> &);
  void _send_file(const FileData &);
  void _receive_file(const FileData &);
  void _delete_local_files(QList<FileData> &);

  QTcpServer* _server;
  QTcpSocket* _socket;
  int _port;
  quint32 _packet_size;
  quint32 _block_size;
  QString _dir;

  QByteArray buffer;
  quint32 _max_buffer_size;

  QMutex _mutex;
  bool _stop;
  bool _quit;

  QList<FileData> _current_files_list;
};

#endif
