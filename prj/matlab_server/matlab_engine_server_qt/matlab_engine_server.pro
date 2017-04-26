# File matlab_engine_server.pro
# File created : 24 Apr 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

include(../../common/common_qt/matlab_matrix_common.pri)
TEMPLATE = app
INCLUDEPATH += ../../../include

# message("includepath=$$INCLUDEPATH")
LIBS += -leng
LIBS += -lmat
LIBS += -lhdf5_hl
LIBS += -lhdf5
#CONFIG += c++11
SOURCES += ../../../src/common/asockettcp.cpp \
    ../../../src/common/asocketb.cpp \
    ../../../src/util/matlab_bytestream_routines.cpp \
    ../../../src/util/matlab_engine_mathandlebase.cpp \
    ../../../src/util/matlab_engine_serializer.cpp \
    ../../../src/util/matlab_pipe_name.cpp \
    ../../../src/server/matlab_engine_serverbase.cpp \
    ../../../src/server/matlab_engine_servertcpbase.cpp \
    ../../../src/cpp11/thread_cpp11.cpp \
    ../../../src/cpp11/mutex_cpp11.cpp \
    ../../../src/common/common_functionalities.cpp \
    ../../../src/server/main_matlab_engine_server.cpp \
    ../../../src/common/aservertcp.cpp \
    ../../../src/util/matlab_engine_mathandleeng.cpp \
    ../../../src/server/matlab_engine_servertcpeng.cpp \
    ../../../src/common/common_serializer.cpp \
    ../../../src/common/common_socketbase.cpp
HEADERS += ../../../src/client/matlab_engine_clienttcp.hpp \
    ../../../include/matlab_pipe_name.h \
    ../../../include/matlab_engine_serializer.hpp \
    ../../../include/matlab_engine_mathandlebase.hpp \
    ../../../include/matlab_bytestream_routines.h \
    ../../../src/server/matlab_engine_serverbase.hpp \
    ../../../src/server/matlab_engine_servertcpbase.hpp \
    ../../../include/common_defination.h \
    ../../../include/thread_cpp11.hpp \
    ../../../include/mutex_cpp11.hpp \
    ../../../include/thread_cpp11.tos \
    ../../../include/aservertcp.h \
    ../../../include/asocketb.h \
    ../../../include/asockettcp.h \
    ../../../include/matlab_engine_mathandleeng.hpp \
    ../../../src/server/matlab_engine_servertcpeng.hpp \
    ../../../include/common_unnamedsemaphorelite.hpp \
    ../../../include/common_serializer.hpp \
    ../../../include/common_socketbase.hpp
