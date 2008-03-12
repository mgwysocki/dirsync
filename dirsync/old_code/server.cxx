#include <QtGui>
#include <QtNetwork>
#include <QHostAddress>
#include <QDir>

#include <stdlib.h>
#include <sys/stat.h>
#include <utime.h>

#include <iostream>
using namespace std;

#include "../protocol.h"
#include "server.h"

Server::Server(QWidget *parent)
  : QDialog(parent),
    _socket(0)
{
  _status_label = new QLabel;
  _quit_button = new QPushButton(tr("Quit"));
  _quit_button->setAutoDefault(false);

  _server = new QTcpServer(this);
  if (!_server->listen(QHostAddress::Any, 52614)) {
    QMessageBox::critical(this, tr("DirSync Server"),
			  tr("Unable to start the server: %1.")
			  .arg(_server->errorString()));
    this->close();
    return;
  }

  _status_label->setText(tr("The server is running on port %1.\n"
			    "Run the DirSync Client now.")
			 .arg(_server->serverPort()));

  connect(_quit_button, SIGNAL(clicked()), this, SLOT(close()));

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  buttonLayout->addStretch(1);
  buttonLayout->addWidget(_quit_button);
  buttonLayout->addStretch(1);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(_status_label);
  mainLayout->addLayout(buttonLayout);
  setLayout(mainLayout);

  this->setWindowTitle(tr("DirSync Server"));
}


void Server::make_local_changes_list()
{
  cout << "Server::make_local_changes_list()" << endl;;
  _local_changes.make_changes_list();
  return;
}



void Server::send_list( const QList<FileData> &l ) 
{
  cout << "Server::send_list()" << endl;;
  QDataStream tcp(_socket);
  tcp.setVersion(QDataStream::Qt_4_0);
  tcp << l;
  return;
}

void Server::send_file(const FileData &fd)
{
  QDataStream tcp(_socket);
  tcp.setVersion(QDataStream::Qt_4_0);

  //tcp >> filename;  // this should be  relative to the Music Directory

  //QFileInfo fi(_local_changes.get_dir(), filename);
  QFile infile(fd.filename);
  infile.open(QIODevice::ReadOnly);
  QByteArray block( infile.readAll() );  
  tcp << block;

  printf("Send filename: %s\n", fd.filename.toAscii().constData() );
  printf("Bytes to send: %ld\n", tcp.device()->bytesToWrite() );

  infile.close();  
  return;
}

void Server::receive_file(const FileData &fd)
{
  cout << "Receiving file: " << qPrintable(fd.relative_filename) << endl;
  QDataStream tcp(_socket);
  tcp.setVersion(QDataStream::Qt_4_0);

  while(_socket->bytesAvailable() < fd.size) continue;

  QByteArray block( _socket->readAll() );  

  QString full_path = QDir::cleanPath(_local_changes.get_dir() + "/" + fd.relative_filename);
  cout << "Writing file: " << qPrintable(full_path) << endl;
  QFile outfile(full_path);
  outfile.open(QIODevice::WriteOnly);
  outfile.write( block );
  outfile.close();

  printf("Received filename: %s (%d bytes)\n", fd.filename.toAscii().constData(),
	 block.size());

  struct utimbuf ubuf;
  ubuf.modtime = fd.modtime;
  utime(fd.filename.toAscii().constData(), &ubuf);
  return;
}


void Server::send_data()
{
  QFile infile(_filename);
  quint64 filesize( infile.size() );

  struct stat buf;
  stat(_filename.toAscii().constData(), &buf);
  printf("Acc time: %d\n", buf.st_atime);
  printf("Mod time: %d\n", buf.st_mtime);
  printf("Sizeof mod time: %d\n", sizeof(buf.st_mtime));
  quint32 modtime(buf.st_mtime);
  float fmodtime(buf.st_mtime);
  printf("quint32 mod time: %d\n", modtime);
  printf("float mod time: %f\n", fmodtime);

  quint64 blocksize(65536);
  quint64 written(0);

  infile.open(QIODevice::ReadOnly);

  QByteArray block( infile.readAll() );
  
  _socket = _server->nextPendingConnection();
  connect(_socket, SIGNAL(readyRead()),
	  this, SLOT(read_incoming()));
  connect(_socket, SIGNAL(disconnected()),
	  _socket, SLOT(deleteLater()));

  printf("Bytearray size: %d\n", block.size());

  QDataStream out(_socket);
  out.setVersion(QDataStream::Qt_4_0);
  
  out << _filename;
  out << modtime;
  out << block;

  printf("Send filename: %s\n", _filename.toAscii().constData() );
  printf("Bytes to send: %ld\n", out.device()->bytesToWrite() );

  _socket->disconnectFromHost();
  infile.close();
}

