#
# File matlab_matrix_common.pro
# File created : 19 Apr 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
#

MATLAB_VERSION = R2010a

include(../../common/common_qt/sys_common.pri)
message("!!! matlab_matrix_common.pri CODENAME=$$CODENAME")

#TEMPLATE = app
#TARGET = .

equals(CODENAME,"Santiago"){
    #LIBS += -L/products/matlab/R2010a/extern/lib
    MATLAB_VERSION = R2010a
    #MATLAB_VERSION = R2016b
}else{
    equals(CODENAME,"trusty") {
        #LIBS += -L/usr/local/MATLAB/R2016a/extern/lib/trusty
        MATLAB_VERSION = R2016a
    }
}

LIBS += -L/doocs/lib/matlab/$$MATLAB_VERSION
INCLUDEPATH += /doocs/lib/matlab/$$MATLAB_VERSION/include

equals(MATLAB_VERSION,"R2016a"){
    #CONFIG += c++11
    message("R2016a is used")
    LIBS += -lmx
    LIBS += -lut
    LIBS += -lmwresource_core
    LIBS += -lmwi18n
    LIBS += -lmwfl
    LIBS += -lboost_chrono
    LIBS += -lboost_date_time
    LIBS += -lboost_filesystem
    LIBS += -lboost_log
    LIBS += -lboost_regex
    LIBS += -lboost_signals
    LIBS += -lboost_system
    LIBS += -lboost_thread
    LIBS += -lmwcpp11compat
    LIBS += -licudata
    LIBS += -licuuc
    LIBS += -licui18n
    LIBS += -licuio
    LIBS += -ltbb
    LIBS += -ltbbmalloc
    LIBS += -lboost_serialization
    LIBS += -lunwind
}else{
    equals(MATLAB_VERSION,"R2010a"){
        message("R2010a is used")
        LIBS += -lmx
        LIBS += -lut
        LIBS += -lmwfl
        LIBS += -licudata
        LIBS += -licuuc
        LIBS += -licui18n
        LIBS += -licuio
        LIBS += -lboost_thread-gcc42-mt-1_36
        LIBS += -lboost_signals-gcc42-mt-1_36
        LIBS += -lboost_system-gcc42-mt-1_36
        LIBS += -lboost_filesystem-gcc42-mt-1_36
    }else{
        equals(MATLAB_VERSION,"R2016b"){
            #CONFIG += c++11
            message("R2016b is used")
            LIBS += -lmx
            LIBS += -lut
            LIBS += -lmwresource_core
            LIBS += -lmwi18n
            LIBS += -lmwfl
            LIBS += -lboost_chrono
            LIBS += -lboost_date_time
            LIBS += -lboost_filesystem
            LIBS += -lboost_log
            LIBS += -lboost_regex
            LIBS += -lboost_signals
            LIBS += -lboost_system
            LIBS += -lboost_thread
            LIBS += -lmwcpp11compat
            LIBS += -licudata
            LIBS += -licuuc
            LIBS += -licui18n
            LIBS += -licuio
            LIBS += -ltbb
            LIBS += -ltbbmalloc
            LIBS += -lboost_serialization
            LIBS += -lunwind
            LIBS += -lssl
            LIBS += -lcrypto
        }
    }
}

QT -= core
QT -= gui

message("!!! matlab_matrix_common.pri including libs")
