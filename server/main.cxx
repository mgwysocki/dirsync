/****************************************************************************
**
** Copyright (C) 2004-2006 Trolltech ASA. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QApplication>
#include <iostream>
#include "ServerDialog.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  
  bool nogui(false);
  int port(52614);

  for(int i=0; i<app.arguments().size(); i++) {
    QString arg = app.arguments().at(i);
    if(arg == QString("-nw")) 
      nogui = true;
    else if(arg == QString("-p")) {
      if( i<app.arguments().size()-1 ) {
	port = app.arguments().at(i+1).toInt();
      } else {
	std::cerr << "No port number following option -p" << std::endl;
	return 2;
      }
    }
  }

  ServerDialog* server;
  if(!nogui) {
    server = new ServerDialog(port);
    server->show();
  }

  NetworkServerThread _net_thread(port);
  return app.exec();
}
