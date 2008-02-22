TEMPLATE = app
TARGET = client

OBJECTS_DIR = build
MOC_DIR = $$OBJECTS_DIR


HEADERS       = ../FileData.h \
                ../FileHandler.h \
                NetworkClientThread.h \
                ClientDialog.h \
                MainWindow.h \
                SyncModel.h

SOURCES       = ../FileData.cxx \
                ../FileHandler.cxx \
                NetworkClientThread.cxx \
                ClientDialog.cxx \
                MainWindow.cxx \
                SyncModel.cxx \
                main.cxx

QT           += network
