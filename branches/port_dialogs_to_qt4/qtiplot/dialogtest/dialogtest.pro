######################################################################
# Automatically generated by qmake (2.00a) Sa Mai 13 14:37:45 2006
######################################################################

TEMPLATE = app
TARGET = dlgtest
DEPENDPATH += . ../src
INCLUDEPATH += . ../src

# Input
SOURCES +=  main.cpp \
		 	../src/textDialog.cpp \
			../src/colorButton.cpp \
			../src/symbolDialog.cpp \
			../src/plotWizard.cpp
	
HEADERS +=  ../src/textDialog.h \
			../src/colorButton.h \
			../src/symbolDialog.h \
			../src/plotWizard.h

QT +=  opengl
CONFIG      += qt warn_on debug thread opengl
