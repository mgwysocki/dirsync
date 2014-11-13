#include <QtGui>
#include <QtNetwork>

#include <utime.h>
#include <iostream>
using namespace std;

#include "../protocol.h"
#include "client.h"

ClientDialog::ClientDialog(QWidget *parent)
  : QDialog(parent)
{
  _host_label = new QLabel(tr("&Server name:"));
  _port_label = new QLabel(tr("S&erver port:"));
  _clientdir_label = new QLabel(tr("ClientDialog (local) Directory:"));
  _serverdir_label = new QLabel(tr("Server Directory:"));

  _host_lineedit = new QLineEdit("Localhost");
  _port_lineedit = new QLineEdit("52614");
  _port_lineedit->setValidator(new QIntValidator(1, 65535, this));
  _clientdir_lineedit = new QLineEdit("/Users/mwysocki/testsync/");
  _serverdir_lineedit = new QLineEdit("/Users/mwysocki/testsync2/");

  _host_label->setBuddy(_host_lineedit);
  _port_label->setBuddy(_port_lineedit);
  _clientdir_label->setBuddy(_clientdir_lineedit);
  _serverdir_label->setBuddy(_serverdir_lineedit);

  _status_label = new QLabel(tr("This examples requires that you run the "
				"Fortune Server example as well."));

  _make_button = new QPushButton(tr("Make Sync List"));
  _make_button->setDefault(true);
  _make_button->setEnabled(true);

  _quit_button = new QPushButton(tr("Quit"));

  buttonBox = new QDialogButtonBox;
  buttonBox->addButton(_make_button, QDialogButtonBox::ActionRole);
  buttonBox->addButton(_quit_button, QDialogButtonBox::RejectRole);

  _socket = new QTcpSocket(this);

  connect(_host_lineedit, SIGNAL(textChanged(const QString &)),
	  this, SLOT(enable_make_button()));
  connect(_port_lineedit, SIGNAL(textChanged(const QString &)),
	  this, SLOT(enable_make_button()));
  connect(_make_button, SIGNAL(clicked()),
	  this, SLOT(connect_socket()));
  connect(_quit_button, SIGNAL(clicked()), this, SLOT(close()));
  //connect(_socket, SIGNAL(readyRead()), this, SLOT(read_data()));
  connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),
	  this, SLOT(display_error(QAbstractSocket::SocketError)));

  QGridLayout *mainLayout = new QGridLayout;
  mainLayout->addWidget(_host_label, 0, 0);
  mainLayout->addWidget(_host_lineedit, 0, 1);

  mainLayout->addWidget(_port_label, 1, 0);
  mainLayout->addWidget(_port_lineedit, 1, 1);

  mainLayout->addWidget(_clientdir_label, 2, 0);
  mainLayout->addWidget(_clientdir_lineedit, 2, 1);

  mainLayout->addWidget(_serverdir_label, 3, 0);
  mainLayout->addWidget(_serverdir_lineedit, 3, 1);

  mainLayout->addWidget(_status_label, 4, 0, 1, 2);
  mainLayout->addWidget(buttonBox, 5, 0, 1, 2);
  this->setLayout(mainLayout);

  this->setWindowTitle(tr("DirSync ClientDialog"));
  _port_lineedit->setFocus();
}


void ClientDialog::connect_socket()
{
  _make_button->setEnabled(false);
  _local_changes.set_dir( _clientdir_lineedit->text() );
  //read_synced_list();
  make_local_changes_list();

  _block_size = 0;
  _socket->abort();
  connect(_socket, SIGNAL(connected()), this, SLOT(get_remote_changes_list()));
  _socket->connectToHost(_host_lineedit->text(),
			 _port_lineedit->text().toInt());
  return;
}


void ClientDialog::read_data()
{
  QDataStream in(_socket);
  in.setVersion(QDataStream::Qt_4_0);

  QString remote_filename("");
  static quint32 received(0);

  if(_current_filename.isEmpty()) {
    in >> remote_filename;
    in >> _current_filetime;
    in >> _current_filesize;
    remote_filename += ".new";
    _start_file(remote_filename);
    printf("received time: %d\n", _current_filetime);
    printf("received size: %d\n", _current_filesize);
  }
  
  //printf("bytes available: %ld\n", _socket->bytesAvailable());

  QByteArray next_block(_socket->readAll());
  printf("block size: %d\n", next_block.size());
  
  received += next_block.size();
  printf("accumulated size: %d\n", received);

  _current_file.write(next_block);

  if(received==_current_filesize) {
    _end_file();
  }
  return;
}


void ClientDialog::_start_file(QString filename)
{
  _current_filename = filename;
  _current_file.setFileName(_current_filename);
  _current_file.open(QIODevice::WriteOnly);
  printf("writing to file: %s\n", _current_filename.toAscii().constData());
}

void ClientDialog::_end_file()
{
  printf("Closing file %s...\n", _current_filename.toAscii().constData());
  _current_file.close();

  printf("Setting modtime to %d...\n", _current_filetime);
  struct utimbuf ubuf;
  ubuf.modtime = _current_filetime;
  utime(_current_filename.toAscii().constData(), &ubuf);
  _current_filename = QString("");

  printf("Done.");
  _make_button->setEnabled(true);
  return;
}


void ClientDialog::display_error(QAbstractSocket::SocketError socketError)
{
  switch (socketError) {
  case QAbstractSocket::RemoteHostClosedError:
    break;
  case QAbstractSocket::HostNotFoundError:
    QMessageBox::information(this, tr("Fortune ClientDialog"),
			     tr("The host was not found. Please check the "
				"host name and port settings."));
    break;
  case QAbstractSocket::ConnectionRefusedError:
    QMessageBox::information(this, tr("Fortune ClientDialog"),
			     tr("The connection was refused by the peer. "
				"Make sure the fortune server is running, "
				"and check that the host name and port "
				"settings are correct."));
    break;
  default:
    QMessageBox::information(this, tr("Fortune ClientDialog"),
			     tr("The following error occurred: %1.")
			     .arg(_socket->errorString()));
  }

  _make_button->setEnabled(true);
}


void ClientDialog::enable_make_button()
{
  _make_button->setEnabled(!_host_lineedit->text().isEmpty()
			   && !_port_lineedit->text().isEmpty()
			   && !_clientdir_lineedit->text().isEmpty());
}

void ClientDialog::make_local_changes_list()
{
  cout << "ClientDialog::make_local_changes_list()" << endl;
  //_local_changes.read_last_sync_file();
  _local_changes.make_changes_list();
  _local_changes.print();
  return;
}

void ClientDialog::get_remote_changes_list()
{
  quint32 handshake(0);
  QDataStream tcp(_socket);
  tcp.setVersion(QDataStream::Qt_4_0);

  cout << "Setting ServerDir: " << qPrintable(_serverdir_lineedit->text()) << endl;
  tcp << HandShake::SetDirectory << _serverdir_lineedit->text();
  _socket->waitForReadyRead();
  tcp >> handshake;
  if(handshake != HandShake::Acknowledge) return;
  cout << "Received Acknowledge" << endl;

  cout << "Requesting List of Remote Changes..." << endl;
  tcp << HandShake::RequestChangesList;
  _socket->waitForReadyRead();
  tcp >> handshake;
  if(handshake != HandShake::SendingChangesList) return;
  tcp >> _remote_changes;
  cout << "Received list of Remote Changed Files, size=" 
       << _remote_changes.added_files.size() + _remote_changes.deleted_files.size() + _remote_changes.modified_files.size() << endl;

  cout << "Remote Added Files:" << endl;
  for(int i=0; i<_remote_changes.added_files.size(); i++)
    cout << " " << _remote_changes.added_files[i] << endl;

  cout << "Remote Deleted Files:" << endl;
  for(int i=0; i<_remote_changes.deleted_files.size(); i++)
    cout << " " << _remote_changes.deleted_files[i] << endl;

  cout << "Remote Modified Files:" << endl;
  for(int i=0; i<_remote_changes.modified_files.size(); i++)
    cout << " " << _remote_changes.modified_files[i] << endl;
  cout << endl;
  return;
}


void ClientDialog::sync()
{
  printf("ClientDialog::sync()\n");

  // Get remote files to add
  
  // Send local files to add
  // Get remote modified files
  // Send local modified files
  // Delete remotely deleted files
  // Tell server to delete locally deleted files
  return;
}


void ClientDialog::send_file()
{
  printf("ClientDialog::send_file()\n");

  // send local file
  return;
}


void ClientDialog::get_file()
{
  printf("ClientDialog::get_file()\n");

  // retrieve remote file
  return;
}


void ClientDialog::delete_remote_file()
{
  printf("ClientDialog::delete_remote_file()\n");

  // tell server to delete file
  return;
}

void ClientDialog::delete_local_file()
{
  printf("ClientDialog::delete_local_file()\n");

  // delete local file
  return;
}


/*
void ClientDialog::AddToLocal(const QString &remote_filename)
{
  // Get remotefile from server
  // Write to disk (create directories?)
  // Add to list of local changed files

  QDataStream in(_socket);
  in.setVersion(QDataStream::Qt_4_0);

  in << HandShake::RequestFile;
  in << remote_filename;

  if(blockSize == 0) {
    if(_socket->bytesAvailable() < (int)sizeof(quint32))
      return;

    in >> blockSize;
  }

  if(_socket->bytesAvailable() < blockSize)
    return;

  QByteArray nextFortune;
  //nextFortune.resize(blockSize);
  in >> nextFortune;

  QFile outfile(local_filename);
  outfile.open(QIODevice::WriteOnly);
  outfile.write(nextFortune);
  outfile.close();
  syncButton->setEnabled(true);
  return;
}

void ClientDialog::AddToRemote(const QString &local_filename)
{
  // Get remotefile from server
  // Write to disk (create directories?)
  // Add to list of local changed files

  QDataStream in(_socket);
  in.setVersion(QDataStream::Qt_4_0);

  if(blockSize == 0) {
    if(_socket->bytesAvailable() < (int)sizeof(quint32))
      return;

    in >> blockSize;
  }

  if(_socket->bytesAvailable() < blockSize)
    return;

  QByteArray nextFortune;
  //nextFortune.resize(blockSize);
  in >> nextFortune;

  QFile outfile("/Users/mwysocki/testfile.out");
  outfile.open(QIODevice::WriteOnly);
  outfile.write(_socket->readAll());
  outfile.close();
  syncButton->setEnabled(true);

}
*/
