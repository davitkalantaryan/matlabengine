# File remote_call.pro
# File created : 19 Apr 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

include(../../common/common_qt/mex_common.pri)
#TARGET = multiengine

# CONFIG += c++11
INCLUDEPATH += ../../../include
INCLUDEPATH += ../../../contrib/cpp-raft/include
LIBS += -L../../../contrib/matlab/sys/$$CODENAME/lib
LIBS += -leng
SOURCES += \
    ../../../src/multi/multi_engine.cpp \
    ../../../src/multi/mex_multiengine.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/thread_cpp11.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/shared_mutex_cpp14.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/mutex_cpp11.cpp
HEADERS += \
    ../../../src/multi/multi_engine.hpp \
    ../../../src/multi/impl.multi_engine.hpp
