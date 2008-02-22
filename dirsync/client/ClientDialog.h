#ifndef CLIENTDIALOG_H
#define CLIENTDIALOG_H

#include <QDialog>
#include <QLineEdit>

class QDialogButtonBox;
class QLabel;
class QPushButton;

class ClientDialog : public QDialog
{
Q_OBJECT

 public:
  ClientDialog(QWidget *parent = 0);
  QString server() {return _host_lineedit->text();}
  int port() {return _port_lineedit->text().toInt();}
  QString client_dir() {return _clientdir_lineedit->text();}
  QString server_dir() {return _serverdir_lineedit->text();}

 private slots:
  void enable_go_button();

 private:
  QLabel *_host_label;
  QLabel *_port_label;
  QLabel *_clientdir_label;
  QLabel *_serverdir_label;
  QLineEdit *_host_lineedit;
  QLineEdit *_port_lineedit;
  QLineEdit *_clientdir_lineedit;
  QLineEdit *_serverdir_lineedit;

  QLabel *_status_label;
  QPushButton *_go_button;
  QPushButton *_quit_button;
  QDialogButtonBox *buttonBox;
};

#endif
