#ifndef PLOTDIALOG_H
#define PLOTDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
#include <qwt_symbol.h>

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QComboBox;
class QListBox;
class QListBoxItem;
class QPushButton;
class QSpinBox;
class QTabWidget;
class QWidget;
class QCheckBox;
class QGroupBox;
class QButtonGroup;
class QLabel;
class QLineEdit;
class ColorBox;
class PatternBox;
class ColorButton;
class QwtSymbol;
class Graph;
class SymbolBox;
class QwtCounter;

typedef struct{ 
  int lCol;
  int lWidth;
  int lStyle;
  int filledArea;
  int aCol;
  int aStyle;
  int symCol;
  int fillCol;
  int penWidth;
  int sSize; 
  int sType;  
  int connectType;
}  curveLayout;

class plotDialog : public QDialog
{ 
    Q_OBJECT

public:
    plotDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~plotDialog();

    QListBox* listBox;
    QPushButton* buttonApply, *btnWorksheet, *btnAssociations;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QComboBox* boxPlotType;
    QWidget* linePage;
    QComboBox* boxConnect;
    QComboBox* boxLineStyle;
    QSpinBox* boxLineWidth, *boxPenWidth;
    ColorBox* boxLineColor, *boxAreaColor;
    QWidget* symbolPage;
    QSpinBox* boxSymbolSize;
    ColorBox* boxSymbolColor,*boxFillColor;
    SymbolBox* boxSymbolStyle;
    PatternBox *boxPattern;
	QTabWidget* privateTabWidget;

	QWidget* errorsPage;
	QButtonGroup* GroupBox0;
	QButtonGroup* GroupBox1;
	QButtonGroup* GroupBox2;
	QButtonGroup* GroupBox3;
	QButtonGroup* fillGroupBox;
    QCheckBox* plusBox;
    QCheckBox* minusBox;
    QCheckBox* xBox;
    ColorButton* colorBox;
	ColorBox* vectColorBox;
    QComboBox* widthBox;
    QComboBox* capBox;
    QCheckBox* throughBox;
	QLabel *TextLabel4_3, *TextLabel4_4, *labelPosition, *labelXEnd, *labelYEnd;

	QButtonGroup* GroupBoxH;
	QWidget *histogramPage, *spacingPage;
	QLineEdit *binSizeBox, *histogramBeginBox, *histogramEndBox;
	QCheckBox *automaticBox;
	QPushButton* buttonStatistics, *btnEditFunction;
	QSpinBox* gapBox, *offsetBox, *boxWidth;

	QWidget *vectPage, *boxPage, *percentilePage;
	QComboBox *xEndBox, *yEndBox, *boxType, *boxWhiskersType, *boxWhiskersRange, *boxRange;
	SymbolBox *boxMaxStyle, *boxMinStyle, *boxMeanStyle, *box99Style, *box1Style;
	QSpinBox* headAngleBox, *headLengthBox, *vectWidthBox, *boxPercSize, *boxEdgeWidth;
	QCheckBox *filledHeadBox, *boxFill;
	QSpinBox *boxCoef, *boxWhiskersCoef;
	QCheckBox *boxFillSymbols, *boxFillSymbol;
	ColorBox *boxPercFillColor, *boxEdgeColor;
	QLabel 	*whiskerCoeffLabel, *whiskerRangeLabel, *boxCoeffLabel;
	QLabel *boxRangeLabel, *whiskerCntLabel, *boxCntLabel;
	QwtCounter *whiskerCnt, *boxCnt;
	QButtonGroup *GroupBoxVectEnd;
	QComboBox *vectPosBox;

public slots:
	void showStatistics();
	void clearTabWidget();
	void initLinePage();
	void initSymbolsPage();
	void initHistogramPage();
	void initErrorsPage();
	void initSpacingPage();
	void initVectPage();
	void initBoxPage();
	void initPercentilePage();

	void customVectorsPage(bool angleMag);

	void insertCurvesList();
	void insertColumnsList(const QStringList& names){columnNames = names;};
	void updateEndPointColumns(const QString& text);
	
	void fillBoxSymbols();
	void fillSymbols();
	bool acceptParams();
	void showWorksheet();
	void quit();

	int setPlotType(int index);
	void changePlotType(int plotType);
	void setActiveCurve(int curveIndex);

	void insertTabs(int plot_type);
	void updateTabWindow(int);
	void showAreaColor(bool show);

	void showPopupMenu(QListBoxItem *it, const QPoint &point);
	void removeCurve();
	void removeSelectedCurve();
	
	/******* error bars options **************/
	void pickErrorBarsColor();
	void changeErrorBarsType();
	void changeErrorBarsPlus();
	void changeErrorBarsMinus();
	void changeErrorBarsThrough();
	
	void setAutomaticBinning();
	bool validInput();
	void showPlotAssociations();
	void showPlotAssociations(QListBoxItem *item);
	void editFunctionCurve();
	void setGraph(Graph *g);
	void selectCurve(int index);

	void setPenStyle(Qt::PenStyle style);

	//box plots
	void setBoxType(int index);
	void setBoxRangeType(int index);
	void setWhiskersRange(int index);
	
protected:
	Graph *graph;
	QStringList columnNames;
	int lastSelectedCurve;
};

#endif
