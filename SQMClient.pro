#
# This work is licensed under the Creative Commons Attribution 3.0 Unported License.
# To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
# or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
#

# Qt Settings
QT       += core gui network

# Compile Settings
TARGET = SQMClient
TEMPLATE = app

# Sources
SOURCES +=  client/src/main.cpp\
            client/src/mainwindow.cpp \
            collective/src/sqmpackethandler.cpp \
            client/src/sqmpacketprocessor.cpp \
            client/src/loginform.cpp \
            client/src/global.cpp \
            collective/proto/src/protocol.pb.cc

# Headers
HEADERS  += client/src/mainwindow.h  \
            collective/src/sqmpackethandler.h \
            client/src/sqmpacketprocessor.h \
            client/src/loginform.h \
            client/src/global.h \
            collective/proto/src/protocol.pb.h

# Forms
FORMS    += client/ui/mainwindow.ui \
            client/ui/loginform.ui

# Ressource files
RESOURCES += client/res/global.qrc

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
system(protoc -I=$$Protosrcdir --cpp_out=$$Prototargetdir $$Protosrcdir/*.proto)

# link against protobuf lib (we can't do that in mkspecs because the protobuf lib has to be set as last lib)
LIBS += -lprotobuf

