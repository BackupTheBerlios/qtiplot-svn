/* vim: set filetype=cpp : */

%Module qti 0
%Import qt/qtmod.sip

class ScriptEdit: QTextEdit
{
%TypeHeaderCode
#include "src/scriptedit.h"
%End
public slots:
  void execute();
  void executeAll();
  void evaluate();
  void print();
  void insertFunction(const QString &);
private:
  ScriptEdit(ScriptEdit&);
};

class myWidget: QWidget /PyName=MDIWindow/
{
%TypeHeaderCode
#include "src/widget.h"
%End
public:
  enum CaptionPolicy{Name = 0, Label = 1, Both = 2};

  SIP_PYOBJECT windowLabel();
%MethodCode
  sipRes = PyString_FromString(sipCpp->windowLabel());
%End
  void setWindowLabel(const QString&);

  CaptionPolicy captionPolicy();
  void setCaptionPolicy(CaptionPolicy);

  virtual SIP_PYOBJECT saveAsTemplate(const QString& );
%MethodCode
  sipRes = PyString_FromString(sipCpp->saveAsTemplate(*a0));
%End
  virtual void restore(const QStringList& );

private:
  myWidget(const myWidget&);
};

class Table: myWidget
{
%TypeHeaderCode
#include "src/worksheet.h"

  inline int sipqti_colArg(Table *t, PyObject *c)
  {
    int col;
    PyObject *tmp;
    if (PyInt_Check(c))
      col = (int)PyInt_AsLong(c)-1;
    else
    {
      tmp = PyObject_Str(c);
      col = t->colNames().findIndex(PyString_AsString(tmp));
      Py_XDECREF(tmp);
    }
    return col;
  }
%End

public:
  int tableRows() /PyName=numRows/;
  int tableCols() /PyName=numCols/;
  void resizeRows(int) /PyName=setNumRows/;
  void resizeCols(int) /PyName=setNumCols/;

  SIP_PYOBJECT text(SIP_PYOBJECT, int);
%MethodCode
    sipRes = PyString_FromString(sipCpp->text(a1-1, sipqti_colArg(sipCpp, a0)));
%End
  double cell(SIP_PYOBJECT, int);
%MethodCode
    sipRes = sipCpp->text(a1-1, sipqti_colArg(sipCpp, a0)).toDouble();
%End
  void setText(SIP_PYOBJECT, int, const char*);
%MethodCode
    sipCpp->setText(a1-1, sipqti_colArg(sipCpp, a0), a2);
%End
  void setCell(SIP_PYOBJECT, int, double);
%MethodCode
  int prec;
  char f;
  int col = sipqti_colArg(sipCpp, a0);
  sipCpp->columnNumericFormat(col, f, prec);
  sipCpp->setText(a1-1,col,QString::number(a2, f, prec));
%End
  SIP_PYOBJECT colName(int);
%MethodCode
    sipRes = PyString_FromString(sipCpp->colName(a0-1));
%End
  void setColName(SIP_PYOBJECT, const QString&);
%MethodCode
    sipCpp->setColName(sipqti_colArg(sipCpp, a0), *a1);
%End
  void notifyChanges();

  void importASCII(const QString&, const QString&="\t", int=0, bool=false, bool=true, bool=false, bool=false);
  void normalizeCol(SIP_PYOBJECT) /PyName=normalize/;
%MethodCode
  sipCpp->normalizeCol(sipqti_colArg(sipCpp, a0));
%End
  void normalizeTable() /PyName=normalize/;
  
private:
  Table(const Table&);
};

class Matrix: myWidget
{
%TypeHeaderCode
#include "src/matrix.h"
%End
public:
  int numRows();
  int numCols();
  SIP_PYOBJECT text(int, int);
%MethodCode
    sipRes = PyString_FromString(sipCpp->text(a0-1, a1-1));
%End
  double cell(int, int);
%MethodCode
    sipRes = sipCpp->text(a0-1, a1-1).toDouble();
%End
  void setText(int, int, const char*);
%MethodCode
    sipCpp->setText(a0-1, a1-1, a2);
%End
  void setCell(int, int, double);
%MethodCode
  int prec = sipCpp->precision();
  char f = sipCpp->textFormat();
  sipCpp->setText(a0-1,a1-1,QString::number(a2, f, prec));
%End
private:
  Matrix(const Matrix&);
};

class LineMarker // : QwtPlotMarker
{
%TypeHeaderCode
#include "src/LineMarker.h"
%End
public:
  // virtual properties?
  QPoint startPoint() /PyName=start/;
  void setStartPoint(QPoint) /PyName=setStart/;
  QPoint endPoint() /PyName=end/;
  void setEndPoint(QPoint) /PyName=setEnd/;
private:
  LineMarker(const LineMarker&);
};

class Graph : QWidget /PyName=Layer/
{
%TypeHeaderCode
#include "src/graph.h"
#include "src/LegendMarker.h"
%End
public:
  bool isPiePlot();
  SIP_PYOBJECT pieLegendText() /PyName=pieLegend/;
%MethodCode
  sipRes = PyString_FromString(sipCpp->pieLegendText());
%End

  bool insertCurve(Table*, const QString&, int=1);
  bool insertCurve(Table*, const QString&, const QString&, int=1);
  void removeCurve(int);
  void removeCurve(const QString&);
  void deleteFitCurves();
  int curves() /PyName=numCurves/;

  void addErrorBars(Table *, const QString&, 
      Table *, const QString&,
      int, int, int, const QColor&,
      bool, bool,bool);

  void insertLineMarker(LineMarker*);
  LineMarker* lineMarker(long);

  void setTitle(const QString& t);
  void newLegend();
  void setLegend(const QString&) /NoDerived/;
%MethodCode
  sipCpp->legend()->setText(*a0);
%End
  void setXAxisTitle(const QString& text) /PyName=setXTitle/;
  void setYAxisTitle(const QString& text) /PyName=setYTitle/;

  void print();
  void exportImage(const QString& fileName, const QString& fileType = "PNG", 
  			int quality = 100, bool transparent = false);
							
  void exportToSVG(const QString& fname);
  void exportToEPS(const QString& fname);
  void exportToWmf(const QString& fname) /PyName=exportToWMF/;
private:
  Graph(const Graph&);
};

class MultiLayer : myWidget /PyName=Graph/
{
%TypeHeaderCode
#include "src/multilayer.h"
%End
public:
  Graph *activeGraph() /PyName=activeLayer/;
  void setActiveGraph(Graph*) /PyName=setActiveLayer/;
  int layers() /PyName=numLayers/;
  Graph* layer(int num);
  Graph* addLayer();
  void updateTransparency();
  void exportToSVG(const QString& fname);
  void exportToEPS(const QString& fname);
  void print();
private:
  MultiLayer(const MultiLayer&);
};

class Note: myWidget
{
%TypeHeaderCode
#include "src/note.h"
%End
public:
  void execute() const;
  bool autoexec() const;
private:
  Note(const Note&);
};

class ApplicationWindow: QMainWindow
{
%TypeHeaderCode
#include "src/application.h"
%End
%ConvertToSubClassCode
// we have to do this to override casting in qt/qobject.sip (PyQt 3.16)
sipClass=NULL;
QString cN = sipCpp->className();
sipTypeDef *t;
#if SIP_VERSION >= 0x040400
  for (int i=0; i<sipModuleAPI_qti.em_nrtypes; i++)
#else
  for (int i=0; sipModuleAPI_qti.em_types[i] != 0; i++)
#endif
  if (((t=sipModuleAPI_qti.em_types[i]->type)->td_cname && cN == t->td_cname) ||
      (!t->td_cname && cN == t->td_name+4))
    sipClass = sipModuleAPI_qti.em_types[i];
%End
public:
  Table* table(const QString&);
  Table* newTable();
  Table* newTable(const QString&, int=2, int=30);
%MethodCode
  sipRes = sipCpp->newTable(*a0, a2, a1);
%End
  Matrix* matrix(const QString&);
  Matrix* newMatrix();
  Matrix* newMatrix(const QString&, int=32, int=32);
  MultiLayer *plot(const QString&) /PyName=graph/;
  MultiLayer* newGraph();
  Note *note(const QString&);
  Note* newNote(const QString& = QString::null);

  MultiLayer *multilayerPlot(Table*, SIP_PYTUPLE, int=1) /PyName=plot/;
%MethodCode
  QStringList l;
  int n = PyTuple_Size(a1);
  for (int i=0; i<n; i++) {
    PyObject *str = PyObject_Str(PyTuple_GET_ITEM(a1,i));
    if (str)
      l << PyString_AsString(str);
    else {
      sipIsErr = 1;
      break;
    }
  }
  sipRes = sipCpp->multilayerPlot(a0, l, a2);
%End
  MultiLayer *multiLayerPlot(Table*, const char*, int=1) /PyName=plot/;
%MethodCode
  QStringList l;
  l << a1;
  sipRes = sipCpp->multilayerPlot(a0, l, a2);
%End
  
  QTextEdit *results;
//  QTextEdit *console;
private:
  ApplicationWindow(const ApplicationWindow&);
};

class Fit : QObject
{
%TypeHeaderCode
#include "src/Fitter.h"
%End
public:
  enum Algorithm{ScaledLevenbergMarquardt, UnscaledLevenbergMarquardt, NelderMeadSimplex};
  enum WeightingMethod{NoWeighting, Instrumental, Statistical, Dataset};

  Fit(ApplicationWindow* /TransferThis/, Graph*=0, const char*=0);
  ~Fit();

  virtual void fit();

  bool setWeightingData(WeightingMethod, const QString&=QString::null);

  bool setDataFromCurve(const QString&, Graph*=0);
  bool setDataFromCurve(const QString&, double, double, Graph*=0);

  void setInterval(double from, double to);

  QString formula();
  virtual void setParametersList(const QStringList&);
  int numParameters();

  void setInitialGuess(int, double) /PyName=setInitialValue/;
  void setInitialGuesses(...) /PyName=setInitialValues/;
%MethodCode
int n = PyTuple_GET_SIZE(a0);
double *values = new double[n];
for (int i=0; i<n; i++) {
  PyObject *item = PyTuple_GET_ITEM(a0, i);
  if (PyNumber_Check(item)) {
    item=PyNumber_Float(item);
    if (!item) {
      sipIsErr=1;
      break;
    }
    values[i] = PyFloat_AS_DOUBLE(item);
    Py_DECREF(item);
  } else
    values[i] = 0;
}
sipCpp->setInitialGuesses(values);
delete values;
%End

  virtual void guessInitialValues();

  void setAlgorithm(Algorithm);

  void setTolerance(double);

  void setColor(int);
  void setColor(const QColor&);

  void setOutputPrecision(int);

  void generateFunction(bool, int=100);

  void setMaximumIterations(int);

  void showLegend();

  virtual QString legendFitInfo();

  SIP_PYTUPLE results();
%MethodCode
double *results = sipCpp->results();
int size=sipCpp->numParameters();
sipRes = PyTuple_New(size);
if(sipRes)
{
  for(int i=0; i<size; i++)
    PyTuple_SET_ITEM(sipRes, i, PyFloat_FromDouble(results[i]));
} else
  sipIsErr = 1;
%End

  SIP_PYTUPLE errors();
%MethodCode
double *errors = sipCpp->errors();
int size=sipCpp->numParameters();
sipRes = PyTuple_New(size);
if(sipRes)
{
  for(int i=0; i<size; i++)
    PyTuple_SET_ITEM(sipRes, i, PyFloat_FromDouble(errors[i]));
} else
  sipIsErr = 1;
%End

  //! Returns the sum of squares of the residuals from the best-fit line
  double chiSquare();

  Table* parametersTable(const QString&);
  Matrix* covarianceMatrix(const QString&);
};

%ModuleCode
ApplicationWindow *sipqti_app()
{
  int iserr = 0;
  PyObject *me = PyImport_ImportModule("qti");
  PyObject *mydict = PyModule_GetDict(me);
  PyObject *pyapp = PyDict_GetItemString(mydict,"app");
  Py_DECREF(me);
  if (sipCanConvertToInstance(pyapp, sipClass_ApplicationWindow, SIP_NOT_NONE))
    return (ApplicationWindow*) sipConvertToInstance(pyapp, sipClass_ApplicationWindow, NULL, SIP_NOT_NONE, NULL, &iserr);
  else
    return NULL;
}
%End
%ModuleHeaderCode
class ApplicationWindow;
ApplicationWindow *sipqti_app();
#define SIPQTI_APP(sipcppexpr)\
ApplicationWindow *app = sipqti_app();\
if (app) sipCpp = sipcppexpr;\
else { sipCpp = NULL; }
%End

class ExponentialFit : Fit
{
%TypeHeaderCode
#include "src/Fitter.h"
%End
public:
  ExponentialFit(ApplicationWindow * /TransferThis/, Graph *, const QString&, bool=false);
  ExponentialFit(ApplicationWindow * /TransferThis/, Graph *, const QString&, int, int, bool=false);
  ExponentialFit(Graph *, const QString&, bool=false);
%MethodCode
  SIPQTI_APP(new sipExponentialFit(app, a0, *a1, a2))
%End
  ExponentialFit(Graph *, const QString&, int, int, bool=false);
%MethodCode
  SIPQTI_APP(new sipExponentialFit(app, a0, *a1, a2, a3, a4))
%End
};

class TwoExpFit : Fit
{
%TypeHeaderCode
#include "src/Fitter.h"
%End
public:
  TwoExpFit(ApplicationWindow * /TransferThis/, Graph *, const QString&);
  TwoExpFit(ApplicationWindow * /TransferThis/, Graph *, const QString&, int, int);
  TwoExpFit(Graph *, const QString&);
%MethodCode
  SIPQTI_APP(new sipTwoExpFit(app, a0, *a1))
%End
  TwoExpFit(Graph *, const QString&, int, int);
%MethodCode
  SIPQTI_APP(new sipTwoExpFit(app, a0, *a1, a2, a3))
%End
};

class ThreeExpFit : Fit
{
%TypeHeaderCode
#include "src/Fitter.h"
%End
public:
  ThreeExpFit(ApplicationWindow * /TransferThis/, Graph *, const QString&);
  ThreeExpFit(ApplicationWindow * /TransferThis/, Graph *, const QString&, int, int);
  ThreeExpFit(Graph *, const QString&);
%MethodCode
  SIPQTI_APP(new sipThreeExpFit(app, a0, *a1))
%End
  ThreeExpFit(Graph *, const QString&, int, int);
%MethodCode
  SIPQTI_APP(new sipThreeExpFit(app, a0, *a1, a2, a3))
%End
};

class SigmoidalFit : Fit
{
%TypeHeaderCode
#include "src/Fitter.h"
%End
public:
  SigmoidalFit(ApplicationWindow * /TransferThis/, Graph *, const QString&);
  SigmoidalFit(ApplicationWindow * /TransferThis/, Graph *, const QString&, int, int);
  SigmoidalFit(Graph *, const QString&);
%MethodCode
  SIPQTI_APP(new sipSigmoidalFit(app, a0, *a1))
%End
  SigmoidalFit(Graph *, const QString&, int, int);
%MethodCode
  SIPQTI_APP(new sipSigmoidalFit(app, a0, *a1, a2, a3))
%End
};

class GaussAmpFit : Fit
{
%TypeHeaderCode
#include "src/Fitter.h"
%End
public:
  GaussAmpFit(ApplicationWindow * /TransferThis/, Graph *, const QString&);
  GaussAmpFit(ApplicationWindow * /TransferThis/, Graph *, const QString&, int, int);
  GaussAmpFit(Graph *, const QString&);
%MethodCode
  SIPQTI_APP(new sipGaussAmpFit(app, a0, *a1))
%End
  GaussAmpFit(Graph *, const QString&, int, int);
%MethodCode
  SIPQTI_APP(new sipGaussAmpFit(app, a0, *a1, a2, a3))
%End
};

class NonLinearFit : Fit
{
%TypeHeaderCode
#include "src/Fitter.h"
%End
public:
  NonLinearFit(ApplicationWindow * /TransferThis/, Graph *, const QString&);
  NonLinearFit(ApplicationWindow * /TransferThis/, Graph *, const QString&, int, int);
  NonLinearFit(Graph *, const QString&);
%MethodCode
  SIPQTI_APP(new sipNonLinearFit(app, a0, *a1))
%End
  NonLinearFit(Graph *, const QString&, int, int);
%MethodCode
  SIPQTI_APP(new sipNonLinearFit(app, a0, *a1, a2, a3))
%End
  void setParametersList(const QStringList&);
  // TODO: make it accept Python callables
  void setFormula(const QString&);
};

class PluginFit : Fit
{
%TypeHeaderCode
#include "src/Fitter.h"
%End
public:
  PluginFit(ApplicationWindow * /TransferThis/, Graph *, const QString&);
  PluginFit(ApplicationWindow * /TransferThis/, Graph *, const QString&, int, int);
  PluginFit(Graph *, const QString&);
%MethodCode
  SIPQTI_APP(new sipPluginFit(app, a0, *a1))
%End
  PluginFit(Graph *, const QString&, int, int);
%MethodCode
  SIPQTI_APP(new sipPluginFit(app, a0, *a1, a2, a3))
%End
  bool load(const QString&);
};

class MultiPeakFit : Fit
{
%TypeHeaderCode
#include "src/Fitter.h"
%End
public:
  enum PeakProfile{Gauss, Lorentz};
  MultiPeakFit(ApplicationWindow * /TransferThis/, Graph *, PeakProfile=Gauss, int=1);

  int peaks() /PyName=numPeaks/;
  void setNumPeaks(int);

  void enablePeakCurves(bool);
  void setPeakCurvesColor(int);

  static QString generateFormula(int, PeakProfile);
  static QStringList generateParameterList(int);
};

class LorentzFit : MultiPeakFit
{
%TypeHeaderCode
#include "src/Fitter.h"
%End
public:
  LorentzFit(ApplicationWindow * /TransferThis/, Graph *, const QString&);
  LorentzFit(ApplicationWindow * /TransferThis/, Graph *, const QString&, int, int);
  LorentzFit(Graph *, const QString&);
%MethodCode
  SIPQTI_APP(new sipLorentzFit(app, a0, *a1))
%End
  LorentzFit(Graph *, const QString&, int, int);
%MethodCode
  SIPQTI_APP(new sipLorentzFit(app, a0, *a1, a2, a3))
%End
};

class GaussFit : MultiPeakFit
{
%TypeHeaderCode
#include "src/Fitter.h"
%End
public:
  GaussFit(ApplicationWindow * /TransferThis/, Graph *, const QString&);
  GaussFit(ApplicationWindow * /TransferThis/, Graph *, const QString&, int, int);
  GaussFit(Graph *, const QString&);
%MethodCode
  SIPQTI_APP(new sipGaussFit(app, a0, *a1))
%End
  GaussFit(Graph *, const QString&, int, int);
%MethodCode
  SIPQTI_APP(new sipGaussFit(app, a0, *a1, a2, a3))
%End
};

class PolynomialFit : Fit
{
%TypeHeaderCode
#include "src/Fitter.h"
%End
public:
  PolynomialFit(ApplicationWindow * /TransferThis/, Graph *, QString&, int=2, bool=false);
  PolynomialFit(ApplicationWindow * /TransferThis/, Graph *, QString&, int, int, int=2, bool=false);
  PolynomialFit(Graph *, QString&, int=2, bool=false);
%MethodCode
  SIPQTI_APP(new sipPolynomialFit(app, a0, *a1, a2, a3))
%End
  PolynomialFit(Graph *, QString&, int, int, int=2, bool=false);
%MethodCode
  SIPQTI_APP(new sipPolynomialFit(app, a0, *a1, a2, a3, a4, a5))
%End

  virtual QString legendFitInfo();
  void fit();

  static QString generateFormula(int);
  static QStringList generateParameterList(int);
};

class LinearFit : Fit
{
%TypeHeaderCode
#include "src/Fitter.h"
%End
public:
  LinearFit(ApplicationWindow * /TransferThis/, Graph *, const QString&);
  LinearFit(ApplicationWindow * /TransferThis/, Graph *, const QString&, int, int);
  LinearFit(Graph *, const QString&);
%MethodCode
  SIPQTI_APP(new sipLinearFit(app, a0, *a1))
%End
  LinearFit(Graph *, const QString&, int, int);
%MethodCode
  SIPQTI_APP(new sipLinearFit(app, a0, *a1, a2, a3))
%End
  void fit();
};

class PythonScript
{
%TypeHeaderCode
#include "src/PythonScripting.h"
%End
public:
  void write(const QString&);
private:
  PythonScript(const PythonScript&);
};
