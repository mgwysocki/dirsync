
#include "SyncData.h"

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

QDataStream & operator<<( QDataStream &dout, const SyncData &sd )
{
  return dout << sd.relative_filename
              << sd.local << sd.remote
              << sd.situ << sd.action;
};

QDataStream & operator>>( QDataStream &din, SyncData &sd )
{
  din >> sd.relative_filename;
  din >> sd.local;
  din >> sd.remote;
  din >> sd.situ;
  din >> sd.action;
  return din;
};
