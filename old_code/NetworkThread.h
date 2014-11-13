#ifndef NETWORKCLIENTTHREAD_H
#define NETWORKCLIENTTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QMutex>
#include <QWaitCondition>

#include "../FileData.h"
#include "../DirComparator.h"

class NetworkClientThread : public QThread
{
Q_OBJECT

 public:
  NetworkClientThread(QObject*);
  ~NetworkClientThread();

  void set_server(QString s) {std::cout << "NetworkClientThread::set_server(" << qPrintable(s) << ")" << std::endl; _server = s;}
  void set_port(int p) {std::cout << "NetworkClientThread::set_port(" << p << ")" << std::endl; _port = p;}
  void set_mode(quint32);
  void reset_server();
  void send_files(QList<FileData>);
  void run();

 signals:
  void done();
  void error(const QString &message);
  void progress(int);

/*  public slots: */
/*   void read_data(); */

 private:
  void _connect_socket();
  void _disconnect_socket();
  void _get_remote_filelist();
  void _send_files();
  void _get_files();
  void _delete_remote_files();
  void _delete_local_files();

  QTcpSocket *_socket;
  quint32 _packet_size;
  QString _server;
  int _port;
  QString _server_dir;
  QString _dir;

  QByteArray buffer;

  QMutex _mutex;
  QWaitCondition _cond;
  quint32 _mode;
  bool _quit;

  QList<FileData> _remote_filelist;
  QList<FileData> _files_to_send;
  QList<FileData> _files_to_get;
  QList<FileData> _remote_files_to_delete;
  QList<FileData> _local_files_to_delete;

  QWidget* _main_window;
};

#endif
