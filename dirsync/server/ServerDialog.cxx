#include <QtGui>
#include <QtNetwork>
#include <QHostAddress>
#include <QDir>

#include <stdlib.h>
#include <sys/stat.h>
#include <utime.h>

#include <iostream>
using namespace std;

//#include "../protocol.h"
#include "ServerDialog.h"

ServerDialog::ServerDialog(QWidget *parent)
  : QDialog(parent)
{
  _status_label = new QLabel;
  _quit_button = new QPushButton(tr("Quit"));
  _quit_button->setAutoDefault(false);

  _status_label->setText(tr("The server is running on port %1.\n"
			    "Run the DirSync Client now.")
			 .arg(52614));

  connect(_quit_button, SIGNAL(clicked()), this, SLOT(close()));

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  buttonLayout->addStretch(1);
  buttonLayout->addWidget(_quit_button);
  buttonLayout->addStretch(1);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(_status_label);
  mainLayout->addLayout(buttonLayout);
  setLayout(mainLayout);

  this->setWindowTitle(tr("DirSync ServerDialog"));
  //_net_thread.start();
}
