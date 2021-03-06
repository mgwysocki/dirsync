#include <QMessageBox>
#include <QProgressDialog>
#include <QtNetwork>

#include <sys/stat.h>
#include <utime.h>
#include <iostream>
using namespace std;

#include "../protocol.h"
#include "../FileHandler.h"
#include "../SocketTool.h"
#include "NetworkClientThread.h"
//#include "SyncModel.h"


QString convert_to_hex(const unsigned char* digest, const uint size) {
  QString hash;
  char bits[3];
  
  fill(bits, bits + sizeof(bits), '\0');
  
  for(uint i=0 ; i<size; i++) {
    snprintf(bits, sizeof(bits), "%02x", digest[i]);
    hash += bits;
  }
  
  return hash;
};


NetworkClientThread::NetworkClientThread(QObject* parent) :
  QThread(parent),
  _socket(0),
  _packet_size(100*1024),
  _mode( ClientMode::None ),
  _quit(false)
{
  qRegisterMetaType< QList<FileData> >("QList<FileData>");
  qRegisterMetaType< QIODevice::OpenMode >("OpenMode");
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
  
  quint32 handshake( SocketTool::get_handshake(_socket) );
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
  if(!_socket) _socket = new QTcpSocket;
  while (!_quit) {
    QMutexLocker locker(&_mutex);
    cout << "mode: " << _mode << endl;
    switch(_mode) {

    case ClientMode::Reading:
      if( _socket->state() != QAbstractSocket::ConnectedState || 
	  _server != _socket->peerName() ){
	if( !_connect_socket() ){
	  cout << "Failed to connect to host " << qPrintable(_server) << endl;
	  break;
	}
      }

      if( _get_remote_filelist() )
	emit got_filelist(_remote_filelist);
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


bool NetworkClientThread::_get_remote_filelist()
{
  cout << "NetworkClientThread::_get_remote_filelist()" << endl;
  _remote_filelist.clear();
  quint32 handshake(0);

  cout << "Setting ServerDir: " << qPrintable(_server_dir) << endl;
  SocketTool::send_handshake( _socket, HandShake::SetDirectory );
  SocketTool::send_QString( _socket, _server_dir );
  handshake = SocketTool::get_handshake( _socket );
  if(handshake != HandShake::Acknowledge) return false;
  cout << "Received Acknowledge" << endl;

  cout << "Requesting List of Remote Changes..." << endl;
  SocketTool::send_handshake( _socket, HandShake::RequestChangesList );
  handshake = SocketTool::get_handshake( _socket );
  if(handshake != HandShake::SendingChangesList) return false;

  _remote_filelist = SocketTool::get_QList_FileData( _socket );
  cout << "Received list of Remote Changed Files, size=" 
       << _remote_filelist.size() << endl;

  return true;
}

bool NetworkClientThread::_send_files()
{
  cout << "NetworkClientThread::_send_files()" << endl;
  while(!_quit && _files_to_send.size()>0) {

    FileData local_fd(_files_to_send.takeFirst());
    cout << "Sending file to server: " << qPrintable(local_fd.relative_filename) << endl;
    emit change_upload_status( QString("Sending to server:\n%1").arg(local_fd.relative_filename) );

    quint32 handshake(0);
    cout << "NetworkClientThread: sending FileData...\n" << local_fd << endl;

    SocketTool::send_handshake( _socket, HandShake::SendingFile );
    SocketTool::send_FileData(_socket, local_fd);

    handshake = SocketTool::get_handshake(_socket);
    if(handshake != HandShake::Acknowledge) {
      cout << "Wrong handshake! (" << handshake << ")" << endl;
      return false;
    }

    if(local_fd.isdir) {
      cout << "fd.isdir == true... FileData sent." << endl;
      emit increment_upload();
      continue;
    }

    md5 tmp_md5;
    QFile outfile(local_fd.filename);
    outfile.open(QIODevice::ReadOnly);
    connect(_socket, SIGNAL(bytesWritten(qint64)), this, SIGNAL(bytesWritten(qint64)));

    quint64 remaining_size(local_fd.size);
    quint32 blocksize(_packet_size);
    cout << "blocksize = " << blocksize << endl;
    while(remaining_size>0) {
      if(remaining_size<blocksize) blocksize = remaining_size;
      QByteArray data( outfile.read(blocksize) );
      SocketTool::send_block( _socket, data );
      tmp_md5.add_data(data);
      remaining_size -= blocksize;
    }
    outfile.close();

    QString md5str = tmp_md5.get_hex_string();
    cout << "md5: " << qPrintable(md5str) << endl;
    SocketTool::send_QString( _socket, md5str );
    
    while(_socket->bytesToWrite()>0) {
      _socket->waitForBytesWritten();
    }

    cout << "Waiting for acknowledge..." << endl;
    handshake = SocketTool::get_handshake( _socket );
    if(handshake != HandShake::Acknowledge) {
      cout << "Wrong handshake! (" << handshake << ")" << endl;
      return false;
    }
    disconnect(_socket, SIGNAL(bytesWritten(qint64)), this, SIGNAL(bytesWritten(qint64)));
  }
  
  emit change_upload_status( QString("Upload complete.") );
  return true;
}


bool NetworkClientThread::_get_files()
{
  cout << "NetworkClientThread::_get_files()" << endl;
  while(!_quit && _files_to_get.size()>0) {

    FileData remote_fd(_files_to_get.takeFirst());
    emit change_download_status(QString("Downloading from server:\n%1").arg(remote_fd.relative_filename) );

    cout << "NetworkClientThread: requesting file...\n" << remote_fd << endl;
    buffer.clear();
    quint32 handshake;
    SocketTool::send_handshake( _socket, HandShake::RequestFile );
    SocketTool::send_FileData(_socket, remote_fd);
    handshake = SocketTool::get_handshake( _socket );
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
      continue;
    }

    // Read data into buffer, one chunk at a time
    quint64 remaining_size(remote_fd.size);
    while(remaining_size>0) {

      buffer.append( SocketTool::get_block(_socket) );
      remaining_size -= buffer.size();
      fh.write_to_file(buffer);
      emit bytesReceived(buffer.size());
      buffer.clear();
    }

    QString md5str( SocketTool::get_QString(_socket) );
    fh.end_file_write( md5str );  // compare local/remote md5-sums
    buffer.clear();
  }

  emit change_download_status( QString("Download complete.") );
  return true;
}


bool NetworkClientThread::_delete_remote_files()
{
  cout << "NetworkClientThread::_delete_remote_files()" << endl;
  if(!_quit && _remote_files_to_delete.size()>0) {

    SocketTool::send_handshake( _socket, HandShake::DeleteFiles );
    SocketTool::send_QList_FileData(_socket, _remote_files_to_delete);

    quint32 handshake = SocketTool::get_handshake( _socket );
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
