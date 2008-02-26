#ifndef CLIENTDIALOG_H
#define CLIENTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QAbstractListModel>

class QDialogButtonBox;
class QLabel;
class QPushButton;
class QListView;

#include "SaveFile.h"

class ProfileReader : public QAbstractListModel
{
Q_OBJECT

 public:
  ProfileReader();
  QVariant data(const QModelIndex &index, int role) const;
  int rowCount(const QModelIndex &) const {return profile_list.size();}

  void update_profile(const QModelIndex &, QString, QString, QString);
  QModelIndex add_new_profile();

  SaveFile get_profile(const QModelIndex &) const;
  QString get_name(const QModelIndex &) const;
  QString get_client_dir(const QModelIndex &) const;
  QString get_server_dir(const QModelIndex &) const;

  void read_profiles();
  QDir profile_dir;
  QList<SaveFile> profile_list;
};



class NewSyncDialog : public QDialog
{
Q_OBJECT

 public:
  NewSyncDialog(QWidget *parent = 0);
  QString server() {return _host_lineedit->text();}
  int port() {return _port_lineedit->text().toInt();}
  QString client_dir() {return _clientdir_lineedit->text();}
  QString server_dir() {return _serverdir_lineedit->text();}
  SaveFile get_current_profile();

 private slots:
  void enable_go_button();
  void start_new_profile();
  void save_profile();
  void change_selected_profile(const QModelIndex &, const QModelIndex &);

 private:
  QLabel *_name_label;
  QLabel *_host_label;
  QLabel *_port_label;
  QLabel *_clientdir_label;
  QLabel *_serverdir_label;
  QLineEdit *_name_lineedit;
  QLineEdit *_host_lineedit;
  QLineEdit *_port_lineedit;
  QLineEdit *_clientdir_lineedit;
  QLineEdit *_serverdir_lineedit;

  QPushButton *_go_button;
  QPushButton *_new_profile_button;
  QPushButton *_update_profile_button;
  QPushButton *_quit_button;
  QDialogButtonBox *buttonBox;

  ProfileReader _profile_reader;
  QLabel *_profiles_label;
  QListView* _profiles_lv;
};

#endif
