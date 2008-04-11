/*
 *  RFC 1321 compliant MD5 implementation
 *
 *  Copyright (C) 2006-2007  Christophe Devine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 *  The MD5 algorithm was designed by Ron Rivest in 1991.
 *
 *  http://www.ietf.org/rfc/rfc1321.txt
 */


#include "md5.h"

#include <cstring>
#include <algorithm>


//----------------------------------------------------------------------------------
// Constructor
//
md5::md5()
{ reset(); }

//----------------------------------------------------------------------------------
// reset internal variables to initial state
//
void md5::reset()
{
  _total[0] = 0;
  _total[1] = 0;

  _state[0] = 0x67452301;
  _state[1] = 0xEFCDAB89;
  _state[2] = 0x98BADCFE;
  _state[3] = 0x10325476;
}


//----------------------------------------------------------------------------------
// update MD5 w/ the data from a QByteArray
//
void md5::add_data(const QByteArray &buffer)
{
  const unsigned char* data = reinterpret_cast<const unsigned char*>( buffer.constData() );
  update(data, buffer.size());
  return;
}

//----------------------------------------------------------------------------------
// finalize MD5 digest & return QString of digest in hex
//
QString md5::get_hex_string()
{
  QString hash;
  char bits[3];  
  std::fill(bits, bits + sizeof(bits), '\0');

  unsigned char output[16];
  finish(output);
  for(uint i=0 ; i<16; i++) {
    snprintf(bits, sizeof(bits), "%02x", output[i]);
    hash += bits;
  }
  
  return hash;
}


//----------------------------------------------------------------------------------
// update MD5 with new input data
//
void md5::update( const unsigned char *input, int ilen )
{
  int fill;
  unsigned long left;

  if( ilen <= 0 )
    return;

  left = _total[0] & 0x3F;
  fill = 64 - left;

  _total[0] += ilen;
  _total[0] &= 0xFFFFFFFF;

  if( _total[0] < (unsigned long) ilen )
    _total[1]++;

  if( left && ilen >= fill ){
    memcpy( (void *) (_buffer + left),
	    (void *) input, fill );
    process( _buffer );
    input += fill;
    ilen  -= fill;
    left = 0;
  }

  while( ilen >= 64 ){
    process( input );
    input += 64;
    ilen  -= 64;
  }

  if( ilen > 0 ){
    memcpy( (void *) (_buffer + left),
	    (void *) input, ilen );
  }
  return;
}


//----------------------------------------------------------------------------------
// finalize MD5 digest
//
void md5::finish( unsigned char output[16] )
{
  static const unsigned char md5_padding[64] =
    {
      0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

  unsigned long last, padn;
  unsigned long high, low;
  unsigned char msglen[8];

  high = ( _total[0] >> 29 ) | ( _total[1] <<  3 );
  low  = ( _total[0] <<  3 );

  put_ulong_le( low,  msglen, 0 );
  put_ulong_le( high, msglen, 4 );

  last = _total[0] & 0x3F;
  padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

  update( (unsigned char *) md5_padding, padn );
  update( msglen, 8 );

  put_ulong_le( _state[0], output,  0 );
  put_ulong_le( _state[1], output,  4 );
  put_ulong_le( _state[2], output,  8 );
  put_ulong_le( _state[3], output, 12 );
}


//----------------------------------------------------------------------------------
// process buffer
//
void md5::process( const unsigned char data[64] )
{
  unsigned long X[16], A, B, C, D;

  get_ulong_le( X[ 0], data,  0 );
  get_ulong_le( X[ 1], data,  4 );
  get_ulong_le( X[ 2], data,  8 );
  get_ulong_le( X[ 3], data, 12 );
  get_ulong_le( X[ 4], data, 16 );
  get_ulong_le( X[ 5], data, 20 );
  get_ulong_le( X[ 6], data, 24 );
  get_ulong_le( X[ 7], data, 28 );
  get_ulong_le( X[ 8], data, 32 );
  get_ulong_le( X[ 9], data, 36 );
  get_ulong_le( X[10], data, 40 );
  get_ulong_le( X[11], data, 44 );
  get_ulong_le( X[12], data, 48 );
  get_ulong_le( X[13], data, 52 );
  get_ulong_le( X[14], data, 56 );
  get_ulong_le( X[15], data, 60 );

  A = _state[0];
  B = _state[1];
  C = _state[2];
  D = _state[3];

  PF( A, B, C, D,  X[0],  7, 0xD76AA478 );
  PF( D, A, B, C,  X[1], 12, 0xE8C7B756 );
  PF( C, D, A, B,  X[2], 17, 0x242070DB );
  PF( B, C, D, A,  X[3], 22, 0xC1BDCEEE );
  PF( A, B, C, D,  X[4],  7, 0xF57C0FAF );
  PF( D, A, B, C,  X[5], 12, 0x4787C62A );
  PF( C, D, A, B,  X[6], 17, 0xA8304613 );
  PF( B, C, D, A,  X[7], 22, 0xFD469501 );
  PF( A, B, C, D,  X[8],  7, 0x698098D8 );
  PF( D, A, B, C,  X[9], 12, 0x8B44F7AF );
  PF( C, D, A, B, X[10], 17, 0xFFFF5BB1 );
  PF( B, C, D, A, X[11], 22, 0x895CD7BE );
  PF( A, B, C, D, X[12],  7, 0x6B901122 );
  PF( D, A, B, C, X[13], 12, 0xFD987193 );
  PF( C, D, A, B, X[14], 17, 0xA679438E );
  PF( B, C, D, A, X[15], 22, 0x49B40821 );

  PG( A, B, C, D,  X[1],  5, 0xF61E2562 );
  PG( D, A, B, C,  X[6],  9, 0xC040B340 );
  PG( C, D, A, B, X[11], 14, 0x265E5A51 );
  PG( B, C, D, A,  X[0], 20, 0xE9B6C7AA );
  PG( A, B, C, D,  X[5],  5, 0xD62F105D );
  PG( D, A, B, C, X[10],  9, 0x02441453 );
  PG( C, D, A, B, X[15], 14, 0xD8A1E681 );
  PG( B, C, D, A,  X[4], 20, 0xE7D3FBC8 );
  PG( A, B, C, D,  X[9],  5, 0x21E1CDE6 );
  PG( D, A, B, C, X[14],  9, 0xC33707D6 );
  PG( C, D, A, B,  X[3], 14, 0xF4D50D87 );
  PG( B, C, D, A,  X[8], 20, 0x455A14ED );
  PG( A, B, C, D, X[13],  5, 0xA9E3E905 );
  PG( D, A, B, C,  X[2],  9, 0xFCEFA3F8 );
  PG( C, D, A, B,  X[7], 14, 0x676F02D9 );
  PG( B, C, D, A, X[12], 20, 0x8D2A4C8A );

  PH( A, B, C, D,  X[5],  4, 0xFFFA3942 );
  PH( D, A, B, C,  X[8], 11, 0x8771F681 );
  PH( C, D, A, B, X[11], 16, 0x6D9D6122 );
  PH( B, C, D, A, X[14], 23, 0xFDE5380C );
  PH( A, B, C, D,  X[1],  4, 0xA4BEEA44 );
  PH( D, A, B, C,  X[4], 11, 0x4BDECFA9 );
  PH( C, D, A, B,  X[7], 16, 0xF6BB4B60 );
  PH( B, C, D, A, X[10], 23, 0xBEBFBC70 );
  PH( A, B, C, D, X[13],  4, 0x289B7EC6 );
  PH( D, A, B, C,  X[0], 11, 0xEAA127FA );
  PH( C, D, A, B,  X[3], 16, 0xD4EF3085 );
  PH( B, C, D, A,  X[6], 23, 0x04881D05 );
  PH( A, B, C, D,  X[9],  4, 0xD9D4D039 );
  PH( D, A, B, C, X[12], 11, 0xE6DB99E5 );
  PH( C, D, A, B, X[15], 16, 0x1FA27CF8 );
  PH( B, C, D, A,  X[2], 23, 0xC4AC5665 );

  PI( A, B, C, D,  X[0],  6, 0xF4292244 );
  PI( D, A, B, C,  X[7], 10, 0x432AFF97 );
  PI( C, D, A, B, X[14], 15, 0xAB9423A7 );
  PI( B, C, D, A,  X[5], 21, 0xFC93A039 );
  PI( A, B, C, D, X[12],  6, 0x655B59C3 );
  PI( D, A, B, C,  X[3], 10, 0x8F0CCC92 );
  PI( C, D, A, B, X[10], 15, 0xFFEFF47D );
  PI( B, C, D, A,  X[1], 21, 0x85845DD1 );
  PI( A, B, C, D,  X[8],  6, 0x6FA87E4F );
  PI( D, A, B, C, X[15], 10, 0xFE2CE6E0 );
  PI( C, D, A, B,  X[6], 15, 0xA3014314 );
  PI( B, C, D, A, X[13], 21, 0x4E0811A1 );
  PI( A, B, C, D,  X[4],  6, 0xF7537E82 );
  PI( D, A, B, C, X[11], 10, 0xBD3AF235 );
  PI( C, D, A, B,  X[2], 15, 0x2AD7D2BB );
  PI( B, C, D, A,  X[9], 21, 0xEB86D391 );

  _state[0] += A;
  _state[1] += B;
  _state[2] += C;
  _state[3] += D;
}

//----------------------------------------------------------------------------------
// manipulation functions
//
inline unsigned long md5::S(const unsigned long &x, const unsigned int &n) const
{ return ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n))); }

inline void md5::PF(unsigned long &a, const unsigned long &b, const unsigned long &c, const unsigned long &d, 
		    const unsigned long &Xk, const unsigned int &s, const unsigned long &t)
{ 
  a += (d ^ (b & (c ^ d))) + Xk + t;
  a = S(a,s) + b;
}

inline void md5::PG(unsigned long &a, const unsigned long &b, const unsigned long &c, const unsigned long &d, 
		    const unsigned long &Xk, const unsigned int &s, const unsigned long &t)
{
  a += (c ^ (d & (b ^ c))) + Xk + t;
  a = S(a,s) + b;
}

inline void md5::PH(unsigned long &a, const unsigned long &b, const unsigned long &c, const unsigned long &d, 
		    const unsigned long &Xk, const unsigned int &s, const unsigned long &t)
{
  a += (b ^ c ^ d) + Xk + t;
  a = S(a,s) + b;
}

inline void md5::PI(unsigned long &a, const unsigned long &b, const unsigned long &c, const unsigned long &d, 
		    const unsigned long &Xk, const unsigned int &s, const unsigned long &t)
{
  a += (c ^ (b | ~d)) + Xk + t;
  a = S(a,s) + b;
}


//----------------------------------------------------------------------------------
// 32-bit integer manipulation macros (little endian)
//
inline void md5::get_ulong_le(unsigned long &n, const unsigned char* b, const unsigned int &i)
{
  n = (( (unsigned long) b[i])
       | ((unsigned long) b[i+1] <<  8)
       | ((unsigned long) b[i+2] << 16)
       | ((unsigned long) b[i+3] << 24));
}

inline void md5::put_ulong_le(const unsigned long &n, unsigned char* b, const unsigned int &i)
{
  b[i] = (unsigned char) n;
  b[i+1] = (unsigned char) (n>> 8);
  b[i+2] = (unsigned char) (n>>16);
  b[i+3] = (unsigned char) (n>>24);
}
