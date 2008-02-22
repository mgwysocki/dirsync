#ifndef NETWORKCLIENTTHREAD_H
#define NETWORKCLIENTTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QMutex>
#include <QWaitCondition>
class QProgressDialog;

#include "../FileData.h"
//#include "../DirComparator.h"

namespace ClientMode {
  const quint32 None = 0;
  const quint32 Reading = 1;
  const quint32 Syncing = 2;
};


class NetworkClientThread : public QThread
{
Q_OBJECT

 public:
  NetworkClientThread(QObject* parent = 0);
  ~NetworkClientThread();

  void set_server(QString s) {std::cout << "NetworkClientThread::set_server(" << qPrintable(s) << ")" << std::endl; _server = s;}
  void set_port(int p) {std::cout << "NetworkClientThread::set_port(" << p << ")" << std::endl; _port = p;}
  void set_mode(quint32);
  QList<FileData> get_remote_filelist();
  void set_files_to_send(const QList<FileData>);
  void set_files_to_get(const QList<FileData>);
  void set_remote_files_to_delete(const QList<FileData>);
  void set_local_files_to_delete(const QList<FileData>);
  void set_remote_dir(const QString &);
  void set_local_dir(const QString &);
  void set_progress_dialog(QProgressDialog* pd) {_pd=pd;}
  int get_size_of_files_to_send();
  int get_size_of_files_to_get();
  void reset_server();
  void reset();
  void run();

 signals:
  void got_filelist(QList<FileData>);
  void done();
  void success();
  void error(const QString &message);
  void size_sent(int);
  void size_received(int);
  void change_text(QString);

 public slots:
  void wake_up();

 private:
  bool _connect_socket();
  void _disconnect_socket();
  void _get_remote_filelist();
  bool _send_files();
  bool _get_files();
  void _delete_remote_files();
  void _delete_local_files();

  QTcpSocket *_socket;
  quint32 _packet_size;
  QString _server;
  int _port;
  QString _server_dir;
  QString _dir;

  QProgressDialog* _pd;
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
