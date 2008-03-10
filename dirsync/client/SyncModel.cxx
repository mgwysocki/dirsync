#include "SyncModel.h"

#include <QDateTime>

#include <iostream>
using namespace std;

QString format_size(const quint64 size)
{
  QString sizestr = QString("%1").arg(size);
  if(sizestr.length()>9) sizestr.insert(sizestr.length()-9, ",");
  if(sizestr.length()>6) sizestr.insert(sizestr.length()-6, ",");
  if(sizestr.length()>3) sizestr.insert(sizestr.length()-3, ",");
  return QString("%1  bytes").arg(sizestr);
}

QString format_time(const quint32 time)
{
  QDateTime dt = QDateTime::fromTime_t(time);
  return dt.toString("h:mm:ss  MMM d, yyyy");
}


//----------------------------------------------------------------------------------
SyncData::SyncData() :
  action(0)
{}

SyncData::SyncData(const SyncData &sd) :
  relative_filename(sd.relative_filename),
  local(sd.local),
  remote(sd.remote),
  situ(sd.situ),
  action(sd.action)
{}

SyncData::SyncData(const FileAgent &localfa, const FileAgent &remotefa, bool no_previous_info=false) :
  local(localfa),
  remote(remotefa),
  action(0)
{ 
  relative_filename = localfa.relative_filename.isEmpty() ? remotefa.relative_filename : localfa.relative_filename;
  if(no_previous_info) determine_situation_first_time();
  else                 determine_situation(); 
}

void SyncData::determine_situation_first_time()
{
  if( remote.current_fd == local.current_fd ){
    // No difference
    action = Action::None;
    situ = QObject::tr("Synced");

  } else if( !local.current_fd.initialized ){
    // Local doesn't exist
    action = Action::GetFromServer;
    situ = QObject::tr("L doesn't exist");

  } else if( !remote.current_fd.initialized ){
    // Remote doesn't exist
    action = Action::SendToServer;
    situ = QObject::tr("R doesn't exist");

  } else if( local.current_fd.modtime > remote.current_fd.modtime ){
    // Local is newer
    action = Action::SendToServer;
    situ = QObject::tr("L is newer");

  } else if( local.current_fd.modtime < remote.current_fd.modtime ){
    // Remote is newer
    action = Action::GetFromServer;
    situ = QObject::tr("R is newer");

  } else {
    // Differ in another way (size, checksum, etc.)
    action = Action::Unclear;
    situ = QObject::tr("L and R differ");

    cout << "Local file:\n" << local.current_fd << endl
	 << "Remote file:\n" << remote.current_fd << endl << endl;
  }

  return;
}

void SyncData::determine_situation()
{
  if( !remote.added && !remote.deleted && !remote.modified && !local.added && !local.deleted && !local.modified ){
    // No difference in either case
    action = Action::None;
    situ = QObject::tr("No Changes");

  } else if( !remote.added && !remote.deleted && !remote.modified ){
    if( local.added ){
      // added to local dir
      action = Action::SendToServer;
      situ = QObject::tr("Added to A");

    } else if( local.deleted ){
      // deleted from local dir
      action = Action::DeleteFromServer;
      situ = QObject::tr("Deleted from A");

    } else if( local.modified ){
      // modified in local dir only
      action = Action::SendToServer;
      situ = QObject::tr("A modified");
    }

  } else if( !local.added && !local.deleted && !local.modified ) {
    if( remote.added ){
      // added to remote dir
      action = Action::GetFromServer;
      situ = QObject::tr("Added to B");

    } else if( remote.deleted ){
      // deleted from remote dir
      action = Action::DeleteFromClient;
      situ = QObject::tr("Deleted from B");

    } else if( remote.modified ){
      // modified in remote dir only
      action = Action::GetFromServer;
      situ = QObject::tr("B modified");
    }

  } else if( local.added ){
    if( remote.added ){
      // added to both dirs
      action = Action::Unclear;
      situ = QObject::tr("Added to A and B");

    } else if( remote.deleted ){
      // added to local and deleted from remote
      action = Action::Unclear;
      situ = QObject::tr("Added to A and deleted from B");

    } else if( remote.modified ){
      // added to local and modified in remote
      action = Action::Unclear;
      situ = QObject::tr("Added to A and modified in B");
    }

  } else if( local.deleted ){
    if( remote.added ){
      // Deleted locally and added to remote
      action = Action::Unclear;
      situ = QObject::tr("Deleted from A and added to B");

    } else if( remote.deleted ){
      // deleted from both
      action = Action::Unclear;
      situ = QObject::tr("Deleted from A and B");

    } else if( remote.modified ){
      // Deleted from local and modified in remote
      action = Action::Unclear;
      situ = QObject::tr("Deleted from A and modified in B");
    }

  } else if( local.modified ){
    if( remote.added ){
      // Modified locally and added to remote
      action = Action::Unclear;
      situ = QObject::tr("Modified in A and added to B");

    } else if( remote.deleted ){
      // Modified locally and deleted from remote
      action = Action::Unclear;
      situ = QObject::tr("Modified in A and deleted from B");

    } else if( remote.modified ){
      // Modified in both
      action = Action::Unclear;
      situ = QObject::tr("Modified in A and B");
    }

  } else {
    // No difference in either case
    action = Action::Unclear;
    situ = QObject::tr("Should never see this!");
  }

  return;
}

std::pair<QString,QString> SyncData::get_info() const
{
  FileData lfd(local.current_fd);
  FileData rfd(remote.current_fd);
  QString local_text, remote_text;

  if(lfd.initialized && rfd.initialized) {
    local_text += QString("%1<br/>").arg(lfd.relative_filename);
    remote_text += QString("%1<br/>").arg(rfd.relative_filename);

    if(lfd.size == rfd.size) {
      local_text += QString("Size:  %1<br/>").arg( format_size(lfd.size) );
      remote_text += QString("Size:  %1<br/>").arg( format_size(rfd.size) );
    } else {
      local_text += QString("<font color=crimson>Size:  %1</font><br/>").arg( format_size(lfd.size) );
      remote_text += QString("<font color=crimson>Size:  %1</font><br/>").arg( format_size(rfd.size) );
    }

    if(lfd.modtime == rfd.modtime) {
      local_text += QString("%1").arg( format_time(lfd.modtime) );
      remote_text += QString("%1").arg( format_time(rfd.modtime) );
    } else {
      local_text += QString("<font color=crimson>%1</font>").arg( format_time(lfd.modtime) );
      remote_text += QString("<font color=crimson>%1</font>").arg( format_time(rfd.modtime) );
    }

  } else if(lfd.initialized) {
    local_text += QString("%1<br/>").arg(lfd.relative_filename);
    local_text += QString("Size:  %1<br/>").arg( format_size(lfd.size) );
    local_text += QString("%1").arg( format_time(lfd.modtime) );
    remote_text += "<font color=red>Does not exist!</font>";

  } else if(rfd.initialized) {
    local_text += "<font color=red>Does not exist!</font>";
    remote_text += QString("%1<br/>").arg(rfd.relative_filename);
    remote_text += QString("Size:  %1<br/>").arg( format_size(rfd.size) );
    remote_text += QString("%1").arg( format_time(rfd.modtime) );
    
  } else {
    local_text += "<font color=red>Does not exist!</font>";
    remote_text += "<font color=red>Does not exist!</font>";
  }

  return std::pair<QString,QString>(local_text, remote_text);
}


//=================================================================================
//
SyncModel::SyncModel() :
  _loaded_previous_state(false),
  changes_compiled(false)
{}

QVariant SyncModel::data(const QModelIndex &index, int role = Qt::DisplayRole) const
{
  if(role != Qt::DisplayRole)
    return QVariant();

  int c = index.column();
  int r = index.row();
  if(c==0) {
    QString var(tr(" ") + sync_list[r].local.current_fd.relative_filename);
    if(sync_list[r].local.current_fd.isdir)
      var += tr("/");
    return  QVariant(var);
  } else if(c==1) {
    return QVariant(sync_list[r].situ);
  } else if(c==2) {
    return QVariant(Action::strings[ sync_list[r].action ]);
  } else if(c==3) {
    QString var(tr(" ") + sync_list[r].remote.current_fd.relative_filename);
    if(sync_list[r].remote.current_fd.isdir)
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
    sync_list[row].action = action;
    emit dataChanged(index(row, 0), index(row, 2));
  }
  return;
}

void SyncModel::set_sync_to_client(const QModelIndexList &ilist)
{
  for(int i=0; i<ilist.size(); i++) {
    int row = ilist[i].row();
    SyncData sd = sync_list[row];
    if(sd.local.current_fd != sd.remote.current_fd) {
      if(sd.local.deleted || !sd.local.current_fd.initialized)
	sd.action = Action::DeleteFromServer;
      else
	sd.action = Action::SendToServer;
    } else {
      sd.action = Action::None;
    }
    sync_list[row] = sd;
    emit dataChanged(index(row, 0), index(row, 2));
  }
  return;
}

void SyncModel::set_sync_to_server(const QModelIndexList &ilist)
{
  for(int i=0; i<ilist.size(); i++) {
    int row = ilist[i].row();
    SyncData sd = sync_list[row];
    if(sd.local.current_fd != sd.remote.current_fd) {
      if(sd.remote.deleted || !sd.remote.current_fd.initialized)
	sd.action = Action::DeleteFromClient;
      else
	sd.action = Action::GetFromServer;
    } else {
      sd.action = Action::None;
    }
    sync_list[row] = sd;
    emit dataChanged(index(row, 0), index(row, 2));
  }
  return;
}


void SyncModel::make_local_list()
{
  _local_dirdata.generate_current_list();
  _local_dirdata.generate_file_agents();
  return;
}


bool SyncModel::_read_last_sync_file()
{
//   QFile syncfile("lastsync.dat");
//   if( !syncfile.open(QIODevice::ReadOnly) ){
//     cout << "SyncModel::_read_last_sync_file() - Could not open sync file!!!" << endl;
//     return false;
//   }
//   QDataStream in(&syncfile);
//   in.setVersion(QDataStream::Qt_4_0);
//   in >> _sync_files;
//   syncfile.close();
  return true;
}

void SyncModel::save_sync_file()
{
//   if(!_sync_files.size()) _make_fresh_sync_list();
//   QFile syncfile("lastsync.dat");
//   if( !syncfile.open(QIODevice::WriteOnly) ){
//     cout << "SyncModel::_save_sync_file() - Could not open sync file!!!" << endl;
//     return;
//   }
//   QDataStream out(&syncfile);
//   out.setVersion(QDataStream::Qt_4_0);
//   out << _sync_files;
//   syncfile.close();
  return;
}

void SyncModel::_make_fresh_sync_list()
{
//   QList<SyncData>::const_iterator siter = sync_list.begin();
//   while(siter != sync_list.end()) {
//     if(siter->action == Action::SendToServer)
//       _sync_files.append(siter->local);
//     else if(siter->action == Action::GetFromServer)
//       _sync_files.append(siter->remote);
//     siter++;
//   }
  return;
}


void SyncModel::make_changes_list()
{
  cout << "SyncModel::make_changes_list()" << endl;;

  // Clear the sync list
  if(sync_list.size() > 0) {
    beginRemoveRows( QModelIndex(), 0, sync_list.size()-1 );
    sync_list.clear();
    endRemoveRows();
  }

  //   _read_last_sync_file();
  //   if( ! _read_last_sync_file() ) {
  //     modified_files += _current_files_list;
  //     return;
  //   }

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

  cout << temp_sync_list.size() << " files added to the sync list." << endl;
  beginInsertRows( QModelIndex(), 0, temp_sync_list.size()-1);
  sync_list = temp_sync_list;
  endInsertRows();
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
    beginRemoveRows( QModelIndex(), 0, sync_list.size()-1 );
    sync_list.clear();
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

void SyncModel::selection_changed(QItemSelection selected, QItemSelection deselected)
{
  QModelIndex index;
  QModelIndexList items = selected.indexes();

  index = items[0];
  SyncData sd( sync_list[index.row()] );
  std::pair<QString,QString> info = sd.get_info();
  emit set_info(info.first, info.second);
  return;
}
