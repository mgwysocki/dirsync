// This is the main window of the application; it talks to all the sub-widgets,
// and connects the buttons of the program to the actual guts.

#ifndef __MAIN_WINDOW_H
#define __MAIN_WINDOW_H

#include <QMainWindow>

#include "NetworkClientThread.h"

class QAction;
class QMenu;
class QToolBar;
class QTableView;
class SyncModel;

class MainWindow : public QMainWindow
{
 Q_OBJECT

 public:
  MainWindow();

 public slots:
  void enable() {
    setEnabled(true);
    disconnect(&_net_thread, SIGNAL(done()), this, SLOT(enable()));
    return;
  }
  void disable() {setDisabled(true); return;}

 private slots:
  void new_sync();
  void perform_sync();
  void about();
  void display_error(const QString &message);
  void set_to_send();
  void set_to_get();
  void set_to_remote_delete();
  void set_to_local_delete();

 private:
  void delete_local_file(const FileData &);

  NetworkClientThread _net_thread;

  //DirComparator _local_changes;
  //DirComparator _remote_changes;
  QString _local_dir;
  QString _remote_dir;

  SyncModel* _sync_model;

  void createActions();
  void createMenus();

  QTableView* _view;

  QToolBar* _toolbar;
  QAction* _new_act;
  QAction* _sync_act;
  QAction* _exit_act;
  QAction* _delete_remote_act;
  QAction* _delete_local_act;
  QAction* _send_act;
  QAction* _get_act;
  QAction* _about_act;
  QAction* _about_qt_act;

  QMenu* _file_menu;
  QMenu* _help_menu;
};

#endif