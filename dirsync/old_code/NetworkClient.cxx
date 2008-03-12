#include <QMessageBox>
#include <QtNetwork>

#include <iostream>
using namespace std;

#include "../protocol.h"
#include "NetworkClient.h"

NetworkClient::NetworkClient(QWidget* mw = 0) :
  _main_window(mw)
{
  _socket = new QTcpSocket(this);
  connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),
	  this, SLOT(display_error(QAbstractSocket::SocketError)));
}

NetworkClient::~NetworkClient()
{
  this->disconnect_socket();
  delete _socket;
}

void NetworkClient::disconnect_socket()
{
  if(_socket->state() != QAbstractSocket::UnconnectedState) {
    _socket->disconnectFromHost();
    _socket->waitForDisconnected();
  }
  cout << "NetworkClient:: socket disconnected!" << endl;
  return;
}

void NetworkClient::connect_socket()
{
  _block_size = 0;
  _socket->abort();
  _socket->connectToHost(_server, _port);
  return;
}


void NetworkClient::reset_server()
{
  return;
}

void NetworkClient::get_remote_changes(DirComparator &dc)
{
  connect_socket();

  quint32 handshake(0);
  QDataStream tcp(_socket);
  tcp.setVersion(QDataStream::Qt_4_0);

  cout << "Reseting Server" << endl;
  tcp << HandShake::Reset;
  _socket->waitForReadyRead();
  tcp >> handshake;
  if(handshake != HandShake::Acknowledge) return;
  cout << "Received Acknowledge" << endl;

  cout << "Setting ServerDir: " << qPrintable(dc.get_dir()) << endl;
  tcp << HandShake::SetDirectory << dc.get_dir();
  _socket->waitForReadyRead();
  tcp >> handshake;
  if(handshake != HandShake::Acknowledge) return;
  cout << "Received Acknowledge" << endl;

  cout << "Requesting List of Remote Changes..." << endl;
  tcp << HandShake::RequestChangesList;
  _socket->waitForReadyRead();
  tcp >> handshake;
  if(handshake != HandShake::SendingChangesList) return;
  tcp >> dc;
  cout << "Received list of Remote Changed Files, size=" 
       << dc.added_files.size() + dc.deleted_files.size() + dc.modified_files.size() << endl;

  cout << "Remote Added Files:" << endl;
  for(int i=0; i<dc.added_files.size(); i++)
    cout << " " << dc.added_files[i] << endl;

  cout << "Remote Deleted Files:" << endl;
  for(int i=0; i<dc.deleted_files.size(); i++)
    cout << " " << dc.deleted_files[i] << endl;

  cout << "Remote Modified Files:" << endl;
  for(int i=0; i<dc.modified_files.size(); i++)
    cout << " " << dc.modified_files[i] << endl;
  cout << endl;
  return;
}

void NetworkClient::get_remote_file(const FileData &remote_file, 
				    const FileData &local_file)
{

  quint32 handshake;
  QDataStream tcp(_socket);
  tcp.setVersion(QDataStream::Qt_4_0);
  tcp << HandShake::RequestFile;
  tcp << remote_file;
  _socket->waitForReadyRead();
  tcp >> handshake;
  if(handshake != HandShake::SendingFile) {
    cout << "Wrong handshake! (" << handshake << ")" << endl;
    return;
  }

//   if(blockSize == 0) {
//     if(_socket->bytesAvailable() < (int)sizeof(quint32))
//       return;

//     in >> blockSize;
//   }

//   if(_socket->bytesAvailable() < blockSize)
//     return;

  QByteArray nextFortune;
  //nextFortune.resize(blockSize);
  _socket->waitForReadyRead();
  tcp >> nextFortune;

  QFile outfile(local_file.filename);
  outfile.open(QIODevice::WriteOnly);
  outfile.write(nextFortune);
  outfile.close();
  return;
}

void NetworkClient::send_files(QList<FileData> list_of_files)
{
  _files_to_send = list_of_files;
  send_next_file();
  return;
}

void NetworkClient::send_next_file()
{
  cout << "Sending file to server: " << qPrintable(sd.local.relative_filename) << endl;

  quint32 handshake;
  QDataStream tcp(_socket);
  tcp.setVersion(QDataStream::Qt_4_0);
  tcp << HandShake::SendingFile;
  cout << "NetworkClient: sending FileData..."
       << "FileData::filename = " << qPrintable(local_file.filename) << endl
       << "FileData::size = " << local_file.size << endl
       << "FileData::modtime = " << local_file.modtime << endl;

  tcp << _files_to_send.takeFirst();

  _socket->waitForReadyRead();
  tcp >> handshake;
  if(handshake != HandShake::Acknowledge) {
    cout << "Wrong handshake! (" << handshake << ")" << endl;
    return;
  }

  QFile outfile(local_file.filename);
  outfile.open(QIODevice::ReadOnly);
  QByteArray nextFortune(outfile.readAll());
  tcp << nextFortune;
  outfile.close();
  
  connect(_socket, SIGNAL(readyRead()), this, SLOT(read_data()));
  return;
}

void NetworkClient::read_data()
{
  cout << "NetworkClient::read_data()" << endl;
  tcp >> handshake;
  if(handshake != HandShake::Acknowledge) {
    cout << "Wrong handshake! (" << handshake << ")" << endl;
    return;
  }
  disconnect(_socket, SIGNAL(readyRead()), this, SLOT(read_data()));

  if(_files_to_send.size() == 0) {
    emit all_files_sent();
    return;
  }
  send_next_file();
  return;
}


void NetworkClient::send_local_file(const FileData &local_file)
{
  quint32 handshake;
  QDataStream tcp(_socket);
  tcp.setVersion(QDataStream::Qt_4_0);
  tcp << HandShake::SendingFile;
  cout << "NetworkClient: sending FileData..."
       << "FileData::filename = " << qPrintable(local_file.filename) << endl
       << "FileData::size = " << local_file.size << endl
       << "FileData::modtime = " << local_file.modtime << endl;

  tcp << local_file;

  _socket->waitForReadyRead();
  tcp >> handshake;
  if(handshake != HandShake::Acknowledge) {
    cout << "Wrong handshake! (" << handshake << ")" << endl;
    return;
  }

//   if(blockSize == 0) {
//     if(_socket->bytesAvailable() < (int)sizeof(quint32))
//       return;

//     in >> blockSize;
//   }

//   if(_socket->bytesAvailable() < blockSize)
//     return;

  QFile outfile(local_file.filename);
  outfile.open(QIODevice::ReadOnly);
  QByteArray nextFortune(outfile.readAll());
  tcp << nextFortune;
  outfile.close();

  return;
}

void NetworkClient::delete_remote_file(const FileData &remote_file)
{
  quint32 handshake;
  QDataStream out(_socket);
  out.setVersion(QDataStream::Qt_4_0);
  out << HandShake::DeleteFile;
  out << remote_file;
  out >> handshake;
  if(handshake != HandShake::Acknowledge) {
    cout << "Bad handshake from server! (" << handshake << ")" << endl;
    return;
  }

//   if(blockSize == 0) {
//     if(_socket->bytesAvailable() < (int)sizeof(quint32))
//       return;

//     in >> blockSize;
//   }

//   if(_socket->bytesAvailable() < blockSize)
//     return;

  return;
}

void NetworkClient::display_error(QAbstractSocket::SocketError socketError)
{
  switch (socketError) {
  case QAbstractSocket::RemoteHostClosedError:
    break;
  case QAbstractSocket::HostNotFoundError:
    QMessageBox::information(_main_window, 
			     tr("Fortune NetworkClient"),
			     tr("The host was not found. Please check the "
				"host name and port settings."));
    break;
  case QAbstractSocket::ConnectionRefusedError:
    QMessageBox::information(_main_window, 
			     tr("Fortune NetworkClient"),
			     tr("The connection was refused by the peer. "
				"Make sure the fortune server is running, "
				"and check that the host name and port "
				"settings are correct."));
    break;
  default:
    QMessageBox::information(_main_window, 
			     tr("Fortune NetworkClient"),
			     tr("The following error occurred: %1.")
			     .arg(_socket->errorString()));
  }
  return;
}
