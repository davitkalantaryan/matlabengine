#
# File doocs_client_common.pri
# File created : 12 Feb 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
#

message("!!! doocs_client_common.pri:")

DEFINES += LINUX

LIBS += -L/doocs/lib
#LIBS += -L/doocs/develop/bagrat/doocs.git/amd64_rhel60/lib
LIBS += -lDOOCSapi
LIBS += -lldap
LIBS += -lrt

include(../../common/common_qt/sys_common.pri)

INCLUDEPATH += /doocs/lib/include
#INCLUDEPATH += /doocs/develop/bagrat/doocs.git/include
