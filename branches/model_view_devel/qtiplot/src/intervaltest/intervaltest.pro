######################################################################
# Automatically generated by qmake (2.01a) So Mai 13 22:04:04 2007
######################################################################

QMAKE_CXX = distcc
TEMPLATE = app
TARGET = 
DEPENDPATH += . ..
INCLUDEPATH += . ..
CONFIG += debug

# Input
HEADERS += AbstractColumnData.h \
           DateTimeColumnData.h \
           DoubleColumnData.h \
           StringColumnData.h \
		   AbstractDataSource.h \
		   AbstractDateTimeDataSource.h \
		   AbstractDoubleDataSource.h \
		   AbstractStringDataSource.h \
		   AbstractFilter.h \
		   IntervalAttribute.h \
		   Interval.h

SOURCES += main.cpp 


SOURCES += StringColumnData.cpp \
           DateTimeColumnData.cpp \
		   DoubleColumnData.cpp \
		   AbstractFilter.cpp 

