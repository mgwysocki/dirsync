#include <QFile>
#include <QDir>
#include <QDataStream>

#include <iostream>

#include "Preferences.h"

bool Preferences::exists(false);
Preferences* Preferences::_singleton(0);

Preferences::Preferences() :
  _size(800, 600),
  _copy_permission_bits(false)
{ 
  QDir data_dir = QDir::home();
#ifdef Q_WS_WIN
  QString dirname( "DirSync" );
#else
  QString dirname( ".dirsync" );
#endif

  if( !data_dir.exists( dirname ) ){
    if( !data_dir.mkpath( dirname ) ){
      std::cout << "Could not create directory " << qPrintable( data_dir.absoluteFilePath(dirname) ) << std::endl;
      return;
    }
  }

  data_dir.cd(dirname);
  _dirsync_dir = data_dir.absolutePath();
  _temp_dir = _dirsync_dir;
}

Preferences::~Preferences()
{ exists = false; }

Preferences* Preferences::get_instance()
{
  if( !exists ){
    _singleton = new Preferences;
    exists = true;
  }
  return _singleton;
}



bool Preferences::load(QString filename)
{
  QFile file(filename);
  if( !file.open(QIODevice::ReadOnly) )
    return false;

  quint32 temp(0);
  QDataStream in(&file);    // read the data serialized from the file
  in >> _size >> _position >> _temp_dir >> temp;
  file.close();
  _copy_permission_bits = (temp&1);
  printf("Loaded QSize(%d, %d), QPoint(%d, %d), QString(\"%s\"), int(%d)\n",
	 _size.width(), _size.height(), _position.x(), _position.y(), 
	 qPrintable(_temp_dir), temp);
  return true;
}

bool Preferences::save(QString filename)
{
  QFile file(filename);
  if( !file.open(QIODevice::WriteOnly) )
    return false;

  QDataStream out(&file);   // we will serialize the data into the file
  out << _size << _position << _temp_dir << quint32(_copy_permission_bits);
  file.close();

  printf("Saved QSize(%d, %d), QPoint(%d, %d), QString(\"%s\"), int(%d)\n",
	 _size.width(), _size.height(), _position.x(), _position.y(), 
	 qPrintable(_temp_dir), quint32(_copy_permission_bits));
  return true;
}
