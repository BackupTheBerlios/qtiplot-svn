###############################################################
##################### SCRIPTING LANGUAGES SECTION #############
###############################################################

INCLUDEPATH += src/scripting/

HEADERS  += src/scripting/customevents.h\
            src/scripting/FindReplaceDialog.h\
            src/scripting/MyParser.h\
            src/scripting/Note.h\
            src/scripting/ScriptingEnv.h\
            src/scripting/Script.h\
            src/scripting/ScriptEdit.h\
            src/scripting/ScriptingLangDialog.h\
            src/scripting/ScriptWindow.h\

SOURCES  += src/scripting/FindReplaceDialog.cpp\
            src/scripting/MyParser.cpp\
            src/scripting/Note.cpp\
            src/scripting/ScriptingEnv.cpp\
            src/scripting/Script.cpp\
            src/scripting/ScriptEdit.cpp\
            src/scripting/ScriptingLangDialog.cpp\
            src/scripting/ScriptWindow.cpp\

##################### Default: muParser v1.28 #################

contains(SCRIPTING_LANGS, muParser) {
  DEFINES += SCRIPTING_MUPARSER

  HEADERS += src/scripting/muParserScript.h \
             src/scripting/muParserScripting.h \

  SOURCES += src/scripting/muParserScript.cpp \
             src/scripting/muParserScripting.cpp \
}

##################### PYTHON + SIP + PyQT #####################

contains(SCRIPTING_LANGS, Python) {

  DEFINES += SCRIPTING_PYTHON

  HEADERS += src/scripting/PythonScript.h\
             src/scripting/PythonScripting.h\
             src/scripting/PythonSyntaxHighlighter.h\

  SOURCES += src/scripting/PythonScript.cpp\
  			 src/scripting/PythonScripting.cpp\
			 src/scripting/PythonSyntaxHighlighter.cpp\

##################### SIP generated files #####################

  SOURCES += $${SIP_DIR}/sipqticmodule.cpp\
             $${SIP_DIR}/sipqtiApplicationWindow.cpp\
             $${SIP_DIR}/sipqtiQwtPlot.cpp\
             $${SIP_DIR}/sipqtiGraph.cpp\
             $${SIP_DIR}/sipqtiGraph3D.cpp\
             $${SIP_DIR}/sipqtiArrowMarker.cpp\
             $${SIP_DIR}/sipqtiFrameWidget.cpp\
			 $${SIP_DIR}/sipqtiEllipseWidget.cpp\
			 $${SIP_DIR}/sipqtiImageWidget.cpp\
			 $${SIP_DIR}/sipqtiLegendWidget.cpp\
			 $${SIP_DIR}/sipqtiRectangleWidget.cpp\
			 $${SIP_DIR}/sipqtiGrid.cpp\
             $${SIP_DIR}/sipqtiMultiLayer.cpp\
             $${SIP_DIR}/sipqtiTable.cpp\
			 $${SIP_DIR}/sipqtiQwtLinearColorMap.cpp\
             $${SIP_DIR}/sipqtiMatrix.cpp\
             $${SIP_DIR}/sipqtiMdiSubWindow.cpp\
             $${SIP_DIR}/sipqtiScriptEdit.cpp\
             $${SIP_DIR}/sipqtiNote.cpp\
             $${SIP_DIR}/sipqtiPythonScript.cpp\
             $${SIP_DIR}/sipqtiPythonScripting.cpp\
             $${SIP_DIR}/sipqtiFolder.cpp\
             $${SIP_DIR}/sipqtiQList.cpp\
             $${SIP_DIR}/sipqtiFit.cpp \
             $${SIP_DIR}/sipqtiExponentialFit.cpp \
             $${SIP_DIR}/sipqtiTwoExpFit.cpp \
             $${SIP_DIR}/sipqtiThreeExpFit.cpp \
             $${SIP_DIR}/sipqtiSigmoidalFit.cpp \
			 $${SIP_DIR}/sipqtiLogisticFit.cpp \
             $${SIP_DIR}/sipqtiGaussAmpFit.cpp \
             $${SIP_DIR}/sipqtiLorentzFit.cpp \
             $${SIP_DIR}/sipqtiNonLinearFit.cpp \
             $${SIP_DIR}/sipqtiPluginFit.cpp \
             $${SIP_DIR}/sipqtiMultiPeakFit.cpp \
             $${SIP_DIR}/sipqtiPolynomialFit.cpp \
             $${SIP_DIR}/sipqtiLinearFit.cpp \
             $${SIP_DIR}/sipqtiGaussFit.cpp \
             $${SIP_DIR}/sipqtiFilter.cpp \
             $${SIP_DIR}/sipqtiDifferentiation.cpp \
             $${SIP_DIR}/sipqtiIntegration.cpp \
			 $${SIP_DIR}/sipqtiInterpolation.cpp \
			 $${SIP_DIR}/sipqtiSmoothFilter.cpp \
			 $${SIP_DIR}/sipqtiFFTFilter.cpp \
			 $${SIP_DIR}/sipqtiFFT.cpp \
			 $${SIP_DIR}/sipqtiCorrelation.cpp \
			 $${SIP_DIR}/sipqtiConvolution.cpp \
			 $${SIP_DIR}/sipqtiDeconvolution.cpp \
}
