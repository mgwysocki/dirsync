#include "FileHandler.h"

#include <QTcpSocket>

#include <sys/stat.h>
#include <utime.h>
#include <iostream>
using namespace std;

FileHandler::FileHandler() :
  _isopen(false)
{}

FileHandler::FileHandler(const QString &full_path) :
  _isopen(false)
{ 
  _load_file_data(full_path); 
}

FileHandler::FileHandler(const FileData &fd) :
  _fd(fd),
  _isopen(false)
{}


void FileHandler::_load_file_data(const QString &full_path)
{
  _fd.filename = full_path;
  QFileInfo fileinfo(full_path);
  _fd.size = fileinfo.size();
  _fd.isdir = fileinfo.isDir();
  _fd.perms = fileinfo.permissions();

  struct stat buf;
  stat(full_path.toAscii().constData(), &buf);
  _fd.acctime = buf.st_atime;
  _fd.modtime = buf.st_mtime;
  _fd.initialized = true;
  return;
}


FileData FileHandler::get_fd_from_socket(QTcpSocket* socket)
{
  quint32 size(0);
  FileData fd;
  QDataStream tcp(socket);    // read the data serialized from the socket
  while(socket->bytesAvailable() < 4) {
    if(! socket->waitForReadyRead(10*1000)) {
      //emit error(tr("FileHandler::get_fd_from_socket() - socket timed out!"));
      cout << "FileHandler::get_fd_from_socket() - socket timed out!" << endl;
      return fd;
    }
  }
  tcp >> size;

  while(socket->bytesAvailable() < size) {
    if(! socket->waitForReadyRead(10*1000)) {
      //emit error(tr("FileHandler::get_fd_from_socket() - socket timed out!"));
      cout << "FileHandler::get_fd_from_socket() - socket timed out!" << endl;
      return fd;
    }
  }
  tcp >> fd;
  return fd;
}


void FileHandler::send_fd_to_socket(const FileData &fd, QTcpSocket* socket)
{
  QDataStream tcp(socket);    // write the data serialized to socket
  tcp << fd.socket_size();
  tcp << fd;
  return;
}

bool FileHandler::begin_file_write()
{
  if(_isopen) return false;
  _isopen = true;
 
  _file.setFileName(_fd.filename);
  if(_fd.isdir) {
    QDir d;
    d.mkpath(_fd.filename);

    struct utimbuf ubuf;
    ubuf.modtime = _fd.modtime;
    utime(qPrintable(_fd.filename), &ubuf);

    _file.setPermissions(_fd.perms);
    return true;
  }

  cout << "Writing file: " << qPrintable(_fd.filename) << endl;
  _file.open(QIODevice::WriteOnly);
  return true;
}

void FileHandler::write_to_file(const QByteArray &buffer)
{
  if(_isopen)
    _file.write(buffer);
  return;
}

void FileHandler::end_file_write()
{
  _file.close();

  struct utimbuf ubuf;
  ubuf.modtime = _fd.modtime;
  utime(qPrintable(_fd.filename), &ubuf);

  _file.setPermissions(_fd.perms);
  _isopen = false;
  return;
}


quint16 FileHandler::get_checksum()
{
  quint16 cs(0);
  _file.setFileName(_fd.filename);
  _file.open(QIODevice::ReadOnly);
  cs = qChecksum(_file.readAll(), _fd.size);
  _file.close();
  return cs;
}