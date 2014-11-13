TEMPLATE = app
TARGET = server

OBJECTS_DIR = build
MOC_DIR = $$OBJECTS_DIR

CONFIG += qt thread

HEADERS       = ../md5/md5.h \
                ../FileData.h \
                ../SocketTool.h \
                ../FileHandler.h \
                ../Preferences.h \
                NetworkServer.h \
                NetworkServerThread.h \
                ServerDialog.h

SOURCES       = ../md5/md5.cxx \
                ../FileData.cxx \
                ../SocketTool.cxx \
                ../FileHandler.cxx \
                ../Preferences.cxx \
                NetworkServer.cxx \
                NetworkServerThread.cxx \
                ServerDialog.cxx \
                main.cxx

QT           += network
