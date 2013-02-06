#
# This work is licensed under the Creative Commons Attribution 3.0 Unported License.
# To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
# or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
#

# Qt Settings
QT       += core network sql
QT       -= gui

# Compile Settings
TARGET	= SQMServer
CONFIG	+= console
CONFIG	-= app_bundle
TEMPLATE = app

# Sources
SOURCES +=  server/src/main.cpp \
            server/src/packetprocessor.cpp \
            server/src/global.cpp \
            server/src/databasehelper.cpp \
            server/src/usermanager.cpp \
            collective/src/eleaphprotorpc.cpp \
            collective/proto/src/protocol.pb.cc \
            collective/src/ieleaph.cpp \
            server/src/querybuilder.cpp \
            server/src/chatmanager.cpp \
            server/src/contactmanager.cpp

# Headers
HEADERS +=  \
            server/src/packetprocessor.h \
            server/src/global.h \
            server/src/databasehelper.h \
            server/src/usermanager.h \
            collective/src/eleaphprotorpc.h \
            collective/proto/src/protocol.pb.cc \
            collective/src/ieleaph.h \
            server/src/querybuilder.h \
            server/src/chatmanager.h \
            server/src/contactmanager.h

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
