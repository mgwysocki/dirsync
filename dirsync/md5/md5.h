/**
 * \file md5.h
 */
#ifndef XYSSL_MD5_H
#define XYSSL_MD5_H

#include <QString>

class md5
{
 public:
  md5();
  void reset();
  void add_data(const QByteArray &);
  QString get_hex_string();

 private:
  void update( const unsigned char *input, int ilen );
  void process( const unsigned char * );
  void finish( unsigned char output[16] );

  unsigned long _total[2];     /*!< number of bytes processed  */
  unsigned long _state[4];     /*!< intermediate digest state  */
  unsigned char _buffer[64];   /*!< data block being processed */

  static const unsigned char md5_padding[64];
};

#endif /* md5.h */
