
#include "NetworkServerThread.h"

#include <iostream>
using namespace std;

NetworkServerThread::NetworkServerThread(int port, QObject* parent) :
  QThread(parent),
  _netserver(0),
  _port(port),
  _stop(true),
  _quit(false)
{
  start();
}

NetworkServerThread::~NetworkServerThread()
{
  cout << "NetworkServerThread::~NetworkServerThread()" << endl;
  _quit = true;
  _netserver->deleteLater();

  cout << "exit(0); wait();" << endl;
  exit(0);   // Tell the child thread's event loop to finish
  wait();    // Wait for child thread to return from exec() (and hence run())
}


void NetworkServerThread::run()
{
  cout << "NetworkServerThread::run()" << endl;
  _mutex.lock();
  _netserver = new NetworkServer(_port);
  _mutex.unlock();

  exec();
  cout << "END NetworkServerThread::run()" << endl;
  return;
}
