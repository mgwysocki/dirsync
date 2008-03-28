TEMPLATE = app
TARGET = server

OBJECTS_DIR = build
MOC_DIR = $$OBJECTS_DIR

CONFIG += qt thread debug

HEADERS       = ../FileData.h \
                ../FileHandler.h \
                ../Preferences.h \
                NetworkServer.h \
                NetworkServerThread.h \
                ServerDialog.h

SOURCES       = ../FileData.cxx \
                ../FileHandler.cxx \
                ../Preferences.cxx \
                NetworkServer.cxx \
                NetworkServerThread.cxx \
                ServerDialog.cxx \
                main.cxx

QT           += network
