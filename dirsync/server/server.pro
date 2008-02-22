TEMPLATE = app
TARGET = server

OBJECTS_DIR = build
MOC_DIR = $$OBJECTS_DIR


HEADERS       = ../FileData.h \
                ../FileHandler.h \
                NetworkServer.h \
                NetworkServerThread.h \
                ServerDialog.h

SOURCES       = ../FileData.cxx \
                ../FileHandler.cxx \
                NetworkServer.cxx \
                NetworkServerThread.cxx \
                ServerDialog.cxx \
                main.cxx

QT           += network
