#ifndef __DIRCOMPARATOR_H
#define __DIRCOMPARATOR_H

#include <sys/stat.h>

#include <QFile>
#include <QString>
#include <iostream>
using namespace std;

#include "FileData.h"

class DirComparator {
 public:
  DirComparator(QString dir = QString() );

  bool read_last_sync_file();
  void make_changes_list();
  void print();
  void clear();

  void set_dir(const QString d) {
    _dir_path = d;
    cout << "dir before: " << qPrintable(d) << endl;
    _syncfilename = d;
    _syncfilename += "/";
    _syncfilename += ".dirsync";
    cout << "dir after: " << qPrintable(d) << endl;

  }
  const QString get_dir() {return _dir_path;}

  void set_syncfilename(QString n) {_syncfilename = n;}
  const QString get_syncfilename() {return _syncfilename;}

  QList<FileData> added_files;
  QList<FileData> deleted_files;
  QList<FileData> modified_files;
  QList<FileData> orig_modified_files;

  friend QDataStream & operator<<( QDataStream &dout, const DirComparator &dc );
  friend QDataStream & operator>>( QDataStream &din, DirComparator &dc );

 private:
  void read_dir(QString, const QDir &);
  QString _dir_path;
  QString _syncfilename;

  QList<FileData> _last_synced_list;
  QList<FileData> _current_files_list;
};

#endif
