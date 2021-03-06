TARGET  	 = origin2
TEMPLATE     = lib
CONFIG      += warn_on release thread
#CONFIG      += staticlib 
MOC_DIR      = ./tmp
OBJECTS_DIR  = ./tmp

DESTDIR      = ./

#INCLUDEPATH += boost_1_33_0
LIBS        += -lboost_date_time
LIBS        += -lboost_thread

HEADERS += endianfstream.hh
HEADERS += logging.hpp
HEADERS += OriginObj.h
HEADERS += OriginFile.h
HEADERS += OriginParser.h
HEADERS += OriginDefaultParser.h
HEADERS += Origin750Parser.h

SOURCES += OriginFile.cpp
SOURCES += OriginParser.cpp
SOURCES += OriginDefaultParser.cpp
SOURCES += Origin750Parser.cpp

