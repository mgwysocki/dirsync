#ifndef __PREFERENCES_H
#define __PREFERENCES_H

#include <QString>
#include <QSize>
#include <QPoint>

/* 
 preferences:
  window size, position
  temp file directory
  copy permission bits (doesn't work when going Unix <-> Windows)
*/

class Preferences
{
 public:
  ~Preferences();

  static Preferences* get_instance();

  bool load(QString filename = "prefs.dat");
  bool save(QString filename = "prefs.dat");

  QSize get_client_window_size() {return _size;}
  void set_client_window_size(const QSize size) {_size = size;}

  QPoint get_client_window_position() {return _position;}
  void set_client_window_position(const QPoint position) {_position = position;}

  QString get_temp_dir() {return _temp_dir;}
  void set_temp_dir(const QString dir) {_temp_dir = dir;}

  bool get_copy_permission_bits() {return _copy_permission_bits;}
  void set_copy_permission_bits(const bool b) {_copy_permission_bits = b;}

  QString get_dirsync_dir() {return _dirsync_dir;}

 private:
  Preferences();
  static Preferences* _singleton;
  static bool exists;

  QSize _size;
  QPoint _position;
  QString _temp_dir;
  QString _dirsync_dir;
  bool _copy_permission_bits;
  const quint32 _version;
};

#endif
