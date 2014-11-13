#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include "../FileData.h"
#include "../DirComparator.h"

class QLabel;
class QPushButton;
class QTcpServer;
class QTcpSocket;

class Server : public QDialog
{
Q_OBJECT

 public:
  Server(QWidget *parent = 0);

 private slots:
  void read_incoming();
  void send_data();
  void socket_disconnected();

 private:
  void make_local_changes_list();
  void send_list( const QList<FileData> &);
  void send_file(const FileData &);
  void receive_file(const FileData &);

  QLabel* _status_label;
  QPushButton* _quit_button;

  QTcpServer* _server;
  QTcpSocket* _socket;
  QString _filename;  

  DirComparator _local_changes;
};

#endif
