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
SOURCES +=	server/src/main.cpp \
			collective/src/sqmpackethandler.cpp \
			server/src/sqmpacketprocessor.cpp \
			collective/proto/src/protocol.pb.cc \
			server/src/global.cpp

# Headers
HEADERS +=	\
			collective/src/sqmpackethandler.h \
			server/src/sqmpacketprocessor.h \
			collective/proto/src/login.pb.h \
			collective/proto/src/protocol.pb.h \
			server/src/global.h

# include pathes
INCLUDEPATH += "include/"
INCLUDEPATH += "collective/proto/src/"

# remove all existing compiled .proto files and recompile all .proto files
Protosrcdir = $$_PRO_FILE_PWD_/collective/proto
Prototargetdir = $$Protosrcdir/src
win32:system("del /q \"$$Prototargetdir/\"")
linux:system("rm -f $$Prototargetdir/*")
system(protoc -I=$$Protosrcdir --cpp_out=$$Prototargetdir $$Protosrcdir/*.proto)

# add protobuf lib (we can't do that in mkspecs because the protobuf lib has to be set as last lib)
LIBS += -lprotobuf
