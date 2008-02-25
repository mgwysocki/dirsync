#ifndef __FILEAGENT_H
#define __FILEAGENT_H

#include "FileData.h"

class FileAgent {
 public:
  FileAgent();
  FileAgent(const FileData previous, const FileData current);
  FileAgent(const FileAgent &fa);

  bool operator<(const FileAgent &fa)
  { return (filename < fa.filename); }

  bool operator>(const FileAgent &fa)
  { return (filename > fa.filename); }

  QString filename;
  QString relative_filename;
  FileData previous_fd;
  FileData current_fd;
  bool added;
  bool deleted;
  bool modified;

  friend std::ostream& operator<<( std::ostream& os, const FileAgent &fd );
  friend QDataStream & operator<<( QDataStream &dout, const FileAgent &fd );
  friend QDataStream & operator>>( QDataStream &din, FileAgent &fd );
};

#endif
