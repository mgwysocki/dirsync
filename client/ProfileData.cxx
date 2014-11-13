
#include "ProfileData.h"

#include <iostream>
using namespace std;

ProfileData::ProfileData(const QString &filename) :
  filename_(filename)
{}


std::pair<QString,QString> ProfileData::read_dirs_from_file(const QString &filename)
{
  QString local, remote;
  QFile infile(filename);
  if( !infile.open(QIODevice::ReadOnly) ){
    cout << "ProfileData::read_dirs_from_file() - Could not open file " << qPrintable(filename) << "!!!" << endl;
    return std::pair<QString,QString>(local,remote);
  }

  QDataStream in(&infile);
  in.setVersion(QDataStream::Qt_4_0);
  in >> local
     >> remote;
  infile.close();
  return std::pair<QString,QString>(local,remote);
}


void ProfileData::load_header_info()
{
  cout << "ProfileData::load_header_info(" << qPrintable(filename_) << ")" << endl;
  QFile infile(filename_);
  if( !infile.open(QIODevice::ReadOnly) ){
    cout << "ProfileData::load_header_info() - Could not open file " << qPrintable(filename_) << "!!!" << endl;
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


bool ProfileData::load_sync_data()
{
  QString sd_filename( filename_ );
  sd_filename.replace(".dat", "_sync_data.dat");

  cout << "ProfileData::load_sync_data(" << qPrintable(sd_filename) << ")" << endl;
  QFile infile(sd_filename);
  if( !infile.open(QIODevice::ReadOnly) ){
    cout << "ProfileData::load_sync_data() - Could not open file " << qPrintable(sd_filename) << "!!!" << endl;
    return false;
  }

  QDataStream in(&infile);
  in.setVersion(QDataStream::Qt_4_0);
  in >> sync_data_;
  infile.close();
  return true;
}


bool ProfileData::save_header_info()
{
  cout << "ProfileData::save_header_info(" << qPrintable(filename_) << ")" << endl;
  QFile outfile(filename_);
  if( !outfile.open(QIODevice::WriteOnly) ){
    cout << "ProfileData::save_header_info() - Could not open file "
         << qPrintable(filename_) << "!!!" << endl;
    return false;
  }

  QDataStream out(&outfile);
  out.setVersion(QDataStream::Qt_4_0);
  out << name
      << local_dir
      << remote_dir;
  outfile.close();
  return true;
}

bool ProfileData::save_sync_data()
{
  QString sd_filename( filename_ );
  sd_filename.replace(".dat", "_sync_data.dat");

  cout << "ProfileData::save_sync_data(" << qPrintable(sd_filename) << ")" << endl;
  QFile outfile(sd_filename);
  if( !outfile.open(QIODevice::WriteOnly) ){
    cout << "ProfileData::save_sync_data() - Could not open file "
         << qPrintable(sd_filename) << "!!!" << endl;
    return false;
  }

  QDataStream out(&outfile);
  out.setVersion(QDataStream::Qt_4_0);
  out << sync_data_;
  outfile.close();
  return true;
}
