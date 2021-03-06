#ifndef __FILEDATA_H
#define __FILEDATA_H

//--------------------------------------------------------------- 
// The FileData class stores the properties for an individual file,
// and can be read & written to QDataStream.
// ---------------------------------------------------------------

#include <sys/stat.h>

#include <QDir>
#include <QFile>
#include <QString>
#include <iostream>

class FileData {
 public:
  FileData()
    {size = 0; acctime = 0; modtime = 0; isdir=false; initialized=false; checksum=0; synced=false;}

  FileData(const FileData &);

  // size in bytes sent over socket
  quint32 socket_size() const
    {return 36+2*(filename.length()+relative_filename.length());}

  bool operator<(const FileData &fd) const
    { return (filename < fd.filename); }

  bool operator>(const FileData &fd) const
    { return (filename > fd.filename); }

  bool operator==(const FileData &fd) const
    { if(isdir)  return ((relative_filename == fd.relative_filename) &&
			 (isdir == fd.isdir));
      else       return ((relative_filename == fd.relative_filename) &&
			 (size == fd.size) &&
			 (modtime == fd.modtime) &&
			 (isdir == fd.isdir) &&
			 (checksum == fd.checksum)); }
  
  bool operator!=(const FileData &fd) const
    { return !(*this==fd);}

  QString filename;
  QString relative_filename;
  quint64 size;
  quint32 acctime;
  quint32 modtime;
  QFile::Permissions perms;
  bool isdir;
  quint32 checksum;
  bool synced;
  
  bool initialized;

  friend std::ostream& operator<<( std::ostream& os, const FileData &fd );
  friend QDataStream & operator<<( QDataStream &dout, const FileData &fd );
  friend QDataStream & operator>>( QDataStream &din, FileData &fd );
};

#endif
