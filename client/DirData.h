#ifndef __DIRDATA_H
#define __DIRDATA_H

#include "../FileAgent.h"

class DirData
{
 public:
  DirData();
  DirData(const QString &);

  void set_dir(const QString &d) {_dir = d;}

  void set_current_list(const QList<FileData> &c) {_current_files = c;}
  void set_previous_list(const QList<FileData> &p) {_previous_files = p;}

  void generate_current_list();

  void generate_file_agents();
  void reset();

  QList<FileAgent> file_agents;

 private:
  QString _dir;
  QList<FileData> _current_files;
  QList<FileData> _previous_files;

  void _read_dir(QString, const QDir &);
};

#endif
