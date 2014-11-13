#include <QDir>
#include <iostream>
using namespace std;

#include "DirComparator.h"

DirComparator::DirComparator(QString dir) :
  _dir_path(dir)
{
  _syncfilename = dir;
  _syncfilename += "/";
  _syncfilename += ".dirsync";
}



bool DirComparator::read_last_sync_file()
{
  cout << "DirComparator::read_last_sync_file()" << endl;;
  QFile syncfile( _syncfilename );
  if( ! syncfile.exists() ) return false;

  if( !syncfile.open( QIODevice::ReadOnly ) )
    return false;

  _last_synced_list.clear();

  QDataStream din(&syncfile);
  din >> _last_synced_list;
  return true;
}


void DirComparator::make_changes_list()
{
  cout << "DirComparator::make_changes_list()" << endl;;
  read_dir(_dir_path, QDir(_dir_path));  // recursively build _current_files_list

  qint32 isync = 0;
  qint32 icurrent = 0;

  if( ! read_last_sync_file() ) {
    modified_files += _current_files_list;
    return;
  }

  while( isync < _last_synced_list.size() && icurrent < _current_files_list.size() ) {
    if(_last_synced_list[isync].filename > _current_files_list[icurrent].filename)
      added_files.append( _current_files_list[icurrent++] );
    else if(_last_synced_list[isync].filename < _current_files_list[icurrent].filename)
      deleted_files.append( _last_synced_list[isync++] );
    else if(_last_synced_list[isync++].modtime < _current_files_list[icurrent++].modtime) {
      modified_files.append( _current_files_list[icurrent-1] );
      orig_modified_files.append( _last_synced_list[isync-1] );
    }
  }
  return;
}

void DirComparator::read_dir(QString dir, const QDir &basedir)
{
  QDir d(dir);
  QStringList subdirs = d.entryList((QDir::Dirs|QDir::NoDotAndDotDot), QDir::Name);
  QStringList files = d.entryList(QDir::Files, QDir::Name);

  for(int i=0; i<subdirs.size(); ++i) 
    read_dir( d.absoluteFilePath(subdirs[i]), basedir );

  for(int j=0; j<files.size(); ++j) {
    FileData fd( basedir, basedir.relativeFilePath(d.absoluteFilePath(files[j])) );
    _current_files_list.append(fd);
  }
  return;
}


void DirComparator::print()
{
  cout << qPrintable(_dir_path) << ":\n";
  cout << "Added Files:\n";
  for(int i=0; i<added_files.size(); i++)
    cout << " " << added_files[i] << "\n";

  cout << "Deleted Files:\n";
  for(int i=0; i<deleted_files.size(); i++)
    cout << " " << deleted_files[i] << "\n";

  cout << "Modified Files:\n";
  for(int i=0; i<modified_files.size(); i++)
    cout << " " << modified_files[i] << "\n";
  cout << endl;

  return;
};

void DirComparator::clear()
{
  cout << "DirComparator::clear()" << endl;
  added_files.clear();
  deleted_files.clear();
  modified_files.clear();
  orig_modified_files.clear();
  _last_synced_list.clear();
  _current_files_list.clear();
  _dir_path.clear();
  _syncfilename.clear();
  return;
}

QDataStream & operator<<( QDataStream &dout, const DirComparator &dc )
{
  return dout << dc._dir_path << dc.added_files << dc.deleted_files << dc.modified_files;
};

QDataStream & operator>>( QDataStream &din, DirComparator &dc )
{
  din >> dc._dir_path;
  din >> dc.added_files;
  din >> dc.deleted_files;
  din >> dc.modified_files;
  return din;
};
