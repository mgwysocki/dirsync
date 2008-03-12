#include <QMessageBox>
#include <QtNetwork>

#include <iostream>
using namespace std;

#include "../protocol.h"
#include "NetworkClientThread.h"
#include "SyncModel.h"

NetworkClientThread::NetworkClientThread(QObject* parent = 0) :
  QThread(parent),
  _socket( new QTcpSocket ),
  _mode( ClientMode::None ),
  _quit(false)
{}

NetworkClientThread::~NetworkClientThread()
{
  _quit = true;
  _cond.wakeOne();
  _disconnect_socket();
  delete _socket;
  wait();
}

void NetworkClientThread::set_mode(quint32 m)
{
  QMutexLocker locker(&_mutex);
  _mode = m;
  return;
}

void NetworkClientThread::_connect_socket()
{
  QMutexLocker locker(&_mutex);
  _socket->abort();
  _socket->connectToHost(_server, _port);
  if (!_socket->waitForConnected()) {
    emit error(_socket->errorString());
  }
  return;
}

void NetworkClientThread::_disconnect_socket()
{
  QMutexLocker locker(&_mutex);
  if(_socket->state() != QAbstractSocket::UnconnectedState) {
    _socket->disconnectFromHost();
    _socket->waitForDisconnected();
  }
  cout << "NetworkClientThread:: socket disconnected!" << endl;
  return;
}


void NetworkClientThread::reset_server()
{
  return;
}

void NetworkClientThread::run()
{
  exec();
  while (!_quit) {
    switch(_mode) {
    case ClientMode::Reading:
      _connect_socket();
      _get_remote_filelist();
      break;
    case ClientMode::Syncing:
      _send_files();
      _get_files();
      break;
    }
    
    emit done();
    wait();
  }
  return;
}


void NetworkClientThread::_get_remote_filelist()
{
  QMutexLocker locker(&_mutex);
  quint32 handshake(0);
  QDataStream tcp(_socket);
  tcp.setVersion(QDataStream::Qt_4_0);

  cout << "Reseting Server" << endl;
  tcp << HandShake::Reset;
  _socket->waitForReadyRead();
  tcp >> handshake;
  if(handshake != HandShake::Acknowledge) return;
  cout << "Received Acknowledge" << endl;

  cout << "Setting ServerDir: " << qPrintable(_server_dir) << endl;
  tcp << HandShake::SetDirectory << _server_dir;
  _socket->waitForReadyRead();
  tcp >> handshake;
  if(handshake != HandShake::Acknowledge) return;
  cout << "Received Acknowledge" << endl;

  cout << "Requesting List of Remote Changes..." << endl;
  tcp << HandShake::RequestChangesList;
  _socket->waitForReadyRead();
  tcp >> handshake;
  if(handshake != HandShake::SendingChangesList) return;
  tcp >> _remote_filelist;
  cout << "Received list of Remote Changed Files, size=" 
       << _remote_filelist.size() << endl;
  return;
}

void NetworkClientThread::_send_files()
{
  while(!_quit && _files_to_send.size()>0) {

    QMutexLocker locker(&_mutex);    
    FileData local_fd(_files_to_send.takeFirst());

    cout << "Sending file to server: " << qPrintable(local_fd.relative_filename) << endl;

    quint32 handshake;
    QDataStream tcp(_socket);
    tcp.setVersion(QDataStream::Qt_4_0);
    tcp << HandShake::SendingFile;
    cout << "NetworkClientThread: sending FileData..."
	 << "FileData::filename = " << qPrintable(local_fd.filename) << endl
	 << "FileData::size = " << local_fd.size << endl
	 << "FileData::modtime = " << local_fd.modtime << endl;

    tcp << local_fd;

    _socket->waitForReadyRead();
    tcp >> handshake;
    if(handshake != HandShake::Acknowledge) {
      cout << "Wrong handshake! (" << handshake << ")" << endl;
      return;
    }

    QFile outfile(local_fd.filename);
    outfile.open(QIODevice::ReadOnly);

    quint32 remaining_size(local_fd.size);
    quint32 blocksize(_packet_size);
    while(remaining_size>0) {
      if(remaining_size<blocksize) blocksize = remaining_size;
      tcp << outfile.read(blocksize);
    }
    outfile.close();

    _socket->waitForReadyRead();
    tcp >> handshake;
    if(handshake != HandShake::Acknowledge) {
      cout << "Wrong handshake! (" << handshake << ")" << endl;
      return;
    }
  }

  return;
}

void NetworkClientThread::_get_files()
{
  while(!_quit && _files_to_get.size()>0) {

    QMutexLocker locker(&_mutex);    
    FileData remote_fd(_files_to_get.takeFirst());

    quint32 handshake;
    QDataStream tcp(_socket);
    tcp.setVersion(QDataStream::Qt_4_0);
    tcp << HandShake::RequestFile;
    tcp << remote_fd.filename;
    _socket->waitForReadyRead();
    tcp >> handshake;
    if(handshake != HandShake::SendingFile) {
      cout << "Wrong handshake! (" << handshake << ")" << endl;
      return;
    }
    
    // Read data into buffer, one chunk at a time
    quint32 remaining_size(remote_fd.size);
    while(remaining_size>0) {
      quint32 block_size = (remaining_size<_packet_size)? remaining_size : _packet_size;
      while( _socket->bytesAvailable() < block_size) {
	if( !_socket->waitForReadyRead(5000)) {
	  emit error(_socket->errorString());
	  return;
	}
      }
      buffer.append( _socket->read(block_size) );
      remaining_size -= block_size;
    }

    QString full_path = QDir::cleanPath(_dir + "/" + remote_fd.relative_filename);
    cout << "Writing file: " << qPrintable(full_path) << endl;

    // Write buffer to file
    QFile outfile(full_path);
    outfile.open(QIODevice::WriteOnly);
    outfile.write(buffer);
    outfile.close();
    buffer.clear();
  }
  return;
}


void NetworkClientThread::_delete_remote_files()
{
  while(!_quit && _remote_files_to_delete.size()>0) {

    QMutexLocker locker(&_mutex);    

    quint32 handshake;
    QDataStream tcp(_socket);
    tcp.setVersion(QDataStream::Qt_4_0);
    tcp << HandShake::DeleteFiles;
    tcp << _remote_files_to_delete;
    tcp >> handshake;
    if(handshake != HandShake::Acknowledge) {
      cout << "Bad handshake from server! (" << handshake << ")" << endl;
      return;
    }
  }
  return;
}


void NetworkClientThread::_delete_local_files()
{
  while(!_quit && _local_files_to_delete.size()>0) {

    QMutexLocker locker(&_mutex);    
    FileData local_fd(_local_files_to_delete.takeFirst());

    QFileInfo fi(local_fd.filename);
    QDir d(fi.path());
    if( !d.remove(fi.fileName()) ){
      cout << "Unable to remove file " << qPrintable(local_fd.filename) << " !!!" << endl;
      continue;
    }

    if( d.count() == 0 ){
      QString dirname = d.dirName();
      d.cdUp();
      if( !d.rmdir(dirname) )
	cout << "Unable to remove directory " << qPrintable(dirname) << endl;
    }
  }
  return;
}

