# building without muParser doesn't work yet
SCRIPTING_LANGS += muParser
SCRIPTING_LANGS += Python

# a console displaying output of scripts; particularly useful on Windows
# where running QtiPlot from a terminal is inconvenient
#DEFINES         += SCRIPTING_CONSOLE

# a dialog for selecting the scripting language on a per-project basis
DEFINES         += SCRIPTING_DIALOG

#DEFINES         += QTIPLOT_DEMO

# Comment the following lines to disable donations start-up message.
#DEFINES         += QTIPLOT_SUPPORT

# Comment the following line if you haven't subscribed for a QtiPlot binaries maintenance contract.
#RESTRICTED_MODULES += EMF

# Comment the next line, if you don't have libpng on your windows system.
DEFINES += GL2PS_HAVE_LIBPNG
win32:INCLUDEPATH += ../3rdparty/libpng/
win32:LIBS 		  += ../3rdparty/libpng/libpng.a

# Uncomment the following line if you want to perform a custom installation using the *.path variables defined bellow.
#CONFIG          += CustomInstall

CONFIG          += release
#CONFIG          += debug
#win32: CONFIG   += console

##################### 3rd PARTY HEADER FILES SECTION ########################
#!!! Warning: You must modify these paths according to your computer settings
#############################################################################

INCLUDEPATH       += ../3rdparty/muparser/include
INCLUDEPATH       += ../3rdparty/qwtplot3d/include
INCLUDEPATH       += ../3rdparty/qwt/src
INCLUDEPATH       += ../3rdparty/liborigin
INCLUDEPATH       += ../3rdparty/gsl/include
INCLUDEPATH       += ../3rdparty/zlib123/include
INCLUDEPATH       += ../3rdparty/boost_1_36_0

##################### 3rd PARTY LIBRARIES SECTION ###########################
#!!! Warning: You must modify these paths according to your computer settings
#############################################################################

##################### Linux (Mac OS X) ######################################

# statically link against libraries in 3rdparty
unix:LIBS         += ../3rdparty/muparser/lib/libmuparser.a
unix:LIBS         += ../3rdparty/qwt/lib/libqwt.a
unix:LIBS         += ../3rdparty/gsl/lib/libgsl.a
unix:LIBS         += ../3rdparty/gsl/lib/libgslcblas.a
unix:LIBS         += /usr/local/lib/libboost_date_time-gcc41-mt.a
unix:LIBS         += /usr/local/lib/libboost_thread-gcc41-mt.a

# dynamically link against dependencies if they are installed system-wide
#unix:LIBS         += -lmuparser
#unix:LIBS         += -lqwt
#unix:LIBS         += -lgsl -lgslcblas

##################### Windows ###############################################

win32:LIBS        += ../3rdparty/muparser/lib/libmuparser.a
win32:LIBS        += ../3rdparty/qwt/lib/libqwt.a
win32:LIBS        += ../3rdparty/gsl/lib/libgsl.a
win32:LIBS        += ../3rdparty/gsl/lib/libgslcblas.a
win32:LIBS        += ../3rdparty/zlib123/lib/zdll.lib
win32:LIBS        += ../3rdparty/boost_1_36_0/lib/libboost_date_time-mgw34-mt.lib
win32:LIBS        += ../3rdparty/boost_1_36_0/lib/libboost_thread-mgw34-mt.lib

#############################################################################
###################### BASIC PROJECT PROPERTIES #############################
#############################################################################

QMAKE_PROJECT_DEPTH = 0

TARGET         = qtiplot
TEMPLATE       = app
CONFIG        += qt warn_on exceptions opengl thread
CONFIG        += assistant

DEFINES       += QT_PLUGIN
contains(CONFIG, CustomInstall){
	INSTALLS        += target
	INSTALLS        += translations
	INSTALLS        += manual
	INSTALLS        += documentation
	unix:INSTALLS        += man

	unix: INSTALLBASE = /usr
	win32: INSTALLBASE = C:/QtiPlot

	unix: target.path = $$INSTALLBASE/bin
	unix: translations.path = $$INSTALLBASE/share/qtiplot/translations
	unix: manual.path = $$INSTALLBASE/share/doc/qtiplot/manual
	unix: documentation.path = $$INSTALLBASE/share/doc/qtiplot
	unix: man.path = $$INSTALLBASE/share/man/man1/

	win32: target.path = $$INSTALLBASE
	win32: translations.path = $$INSTALLBASE/translations
	win32: manual.path = $$INSTALLBASE/manual
	win32: documentation.path = $$INSTALLBASE/doc

	DEFINES       += TRANSLATIONS_PATH="\\\"$$replace(translations.path," ","\ ")\\\"
	DEFINES       += MANUAL_PATH="\\\"$$replace(manual.path," ","\ ")\\\"
	}

win32:DEFINES += QT_DLL QT_THREAD_SUPPORT
QT            += opengl qt3support network svg xml

MOC_DIR        = ../tmp/qtiplot
OBJECTS_DIR    = ../tmp/qtiplot
SIP_DIR        = ../tmp/qtiplot
DESTDIR        = ./

#############################################################################
###################### PROJECT FILES SECTION ################################
#############################################################################

###################### ICONS ################################################
INCLUDEPATH  += icons/
HEADERS      += icons/pixmaps.h
win32:RC_FILE = icons/qtiplot.rc
mac:RC_FILE   = icons/qtiplot.icns

###################### TRANSLATIONS #########################################

TRANSLATIONS    = translations/qtiplot_de.ts \
                  translations/qtiplot_es.ts \
                  translations/qtiplot_fr.ts \
                  #translations/qtiplot_pt.ts \
                  translations/qtiplot_ru.ts \
                  translations/qtiplot_ja.ts \
                  translations/qtiplot_sv.ts

#system(lupdate -verbose qtiplot.pro)
#system(lrelease -verbose qtiplot.pro)

translations.files += translations/qtiplot_de.qm \
                  translations/qtiplot_es.qm \
                  translations/qtiplot_fr.qm \
                  #translations/qtiplot_pt.qm \
                  translations/qtiplot_ru.qm \
                  translations/qtiplot_ja.qm \
                  translations/qtiplot_sv.qm

###################### DOCUMENTATION ########################################

manual.files += ../manual/html \
				../manual/qtiplot-manual-en.pdf

documentation.files += ../README.html \
                       ../gpl_licence.txt

unix: man.files += ../qtiplot.1

###############################################################
##################### Compression (zlib123) ###################
###############################################################

SOURCES += ../3rdparty/zlib123/minigzip.c

###############################################################
################# Origin Import (liborigin) ###################
###############################################################
INCLUDEPATH += src/

HEADERS += src/importOPJ.h
HEADERS += ../3rdparty/liborigin/OriginObj.h
HEADERS += ../3rdparty/liborigin/OriginFile.h
HEADERS += ../3rdparty/liborigin/OriginParser.h
HEADERS += ../3rdparty/liborigin/OriginDefaultParser.h
HEADERS += ../3rdparty/liborigin/Origin750Parser.h

SOURCES += src/importOPJ.cpp
SOURCES += ../3rdparty/liborigin/OriginFile.cpp
SOURCES += ../3rdparty/liborigin/OriginParser.cpp
SOURCES += ../3rdparty/liborigin/OriginDefaultParser.cpp
SOURCES += ../3rdparty/liborigin/Origin750Parser.cpp

###############################################################
################# Default Modules #############################
###############################################################

include(../3rdparty/qwtplot3d/qwtplot3d.pri)
include(src/analysis/analysis.pri)
include(src/core/core.pri)
include(src/lib/libqti.pri)
include(src/plot2D/plot2D.pri)
include(src/plot3D/plot3D.pri)
include(src/matrix/matrix.pri)
include(src/table/table.pri)
include(src/scripting/scripting.pri)

###############################################################
################# Restricted Module: EmfEngine ################
###############################################################

contains(RESTRICTED_MODULES, EMF) {
	DEFINES += EMF_OUTPUT

    INCLUDEPATH += ../3rdparty/libEMF/include
	unix:LIBS += -L../3rdparty/libEMF/lib
	win32:LIBS += -lgdi32

	INCLUDEPATH += ../3rdparty/EmfEngine
    HEADERS += ../3rdparty/EmfEngine/EmfEngine.h
    SOURCES += ../3rdparty/EmfEngine/EmfEngine.cpp
}

###############################################################
##################### Scripting: PYTHON + SIP + PyQT ##########
###############################################################

contains(SCRIPTING_LANGS, Python) {

  contains(CONFIG, CustomInstall){
  	INSTALLS += pythonconfig
  	pythonconfig.files += qtiplotrc.py \
  						qtiUtil.py \
  						qti_wordlist.txt \

  	unix: pythonconfig.path = /usr/local/qtiplot
  	win32: pythonconfig.path = $$INSTALLBASE
  	DEFINES += PYTHON_CONFIG_PATH="\\\"$$replace(pythonconfig.path," ","\ ")\\\"
  }

  unix {
    INCLUDEPATH += $$system(python python-includepath.py)
    LIBS        += $$system(python -c "\"from distutils import sysconfig; print '-lpython'+sysconfig.get_config_var('VERSION')\"")
    LIBS        += -lm
    system(mkdir -p $${SIP_DIR})
    system($$system(python python-sipcmd.py) -c $${SIP_DIR} src/scripting/qti.sip)
  }

  win32 {
    INCLUDEPATH += $$system(call python-includepath.py)
    LIBS        += $$system(call python-libs-win.py)
    system($$system(call python-sipcmd.py) -c $${SIP_DIR} src/scripting/qti.sip)
  }
}

###############################################################
