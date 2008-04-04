#include <QtGui>

#include <iostream>
using namespace std;

#include "NewSyncDialog.h"
#include "SyncModel.h"
#include "MainWindow.h"
#include "ProgressDialog.h"
#include "InfoDockWidget.h"
#include "MyTableView.h"

MainWindow::MainWindow() :
  QMainWindow(),
  _sync_model(new SyncModel),
  _view(new MyTableView),
  _toolbar(new QToolBar),
  _info_dock(new InfoDockWidget)
{
  _view->setModel(_sync_model);
  _view->setSelectionBehavior(QAbstractItemView::SelectRows);
  QHeaderView* header = _view->horizontalHeader();
  _view->horizontalHeader()->setHighlightSections(false);
  this->setCentralWidget(_view);
  
  this->addToolBar(_toolbar);
  this->addDockWidget(Qt::BottomDockWidgetArea, _info_dock);
  //_info_dock->setFloating(true);

  _toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  createActions();
  createMenus();
  
//   connect(_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), 
// 	  _sync_model, SLOT(selection_changed(QItemSelection, QItemSelection)));
  connect(_view->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), 
	  _sync_model, SLOT(selection_changed(QModelIndex, QModelIndex)));

  connect(_sync_model, SIGNAL(set_info(QString, QString)),
	  _info_dock, SLOT(set_info(QString, QString)));

  setWindowTitle(tr("DirSync Client"));
  QSize maxsize(QApplication::desktop()->size());
  maxsize += QSize(-50, -50);
  setMaximumSize(maxsize);
  resize(800, 600);

  new_sync();
}

void MainWindow::new_sync()
{
  cout << "MainWindow::new_sync()" << endl;

  _sync_model->reset();

  NewSyncDialog cd(this);
  int value = cd.exec();
  if(value == QDialog::Rejected)
    return;

  _net_thread.reset();

  cout << "MainWindow: setting the server, port, and dirs..." << endl;
  _local_dir = ( cd.client_dir() );
  _remote_dir = ( cd.server_dir() );
  _sync_model->set_local_dir(_local_dir);
  _sync_model->make_local_list();

  _net_thread.set_server( cd.server() );
  _net_thread.set_port( cd.port() );
  _net_thread.set_local_dir( _local_dir );
  _net_thread.set_remote_dir( _remote_dir );
  _net_thread.reset_server();

  this->disable();
  connect(&_net_thread, SIGNAL(got_filelist(QList<FileData>)), 
	  this, SLOT(set_remote_filelist(QList<FileData>)));
  connect(&_net_thread, SIGNAL(done()), 
	  this, SLOT(enable()));
  _net_thread.set_mode(ClientMode::Reading);
  _net_thread.start();
  //  _net_thread.wake_up();
  return;
}

void MainWindow::set_remote_filelist(QList<FileData> fl)
{
  disconnect(&_net_thread, SIGNAL(got_filelist(QList<FileData>)), 
	     this, SLOT(set_remote_filelist(QList<FileData>)));
  _sync_model->set_remote_filelist(fl);
  return;
}

void MainWindow::refresh_lists()
{
  _sync_model->save_sync_file();
  _sync_model->reset();

  this->disable();
  _sync_model->make_local_list();
  connect(&_net_thread, SIGNAL(got_filelist(QList<FileData>)), 
	  this, SLOT(set_remote_filelist(QList<FileData>)));
  connect(&_net_thread, SIGNAL(done()), 
	  this, SLOT(disconnect_signals()));
  connect(&_net_thread, SIGNAL(success()), _sync_model, SLOT(save_sync_file()));

  _net_thread.set_mode(ClientMode::Reading);
  _net_thread.wake_up();
  return;
}

void MainWindow::disconnect_signals()
{
  this->enable();
  disconnect(&_net_thread, SIGNAL(got_filelist(QList<FileData>)), 
	     this, SLOT(set_remote_filelist(QList<FileData>)));
  disconnect(&_net_thread, SIGNAL(done()), 
	     this, SLOT(enable()));
  disconnect(&_net_thread, SIGNAL(success()), _sync_model, SLOT(save_sync_file()));

  disconnect(&_net_thread, SIGNAL(success()), this, SLOT(refresh_lists()));
  return;
}

void MainWindow::perform_sync()
{
  cout << "MainWindow::perform_sync()" << endl;

  _net_thread.set_files_to_send( _sync_model->get_files_to_send() );
  _net_thread.set_files_to_get( _sync_model->get_files_to_get() );
  _net_thread.set_remote_files_to_delete( _sync_model->get_remote_files_to_delete() );
  _net_thread.set_local_files_to_delete( _sync_model->get_local_files_to_delete() );
  
  ProgressDialog* pd = new ProgressDialog(this);
  pd->set_n_upload( _sync_model->get_size_to_send() );
  pd->set_n_download( _sync_model->get_size_to_get() );
  connect(&_net_thread, SIGNAL(change_upload_status(QString)), pd, SLOT(set_upload_status(QString)));
  connect(&_net_thread, SIGNAL(change_download_status(QString)), pd, SLOT(set_download_status(QString)));
  connect(&_net_thread, SIGNAL(bytesWritten(qint64)), pd, SLOT(increment_upload(qint64)));
  connect(&_net_thread, SIGNAL(bytesReceived(qint64)), pd, SLOT(increment_download(qint64)));
  connect(&_net_thread, SIGNAL(done()), pd, SLOT(done()));
  pd->show();

  connect(&_net_thread, SIGNAL(done()), this, SLOT(disconnect_signals()));
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
  QIcon new_icon(":/icons/add-file-48x48.png");
  _new_act = _toolbar->addAction(new_icon, "&New Sync");
  _new_act->setShortcut(tr("Ctrl+N"));
  connect(_new_act, SIGNAL(triggered()), this, SLOT(new_sync()));

  QIcon sync_icon(":/icons/synchronize-48x48.png");
  _sync_act = _toolbar->addAction(sync_icon, "&Perform Sync");
  _sync_act->setShortcut(tr("Ctrl+G"));
  connect(_sync_act, SIGNAL(triggered()), this, SLOT(perform_sync()));

  QIcon refresh_icon(":/icons/refresh.png");
  _refresh_act = _toolbar->addAction(refresh_icon, "&Refresh");
  _refresh_act->setShortcut(tr("Ctrl+R"));
  connect(_refresh_act, SIGNAL(triggered()), this, SLOT(refresh_lists()));

  _toolbar->addSeparator();

  QIcon left_arrow_icon(":/icons/left_arrow.png");
  _sync_to_server_act = _toolbar->addAction(left_arrow_icon, "Sync to Server");
  //_sync_to_server_act->setShortcut(tr("Ctrl+1"));
  connect(_sync_to_server_act, SIGNAL(triggered()), this, SLOT(set_to_server()));

  QIcon no_icon(":/icons/no_symbol.png");
  _no_act = _toolbar->addAction(no_icon, "Do Nothing");
  //_no_act->setShortcut(tr("Ctrl+4"));
  connect(_no_act, SIGNAL(triggered()), this, SLOT(set_to_no_action()));

  QIcon right_arrow_icon(":/icons/right_arrow.png");
  _sync_to_client_act = _toolbar->addAction(right_arrow_icon, "Sync to Client");
  //_sync_to_client_act->setShortcut(tr("Ctrl+1"));
  connect(_sync_to_client_act, SIGNAL(triggered()), this, SLOT(set_to_client()));  
  
  _toolbar->addSeparator();

  _diff_act = new QAction(tr("Hide Files That Are Synced"), this);
  //_diff_act->setShortcut(tr("Ctrl+1"));
  connect(_diff_act, SIGNAL(triggered()), this, SLOT(toggle_show_diff()));
  _toolbar->addAction(_diff_act);

  _send_act = new QAction(tr("Send File to Server"), this);
  _send_act->setShortcut(tr("Ctrl+1"));
  connect(_send_act, SIGNAL(triggered()), this, SLOT(set_to_send()));
  //_toolbar->addAction(_send_act);

  _get_act = new QAction(tr("Get File from Server"), this);
  _get_act->setShortcut(tr("Ctrl+2"));
  connect(_get_act, SIGNAL(triggered()), this, SLOT(set_to_get()));
  //_toolbar->addAction(_get_act);

  _delete_remote_act = new QAction(tr("Delete File from Server"), this);
  _delete_remote_act->setShortcut(tr("Ctrl+3"));
  connect(_delete_remote_act, SIGNAL(triggered()), this, SLOT(set_to_remote_delete()));
  //_toolbar->addAction(_delete_remote_act);

  _delete_local_act = new QAction(tr("Delete File from Client"), this);
  _delete_local_act->setShortcut(tr("Ctrl+4"));
  connect(_delete_local_act, SIGNAL(triggered()), this, SLOT(set_to_local_delete()));
  //_toolbar->addAction(_delete_local_act);

  _exit_act = new QAction(tr("E&xit"), this);
  _exit_act->setShortcut(tr("Ctrl+Q"));
  connect(_exit_act, SIGNAL(triggered()), this, SLOT(close()));

  _about_act = new QAction(tr("&About"), this);
  connect(_about_act, SIGNAL(triggered()), this, SLOT(about()));

  _about_qt_act = new QAction(tr("About &Qt"), this);
  connect(_about_qt_act, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}


void MainWindow::set_to_client()
{
  QModelIndexList ilist = _view->selectionModel()->selectedIndexes();
  _sync_model->set_sync_to_client(ilist);
  return;
}

void MainWindow::set_to_server()
{
  QModelIndexList ilist = _view->selectionModel()->selectedIndexes();
  _sync_model->set_sync_to_server(ilist);
  return;
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

void MainWindow::set_to_no_action()
{
  QModelIndexList ilist = _view->selectionModel()->selectedIndexes();
  _sync_model->set_action(ilist, Action::None);
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
  _file_menu->addAction(_refresh_act);
  _file_menu->addAction(_sync_act);
  _file_menu->addSeparator();
  _file_menu->addAction(_exit_act);

  _action_menu = new QMenu(tr("&Action"), this);
  _action_menu->addAction(_sync_to_client_act);
  _action_menu->addAction(_sync_to_server_act);
  _action_menu->addSeparator();
  _action_menu->addAction(_send_act);
  _action_menu->addAction(_get_act);
  _action_menu->addAction(_delete_remote_act);
  _action_menu->addAction(_delete_local_act);
  _action_menu->addAction(_no_act);

  _help_menu = new QMenu(tr("&Help"), this);
  _help_menu->addAction(_about_act);
  _help_menu->addAction(_about_qt_act);

  menuBar()->addMenu(_file_menu);
  menuBar()->addMenu(_action_menu);
  menuBar()->addMenu(_help_menu);
}

void MainWindow::display_error(const QString &message)
{
  cout << "MainWindow::display_error - " << qPrintable(message) << endl;
  QMessageBox::information(this, tr("DirSync Client"), message);
  return;
}

void MainWindow::toggle_show_diff()
{
  bool diff_only_old = _sync_model->get_diff_only();
  if(diff_only_old) {
    _sync_model->set_diff_only(false);
    _diff_act->setText("Hide Files That Are Synced");
  } else {
    _sync_model->set_diff_only(true);
    _diff_act->setText("Show Files That Are Synced");
  }
}
