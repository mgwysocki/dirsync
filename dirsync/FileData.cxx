#include "FileData.h"
#include <QDataStream>
#include <QDateTime>

FileData::FileData(const FileData &fd) :
  filename(fd.filename), 
  relative_filename(fd.relative_filename),
  size(fd.size), 
  acctime(fd.acctime), 
  modtime(fd.modtime),
  perms(fd.perms),
  isdir(fd.isdir),
  checksum(fd.checksum),
  synced(fd.synced),
  initialized(fd.initialized)
{}

std::ostream& operator<<(std::ostream& os, const FileData& fd)
{
  QDateTime dt;
  dt.setTime_t(fd.modtime);
  os << qPrintable(fd.filename) << " (" << fd.size << " bytes, chksum " << fd.checksum << ") " << qPrintable(dt.toString());
  return os;
};


QDataStream & operator<<( QDataStream &dout, const FileData &fd )
{
  quint32 temp = fd.isdir;
  temp ^= (fd.initialized<<1);
  temp ^= (fd.synced<<2);
  
  return dout << fd.filename << fd.relative_filename << fd.size 
	      << fd.acctime << fd.modtime << fd.perms << temp << fd.checksum;
};

QDataStream & operator>>( QDataStream &din, FileData &fd )
{
  quint32 temp(0);
  din >> fd.filename;
  din >> fd.relative_filename;
  din >> fd.size;
  din >> fd.acctime;
  din >> fd.modtime;
  din >> temp; fd.perms = static_cast<QFlags<QFile::Permissions>::enum_type >(temp);
  din >> temp;
  if(temp&1) fd.isdir = true;
  else       fd.isdir = false;
  if(temp&2) fd.initialized = true;
  else       fd.initialized = false;
  if(temp&4) fd.synced = true;
  else       fd.synced = false;
  din >> fd.checksum;
  return din;
};
