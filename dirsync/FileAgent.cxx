#include "FileAgent.h"
#include <QDataStream>

FileAgent::FileAgent() :
  added(false),
  deleted(false),
  modified(false)
{}


FileAgent::FileAgent(const FileData previous, const FileData current) :
  previous_fd(previous),
  current_fd(current),
  added(false),
  deleted(false),
  modified(false)
{
  if(!previous_fd.initialized) {
    filename = current_fd.filename;
    relative_filename = current_fd.relative_filename;
  } else {
    filename = previous_fd.filename;
    relative_filename = previous_fd.relative_filename;
  }
  
  if(!previous_fd.initialized) added = true;
  else if(!current_fd.initialized) deleted = true;
  else if(previous_fd != current_fd) modified = true;
}


FileAgent::FileAgent(const FileAgent &fa) :
  filename(fa.filename),
  relative_filename(fa.relative_filename),
  previous_fd(fa.previous_fd),
  current_fd(fa.current_fd),
  added(fa.added),
  deleted(fa.deleted),
  modified(fa.modified)
{}


std::ostream& operator<<(std::ostream& os, const FileAgent& fa)
{
  os << qPrintable(fa.filename);
  if(fa.added)         os << "\nAdded.\n";
  else if(fa.modified) os << "\nModified.\n";
  else if(fa.deleted)  os << "\nDeleted.\n";
  else                 os << "\nNo change.\n";
  os << fa.previous_fd << "\n" << fa.current_fd;
  return os;
};


QDataStream & operator<<( QDataStream &dout, const FileAgent &fa )
{
  quint32 temp = fa.added ? 1 : 0;
  temp |= (fa.modified ? 2 : 0);
  temp |= (fa.deleted ? 4 : 0);
  return dout << fa.filename << fa.relative_filename
	      << fa.previous_fd << fa.current_fd
	      << temp;
};

QDataStream & operator>>( QDataStream &din, FileAgent &fa )
{
  din >> fa.filename;
  din >> fa.relative_filename;
  din >> fa.previous_fd;
  din >> fa.current_fd;

  quint32 temp(0);
  din >> temp;
  if(temp&1) fa.added = true;
  else       fa.added = false;
  if(temp&2) fa.modified = true;
  else       fa.modified = false;
  if(temp&4) fa.deleted = true;
  else       fa.deleted = false;
  return din;
};
