#include "SocketTool.h"

#include <QByteArray>
#include <QTcpSocket>

#include <iostream>
using namespace std;



//======================================================================================
// Sends handshake h over the socket
//
void SocketTool::send_handshake(QTcpSocket* socket, const quint32 &h)
{
  QDataStream tcp(socket);
  tcp.setVersion(QDataStream::Qt_4_0);
  tcp << h;
  socket->flush();
  return;
}

//======================================================================================
// Sends a QString over the socket
//
void SocketTool::send_QString(QTcpSocket* socket, const QString s)
{
  QDataStream tcp(socket);
  tcp.setVersion(QDataStream::Qt_4_0);
  tcp << (4 + 2*s.length())
      << s;
  return;
}

//======================================================================================
// Sends a QByteArray over the socket
//
void SocketTool::send_block(QTcpSocket* socket, const QByteArray &b)
{
  QDataStream tcp(socket);
  tcp.setVersion(QDataStream::Qt_4_0);
  tcp << b;
  return;
}

//======================================================================================
// Sends a QList<FileData> over the socket
//
void SocketTool::send_QList_FileData(QTcpSocket* socket, const QList<FileData> &fdlist)
{
  QDataStream tcp(socket);
  tcp.setVersion(QDataStream::Qt_4_0);
  quint32 listsize( fdlist.size() );

  tcp << listsize;
  for(quint32 i=0; i<listsize; i++)
    send_FileData( socket, fdlist[i] );
  return;
}

//======================================================================================
// Sends a FileData over the socket
//
void SocketTool::send_FileData(QTcpSocket* socket, const FileData &fd)
{
  cout << "SocketTool Sending: " << fd << endl;
  QDataStream tcp(socket);
  tcp.setVersion(QDataStream::Qt_4_0);
  tcp << fd.socket_size() << fd;
  return;
}


//======================================================================================
// Waits until enough data is available and then reads a quint32 from the socket
//
quint32 SocketTool::get_handshake(QTcpSocket* socket)
{
  quint32 handshake(0);
  QDataStream tcp(socket);
  tcp.setVersion(QDataStream::Qt_4_0);

  while(socket->bytesAvailable() < 4) {
    socket->waitForReadyRead();
  }
  tcp >> handshake;
  return handshake;
}


//======================================================================================
// Waits until enough data is available and then reads a quint32 from the socket
//
QString SocketTool::get_QString(QTcpSocket* socket)
{
  QDataStream tcp(socket);
  tcp.setVersion(QDataStream::Qt_4_0);

  quint32 string_size(0);
  QString s;
  while( socket->bytesAvailable() < 4) socket->waitForReadyRead();
  tcp >> string_size;
  while( socket->bytesAvailable() < string_size) socket->waitForReadyRead();
  tcp >> s;
  return s;
}


//======================================================================================
// Waits until enough data is available and then reads a block of data from the socket
//
QByteArray SocketTool::get_block(QTcpSocket* socket)
{
  QDataStream tcp(socket);
  tcp.setVersion(QDataStream::Qt_4_0);

  quint32 blocksize(0);
  while( socket->bytesAvailable()<4 ) socket->waitForReadyRead();
  tcp >> blocksize;

  while( socket->bytesAvailable() < blocksize) socket->waitForReadyRead(10000);
  return socket->read(blocksize);
}

//======================================================================================
// Reads a QList of FileData over the socket
//
QList<FileData> SocketTool::get_QList_FileData(QTcpSocket* socket)
{
  QDataStream tcp(socket);
  tcp.setVersion(QDataStream::Qt_4_0);

  quint32 listsize(0);
  QList<FileData> fdlist;
  while( socket->bytesAvailable()<4 ) socket->waitForReadyRead();
  tcp >> listsize;

  for(quint32 i=0; i<listsize; i++) {
    fdlist.append( get_FileData(socket) );
  }
  return fdlist;
}

//======================================================================================
// Reads a FileData over the socket
//
FileData SocketTool::get_FileData(QTcpSocket* socket)
{
  quint32 size(0);
  FileData fd;
  QDataStream tcp(socket);    // read the data serialized from the socket
  while(socket->bytesAvailable() < 4) socket->waitForReadyRead(10*1000);
  tcp >> size;

  while(socket->bytesAvailable() < size) socket->waitForReadyRead(10*1000);
  tcp >> fd;
  cout << "Received FileData:\n" << fd << endl;
  return fd;
}
