#include <QMessageBox>
#include <QProgressDialog>
#include <QtNetwork>

#include <sys/stat.h>
#include <utime.h>
#include <iostream>
using namespace std;

#include "../protocol.h"
#include "../FileHandler.h"
#include "NetworkClientThread.h"
//#include "SyncModel.h"

NetworkClientThread::NetworkClientThread(QObject* parent) :
  QThread(parent),
  _socket(0),
  _packet_size(1024*1024),
  _mode( ClientMode::None ),
  _quit(false)
{
  qRegisterMetaType< QList<FileData> >("QList<FileData>");
  qRegisterMetaType< QIODevice::OpenMode >("OpenMode");

//   QString qstr("0123456789");
//   cout << "qstr.size(): " << qstr.size() << endl;

//   QFile file("file.xxx");
//   file.open(QIODevice::WriteOnly);
//   QDataStream out(&file);
//   out.setVersion(QDataStream::Qt_4_0);
//   out << qstr;
//   file.close();

//   file.open(QIODevice::ReadOnly);
//   QDataStream in(&file);

//   // Read and check the header
//   quint32 magic;
//   in >> magic;
//   cout << "datastream size: " << magic << endl;
}

NetworkClientThread::~NetworkClientThread()
{
  cout << "NetworkClientThread::~NetworkClientThread()" << endl;
  cout << "locking mutex..." << endl;
  _mutex.lock();
  _quit = true;
  _mode = ClientMode::None;
  cout << "unlocking mutex..." << endl;
  _mutex.unlock();

  if(isRunning()) {
    cout << "Waking up the thread..." << endl;
    _cond.wakeOne();
  }

  wait();
}

void NetworkClientThread::reset()
{
  if(! isRunning()) return;
  _mutex.lock();
  _quit = true;
  _mutex.unlock();

  cout << "Waking up the thread..." << endl;
  _cond.wakeOne();
  wait();

  _mutex.lock();
  _quit = false;
  _mutex.unlock();
  return;
}

void NetworkClientThread::set_mode(quint32 m)
{
  QMutexLocker locker(&_mutex);
  _mode = m;
  return;
}

void NetworkClientThread::wake_up()
{
  cout << "NetworkClientThread::wake_up()" << endl;
  QMutexLocker locker(&_mutex);
  //disconnect(_socket, SIGNAL(readyRead()), this, SLOT(wake_up()));
  _cond.wakeOne();
  return;
}

bool NetworkClientThread::_connect_socket()
{
  cout << "NetworkClientThread::_connect_socket()" << endl;
  if(!_socket) _socket = new QTcpSocket;
  _socket->abort();
  _socket->connectToHost(_server, _port, QIODevice::ReadWrite);
  if (!_socket->waitForConnected()) {
    emit error(_socket->errorString());
    cout << "An error occured while connecting!" << endl;
    return false;
  }
  
  quint32 handshake(0);
  QDataStream tcp(_socket);
  tcp.setVersion(QDataStream::Qt_4_0);
  if(_socket->bytesAvailable()==0)
    if(!_socket->waitForReadyRead(10*1000))
      cout << "_socket->waitForReadyRead() timed out!!!" << endl;
  tcp >> handshake;
  if(handshake != HandShake::Acknowledge) return false;
  cout << "Received Acknowledge" << endl;
  
  return true;
}

void NetworkClientThread::_disconnect_socket()
{
  cout << "NetworkClientThread::_disconnect_socket()" << endl;
  if(_socket->state() != QAbstractSocket::UnconnectedState)
    _socket->disconnectFromHost();
  
  if(_socket->state() != QAbstractSocket::UnconnectedState) 
    _socket->waitForDisconnected();
  cout << "NetworkClientThread:: socket disconnected!" << endl;
  return;
}

void NetworkClientThread::run()
{
  cout << "NetworkClientThread::run()" << endl;
  while (!_quit) {
    QMutexLocker locker(&_mutex);
    cout << "mode: " << _mode << endl;
    switch(_mode) {
    case ClientMode::Reading:
      if( _connect_socket() ){
	_get_remote_filelist();
	emit got_filelist(_remote_filelist);
      }
      break;

    case ClientMode::Syncing:
      emit change_upload_status( QString("Upload pending...") );
      emit change_download_status( QString("Download pending...") );

      if( _send_files() &&
	  _get_files() &&
	  _delete_local_files() &&
	  _delete_remote_files()
	  )
	emit success();
      break;

    default:
      break;
    }
    
    emit done();
    cout << "thread going to sleep..." << endl;
    _cond.wait(&_mutex);
    cout << "thread waking up!" << endl;
  }

  cout << "locking mutex for disconnect..." << endl;
  QMutexLocker locker(&_mutex);
  if(_socket) {
    _disconnect_socket();
  }
  return;
}


void NetworkClientThread::_get_remote_filelist()
{
  cout << "NetworkClientThread::_get_remote_filelist()" << endl;
  quint32 handshake(0);
  QDataStream tcp(_socket);
  tcp.setVersion(QDataStream::Qt_4_0);

//   cout << "Reseting Server" << endl;
//   tcp << HandShake::Reset;
//   _socket->waitForReadyRead();
//   tcp >> handshake;
//   if(handshake != HandShake::Acknowledge) return;
//   cout << "Received Acknowledge" << endl;

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

  _remote_filelist.clear();
  quint32 nfiles(0);
  if(_socket->bytesAvailable()==0) _socket->waitForReadyRead();
  tcp >> nfiles;
  cout << "Receiving info on " << nfiles << " files..." << endl;
  for(quint32 i=0; i<nfiles; i++) {
    FileData fd;
    quint32 size(0);
    if(_socket->bytesAvailable()<4) _socket->waitForReadyRead();
    tcp >> size;
    cout << i << "   " << size << endl;
    while(_socket->bytesAvailable() < size ){
      if(!_socket->waitForReadyRead(10*1000)) {
	cout << "_socket->waitForReadyRead() Timed Out!!!" << endl;
	return;
      }
    }
    tcp >> fd;
    cout << "received info on " << qPrintable(fd.filename) 
	 << ", " << fd.size << ", " << fd.modtime << endl;
    _remote_filelist.append(fd);
  }
  cout << "Received list of Remote Changed Files, size=" 
       << _remote_filelist.size() << endl;

  return;
}

bool NetworkClientThread::_send_files()
{
  cout << "NetworkClientThread::_send_files()" << endl;
  int total_sent = 0;

  while(!_quit && _files_to_send.size()>0) {

    FileData local_fd(_files_to_send.takeFirst());
    cout << "Sending file to server: " << qPrintable(local_fd.relative_filename) << endl;
    emit change_upload_status( QString("Sending %1 to server...").arg(local_fd.relative_filename) );

    quint32 handshake;
    QDataStream tcp(_socket);
    tcp.setVersion(QDataStream::Qt_4_0);
    cout << "NetworkClientThread: sending FileData..." << endl
	 << "FileData::filename = " << qPrintable(local_fd.filename) << endl
	 << "FileData::size = " << local_fd.size << endl
	 << "FileData::modtime = " << local_fd.modtime << endl;

    tcp << HandShake::SendingFile;
    FileHandler::send_fd_to_socket(local_fd, _socket);

    if(!_socket->waitForReadyRead())
      cout << "_socket->waitForReadyRead(10000) Timed Out!!!" << endl;

    tcp >> handshake;
    if(handshake != HandShake::Acknowledge) {
      cout << "Wrong handshake! (" << handshake << ")" << endl;
      return false;
    }

    if(local_fd.isdir) {
      cout << "fd.isdir == true... FileData sent." << endl;
      emit increment_upload();
      continue;
    }

    QFile outfile(local_fd.filename);
    outfile.open(QIODevice::ReadOnly);

    quint16 running_sum(0);
    quint64 remaining_size(local_fd.size);
    quint32 blocksize(_packet_size);
    cout << "blocksize = " << blocksize << endl;
    while(remaining_size>0) {
      if(remaining_size<blocksize) blocksize = remaining_size;
      cout << "remaining_size, block_size = " 
	   << remaining_size << ", " << blocksize << endl;
      QByteArray data( outfile.read(blocksize) );
      tcp << data;
      running_sum += qChecksum(data, data.length());
      remaining_size -= blocksize;
    }
    outfile.close();

    cout << "Running checksum: " << running_sum << endl;
    outfile.open(QIODevice::ReadOnly);
    cout << "Checksum: " << qChecksum( outfile.readAll(), local_fd.size ) << endl;
    outfile.close();
    
    cout << "Waiting for acknowledge..." << endl;
    while(_socket->bytesAvailable() < 4)
      _socket->waitForReadyRead();
    tcp >> handshake;
    if(handshake != HandShake::Acknowledge) {
      cout << "Wrong handshake! (" << handshake << ")" << endl;
      return false;
    }
    emit increment_upload();
  }
  
  emit change_upload_status( QString("Upload complete.") );
  return true;
}

bool NetworkClientThread::_get_files()
{
  cout << "NetworkClientThread::_get_files()" << endl;
  int total_received = 0;

  while(!_quit && _files_to_get.size()>0) {

    FileData remote_fd(_files_to_get.takeFirst());
    emit change_download_status(QString("Downloading %1 from server...").arg(remote_fd.relative_filename) );

    cout << "NetworkClientThread: requesting file..." << endl
	 << "FileData::filename = " << qPrintable(remote_fd.filename) << endl
	 << "FileData::size = " << remote_fd.size << endl
	 << "FileData::modtime = " << remote_fd.modtime << endl;

    buffer.clear();
    quint32 handshake;
    QDataStream tcp(_socket);
    tcp.setVersion(QDataStream::Qt_4_0);
    tcp << HandShake::RequestFile;
    FileHandler::send_fd_to_socket(remote_fd, _socket);
    _socket->waitForReadyRead();
    tcp >> handshake;
    if(handshake != HandShake::SendingFile) {
      cout << "Wrong handshake! (" << handshake << ")" << endl;
      return false;
    }
    
    QString full_path = QDir::cleanPath(_dir + "/" + remote_fd.relative_filename);
    FileData fd(remote_fd);
    fd.filename = full_path;
    FileHandler fh(fd);
    
    if( !fh.begin_file_write() ) return false;
    if( fh.get_fd().isdir ) {
      emit increment_download();
      continue;
    }

    // Read data into buffer, one chunk at a time
    quint64 remaining_size(remote_fd.size);
    while(remaining_size>0) {
      quint32 blocksize;

      if( _socket->bytesAvailable() == 0 ){
	_socket->waitForReadyRead();
      }

      tcp >> blocksize;
      
      while( _socket->bytesAvailable() < blocksize) {
	if( !_socket->waitForReadyRead(10000)) {
	  emit error(_socket->errorString());
	  cout << qPrintable(_socket->errorString()) << endl;
	  return false;
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
    emit increment_download();

    cout << "Received filename: " << qPrintable(remote_fd.relative_filename)
	 << " (" << remote_fd.size << " bytes)" << endl;
    cout << "Checksum: " <<  fh.get_checksum() << endl;
  }

  emit change_download_status( QString("Download complete.") );
  return true;
}


bool NetworkClientThread::_delete_remote_files()
{
  cout << "NetworkClientThread::_delete_remote_files()" << endl;
  if(!_quit && _remote_files_to_delete.size()>0) {

    quint32 handshake;
    QDataStream tcp(_socket);
    tcp.setVersion(QDataStream::Qt_4_0);
    tcp << HandShake::DeleteFiles;

    tcp << quint32(_remote_files_to_delete.size());

    for(int i=0; i<_remote_files_to_delete.size(); i++) {
      FileData fd(_remote_files_to_delete[i]);
      cout << "Sending FD:\n" << fd << endl;
      FileHandler::send_fd_to_socket(fd, _socket);
    }

    while( _socket->bytesAvailable() < 4) {
      if( !_socket->waitForReadyRead(10000)) {
	emit error(_socket->errorString());
	cout << qPrintable(_socket->errorString()) << endl;
	return false;
      }
    }
    tcp >> handshake;
    if(handshake != HandShake::Acknowledge) {
      cout << "Bad handshake from server! (" << handshake << ")" << endl;
      return false;
    }
  }
  return true;
}


bool NetworkClientThread::_delete_local_files()
{
  cout << "NetworkClientThread::_delete_local_files()" << endl;
  while(!_quit && _local_files_to_delete.size()>0) {

    FileData local_fd(_local_files_to_delete.takeFirst());

    if(local_fd.isdir) {
      QFileInfo fi(local_fd.filename);
      QDir d(fi.dir());
      if( !d.rmdir(fi.fileName()) )
	cout << "Unable to remove directory " << qPrintable(fi.fileName()) << endl;
      continue;

    } else {
      QFileInfo fi(local_fd.filename);
      QDir d(fi.path());
      if( !d.remove(fi.fileName()) ){
	cout << "Unable to remove file " << qPrintable(local_fd.filename) << " !!!" << endl;
	continue;
      }
    }
  }
  return true;
}


QList<FileData> NetworkClientThread::get_remote_filelist()
{
  QMutexLocker locker(&_mutex);
  return _remote_filelist;
}

void NetworkClientThread::set_files_to_send(const QList<FileData> flist)
{
  QMutexLocker locker(&_mutex);
  _files_to_send = flist;
  return;
}

void NetworkClientThread::set_files_to_get(const QList<FileData> flist)
{
  QMutexLocker locker(&_mutex);
  _files_to_get = flist;
  return;
}

void NetworkClientThread::set_remote_files_to_delete(const QList<FileData> flist)
{
  QMutexLocker locker(&_mutex);
  _remote_files_to_delete = flist;
  return;
}

void NetworkClientThread::set_local_files_to_delete(const QList<FileData> flist)
{
  QMutexLocker locker(&_mutex);
  _local_files_to_delete = flist;
  return;
}

void NetworkClientThread::set_remote_dir(const QString &d)
{
  QMutexLocker locker(&_mutex);
  _server_dir = d;
  return;
}

void NetworkClientThread::set_local_dir(const QString &d)
{
  QMutexLocker locker(&_mutex);
  _dir = d;
  return;
}

int NetworkClientThread::get_size_of_files_to_send()
{
  int total_size = 0;
  for(int i=0; i<_files_to_send.size(); ++i)
    total_size += (_files_to_send[i].size / 1024);
  return total_size;
}

int NetworkClientThread::get_size_of_files_to_get()
{
  int total_size = 0;
  for(int i=0; i<_files_to_get.size(); ++i)
    total_size += (_files_to_get[i].size / 1024);
  return total_size;
}


void NetworkClientThread::reset_server()
{
  return;
}
