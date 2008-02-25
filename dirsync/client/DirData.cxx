#include "DirData.h"
#include "../FileHandler.h"

#include <iostream>
using namespace std;

DirData::DirData()
{}

DirData::DirData(const QString &dir) :
  _dir(dir)
{}


void DirData::generate_current_list()
{
  // Generate current file list
  _current_files.clear();
  QDir d(_dir);
  if(!d.exists())  cout << "Dir doesn't exist: " << qPrintable(_dir) << endl;
  else             _read_dir(_dir, d);  // recursively build _current_files
  return;
}

void DirData::generate_file_agents()
{
  // Compare current to previous file list

  // Make list of FileAgents
  
  // Loop over current files
  //  Loop over previous files
  //   If match:
  //    make FileAgent, mark as Modified
  //    remove previous FileData from list
  //    break
  //  If no match:
  //   make FileAgent
  //   mark as Added
  // Loop over remaining previous files
  //  make FileAgent
  //  mark as Deleted
  //
  QList<FileData>::const_iterator cfiter = _current_files.begin();
  while(cfiter != _current_files.end()) {

    bool found(false);
    QList<FileData>::iterator pfiter = _previous_files.begin();
    while(pfiter != _previous_files.end()) {
      if( cfiter->filename == pfiter->filename ){
	FileAgent fa(*pfiter, *cfiter);
	file_agents.append(fa);
	_previous_files.erase(pfiter);
	found = true;
	break;
      }
      pfiter++;
    }

    if( !found ){
      FileAgent fa(FileData(), *cfiter);
      file_agents.append(fa);
    }
    cfiter++;
  }

  QList<FileData>::const_iterator pfiter = _previous_files.begin();
  while(pfiter != _previous_files.end()) {
    FileAgent fa(*pfiter, FileData());
    file_agents.append(fa);
    pfiter++;
  }

  return;
}

void DirData::reset()
{
  _current_files.clear();
  _previous_files.clear();
  file_agents.clear();
  return;
}

void DirData::_read_dir(QString dir, const QDir &basedir)
{
  QDir d(dir);
  QStringList subdirs = d.entryList((QDir::Dirs|QDir::NoDotAndDotDot), QDir::Name);
  QStringList files = d.entryList(QDir::Files, QDir::Name);

  for(int i=0; i<subdirs.size(); ++i)  {
    QString full_path = d.absoluteFilePath(subdirs[i]);
    QString relative_path = basedir.relativeFilePath(full_path);
    FileHandler fh(full_path);
    fh.get_fd().relative_filename = relative_path;
    _current_files.append(fh.get_fd());
    _read_dir( full_path, basedir );
  }

  for(int j=0; j<files.size(); ++j) {
    QString full_path = d.absoluteFilePath(files[j]);
    QString relative_path = basedir.relativeFilePath(full_path);
    FileHandler fh(full_path);
    fh.get_fd().relative_filename = relative_path;
    _current_files.append(fh.get_fd());
  }
  return;
}
