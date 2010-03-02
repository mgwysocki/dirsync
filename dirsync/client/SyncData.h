#ifndef __SYNCDATA_H
#define __SYNCDATA_H

#include "../FileAgent.h"
#include "../FileHandler.h"
#include "../FileData.h"
#include "DirData.h"

namespace Action {
  const int Unset = 0;
  const int SendToServer = 1;
  const int GetFromServer = 2;
  const int DeleteFromServer = 3;
  const int DeleteFromClient = 4;
  const int Unclear = 5;
  const int None = 6;

  const QString strings[7] = {QString("None"),
			      QString("Copy Right"),
			      QString("Copy Left"),
			      QString("Delete from Right"),
			      QString("Delete from Left"),
			      QString("Unclear"),
			      QString("None")};
};


class SyncData
{
 public:
  SyncData();
  SyncData(const FileAgent&, const FileAgent&, bool);
  SyncData(const SyncData &);
  ~SyncData() {}

  std::pair<QString,QString> get_info() const;

  bool operator<(const SyncData &sd) const
  { return (relative_filename < sd.relative_filename); }

  void determine_situation_first_time();
  void determine_situation();

  QString relative_filename;

  FileAgent local;
  FileAgent remote;

  QString situ;
  int action;

  friend QDataStream & operator<<( QDataStream &dout, const SyncData &sd );
  friend QDataStream & operator>>( QDataStream &din, SyncData &sd );
};

#endif // __SYNCDATA_H
