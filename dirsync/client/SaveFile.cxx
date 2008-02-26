
#include "SaveFile.h"

#include <iostream>
using namespace std;

SaveFile::SaveFile()
{}

SaveFile::SaveFile(const QString &filename) :
  _filename(filename)
{}


std::pair<QString,QString> SaveFile::read_dirs_from_file(const QString &filename)
{
  QString local, remote;
  QFile infile(filename);
  if( !infile.open(QIODevice::ReadOnly) ){
    cout << "SaveFile::read_dirs_from_file() - Could not open file " << qPrintable(filename) << "!!!" << endl;
    return std::pair<QString,QString>(local,remote);
  }

  QDataStream in(&infile);
  in.setVersion(QDataStream::Qt_4_0);
  in >> local
     >> remote;
  infile.close();
  return std::pair<QString,QString>(local,remote);
}


void SaveFile::load_header_info()
{
  cout << "SaveFile::load_header_info(" << qPrintable(_filename) << ")" << endl;
  QFile infile(_filename);
  if( !infile.open(QIODevice::ReadOnly) ){
    cout << "SaveFile::load_header_info() - Could not open file " << qPrintable(_filename) << "!!!" << endl;
    return;
  }

  QDataStream in(&infile);
  in.setVersion(QDataStream::Qt_4_0);
  in >> name
     >> local_dir
     >> remote_dir;
  infile.close();
  return;
}


bool SaveFile::load_from_file(QString filename)
{
  if(filename.isEmpty()) filename = _filename;

  cout << "SaveFile::load_from_file(" << qPrintable(filename) << ")" << endl;
  QFile infile(filename);
  if( !infile.open(QIODevice::ReadOnly) ){
    cout << "SaveFile::save_to_file() - Could not open file " << qPrintable(filename) << "!!!" << endl;
    return false;
  }

  QDataStream in(&infile);
  in.setVersion(QDataStream::Qt_4_0);
  in >> name
     >> local_dir
     >> remote_dir
     >> _local_files
     >> _remote_files;
  infile.close();
  return true;
}


bool SaveFile::save_to_file(QString filename)
{
  if(filename.isEmpty()) filename = _filename;

  cout << "SaveFile::save_to_file(" << qPrintable(filename) << ")" << endl;
  QFile outfile(filename);
  if( !outfile.open(QIODevice::WriteOnly) ){
    cout << "SaveFile::save_to_file() - Could not open file " << qPrintable(filename) << "!!!" << endl;
    return false;
  }

  QDataStream out(&outfile);
  out.setVersion(QDataStream::Qt_4_0);
  out << name
      << local_dir
      << remote_dir
      << _local_files
      << _remote_files;
  outfile.close();
  return true;
}
