QMAKE_PROJECT_DEPTH = 0
linux-g++-64: libsuff=64

TARGET            = fitRational0
TEMPLATE          = lib

CONFIG           += thread
CONFIG           += warn_on 
CONFIG           += release
CONFIG           += dll 

DESTDIR           = ../
 
INCLUDEPATH += ../../3rdparty/gsl

# statically link against GSL in 3rdparty
LIBS         += ../3rdparty/gsl/lib/libgsl.a
LIBS         += ../3rdparty/gsl/lib/libgslcblas.a
#dynamically link against GSL installed system-wide
#unix:LIBS         += -lgsl -lgslcblas

target.path=/usr/lib$${libsuff}/qtiplot/plugins
INSTALLS += target

SOURCES += fitRational0.cpp
