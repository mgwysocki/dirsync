#ifndef NETWORKSERVERTHREAD_H
#define NETWORKSERVERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "NetworkServer.h"

class NetworkServerThread : public QThread
{
Q_OBJECT

 public:
  NetworkServerThread(int port, QObject* parent = 0);
  ~NetworkServerThread();

  void reset_server();
  void run();

 signals:
  void error(const QString &);

/*  public slots: */
/*   void wake_up(); */

 private:
  NetworkServer* _netserver;
  QMutex _mutex;
  QWaitCondition _cond;
  int _port;
  bool _stop;
  bool _quit;
};

#endif
