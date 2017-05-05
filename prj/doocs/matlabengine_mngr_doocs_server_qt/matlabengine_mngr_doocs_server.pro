# File matlab_engine_server.pro
# File created : 24 Apr 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
include(../../common/common_qt/doocs_server_common.pri)
DEFINES += use_mat_matrix
TEMPLATE = app
INCLUDEPATH += /doocs/lib/matlab/R2010a/include
INCLUDEPATH += /doocs/lib/matlab/R2016a/include
INCLUDEPATH += ../../../include
INCLUDEPATH += ../../../src/client
SOURCES += ../../../src/common/common_socketbase.cpp \
    ../../../src/common/common_serializer.cpp \
    ../../../src/common/common_functionalities.cpp \
    ../../../src/common/asockettcp.cpp \
    ../../../src/common/asocketb.cpp \
    ../../../src/util/matlab_pipe_name.cpp \
    ../../../src/util/matlab_engine_serializer.cpp \
    ../../../src/util/matlab_engine_serializer_versioning.cpp \
    ../../../src/seri_versions/matlab_engine_serializer_version5_raw1.cpp \
    ../../../src/mat_matrix/mat_matrix.cpp \
    ../../../src/doocs/matlab_engine_mngr_eqfctmngr.cpp \
    ../../../src/doocs/matlab_engine_mngr_eqfctproxy.cpp \
    ../../../src/doocs/rpc_matlab_engine_mngr_doocs_server.cpp \
    ../../../src/client/matlab_engine_clienttcp.cpp \
    ../../../src/cpp11/thread_cpp11.cpp \
    ../../../src/cpp11/mutex_cpp11.cpp
HEADERS += ../../../src/doocs/matlab_engine_mngr_eqfctmngr.hpp \
    ../../../src/doocs/matlab_engine_mngr_eqfctproxy.hpp \
    ../../../src/doocs/matlab_engine_mngr_eqfctproxy.tos \
    ../../../src/client/matlab_engine_clienttcp.hpp
