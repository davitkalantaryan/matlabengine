# File remote_call.pro
# File created : 19 Apr 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
include(../../common/common_qt/mex_common.pri)
TARGET = remote_call

# CONFIG += c++11
INCLUDEPATH += ../../../include
SOURCES += ../../../src/common/asockettcp.cpp \
    ../../../src/common/asocketb.cpp \
    ../../../src/util/matlab_bytestream_routines.cpp \
    ../../../src/util/matlab_engine_mathandlebase.cpp \
    ../../../src/util/matlab_engine_mathandlemexbase.cpp \
    ../../../src/util/matlab_engine_serializer.cpp \
    ../../../src/util/matlab_pipe_name.cpp \
    ../../../src/client/mex_remote_call.cpp \
    ../../../src/client/matlab_engine_clienttcp.cpp \
    ../../../src/common/common_serializer.cpp \
    ../../../src/common/common_socketbase.cpp \
    ../../../src/common/common_functionalities.cpp \
    ../../../src/seri_versions/matlab_engine_serializer_version5_matundoc.cpp \
    ../../../src/util/matlab_engine_serializer_versioning.cpp
HEADERS += ../../../src/client/matlab_engine_clienttcp.hpp \
    ../../../include/matlab_pipe_name.h \
    ../../../include/matlab_engine_serializer.hpp \
    ../../../include/matlab_engine_mathandlemexbase.hpp \
    ../../../include/matlab_engine_mathandlebase.hpp \
    ../../../include/matlab_bytestream_routines.h
