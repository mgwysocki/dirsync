#include <QtGui>

#include <iostream>
using namespace std;

#include "NewSyncDialog.h"

ProfileReader::ProfileReader()
{read_profiles();}

void ProfileReader::read_profiles()
{
  profile_dir = Preferences::get_instance()->get_dirsync_dir();

  if(!profile_dir.exists(QObject::tr("profiles"))) {
    profile_dir.mkpath("profiles");
    profile_dir.cd("profiles");
  }

  profile_dir.cd("profiles");
  QList<SaveFile> temp_list;
  QFileInfoList profile_files = profile_dir.entryInfoList(QDir::Files);
  for(int i=0; i<profile_files.size(); i++) {
    SaveFile sf(profile_files[i].absoluteFilePath());
    sf.load_header_info();
    temp_list.append(sf);
  }

  beginInsertRows( QModelIndex(), 0, temp_list.size()-1);
  profile_list = temp_list;
  endInsertRows();
  return;
}

void ProfileReader::update_profile(const QModelIndex &index, QString name, QString clientdir, QString serverdir)
{
  int r = index.row();
  profile_list[r].name = name;
  if(profile_list[r].local_dir != clientdir)
    profile_list[r].local_dir = clientdir;
  if(profile_list[r].remote_dir != serverdir)
    profile_list[r].remote_dir = serverdir;

  emit dataChanged(index, index);

  profile_list[r].save_to_file();
  return;
}

QModelIndex ProfileReader::add_new_profile()
{
  quint32 maxnum(0);
  for(int i=0; i<profile_list.size(); i++) {
    cout << "profile_list[" << i << "].get_filenumber() = " << profile_list[i].get_filenumber() << endl;
    if(profile_list[i].get_filenumber() > maxnum)
      maxnum = profile_list[i].get_filenumber();
  }
  QString filename = QString("%1/%2.dat").arg(profile_dir.absolutePath()).arg(maxnum+1);
  filename = QDir::toNativeSeparators(filename);

  SaveFile sf(filename);
  sf.name = tr("New Profile");
  sf.save_to_file();

  beginInsertRows( QModelIndex(), profile_list.size()-1, profile_list.size());
  profile_list.append(sf);
  endInsertRows();

  return index(profile_list.size()-1);
}


QVariant ProfileReader::data(const QModelIndex &index, int role = Qt::DisplayRole) const
{
  int r = index.row();

  if(role != Qt::DisplayRole)
    return QVariant();

  if(r<profile_list.size())
    return QVariant(profile_list[r].name);
  
  return QVariant();
}

SaveFile ProfileReader::get_profile(const QModelIndex &index) const
{
  int r = index.row();
  if(r<profile_list.size())
    return profile_list[r];
  return SaveFile();
}

QString ProfileReader::get_name(const QModelIndex &index) const
{
  int r = index.row();
  if(r<profile_list.size())
    return profile_list[r].name;
  return QString();
}

QString ProfileReader::get_client_dir(const QModelIndex &index) const
{
  int r = index.row();
  if(r<profile_list.size())
    return profile_list[r].local_dir;
  return QString();
}

QString ProfileReader::get_server_dir(const QModelIndex &index) const
{
  int r = index.row();
  if(r<profile_list.size())
    return profile_list[r].remote_dir;
  return QString();
}



NewSyncDialog::NewSyncDialog(QWidget *parent)
  : QDialog(parent)
{
  _host_label = new QLabel(tr("Server (name or IP):"));
  _port_label = new QLabel(tr("Server port:"));
  _name_label = new QLabel(tr("Profile Name:"));
  _clientdir_label = new QLabel(tr("Client (local) Directory:"));
  _serverdir_label = new QLabel(tr("Server Directory:"));

  _name_lineedit = new QLineEdit;
  _name_lineedit->setEnabled(false);
  _host_lineedit = new QLineEdit;
  _port_lineedit = new QLineEdit;
  _port_lineedit->setValidator(new QIntValidator(1, 65535, this));
  _clientdir_lineedit = new QLineEdit;
  _clientdir_lineedit->setEnabled(false);
  _serverdir_lineedit = new QLineEdit;
  _serverdir_lineedit->setEnabled(false);

  _name_label->setBuddy(_name_lineedit);
  _host_label->setBuddy(_host_lineedit);
  _port_label->setBuddy(_port_lineedit);
  _clientdir_label->setBuddy(_clientdir_lineedit);
  _serverdir_label->setBuddy(_serverdir_lineedit);

  _profiles_label = new QLabel(tr("Profiles:"));
  _profiles_lv = new QListView;
  _profiles_lv->setModel(&_profile_reader);
  _profiles_lv->setSelectionMode(QAbstractItemView::SingleSelection);

  _go_button = new QPushButton(tr("Make Sync List"), this);
  _go_button->setDefault(true);
  //_go_button->setEnabled(false);

  _new_profile_button = new QPushButton(tr("New Profile"));
  connect(_new_profile_button, SIGNAL(clicked()), this, SLOT(start_new_profile()));

  _update_profile_button = new QPushButton(tr("Save Profile"));
  connect(_update_profile_button, SIGNAL(clicked()), this, SLOT(save_profile()));

  _quit_button = new QPushButton(tr("Cancel"));

  buttonBox = new QDialogButtonBox;
  buttonBox->addButton(_go_button, QDialogButtonBox::AcceptRole);
  buttonBox->addButton(_quit_button, QDialogButtonBox::RejectRole);

  QHBoxLayout* buttonBox2 = new QHBoxLayout;
  buttonBox2->addWidget(_new_profile_button);
  buttonBox2->addWidget(_update_profile_button);

  connect(_host_lineedit, SIGNAL(textChanged(const QString &)),
	  this, SLOT(enable_go_button()));
  connect(_port_lineedit, SIGNAL(textChanged(const QString &)),
	  this, SLOT(enable_go_button()));
  connect(_go_button, SIGNAL(clicked()), this, SLOT(accept()));
  connect(_quit_button, SIGNAL(clicked()), this, SLOT(reject()));

  QVBoxLayout* main_layout = new QVBoxLayout;
  QGroupBox* server_box = new QGroupBox(tr("Server"));
  QGridLayout *server_layout = new QGridLayout;
  server_layout->addWidget(_host_label,         0, 0);
  server_layout->addWidget(_host_lineedit,      0, 1);

  server_layout->addWidget(_port_label,         1, 0);
  server_layout->addWidget(_port_lineedit,      1, 1);
  server_box->setLayout(server_layout);
  main_layout->addWidget(server_box);

  QGroupBox* profile_box = new QGroupBox(tr("Profile"));
  QGridLayout *profile_layout = new QGridLayout;
  profile_layout->addWidget(_name_label,         0, 0);
  profile_layout->addWidget(_name_lineedit,      0, 1);

  profile_layout->addWidget(_clientdir_label,    1, 0);
  profile_layout->addWidget(_clientdir_lineedit, 1, 1);

  profile_layout->addWidget(_serverdir_label,    2, 0);
  profile_layout->addWidget(_serverdir_lineedit, 2, 1);

  profile_layout->addLayout(buttonBox2,          3, 0, 1, 2);
  profile_box->setLayout(profile_layout);
  main_layout->addWidget(profile_box);

  main_layout->addWidget(_profiles_label);//,     4, 0, 1, 2);
  main_layout->addWidget(_profiles_lv);//,        5, 0, 1, 2);

  main_layout->addWidget(buttonBox);
  this->setLayout(main_layout);

  this->setWindowTitle(tr("DirSync Setup"));
  _host_lineedit->setFocus();

  this->resize(400,400);

  connect(_profiles_lv->selectionModel(), 
	  SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), 
	  this, SLOT(change_selected_profile(const QModelIndex &, const QModelIndex &)));

  // TEST SETUP
  _host_lineedit->setText("localhost");
  _port_lineedit->setText("52614");
  _clientdir_lineedit->setText("/home/mwysocki/testsync/");
  _serverdir_lineedit->setText("/home/mwysocki/testsync2/");
  _go_button->setEnabled(true);

  if( _profile_reader.rowCount(QModelIndex()) > 0 ){
    QModelIndex first_index = _profile_reader.index(0,0);
    _profiles_lv->selectionModel()->select(first_index, QItemSelectionModel::Select);
    change_selected_profile(first_index, QModelIndex());
  } else {
    start_new_profile();
  }
}


void NewSyncDialog::enable_go_button()
{
  _go_button->setEnabled(!_host_lineedit->text().isEmpty() &&
			 !_port_lineedit->text().isEmpty() &&
			 !_clientdir_lineedit->text().isEmpty() &&
			 !_serverdir_lineedit->text().isEmpty());
}

void NewSyncDialog::start_new_profile()
{
  _name_lineedit->setEnabled(true);
  _name_lineedit->setText("New Profile");
  _name_lineedit->selectAll();
  _clientdir_lineedit->setEnabled(true);
  _serverdir_lineedit->setEnabled(true);

  _profiles_lv->selectionModel()->clearSelection();  
  QModelIndex new_entry = _profile_reader.add_new_profile();
  QItemSelection selection(new_entry, new_entry);
  _profiles_lv->selectionModel()->select(selection, QItemSelectionModel::Select);

  _name_lineedit->setFocus();
  return;
}


void NewSyncDialog::save_profile()
{
  QString name = _name_lineedit->text();
  QString clientdir = _clientdir_lineedit->text();
  QString serverdir = _serverdir_lineedit->text();
  _clientdir_lineedit->setEnabled(false);
  _serverdir_lineedit->setEnabled(false);

  QModelIndexList indexes = _profiles_lv->selectionModel()->selectedIndexes();
  QModelIndex index;

  foreach(index, indexes) {
    _profile_reader.update_profile(index, name, clientdir, serverdir);
  }

  return;
}

SaveFile NewSyncDialog::get_current_profile()
{
  QModelIndexList indexes = _profiles_lv->selectionModel()->selectedIndexes();
  if(!indexes.size())
    return SaveFile();

  QModelIndex index = indexes[0];
  return _profile_reader.get_profile(index);
}

void NewSyncDialog::change_selected_profile(const QModelIndex &current, const QModelIndex &)
{
  cout << "NewSyncDialog::change_selected_profile()" << endl;
  _name_lineedit->setText( _profile_reader.get_name(current) );
  _name_lineedit->setEnabled(true);
  _clientdir_lineedit->setText( _profile_reader.get_client_dir(current) );
  _serverdir_lineedit->setText( _profile_reader.get_server_dir(current) );
  
  return;
}
