#
# This work is licensed under the Creative Commons Attribution 3.0 Unported License.
# To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
# or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
#

# Qt Settings
QT       += core gui network webkitwidgets

# Compile Settings
TARGET = EtupromClient
TEMPLATE = app

# enable C++11
CONFIG += c++11

# Include Eleaph Vendor
include("vendor\eleaph-qt\eleaph-qt.pri")

# Sources (Client)
SOURCES +=  client/src/main.cpp\
            client/src/mainwindow.cpp \
            client/src/loginform.cpp \
            client/src/global.cpp \
            client/src/chatbox.cpp \
            client/src/designloader.cpp

# Headers (Client)
HEADERS  += client/src/mainwindow.h  \
            client/src/loginform.h \
            client/src/global.h \
            client/src/chatbox.h \
            client/src/designloader.h


# Sources (Protocol)
SOURCES +=  collective/proto/src/protocol.pb.cc

# Headers (Protocol)
HEADERS +=  collective/proto/src/protocol.pb.h \
            collective/proto/packettypes.h

# Forms
FORMS    += client/ui/mainwindow.ui \
            client/ui/loginform.ui \
            client/ui/chatTab.ui \
            client/ui/chatbox.ui

# Ressource files
RESOURCES += client/res/global.qrc

# Win32: Set Application/Windows Icon
win32:RC_ICONS = "client/res/client.ico"

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

