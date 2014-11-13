TEMPLATE = app
TARGET = client

OBJECTS_DIR = build
MOC_DIR = $$OBJECTS_DIR

CONFIG += qt thread
RESOURCES += icons.qrc


HEADERS       = ../md5/md5.h \
                ../FileData.h \
                ../FileHandler.h \
                ../FileAgent.h \
                ../Preferences.h \
                ../SocketTool.h \
                PreferencesDialog.h \
                ProgressDialog.h \
                ProfileData.h \
                DirData.h \
                SyncData.h \
                SyncModel.h \
                NetworkClientThread.h \
                NewSyncDialog.h \
                InfoDockWidget.h \
                MyTableView.h \
                MainWindow.h

SOURCES       = ../md5/md5.cxx \
                ../FileData.cxx \
                ../FileHandler.cxx \
                ../FileAgent.cxx \
                ../Preferences.cxx \
                ../SocketTool.cxx \
                PreferencesDialog.cxx \
                ProgressDialog.cxx \
                ProfileData.cxx \
                DirData.cxx \
                SyncData.cxx \
                SyncModel.cxx \
                NetworkClientThread.cxx \
                NewSyncDialog.cxx \
                InfoDockWidget.cxx \
                MainWindow.cxx \
                main.cxx

QT           += network
