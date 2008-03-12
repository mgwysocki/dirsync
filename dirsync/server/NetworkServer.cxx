#include <QMessageBox>
#include <QtNetwork>

#include <stdlib.h>
#include <sys/stat.h>
#include <utime.h>

#include <iostream>
using namespace std;

#include "../protocol.h"
#include "NetworkServer.h"

NetworkServer::NetworkServer(QObject* parent) :
  QObject(parent),
  _server(0),
  _socket(0),
  _packet_size(100*1024),
  _max_buffer_size(1024*1024),
  _stop(false),
  _quit(false)
{
  _mutex.lock();
  _server = new QTcpServer;
  if (!_server->listen(QHostAddress::Any, 52614)) {
    emit error(tr("Unable to start the server: %1.").arg(_server->errorString()));
    cout << "Unable to start the server: " << qPrintable(_server->errorString()) << endl;
  }

  connect(_server, SIGNAL(newConnection()), this, SLOT(new_connection()));
  _mutex.unlock();
}


//======================================================================================
// NetworkServer destructor must disconnect _socket, delete _socket, delete _server
//
NetworkServer::~NetworkServer()
{
  cout << "NetworkServer::~NetworkServer()" << endl;
  QMutexLocker locker(&_mutex);
  if(_socket) {
    _disconnect_socket();
  }
  _server->deleteLater();
  cout << "END NetworkServer::~NetworkServer()" << endl;
}


//======================================================================================
// Called internally to disconnect & delete _socket
//
void NetworkServer::_disconnect_socket()
{
  cout << "NetworkServer::disconnect_socket()" << endl;
  cout << "socket state: " << _socket->state() << endl;
  disconnect(_socket, SIGNAL(disconnected()), this, SLOT(socket_disconnected()));
  if(_socket->state() != QAbstractSocket::UnconnectedState) {
    _socket->disconnectFromHost();
    cout << "socket state: " << _socket->state() << endl;
    if(_socket->state() != QAbstractSocket::UnconnectedState)
      _socket->waitForDisconnected(2*1000);
  }
  cout << "NetworkServer socket disconnected!" << endl;
  _socket->deleteLater();
  _socket = 0;
  connect(_server, SIGNAL(newConnection()), this, SLOT(new_connection()));
  return;
}


//======================================================================================
// Slot called when _socket is disconnected externally (deletes _socket)
//
void NetworkServer::socket_disconnected()
{
  cout << "NetworkServer::socket_disconnected()" << endl;
  QMutexLocker locker(&_mutex);
  disconnect(_socket, SIGNAL(readyRead()),
	     this, SLOT(read_incoming()));
  _socket->deleteLater();
  _socket = 0;
  _current_files_list.clear();
  connect(_server, SIGNAL(newConnection()), this, SLOT(new_connection()));
  return;
}


//======================================================================================
// Slot called when _server is connected to (sets up _socket & sends ACKNOWLEDGE)
//
void NetworkServer::new_connection()
{
  cout << "NetworkServer::new_connection()" << endl;
  QMutexLocker locker(&_mutex);
  _socket = _server->nextPendingConnection();
  connect(_socket, SIGNAL(disconnected()), this, SLOT(socket_disconnected()));
  disconnect(_server, SIGNAL(newConnection()), this, SLOT(new_connection()));

  cout << "Sending acknowledge..." << endl;
  QDataStream tcp(_socket);
  tcp.setVersion(QDataStream::Qt_4_0);
  tcp << HandShake::Acknowledge;
  connect(_socket, SIGNAL(readyRead()), this, SLOT(read_incoming()));
  return;
}


//======================================================================================
// Slot called when _socket begins receiving data, reads the handshake, and calls the
//  appropriate internal function.
//
void NetworkServer::read_incoming()
{
  if(_mutex.tryLock()) {
    cout << "_mutex is unlocked" << endl;
    _mutex.unlock();
  } else {
    cout << "_mutex is locked" << endl;
  }
  QMutexLocker locker(&_mutex);    
  disconnect(_socket, SIGNAL(readyRead()), this, SLOT(read_incoming()));
  cout << "NetworkServer::read_incoming()" << endl;;

  quint32 handshake(0);
  QDataStream tcp(_socket);
  tcp.setVersion(QDataStream::Qt_4_0);

  if(_socket->bytesAvailable() == 0) {
    cout << "_socket->waitForReadyRead()" << endl;
    _socket->waitForReadyRead();
  }
  tcp >> handshake;
  cout << "Received handshake: " << HandShake::strings[handshake] << endl;

  if(handshake == HandShake::SetDirectory) {
    QString d;
    tcp >> d;
    _dir = d;
    cout << "Sending acknowledge..." << endl;
    tcp << HandShake::Acknowledge;
    _make_local_file_list();
    connect(_socket, SIGNAL(readyRead()), this, SLOT(read_incoming()));
    return;

  } else if(handshake == HandShake::RequestChangesList) {
    tcp << HandShake::SendingChangesList;
    tcp << _current_files_list.size();
    for(int i=0; i<_current_files_list.size(); i++) {
      FileData fd(_current_files_list[i]);
      cout << "FileData::filename = " << qPrintable(fd.relative_filename) << endl
	   << "FileData::size = " << fd.size << endl
	   << "FileData::modtime = " << fd.modtime << endl;
      FileHandler::send_fd_to_socket(fd, _socket);
    }

    connect(_socket, SIGNAL(readyRead()), this, SLOT(read_incoming()));
    return;

  } else if(handshake == HandShake::RequestFile) {
    if(_socket->bytesAvailable() == 0) _socket->waitForReadyRead();
    FileData fd( FileHandler::get_fd_from_socket(_socket) );
    tcp << HandShake::SendingFile;
    _send_file(fd);
    connect(_socket, SIGNAL(readyRead()), this, SLOT(read_incoming()));
    return;

  } else if(handshake == HandShake::SendingFile) {
    if(_socket->bytesAvailable() == 0) _socket->waitForReadyRead();
    FileData fd( FileHandler::get_fd_from_socket(_socket) );
    cout << "FileData::filename = " << qPrintable(fd.relative_filename) << endl
	 << "FileData::size = " << fd.size << endl
	 << "FileData::modtime = " << fd.modtime << endl;

    tcp << HandShake::Acknowledge;
    _receive_file(fd);
    connect(_socket, SIGNAL(readyRead()), this, SLOT(read_incoming()));
    return;

  } else if(handshake == HandShake::DeleteFiles) {
    if(_socket->bytesAvailable() == 0) _socket->waitForReadyRead();
    quint32 n_files_to_delete(0);
    tcp >> n_files_to_delete;

    QList<FileData> files_to_delete;
    for(quint32 i=0; i<n_files_to_delete; i++) {
      FileData fd( FileHandler::get_fd_from_socket(_socket) );
      files_to_delete.append(fd);
    }

    _delete_local_files(files_to_delete);
    cout << "Sending acknowledge..." << endl;
    tcp << HandShake::Acknowledge;
    connect(_socket, SIGNAL(readyRead()), this, SLOT(read_incoming()));
    return;

  } else if(handshake == HandShake::Reset) {
    _current_files_list.clear();
    cout << "Sending acknowledge..." << endl;
    tcp << HandShake::Acknowledge;
    connect(_socket, SIGNAL(readyRead()), this, SLOT(read_incoming()));
    return;
  }

  connect(_socket, SIGNAL(readyRead()), this, SLOT(read_incoming()));
  printf("Unrecognized handshake!\n");
  return;
}


//======================================================================================
// Called to send the file that the client requested
//
void NetworkServer::_send_file(const FileData &fd)
{
  cout << "Sending file to client: " << qPrintable(fd.relative_filename) << endl;

  QDataStream tcp(_socket);
  tcp.setVersion(QDataStream::Qt_4_0);
  cout << "NetworkServer: sending FileData..." << endl
       << "FileData::filename = " << qPrintable(fd.filename) << endl
       << "FileData::size = " << fd.size << endl
       << "FileData::modtime = " << fd.modtime << endl;

  if(fd.isdir) {
    cout << "fd.isdir == true... FileData sent." << endl;
    return;
  }

  QFile outfile(fd.filename);
  outfile.open(QIODevice::ReadOnly);

  quint64 remaining_size(fd.size);
  quint32 blocksize(_packet_size);
  while(remaining_size>0) {
    cout << "remaining_size,blocksize = " << remaining_size << "," << blocksize << endl;
    if(remaining_size<blocksize) blocksize = remaining_size;
    tcp << outfile.read(blocksize);
    remaining_size -= blocksize;
  }
  outfile.close();

  FileHandler fh(fd);
  cout << "Checksum: " << fh.get_checksum() << endl;
  return;
}


//======================================================================================
// Called to receive the incoming file from the client
//
void NetworkServer::_receive_file(const FileData &client_fd)
{
  cout << "Receiving file: " << qPrintable(client_fd.relative_filename) << endl;
  QDataStream tcp(_socket);
  tcp.setVersion(QDataStream::Qt_4_0);
  buffer.clear();

  QString full_path = QDir::cleanPath(_dir + "/" + client_fd.relative_filename);
  QDir d(_dir);
  FileData fd(client_fd);
  fd.filename = full_path;
  FileHandler fh(fd);
  fh.begin_file_write();

  if(fd.isdir) return;

  // Read data into buffer, one chunk at a time
  quint64 remaining_size(client_fd.size);
  while(remaining_size>0) {
    quint32 blocksize;

    if( _socket->bytesAvailable() == 0 ){
      _socket->waitForReadyRead();
    }

    tcp >> blocksize;
    cout << "remaining_size, block_size = " 
	 << remaining_size << ", " << blocksize << endl;

    while( _socket->bytesAvailable() < blocksize) {
      if( !_socket->waitForReadyRead(10000)) {
	emit error(_socket->errorString());
	cout << qPrintable(_socket->errorString()) << endl;
	return;
      }
    }

    cout << _socket->bytesAvailable() << " bytes available after wait" << endl;
    buffer.append(_socket->read(blocksize));
    remaining_size -= buffer.size();
    cout << "buffer size = " << buffer.size() << endl;
    fh.write_to_file(buffer);
    buffer.clear();
  }
  fh.end_file_write();
  buffer.clear();

  printf("Received filename: %s (%ld bytes)\n", 
	 qPrintable(fd.relative_filename), (long unsigned int) fd.size);

  //cout << "Checksum: " << fh.get_checksum() << endl;
  tcp << HandShake::Acknowledge;
  return;
}


//======================================================================================
// Called to delete the file instructed by the client
//
void NetworkServer::_delete_local_files(QList<FileData> &local_files_to_delete)
{
  while(!_quit && local_files_to_delete.size()>0) {

    FileData fd(local_files_to_delete.takeFirst());
    cout << "Deleting " << qPrintable(fd.filename) << "..." << endl;

    if( fd.isdir ){
      QFileInfo fi(fd.filename);
      QDir d(fi.dir());
      if( !d.rmdir(fi.fileName()) )
	cout << "Unable to remove directory " << qPrintable(fi.fileName()) << endl;
      continue;
    } else {
      QFileInfo fi(fd.filename);
      QDir d(fi.path());
      if( !d.remove(fi.fileName()) ){
	cout << "Unable to remove file " << qPrintable(fd.filename) << " !!!" << endl;
	continue;
      }
    }
  }
  return;
}


//======================================================================================
// Called to generate the QList<FileData> of files in the requested directory
//
void NetworkServer::_make_local_file_list()
{
  cout << "NetworkServer::_make_local_file_list()" << endl;
  _current_files_list.clear();
  _read_dir(_dir, QDir(_dir));  // recursively build _current_files_list
  cout << _current_files_list.size() << " files in " << qPrintable(_dir) << endl;
  return;
}


//======================================================================================
// Called recursively to build QList<FileData> for _make_local_file_list()
//
void NetworkServer::_read_dir(QString dir, const QDir &basedir)
{
  QDir d(dir);
  QStringList subdirs = d.entryList((QDir::Dirs|QDir::NoDotAndDotDot), QDir::Name);
  QStringList files = d.entryList(QDir::Files, QDir::Name);

  for(int i=0; i<subdirs.size(); ++i) {
    QString full_path = d.absoluteFilePath(subdirs[i]);
    QString relative_path = basedir.relativeFilePath(full_path);
    FileHandler fh(full_path);
    fh.get_fd().relative_filename = relative_path;
    _current_files_list.append(fh.get_fd());
    _read_dir( full_path, basedir );
  }

  for(int j=0; j<files.size(); ++j) {
    QString full_path = d.absoluteFilePath(files[j]);
    QString relative_path = basedir.relativeFilePath(full_path);
    FileHandler fh(full_path);
    fh.get_fd().relative_filename = relative_path;
    _current_files_list.append(fh.get_fd());
    cout << "NetworkServer: adding FileData..." << endl
         << "FileData::filename = " << qPrintable(fh.get_fd().filename) << endl
	   << "FileData::size = " << fh.get_fd().size << endl
	   << "FileData::modtime = " << fh.get_fd().modtime << endl;
  }
  return;
}
