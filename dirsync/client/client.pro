TEMPLATE = app
TARGET = client

OBJECTS_DIR = build
MOC_DIR = $$OBJECTS_DIR


HEADERS       = ../FileData.h \
                ../FileHandler.h \
                ../FileAgent.h \
                DirData.h \
                SyncModel.h \
                NetworkClientThread.h \
                ClientDialog.h \
                MainWindow.h

SOURCES       = ../FileData.cxx \
                ../FileHandler.cxx \
                ../FileAgent.cxx \
                DirData.cxx \
                SyncModel.cxx \
                NetworkClientThread.cxx \
                ClientDialog.cxx \
                MainWindow.cxx \
                main.cxx

QT           += network
