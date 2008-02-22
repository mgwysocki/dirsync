#include <QtGui>

#include <iostream>
using namespace std;

#include "ClientDialog.h"

ClientDialog::ClientDialog(QWidget *parent)
  : QDialog(parent)
{
  _host_label = new QLabel(tr("&Server name:"));
  _port_label = new QLabel(tr("S&erver port:"));
  _clientdir_label = new QLabel(tr("ClientDialog (local) Directory:"));
  _serverdir_label = new QLabel(tr("Server Directory:"));

  _host_lineedit = new QLineEdit;
  _port_lineedit = new QLineEdit;
  _port_lineedit->setValidator(new QIntValidator(1, 65535, this));
  _clientdir_lineedit = new QLineEdit;
  _serverdir_lineedit = new QLineEdit;

  _host_label->setBuddy(_host_lineedit);
  _port_label->setBuddy(_port_lineedit);
  _clientdir_label->setBuddy(_clientdir_lineedit);
  _serverdir_label->setBuddy(_serverdir_lineedit);

  _status_label = new QLabel(tr("This examples requires that you run the "
				"Fortune Server example as well."));

  _go_button = new QPushButton(tr("Make Sync List"));
  _go_button->setDefault(true);
  _go_button->setEnabled(false);

  _quit_button = new QPushButton(tr("Cancel"));

  buttonBox = new QDialogButtonBox;
  buttonBox->addButton(_go_button, QDialogButtonBox::ActionRole);
  buttonBox->addButton(_quit_button, QDialogButtonBox::RejectRole);

  connect(_host_lineedit, SIGNAL(textChanged(const QString &)),
	  this, SLOT(enable_go_button()));
  connect(_port_lineedit, SIGNAL(textChanged(const QString &)),
	  this, SLOT(enable_go_button()));
  connect(_go_button, SIGNAL(clicked()), this, SLOT(accept()));
  connect(_quit_button, SIGNAL(clicked()), this, SLOT(reject()));

  QGridLayout *mainLayout = new QGridLayout;
  mainLayout->addWidget(_host_label, 0, 0);
  mainLayout->addWidget(_host_lineedit, 0, 1);

  mainLayout->addWidget(_port_label, 1, 0);
  mainLayout->addWidget(_port_lineedit, 1, 1);

  mainLayout->addWidget(_clientdir_label, 2, 0);
  mainLayout->addWidget(_clientdir_lineedit, 2, 1);

  mainLayout->addWidget(_serverdir_label, 3, 0);
  mainLayout->addWidget(_serverdir_lineedit, 3, 1);

  mainLayout->addWidget(_status_label, 4, 0, 1, 2);
  mainLayout->addWidget(buttonBox, 5, 0, 1, 2);
  this->setLayout(mainLayout);

  this->setWindowTitle(tr("DirSync Setup"));
  _host_lineedit->setFocus();


  // TEST SETUP
  _host_lineedit->setText("Localhost");
  _port_lineedit->setText("52614");
  _clientdir_lineedit->setText("/home/mwysocki/testsync/");
  _serverdir_lineedit->setText("/home/mwysocki/testsync2/");
  _go_button->setEnabled(true);
}


void ClientDialog::enable_go_button()
{
  _go_button->setEnabled(!_host_lineedit->text().isEmpty()
			 && !_port_lineedit->text().isEmpty()
			 && !_clientdir_lineedit->text().isEmpty()
			 && !_serverdir_lineedit->text().isEmpty());
}
