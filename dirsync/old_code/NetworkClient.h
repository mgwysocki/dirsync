#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QTcpSocket>
#include "../FileData.h"
#include "../DirComparator.h"

class NetworkClient : public QObject
{
Q_OBJECT

 public:
  NetworkClient(QWidget*);
  ~NetworkClient();

  void set_server(QString s) {std::cout << "NetworkClient::set_server(" << qPrintable(s) << ")" << std::endl; _server = s;}
  void set_port(int p) {std::cout << "NetworkClient::set_port(" << p << ")" << std::endl; _port = p;}
  void reset_server();
  void send_files(QList<FileData>);

 signals:
  void all_files_sent();

 public slots:
  void connect_socket();
  void disconnect_socket();
  void get_remote_changes(DirComparator &);
  void display_error(QAbstractSocket::SocketError);
  void get_remote_file(const FileData &, const FileData &);
  void send_local_file(const FileData &);
  void delete_remote_file(const FileData &);
  void read_data();

 private:
  QTcpSocket *_socket;
  quint32 _block_size;
  QString _server;
  int _port;

  void send_next_file();
  QList<FileData> _files_to_send;
  
  QWidget* _main_window;
};

#endif
