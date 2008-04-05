#include <QtGui>

#include <iostream>
using namespace std;

#include "PreferencesDialog.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
  QDialog(parent),
  _prefs(Preferences::get_instance())
{
  _tempdir_label = new QLabel(tr("Tempfile directory:"), this);
  _tempdir_lineedit = new QLineEdit( _prefs->get_temp_dir() );
  _tempdir_label->setBuddy(_tempdir_lineedit);

  _perms_checkbox = new QCheckBox(tr("Copy permission bits"), this);
  QLabel* perms_warning = new QLabel("(does not work well between Windows and Unix-like systems, including Mac OSX)", this);
  _apply_button = new QPushButton(tr("Apply"), this);
  _apply_button->setDefault(true);
  _cancel_button = new QPushButton(tr("Cancel"), this);
  connect(_cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

  _okay_button = new QPushButton(tr("Okay"), this);
  connect(_okay_button, SIGNAL(clicked()), this, SLOT(_set_preferences()));
  connect(_apply_button, SIGNAL(clicked()), this, SLOT(_set_preferences()));

  buttonBox = new QDialogButtonBox;
  buttonBox->addButton(_okay_button, QDialogButtonBox::AcceptRole);
  buttonBox->addButton(_cancel_button, QDialogButtonBox::RejectRole);

  QGridLayout *grid_layout = new QGridLayout;
  grid_layout->addWidget(_tempdir_label,    0, 0);
  grid_layout->addWidget(_tempdir_lineedit, 1, 0, 1, 2);
  grid_layout->addWidget(_perms_checkbox,   2, 0, 1, 2);
  grid_layout->addWidget(perms_warning,   3, 0, 1, 2);
  grid_layout->setRowStretch(4, 1);
  grid_layout->addWidget(_apply_button,     5, 0);
  grid_layout->addWidget(buttonBox,         5, 1);
  setLayout(grid_layout);

  this->setWindowTitle(tr("DirSync Preferences"));
  //this->resize(400,400);
}

void PreferencesDialog::_set_preferences()
{
  _prefs->set_temp_dir(_tempdir_lineedit->text());
  _prefs->set_copy_permission_bits(_perms_checkbox->isChecked());
  accept();
  return;
}
