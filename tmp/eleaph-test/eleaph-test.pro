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
CONFIG += qtestlib
TEMPLATE = app

# Sources
SOURCES +=	main.cpp \
                ../../collective/src/ieleaph.cpp \
                ../../collective/src/eleaph.cpp \
                ../../collective/src/eleaphprotorpc.cpp \
                ../../collective/proto/src/protocol.pb.cc \
              ../../server/src/databasehelper.cpp \
    ../../server/src/querypool.cpp \
    ../../server/src/querybuilder.cpp


# Headers
HEADERS +=	../../collective/src/ieleaph.h \
			../../collective/src/eleaph.h \
			../../collective/src/eleaphprotorpc.h \
                        ../../collective/proto/src/protocol.pb.h \
    ../../server/src/databasehelper.h \
    ../../server/src/querypool.h \
    ../../server/src/querybuilder.h

INCLUDEPATH += "../../collective/proto/src/"

# link against protobuf lib (we can't do that in mkspecs because the protobuf lib has to be set as last lib)
LIBS += -lprotobuf
