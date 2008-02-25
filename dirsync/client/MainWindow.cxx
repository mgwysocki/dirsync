#include <QtGui>

#include <iostream>
using namespace std;

#include "ClientDialog.h"
#include "SyncModel.h"
#include "MainWindow.h"


MainWindow::MainWindow() :
  QMainWindow(),
  _sync_model(new SyncModel),
  _view(new QTableView),
  _toolbar(new QToolBar)
{
  _view->setModel(_sync_model);
  _view->setSelectionBehavior(QAbstractItemView::SelectRows);
  _view->horizontalHeader()->setResizeMode(0, QHeaderView::Interactive);
  _view->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
  _view->horizontalHeader()->setResizeMode(2, QHeaderView::ResizeToContents);
  _view->horizontalHeader()->setHighlightSections(false);
  this->setCentralWidget(_view);
  
  this->addToolBar(_toolbar);
  createActions();
  createMenus();
  
  setWindowTitle(tr("DirSync Client"));
  QSize maxsize(QApplication::desktop()->size());
  maxsize += QSize(-50, -50);
  setMaximumSize(maxsize);
  resize(800, 600);

  new_sync();
}

void MainWindow::new_sync()
{
  //cout << "MainWindow::new_sync()" << endl;
  ClientDialog cd(this);
  int value = cd.exec();
  if(value == QDialog::Rejected)
    return;

  _net_thread.reset();

  //cout << "MainWindow: setting the server, port, and dirs..." << endl;
  _local_dir = ( cd.client_dir() );
  _remote_dir = ( cd.server_dir() );
  _sync_model->set_local_dir(_local_dir);

  _net_thread.set_server( cd.server() );
  _net_thread.set_port( cd.port() );
  _net_thread.set_local_dir( _local_dir );
  _net_thread.set_remote_dir( _remote_dir );
  _net_thread.reset_server();

  this->disable();
  connect(&_net_thread, SIGNAL(got_filelist(QList<FileData>)), 
	  _sync_model, SLOT(construct(QList<FileData>)));
  connect(&_net_thread, SIGNAL(done()), 
	  this, SLOT(enable()));
  _net_thread.set_mode(ClientMode::Reading);
  _net_thread.start();
  return;
}


void MainWindow::perform_sync()
{
  cout << "MainWindow::perform_sync()" << endl;

  _net_thread.set_files_to_send( _sync_model->get_files_to_send() );
  _net_thread.set_files_to_get( _sync_model->get_files_to_get() );
  _net_thread.set_remote_files_to_delete( _sync_model->get_remote_files_to_delete() );
  _net_thread.set_local_files_to_delete( _sync_model->get_local_files_to_delete() );
  
  QProgressDialog* pd = new QProgressDialog("", "Cancel", 0, 1, this);
  pd->show();
  _net_thread.set_progress_dialog(pd);

  connect(&_net_thread, SIGNAL(success()), _sync_model, SLOT(save_sync_file()));
  _net_thread.set_mode(ClientMode::Syncing);
  _net_thread.wake_up();
  return;
}

void MainWindow::about()
{
  QMessageBox::about(this, tr("About Image Viewer"),
		     tr("<p>The <b>Image Viewer</b> example shows how to combine QLabel "
			"and QScrollArea to display an image. QLabel is typically used "
			"for displaying a text, but it can also display an image. "
			"QScrollArea provides a scrolling view around another widget. "
			"If the child widget exceeds the size of the frame, QScrollArea "
			"automatically provides scroll bars. </p><p>The example "
			"demonstrates how QLabel's ability to scale its contents "
			"(QLabel::scaledContents), and QScrollArea's ability to "
			"automatically resize its contents "
			"(QScrollArea::widgetResizable), can be used to implement "
			"zooming and scaling features. </p><p>In addition the example "
			"shows how to use QPainter to print an image.</p>"));
}

void MainWindow::createActions()
{
  _new_act = new QAction(tr("&New Sync"), this);
  _new_act->setShortcut(tr("Ctrl+N"));
  connect(_new_act, SIGNAL(triggered()), this, SLOT(new_sync()));
  _toolbar->insertAction(0, _new_act);

  _sync_act = new QAction(tr("&Perform Sync"), this);
  _sync_act->setShortcut(tr("Ctrl+G"));
  connect(_sync_act, SIGNAL(triggered()), this, SLOT(perform_sync()));
  _toolbar->insertAction(0, _sync_act);

  _send_act = new QAction(tr("Send File to Server"), this);
  _send_act->setShortcut(tr("Ctrl+1"));
  connect(_send_act, SIGNAL(triggered()), this, SLOT(set_to_send()));
  _toolbar->insertAction(0, _send_act);

  _get_act = new QAction(tr("Get File from Server"), this);
  _get_act->setShortcut(tr("Ctrl+2"));
  connect(_get_act, SIGNAL(triggered()), this, SLOT(set_to_get()));
  _toolbar->insertAction(0, _get_act);

  _delete_remote_act = new QAction(tr("Delete File from Server"), this);
  _delete_remote_act->setShortcut(tr("Ctrl+3"));
  connect(_delete_remote_act, SIGNAL(triggered()), this, SLOT(set_to_remote_delete()));
  _toolbar->insertAction(0, _delete_remote_act);

  _delete_local_act = new QAction(tr("Delete File from Client"), this);
  _delete_local_act->setShortcut(tr("Ctrl+4"));
  connect(_delete_local_act, SIGNAL(triggered()), this, SLOT(set_to_local_delete()));
  _toolbar->insertAction(0, _delete_local_act);

  _exit_act = new QAction(tr("E&xit"), this);
  _exit_act->setShortcut(tr("Ctrl+Q"));
  connect(_exit_act, SIGNAL(triggered()), this, SLOT(close()));

  _about_act = new QAction(tr("&About"), this);
  connect(_about_act, SIGNAL(triggered()), this, SLOT(about()));

  _about_qt_act = new QAction(tr("About &Qt"), this);
  connect(_about_qt_act, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::set_to_send()
{
  QModelIndexList ilist = _view->selectionModel()->selectedIndexes();
  _sync_model->set_action(ilist, Action::SendToServer);
  return;
}

void MainWindow::set_to_get()
{
  QModelIndexList ilist = _view->selectionModel()->selectedIndexes();
  _sync_model->set_action(ilist, Action::GetFromServer);
  return;
}

void MainWindow::set_to_remote_delete()
{
  QModelIndexList ilist = _view->selectionModel()->selectedIndexes();
  _sync_model->set_action(ilist, Action::DeleteFromServer);
  return;
}

void MainWindow::set_to_local_delete()
{
  QModelIndexList ilist = _view->selectionModel()->selectedIndexes();
  _sync_model->set_action(ilist, Action::DeleteFromClient);
  return;
}

void MainWindow::createMenus()
{
  _file_menu = new QMenu(tr("&File"), this);
  _file_menu->addAction(_new_act);
  _file_menu->addSeparator();
  _file_menu->addAction(_exit_act);

  _help_menu = new QMenu(tr("&Help"), this);
  _help_menu->addAction(_about_act);
  _help_menu->addAction(_about_qt_act);

  menuBar()->addMenu(_file_menu);
  menuBar()->addMenu(_help_menu);
}

void MainWindow::display_error(const QString &message)
{
  cout << "MainWindow::display_error - " << qPrintable(message) << endl;
  QMessageBox::information(this, tr("DirSync Client"), message);
  return;
}