#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include "../FileData.h"
#include "NetworkServerThread.h"

class QLabel;
class QPushButton;
class QTcpServer;
class QTcpSocket;

class ServerDialog : public QDialog
{
Q_OBJECT

 public:
  ServerDialog(QWidget *parent = 0);

 private:
  QLabel* _status_label;
  QPushButton* _quit_button;
  NetworkServerThread _net_thread;
};

#endif
