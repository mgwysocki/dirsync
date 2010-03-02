
#include "SyncModel.h"

#include <iostream>
using namespace std;

//=================================================================================
//
SyncModel::SyncModel() :
  _loaded_previous_state(false),
  profile_data_(0),
  _diff_only(false),
  changes_compiled(false)
{}

QVariant SyncModel::data(const QModelIndex &index, int role = Qt::DisplayRole) const
{
  if(role != Qt::DisplayRole)
    return QVariant();

  int r = index.row();
  if(r<0 || r>=sync_list.size()) return QVariant();
  if(_diff_only && r>=diff_list.size()) return QVariant();

  int c = index.column();
  SyncData sd = _diff_only ? *diff_list[r] : sync_list[r];
  if(c==0) {
    QString var(tr(" ") + sd.local.current_fd.relative_filename);
    if(sd.local.current_fd.isdir)
      var += tr("/");
    return  QVariant(var);
  } else if(c==1) {
    return QVariant(sd.situ);
  } else if(c==2) {
    return QVariant(Action::strings[ sd.action ]);
  } else if(c==3) {
    QString var(tr(" ") + sd.remote.current_fd.relative_filename);
    if(sd.remote.current_fd.isdir)
      var += tr("/");
    return  QVariant(var);
  }
  return QVariant();
}


QVariant SyncModel::headerData(int section, Qt::Orientation orient, int role = Qt::DisplayRole) const
{
  if(orient != Qt::Horizontal || role != Qt::DisplayRole) return QVariant();
  if(section == 0)
    return QVariant("Client File");
  else if(section == 1)
    return QVariant("Situation");
  else if(section == 2)
    return QVariant("Action");
  else if(section == 3)
    return QVariant("Server File");

  return QVariant();
}


void SyncModel::set_action(const QModelIndexList &ilist, quint32 action)
{
  for(int i=0; i<ilist.size(); i++) {
    int row = ilist[i].row();
    if(_diff_only)
      diff_list[row]->action = action;
    else
      sync_list[row].action = action;

    emit dataChanged(index(row, 0), index(row, 2));
  }
  return;
}

void SyncModel::set_sync_to_client(const QModelIndexList &ilist)
{
  for(int i=0; i<ilist.size(); i++) {
    int row = ilist[i].row();
    SyncData sd = _diff_only ? *diff_list[row] : sync_list[row];
    if(sd.local.deleted || !sd.local.current_fd.initialized)
      sd.action = Action::DeleteFromServer;
    else
      sd.action = Action::SendToServer;

    if(_diff_only)      
      *diff_list[row] = sd;
    else
      sync_list[row] = sd;

    emit dataChanged(index(row, 0), index(row, 2));
  }
  return;
}

void SyncModel::set_sync_to_server(const QModelIndexList &ilist)
{
  for(int i=0; i<ilist.size(); i++) {
    int row = ilist[i].row();
    SyncData sd = _diff_only ? *diff_list[row] : sync_list[row];
    if(sd.remote.deleted || !sd.remote.current_fd.initialized)
      sd.action = Action::DeleteFromClient;
    else
      sd.action = Action::GetFromServer;

    if(_diff_only)      
      *diff_list[row] = sd;
    else
      sync_list[row] = sd;

    emit dataChanged(index(row, 0), index(row, columnCount(QModelIndex())));
  }
  return;
}


void SyncModel::make_local_list()
{
  _local_dirdata.generate_current_list();
  _local_dirdata.generate_file_agents();
  return;
}


bool SyncModel::read_sync_file()
{
  profile_data_->load_sync_data();

  QList<SyncData>::const_iterator olditer = profile_data_->get_sync_data().begin();
  while(olditer != profile_data_->get_sync_data().end()) {

    bool found(false);
    QList<SyncData>::iterator newiter = sync_list.begin();
    while(newiter != sync_list.end()) {
      if( olditer->relative_filename == newiter->relative_filename ){
        newiter->local.previous_fd = olditer->local.current_fd;
        newiter->remote.previous_fd = olditer->remote.current_fd;
        found = true;
        break;
      }
      newiter++;
    }

    if( !found ){
      FileAgent local_fa(olditer->local.current_fd, FileData());
      FileAgent remote_fa(olditer->remote.current_fd, FileData());
      SyncData sd(local_fa, remote_fa, _loaded_previous_state);
      sync_list.append(sd);
    }
    olditer++;
  }

  qStableSort( sync_list );
  return true;
}

void SyncModel::save_sync_file()
{
  if( sync_list.size()==0) return;

  profile_data_->set_sync_data(sync_list);
  profile_data_->save_sync_data();

  return;
}


void SyncModel::make_changes_list()
{
  cout << "SyncModel::make_changes_list()" << endl;;

  // Clear the sync list
  if(sync_list.size() > 0) {
    beginRemoveRows( QModelIndex(), 0, rowCount(QModelIndex())-1 );
    diff_list.clear();
    sync_list.clear();
    endRemoveRows();
  }


  // Make our temporary lists of FileAgents
  QList<FileAgent> local_files = _local_dirdata.file_agents;
  QList<FileAgent> remote_files = _remote_dirdata.file_agents;
  QList<SyncData> temp_sync_list;


  //   N A M D  -- local
  // N c c c c
  // A c c
  // M c   c c
  // D c   x c

  // Loop over local FileAgents
  //  Loop over remote FileAgents
  //   If match, make SyncData
  //   Delete remote FileAgent
  //  If no match, make SyncData
  // Loop over remaining remote FileAgents
  //  Make SyncData

  QList<FileAgent>::const_iterator liter = local_files.begin();
  while(liter != local_files.end()) {

    bool found(false);
    QList<FileAgent>::iterator riter = remote_files.begin();
    while(riter != remote_files.end()) {
      if( liter->relative_filename == riter->relative_filename ){
        SyncData sd(*liter, *riter, !_loaded_previous_state);
        temp_sync_list.append(sd);
        remote_files.erase(riter);
        found = true;
        break;
      }
      riter++;
    }

    if( !found ){
      SyncData sd(*liter, FileAgent(), !_loaded_previous_state);
      temp_sync_list.append(sd);
    }
    liter++;
  }

  QList<FileAgent>::const_iterator riter = remote_files.begin();
  while(riter != remote_files.end()) {
    SyncData sd(FileAgent(), *riter, !_loaded_previous_state);
    temp_sync_list.append(sd);
    riter++;
  }

  qStableSort( temp_sync_list );

  //read_sync_file();

  cout << temp_sync_list.size() << " files added to the sync list." << endl;
  sync_list = temp_sync_list;
  _generate_diff_list();
  if(rowCount(QModelIndex())>0) {
    beginInsertRows( QModelIndex(), 0, rowCount(QModelIndex())-1);
    endInsertRows();
  }
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


void SyncModel::_compile_changes()
{
  cout << "SyncModel::_compile_changes()" << endl;
  QList<SyncData>::const_iterator sditer = sync_list.begin();
  while(sditer != sync_list.end()) {
    if( sditer->action == Action::SendToServer )
      _files_to_send.append( sditer->local.current_fd );

    else if( sditer->action == Action::GetFromServer )
      _files_to_get.append( sditer->remote.current_fd );

    else if( sditer->action == Action::DeleteFromServer )
      _remote_files_to_delete.append( sditer->remote.current_fd );

    else if( sditer->action == Action::DeleteFromClient )
      _local_files_to_delete.append( sditer->local.current_fd );
    ++sditer;
  }

  cout << _files_to_send.size() << " files to send" << endl;
  cout << _files_to_get.size() << " files to get" << endl;
  cout << _remote_files_to_delete.size() << " remote files to delete" << endl;
  cout << _local_files_to_delete.size() << " local files to delete" << endl;  
  changes_compiled = true;
  return;
}


void SyncModel::set_remote_filelist(QList<FileData> remote_list)
{
  _remote_dirdata.set_current_list(remote_list);
  _remote_dirdata.generate_file_agents();

  make_changes_list();
  return;
}

void SyncModel::reset()
{
  // Clear the sync list
  if(sync_list.size() > 0) {
    beginRemoveRows( QModelIndex(), 0, rowCount(QModelIndex())-1 );
    sync_list.clear();
    diff_list.clear();
    endRemoveRows();
  }

  // Clear everything else...
  _local_dir.clear();
  _local_dirdata.reset();
  _remote_dirdata.reset();

  changes_compiled = false;
  _files_to_send.clear();
  _files_to_get.clear();
  _remote_files_to_delete.clear();
  _local_files_to_delete.clear();
  return;
}

void SyncModel::selection_changed(const QModelIndex &current, const QModelIndex)
{
  if( current.row() >= 0 && 
      (_diff_only || current.row() < sync_list.size()) &&
      (!_diff_only || current.row() < diff_list.size()) ){

    SyncData sd( _diff_only ? *diff_list[current.row()] : sync_list[current.row()] );
    std::pair<QString,QString> info = sd.get_info();
    emit set_info(info.first, info.second);

  } else {
    emit set_info(QString(), QString());
  }   
  return;
}

void SyncModel::_generate_diff_list()
{
  diff_list.clear();
  for(int r=0; r<sync_list.size(); r++) {
    SyncData sd( sync_list[r] );
    if( sd.remote.current_fd != sd.local.current_fd )
      diff_list.append( &sync_list[r] );
  }
  return;
}

void SyncModel::set_diff_only(const bool b)
{
  cout << "SyncModel::set_diff_only(" << b << ")" << endl;
  if(_diff_only == b) return;
  _diff_only = b;
  //this->reset();

  if(_diff_only) {
    if(diff_list.size()>0) emit dataChanged(index(0, 0), index(diff_list.size()-1, columnCount(QModelIndex())));
    if(sync_list.size()>0 && diff_list.size()<sync_list.size()) {
      beginRemoveRows( QModelIndex(), diff_list.size(), sync_list.size()-1 );
      endRemoveRows();
    }
  } else {
    if(diff_list.size()>0) emit dataChanged(index(0, 0), index(diff_list.size()-1, columnCount(QModelIndex())));
    if(sync_list.size()>0 && diff_list.size()<sync_list.size()) {
      beginInsertRows( QModelIndex(), diff_list.size(), sync_list.size()-1 );
      endInsertRows();
    }
  }
  return;
}
