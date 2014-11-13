#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include "NetworkServerThread.h"

class QLabel;
class QPushButton;

class ServerDialog : public QDialog
{
Q_OBJECT

 public:
  ServerDialog(int port, QWidget *parent = 0);

 private:
  QLabel* _status_label;
  QPushButton* _quit_button;
  //  NetworkServerThread _net_thread;
};

#endif
