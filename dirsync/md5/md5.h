#ifndef __MD5_H
#define __MD5_H

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

  unsigned long S(const unsigned long &, const unsigned int &) const;
  void PF(unsigned long &a, const unsigned long &b, const unsigned long &c, const unsigned long &d, 
	  const unsigned long &Xk, const unsigned int &s, const unsigned long &t);
  void PG(unsigned long &a, const unsigned long &b, const unsigned long &c, const unsigned long &d, 
	  const unsigned long &Xk, const unsigned int &s, const unsigned long &t);
  void PH(unsigned long &a, const unsigned long &b, const unsigned long &c, const unsigned long &d, 
	  const unsigned long &Xk, const unsigned int &s, const unsigned long &t);
  void PI(unsigned long &a, const unsigned long &b, const unsigned long &c, const unsigned long &d, 
	  const unsigned long &Xk, const unsigned int &s, const unsigned long &t);

  void get_ulong_le(unsigned long &, const unsigned char*, const unsigned int &);
  void put_ulong_le(const unsigned long &, unsigned char*, const unsigned int &);

  unsigned long _total[2];     // number of bytes processed
  unsigned long _state[4];     // intermediate digest state
  unsigned char _buffer[64];   // data block being processed
};

#endif // __MD5_H
