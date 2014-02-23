#
# This work is licensed under the Creative Commons Attribution 3.0 Unported License.
# To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
# or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
#

# Qt Settings
QT       += core network sql
QT       -= gui

# enable C++11
CONFIG += c++11

# Compile Settings
TARGET	= EtupromServer
CONFIG	+= console
CONFIG	-= app_bundle
TEMPLATE = app

# Sources (Server)
SOURCES +=  server/src/main.cpp \
            server/src/packetprocessor.cpp \
            server/src/global.cpp \
            server/src/databasehelper.cpp \
            server/src/usermanager.cpp \
            server/src/querybuilder.cpp \
            server/src/chatmanager.cpp \
            server/src/contactmanager.cpp

# Sources (Protocol)
SOURCES +=  collective/proto/src/protocol.pb.cc

# Sources (Eleaph-qt)
SOURCES +=  vendor/eleaph-qt/src/ieleaph.cpp \
            vendor/eleaph-qt/src/eleaphprotorpc.cpp


# Headers (Server)
HEADERS +=  \
            server/src/packetprocessor.h \
            server/src/global.h \
            server/src/databasehelper.h \
            server/src/usermanager.h \
            server/src/querybuilder.h \
            server/src/chatmanager.h \
            server/src/contactmanager.h

# Headers (Protocol)
HEADERS +=  collective/proto/src/protocol.pb.h \
            collective/proto/packettypes.h

# Headers (Eleaph-qt)
HEADERS +=  vendor/eleaph-qt/src/ieleaph.h \
            vendor/eleaph-qt/src/eleaphprotorpc.h

# include path
INCLUDEPATH += "include/"


#
# Protobuf config section
#

# set some protobuf settings
INCLUDEPATH += "collective/proto/src/"
Protosrcdir = $$_PRO_FILE_PWD_/collective/proto
Prototargetdir = $$Protosrcdir/src

# remove all existing compiled .proto files
win32 {
        system("del \"$$Prototargetdir\"\\*.h")
        system("del \"$$Prototargetdir\"\\*.cc")
}
linux {
	system("rm -f $$Prototargetdir/*.h")
	system("rm -f $$Prototargetdir/*.cc")
}

# ...and recompile them
system(protoc -I=\"$$Protosrcdir\" --cpp_out=\"$$Prototargetdir\" \"$$Protosrcdir\"/*.proto)

# link against protobuf lib (we can't do that in mkspecs because the protobuf lib has to be set as last lib)
LIBS += -lprotobuf
