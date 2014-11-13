#include "FileHandler.h"
#include "Preferences.h"

#include <QTcpSocket>

#include <sys/stat.h>
#include <utime.h>
#include <iostream>
using namespace std;

bool FileHandler::_override_perms(false);

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

FileHandler::~FileHandler()
{
  // Close file if open, remove temp file if it exists.
  _file.close();
  if( QFile::exists(get_temp_filename()) ){
    QFile::remove(get_temp_filename());
  }
}


void FileHandler::_load_file_data(const QString &full_path)
{
  _fd.filename = full_path;
  QFileInfo fileinfo(full_path);
  _fd.size = fileinfo.size();
  _fd.isdir = fileinfo.isDir();
  _fd.perms = fileinfo.permissions();

  struct stat buf;
  stat(full_path.toLocal8Bit().constData(), &buf);
  _fd.acctime = buf.st_atime;
  _fd.modtime = buf.st_mtime;
  _fd.initialized = true;
  return;
}


bool FileHandler::begin_file_write()
{
  if(_isopen) return false;
  _isopen = true;
 
  if(_fd.isdir) {
    QDir d;
    d.mkpath(_fd.filename);
    return true;
  }

  if( QFile::exists(get_temp_filename()) && !QFile::remove(get_temp_filename()) ){
    cout << "Unable to remove old tempfile: " << qPrintable(get_temp_filename()) << endl;
    return false;
  }

  _file.setFileName(get_temp_filename());
  _md5.reset();

  cout << "Writing file: " << qPrintable(_fd.filename) << endl;
  _file.open(QIODevice::WriteOnly);
  return true;
}

void FileHandler::write_to_file(const QByteArray &buffer)
{
  if(_isopen)
    _file.write(buffer);
  _md5.add_data(buffer);
  return;
}

void FileHandler::end_file_write(const QString sent_hash = QString())
{
  _file.close();
  _isopen = false;

  QString received_hash = _md5.get_hex_string();
  cout << "local md5:  " << qPrintable(received_hash) << endl;
  cout << "remote md5: " << qPrintable(sent_hash) << endl;
  if( sent_hash != received_hash ){
    cout << "MD5SUMS DO NOT MATCH! ABORTING FILE!" << endl;
    QFile::remove(get_temp_filename());
    return;
  }

  if( QFile::exists(_fd.filename) && !QFile::remove(_fd.filename) ){
    cout << "Unable to remove old file: " << qPrintable(_fd.filename) << endl;
    QFile::remove(get_temp_filename());
    return;
  }

  _file.rename(_fd.filename);

  struct utimbuf ubuf;
  ubuf.modtime = _fd.modtime;
  utime(qPrintable(_fd.filename), &ubuf);

  if(_override_perms) _file.setPermissions(_fd.perms);

  cout << "FileHandler wrote file: " << qPrintable(_fd.filename)
       << " (" << _fd.size << " bytes)" << endl;
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

QString FileHandler::get_temp_filename()
{
  QDir data_dir( Preferences::get_instance()->get_temp_dir() );
  return data_dir.absoluteFilePath("tempfile");
}
