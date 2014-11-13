#ifndef CLIENTDIALOG_H
#define CLIENTDIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QFile>

#include "../FileData.h"
#include "../DirComparator.h"

class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;
class QRadioButton;

class ClientDialog : public QDialog
{
Q_OBJECT

 public:
  ClientDialog(QWidget *parent = 0);

  void make_local_changes_list();
  void sync();

 private slots:
  void connect_socket();
  void get_remote_changes_list();
  void read_data();
  void display_error(QAbstractSocket::SocketError socketError);
  void enable_make_button();

 private:
  void send_file();
  void get_file();
  void delete_remote_file();
  void delete_local_file();

  void _start_file(QString filename);
  void _end_file();
  
  QString _synclist_filename;

  QString _current_filename;
  QFile _current_file;
  quint32 _current_filesize;
  quint32 _current_filetime;

  QLabel *_host_label;
  QLabel *_port_label;
  QLabel *_clientdir_label;
  QLabel *_serverdir_label;
  QLineEdit *_host_lineedit;
  QLineEdit *_port_lineedit;
  QLineEdit *_clientdir_lineedit;
  QLineEdit *_serverdir_lineedit;

  QLabel *_status_label;
  QPushButton *_make_button;
  QPushButton *_quit_button;
  QDialogButtonBox *buttonBox;

  QTcpSocket *_socket;
  QString currentFortune;
  quint32 _block_size;
};

#endif
