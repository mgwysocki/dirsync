#include "SyncModel.h"

#include <iostream>
using namespace std;

SyncData::SyncData() :
  action(0)
{}

SyncData::SyncData(const SyncData &sd) :
  local(sd.local),
  remote(sd.remote),
  prev_local(sd.prev_local),
  prev_remote(sd.prev_remote),
  situ(sd.situ),
  action(sd.action)
{}

SyncData::SyncData(const FileData &localfd, const FileData &syncedfd, const FileData &remotefd, const int &act) :
  local(localfd),
  remote(remotefd),
  lastsync(syncedfd),
  action(act)
{}

//=================================================================================
//
SyncModel::SyncModel() :
  changes_compiled(false)
{}

QVariant SyncModel::data(const QModelIndex &index, int role = Qt::DisplayRole) const
{
  if(role != Qt::DisplayRole)
    return QVariant();

  int c = index.column();
  int r = index.row();
  if(c==0) {
    if(sync_list[r].local.initialized)
      return QVariant(tr("(A) ") + sync_list[r].local.relative_filename);
    else if(sync_list[r].remote.initialized)
      return QVariant(tr("(B) ") + sync_list[r].remote.relative_filename);
  } else if(c==1) {
    return QVariant(sync_list[r].situ);
  } else if(c==2) {
    return QVariant(Action::strings[ sync_list[r].action ]);
  }
  return QVariant();
}


QVariant SyncModel::headerData(int section, Qt::Orientation orient, int role = Qt::DisplayRole) const
{
  if(orient != Qt::Horizontal || role != Qt::DisplayRole) return QVariant();
  if(section == 0)
    return QVariant("File");
  else if(section == 1)
    return QVariant("Situation");
  else if(section == 2)
    return QVariant("Action");

  return QVariant();
}

bool SyncModel::_read_last_sync_file()
{
  QFile syncfile("lastsync.dat");
  if( !syncfile.open(QIODevice::ReadOnly) ){
    cout << "SyncModel::_read_last_sync_file() - Could not open sync file!!!" << endl;
    return false;
  }
  QDataStream in(&syncfile);
  in.setVersion(QDataStream::Qt_4_0);
  in >> _sync_files;
  syncfile.close();
  return true;
}

void SyncModel::save_sync_file()
{
  if(!_sync_files.size()) _make_fresh_sync_list();
  QFile syncfile("lastsync.dat");
  if( !syncfile.open(QIODevice::WriteOnly) ){
    cout << "SyncModel::_save_sync_file() - Could not open sync file!!!" << endl;
    return;
  }
  QDataStream out(&syncfile);
  out.setVersion(QDataStream::Qt_4_0);
  out << _sync_files;
  syncfile.close();
  return;
}

void SyncModel::_make_fresh_sync_list()
{
  QList<SyncData>::const_iterator siter = sync_list.begin();
  while(siter != sync_list.end()) {
    if(siter->action == Action::SendToServer)
      _sync_files.append(siter->local);
    else if(siter->action == Action::GetFromServer)
      _sync_files.append(siter->remote);
    siter++;
  }
  return;
}


void SyncModel::make_changes_list()
{
  cout << "SyncModel::make_changes_list()" << endl;;

  _read_last_sync_file();
//   if( ! _read_last_sync_file() ) {
//     modified_files += _current_files_list;
//     return;
//   }

  QList<FileData>::const_iterator liter = _local_files.begin();
  QList<FileData>::const_iterator riter = _remote_files.begin();
  QList<FileData>::const_iterator siter = _sync_files.begin();
  QList<SyncData> temp_list;
  if(sync_list.size() > 0) {
    beginRemoveRows( QModelIndex(), 0, sync_list.size()-1 );
    sync_list.clear();
    endRemoveRows();
  }


  //   N A M D  -- local
  // N c c c c
  // A c c
  // M c   c c
  // D c   x c

  // SHOULD PROBABLY ADD SIZE CHECKING AT SOME POINT
  while(liter != _local_files.end() || 
	riter != _remote_files.end() || 
	siter != _sync_files.end()) {

    if( liter==_local_files.end() ) cout << "liter at end" << endl;
    else cout << "liter: " << *liter << endl;
    if( riter==_remote_files.end() ) cout << "riter at end" << endl;
    else cout << "riter: " << *riter << endl;
    if( siter==_sync_files.end() ) cout << "siter at end" << endl;
    else cout << "siter: " << *siter << endl;

    if( liter!=_local_files.end() && riter==_remote_files.end() ) {
      if( siter==_sync_files.end() ) {
	// added to local dir
	SyncData sd(*liter, FileData(), FileData(), Action::SendToServer);
	sd.situ = tr("Added to A");
	temp_list.append( sd );
	++liter;
      } else if( liter->relative_filename == siter->relative_filename ) {
	// deleted from remote dir
	SyncData sd(*liter, *siter, FileData(), Action::DeleteFromClient);
	sd.situ = tr("Deleted from B");
	temp_list.append( sd );
	++liter;
	++siter;
      }
    } else if( liter==_local_files.end() && riter!=_remote_files.end() ) {
      if( siter==_sync_files.end() ) {
	// added to remote dir
	SyncData sd(FileData(), FileData(), *riter, Action::GetFromServer);
	sd.situ = tr("Added to B");
	temp_list.append( sd );
	++riter;
      } else if( riter->relative_filename == siter->relative_filename ) {
	// deleted from local dir
	SyncData sd(FileData(), *siter, *riter, Action::DeleteFromServer);
	sd.situ = tr("Deleted from A");
	temp_list.append( sd );
	++riter;
	++siter;
      }

    } else if( liter==_local_files.end() && riter==_remote_files.end() 
	       && siter!=_sync_files.end() ) {
      // deleted from both dirs
      SyncData sd(FileData(), *siter, FileData(), Action::None);
      sd.situ = tr("Deleted From Both");
      temp_list.append( sd );
      ++siter;

    } else if( siter==_sync_files.end() && liter!=_local_files.end() && riter!=_remote_files.end()) {
      if( liter->relative_filename < riter->relative_filename ) {
	// added to local dir
	SyncData sd(*liter, FileData(), FileData(), Action::SendToServer);
	sd.situ = tr("Added to A");
	temp_list.append( sd );
	++liter;

      } else if( liter->relative_filename > riter->relative_filename ) {
	// added to remote dir
	SyncData sd(FileData(), FileData(), *riter, Action::GetFromServer);
	sd.situ = tr("Added to B");
	temp_list.append( sd );
	++riter;

      } else if( liter->relative_filename == riter->relative_filename ) {
	// added to both dirs
	SyncData sd(*liter, FileData(), *riter, Action::Unclear);
	sd.situ = tr("Added to A and B");
	temp_list.append( sd );
	++liter;
	++riter;
      }
      
    }else if( liter->relative_filename < siter->relative_filename ) {
      if( liter->relative_filename < riter->relative_filename ) {
	// added to local dir
	SyncData sd(*liter, *siter, *riter, Action::SendToServer);
	sd.situ = tr("Added to A");
	temp_list.append( sd );
	++liter;

      } else if( liter->relative_filename == riter->relative_filename ) {
	// added to both dirs
	SyncData sd(*liter, *siter, *riter, Action::Unclear);
	sd.situ = tr("Added to both dirs");
	temp_list.append( sd );
	++liter;
	++riter;
      }

    } else if( liter->relative_filename > siter->relative_filename ) {
      if( liter->relative_filename > riter->relative_filename ) {
	if( siter->modtime == riter->modtime ) {
	  // deleted from local dir
	  SyncData sd(*liter, *siter, *riter, Action::DeleteFromServer);
	  sd.situ = tr("Deleted from A");
	  temp_list.append( sd );
	  ++siter;
	  ++riter;
	} else if( siter->modtime < riter->modtime ) {
	  // deleted from local dir & modified in remote dir
	  SyncData sd(*liter, *siter, *riter, Action::Unclear);
	  sd.situ = tr("Deleted from A and Modified in B");
	  temp_list.append( sd );
	  ++siter;
	  ++riter;
	}

      } else if( liter->relative_filename == riter->relative_filename ) {
	// deleted from both dirs
	SyncData sd(*liter, *siter, *riter, Action::None);
	sd.situ = tr("Deleted from both dirs");
	temp_list.append( sd );
	++siter;
      }

    } else if( riter->relative_filename > siter->relative_filename ) {
      if( liter->modtime == siter->modtime ) {
	// deleted from remote dir
	SyncData sd(*liter, *siter, *riter, Action::DeleteFromClient);
	sd.situ = tr("Deleted from B");
	temp_list.append( sd );
	++liter;
	++siter;
      } else if( liter->modtime > siter->modtime ) {
	// deleted from remote dir && modified in local dir
	SyncData sd(*liter, *siter, *riter, Action::Unclear);
	sd.situ = tr("Modified in A and Deleted from B");
	temp_list.append( sd );
	++liter;
	++siter;
      }

    } else if( riter->relative_filename < siter->relative_filename ) {
      // added to remote dir
      SyncData sd(*liter, *siter, *riter, Action::GetFromServer);
      sd.situ = tr("Added to B");
      temp_list.append( sd );
      ++riter;

    } else if( liter->modtime > siter->modtime ) {
      if( riter->modtime > siter->modtime ) {
	// modified in both dirs
	SyncData sd(*liter, *siter, *riter, Action::Unclear);
	sd.situ = tr("Modified in both dirs");
	temp_list.append( sd );
	++liter;
	++riter;
	++siter;

      } else if( riter->modtime == siter->modtime ) {
	// modified in local dir only
	SyncData sd(*liter, *siter, *riter, Action::SendToServer);
	sd.situ = tr("A modified");
	temp_list.append( sd );
	++liter;
	++riter;
	++siter;
      }

    } else if( riter->modtime > siter->modtime ) {
      // modified in remote dir only
      SyncData sd(*liter, *siter, *riter, Action::GetFromServer);
      sd.situ = tr("B modified");
      temp_list.append( sd );
      ++liter;
      ++riter;
      ++siter;
    } else {
      ++liter;
      ++riter;
      ++siter;
    }      
  }

  cout << temp_list.size() << " files added to the sync list." << endl;
  beginInsertRows( QModelIndex(), 0, temp_list.size()-1);
  sync_list = temp_list;
  endInsertRows();
  return;
}


void SyncModel::make_local_list()
{
  cout << "SyncModel::make_local_list()" << endl;
  _local_files.clear();
  QDir ld(_local_dir);
  if(!ld.exists()) {
    cout << "Local dir doesn't exist: " << qPrintable(_local_dir) << endl;
  }
  _read_dir(_local_dir, ld);  // recursively build _local_files
  return;
}

void SyncModel::construct(QList<FileData> remote)
{
  cout << "SyncModel::construct()" << endl;
  make_local_list();
  _remote_files = remote;
  make_changes_list();
  cout << "end SyncModel::construct()" << endl;
  return;
}


QList<FileData> SyncModel::get_files_to_send()
{ 
  if(!changes_compiled) _compile_changes();
  return _files_to_send; 
}

QList<FileData> SyncModel::get_files_to_get()
{
  if(!changes_compiled) _compile_changes();
  return _files_to_get; 
}

QList<FileData> SyncModel::get_remote_files_to_delete()
{
  if(!changes_compiled) _compile_changes();
  return _remote_files_to_delete; 
}

QList<FileData> SyncModel::get_local_files_to_delete()
{
  if(!changes_compiled) _compile_changes();
  return _local_files_to_delete; 
}


void SyncModel::set_action(const QModelIndexList &ilist, quint32 action)
{
  for(int i=0; i<ilist.size(); i++) {
    int row = ilist[i].row();
    sync_list[row].action = action;
    emit dataChanged(index(row, 0), index(row, 2));
  }
  return;
}


void SyncModel::_read_dir(QString dir, const QDir &basedir)
{
  QDir d(dir);
  QStringList subdirs = d.entryList((QDir::Dirs|QDir::NoDotAndDotDot), QDir::Name);
  QStringList files = d.entryList(QDir::Files, QDir::Name);

  for(int i=0; i<subdirs.size(); ++i)  {
    QString full_path = d.absoluteFilePath(subdirs[i]);
    QString relative_path = basedir.relativeFilePath(full_path);
    FileHandler fh(full_path);
    fh.get_fd().relative_filename = relative_path;
    _local_files.append(fh.get_fd());
    _read_dir( full_path, basedir );
  }

  for(int j=0; j<files.size(); ++j) {
    QString full_path = d.absoluteFilePath(files[j]);
    QString relative_path = basedir.relativeFilePath(full_path);
    FileHandler fh(full_path);
    fh.get_fd().relative_filename = relative_path;
    _local_files.append(fh.get_fd());
  }
  return;
}

void SyncModel::_compile_changes()
{
  cout << "SyncModel::_compile_changes()" << endl;
  QList<SyncData>::const_iterator sditer = sync_list.begin();
  while(sditer != sync_list.end()) {
    if( sditer->action == Action::SendToServer )
      _files_to_send.append( sditer->local );

    else if( sditer->action == Action::GetFromServer )
      _files_to_get.append( sditer->remote );

    else if( sditer->action == Action::DeleteFromServer )
      _remote_files_to_delete.append( sditer->remote );

    else if( sditer->action == Action::DeleteFromClient )
      _local_files_to_delete.append( sditer->local );
    ++sditer;
  }

  cout << _files_to_send.size() << " files to send" << endl;
  cout << _files_to_get.size() << " files to get" << endl;
  cout << _remote_files_to_delete.size() << " remote files to delete" << endl;
  cout << _local_files_to_delete.size() << " local files to delete" << endl;  
  changes_compiled = true;
  return;
}
