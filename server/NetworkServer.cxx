#include <QMessageBox>
#include <QtNetwork>

#include <stdlib.h>
#include <sys/stat.h>
#include <utime.h>

#include <iostream>
using namespace std;

#include "../protocol.h"
#include "../SocketTool.h"
#include "NetworkServer.h"

NetworkServer::NetworkServer(int port, QObject* parent) :
  QObject(parent),
  _server(0),
  _socket(0),
  _port(port),
  _packet_size(100*1024),
  _max_buffer_size(1024*1024),
  _stop(false),
  _quit(false)
{
  _mutex.lock();
  _server = new QTcpServer;
  if (!_server->listen(QHostAddress::Any, _port)) {
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
  SocketTool::send_handshake( _socket, HandShake::Acknowledge );
  connect(_socket, SIGNAL(readyRead()), this, SLOT(read_incoming()));
  return;
}


//======================================================================================
// Slot called when _socket begins receiving data, reads the handshake, and calls the
//  appropriate internal function.
//
void NetworkServer::read_incoming()
{
  cout << "NetworkServer::read_incoming()" << endl;;
  if(_mutex.tryLock()) {
    cout << "_mutex is unlocked" << endl;
    _mutex.unlock();
  } else {
    cout << "_mutex is locked" << endl;
  }
  QMutexLocker locker(&_mutex);    
  disconnect(_socket, SIGNAL(readyRead()), this, SLOT(read_incoming()));

  quint32 handshake( SocketTool::get_handshake(_socket) );
  cout << "Received handshake: " << HandShake::strings[handshake] << endl;

  if(handshake == HandShake::SetDirectory) {
    _dir = SocketTool::get_QString(_socket);
    cout << "Setting directory to " << qPrintable(_dir) << endl;
    cout << "Sending acknowledge..." << endl;
    SocketTool::send_handshake( _socket, HandShake::Acknowledge );
    _make_local_file_list();

  } else if(handshake == HandShake::RequestChangesList) {
    SocketTool::send_handshake( _socket, HandShake::SendingChangesList );
    SocketTool::send_QList_FileData(_socket, _current_files_list);

  } else if(handshake == HandShake::RequestFile) {
    FileData fd( SocketTool::get_FileData(_socket) );
    SocketTool::send_handshake( _socket, HandShake::SendingFile );
    _send_file(fd);

  } else if(handshake == HandShake::SendingFile) {
    FileData fd( SocketTool::get_FileData(_socket) );
    cout << fd << endl;

    SocketTool::send_handshake( _socket, HandShake::Acknowledge );
    _receive_file(fd);

  } else if(handshake == HandShake::DeleteFiles) {

    QList<FileData> files_to_delete( SocketTool::get_QList_FileData(_socket) );
    _delete_local_files(files_to_delete);

    cout << "Sending acknowledge..." << endl;
    SocketTool::send_handshake( _socket, HandShake::Acknowledge );

  } else if(handshake == HandShake::Reset) {
    _current_files_list.clear();
    cout << "Sending acknowledge..." << endl;
    SocketTool::send_handshake( _socket, HandShake::Acknowledge );

  } else {
    cout << "Unrecognized handshake!" << endl;
  }

  connect(_socket, SIGNAL(readyRead()), this, SLOT(read_incoming()));
  return;
}


//======================================================================================
// Called to send the file that the client requested
//
void NetworkServer::_send_file(const FileData &fd)
{
  cout << "NetworkServer: sending FileData...\n" << fd << endl;

  if(fd.isdir) {
    cout << "fd.isdir == true... FileData sent." << endl;
    return;
  }

  md5 tmp_md5;
  QFile outfile(fd.filename);
  outfile.open(QIODevice::ReadOnly);

  quint64 remaining_size(fd.size);
  quint32 blocksize(_packet_size);
  while(remaining_size>0) {
    //cout << "remaining_size,blocksize = " << remaining_size << "," << blocksize << endl;
    if(remaining_size<blocksize) blocksize = remaining_size;
    QByteArray data( outfile.read(blocksize) );
    SocketTool::send_block(_socket, data);
    tmp_md5.add_data(data);
    remaining_size -= blocksize;
  }
  outfile.close();
  cout << "Transfer complete." << endl;

  QString hash = tmp_md5.get_hex_string();
  cout << "md5: " << qPrintable(hash) << endl;
  SocketTool::send_QString(_socket, hash);
  return;
}


//======================================================================================
// Called to receive the incoming file from the client
//
void NetworkServer::_receive_file(const FileData &client_fd)
{
  cout << "Receiving file: " << qPrintable(client_fd.relative_filename) << endl;
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
    buffer.append( SocketTool::get_block(_socket) );
    remaining_size -= buffer.size();
    fh.write_to_file(buffer);
    buffer.clear();
  }

  QString sent_hash( SocketTool::get_QString(_socket) );
  fh.end_file_write( sent_hash );
  buffer.clear();

  cout << "Received:\n" << fd << endl;
  SocketTool::send_handshake( _socket, HandShake::Acknowledge );
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
