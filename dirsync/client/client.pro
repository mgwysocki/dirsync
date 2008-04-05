TEMPLATE = app
TARGET = client

OBJECTS_DIR = build
MOC_DIR = $$OBJECTS_DIR

CONFIG += qt thread
RESOURCES += icons.qrc

HEADERS       = ../FileData.h \
                ../FileHandler.h \
                ../FileAgent.h \
                ../Preferences.h \
                PreferencesDialog.h \
                ProgressDialog.h \
                SaveFile.h \
                DirData.h \
                SyncModel.h \
                NetworkClientThread.h \
                NewSyncDialog.h \
                InfoDockWidget.h \
                MyTableView.h \
                MainWindow.h

SOURCES       = ../FileData.cxx \
                ../FileHandler.cxx \
                ../FileAgent.cxx \
                ../Preferences.cxx \
                PreferencesDialog.cxx \
                ProgressDialog.cxx \
                SaveFile.cxx \
                DirData.cxx \
                SyncModel.cxx \
                NetworkClientThread.cxx \
                NewSyncDialog.cxx \
                InfoDockWidget.cxx \
                MainWindow.cxx \
                main.cxx

QT           += network
