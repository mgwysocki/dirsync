#ifndef __SOCKETTOOL_H
#define __SOCKETTOOL_H

//-----------------------------------------------------------------
// The SocketTool class is the interface for reading & writing data
// over QTcpSockets.
// -----------------------------------------------------------------

#include "FileData.h"
class QTcpSocket;

class SocketTool
{
 public:
  static void send_handshake(QTcpSocket* socket, const quint32 &h);
  static void send_QString(QTcpSocket* socket, const QString s);
  static void send_block(QTcpSocket* socket, const QByteArray &b);
  static void send_QList_FileData(QTcpSocket* socket, const QList<FileData> &fdlist);
  static void send_FileData(QTcpSocket* socket, const FileData &fd);

  static quint32 get_handshake(QTcpSocket* socket);
  static QString get_QString(QTcpSocket* socket);
  static QByteArray get_block(QTcpSocket* socket);
  static QList<FileData> get_QList_FileData(QTcpSocket* socket);
  static FileData get_FileData(QTcpSocket* socket);
};

#endif //__SOCKETTOOL_H
