#ifndef APPLICATION_H
#define APPLICATION_H

#include <qmainwindow.h>
#include <qlistview.h> 

#include "graph3D.h"
#include "plot3DDialog.h"

class QAction;
class QActionGroup;
class QTextEdit;
class QToolBar;
class QToolButton;
class QPopupMenu;
class QWorkspace;
class QPopupMenu;
class QListView;
class QToolButton;
class QListBoxItem;
class QTextBrowser;
class QLineEdit;
class QDockWindow;
class QTranslator;
class QNetworkOperation;

class Matrix;
class Table;
class Graph;
class ScalePicker;
class Graph3D;
class Note;
class MultiLayer;
class fDialog;

class dockWindow : public QDockWindow 
{
public:	
	
	dockWindow(QWidget *, const char *, WFlags ){};
	~dockWindow(){};

	void contextMenuEvent(QContextMenuEvent *){};
};

class ApplicationWindow: public QMainWindow
{
    Q_OBJECT
public:
    ApplicationWindow();
	ApplicationWindow(const QStringList& l);
	~ApplicationWindow();

	QTranslator *appTranslator, *qtTranslator;
	dockWindow *explorerWindow;
	QDockWindow *logWindow;
	QTextEdit *results;
	QWorkspace* ws;
    QToolBar *fileTools, *plotTools, *tableTools, *plot3DTools, *displayBar, *editTools;
    QPopupMenu *windowsMenu,*view,*graph,*file,*format,*calcul,*edit,*dataMenu,*recent, *exportPlot;
	QPopupMenu *help,*type,*import,*plot2D,*plot3D, *specialPlot, *panels,*stat,*decay, *filter;
	QPopupMenu *matrixMenu, *plot3DMenu, *plotDataMenu, *tableMenu, *tablesDepend; 
	QPopupMenu *smooth, *normMenu, *translateMenu, *fillMenu, *setAsMenu, *multiPeakMenu;
	QListView *lv;
	QToolButton *btnResults;
	QWidgetList *hiddenWindows, *outWindows;
	QTextBrowser *browser;
	QLineEdit *info;
	QWidget *lastModified;

	QActionGroup* dataTools;
	QAction *btnCursor, *btnSelect, *btnPicker, *btnRemovePoints, *btnMovePoints;
	QAction  *btnZoom, *btnPointer, *btnLine;
	
	QActionGroup* coord;
	QAction* Box;
    QAction* Frame;
    QAction* None;

	QActionGroup* grids;
    QAction* front;
    QAction* back;
    QAction* right;
    QAction* left;
    QAction* ceil;
    QAction* floor;

	QActionGroup* floorstyle;
    QAction* floordata;
    QAction* flooriso;
    QAction* floornone;

	QActionGroup* plotstyle;
    QAction* wireframe;
    QAction* hiddenline;
    QAction* polygon;
    QAction* filledmesh;
    QAction* pointstyle;
	QAction* barstyle;
	QAction *conestyle, *crossHairStyle;
	Graph *activeGraph,  *lastCopiedLayer;

public slots:
	void open();
	ApplicationWindow* open(const QString& fn);
	ApplicationWindow* openProject(const QString& fn);
	ApplicationWindow* importOPJ(const QString& filename);
	ApplicationWindow * plotFile(const QString& fn);
	void updatePlotsTransparency();

	QWidgetList* windowsList();
	void updateWindowLists(QWidget *w);

	void setSaveSettings(bool autoSaving, int min);
	void changeAppStyle(const QString& s);
	void changeAppFont(const QFont& f);
	void updateAppFonts();
	void setAppColors(const QColor& wc,const QColor& pc,const QColor& tpc);

	//multilayer plots
	MultiLayer* plot(const QString& name);
	MultiLayer* copyGraph();
	MultiLayer* multilayerPlot(int c, int r, int style);
	MultiLayer* multilayerPlot(Table* w,const QStringList& colList, int style);
	MultiLayer* multilayerPlot(const QString& caption);
	MultiLayer* multilayerPlot(const QStringList& colList); //for plot wizard
	void connectMultilayerPlot(MultiLayer *g);
	void addLayer();
	void deleteLayer();
	void initMultilayerPlot(MultiLayer* g, const QString& name);
	void polishGraph(Graph *g, int style);
	void plot2VerticalLayers();
	void plot2HorizontalLayers();
	void plot4Layers();
	void plotStackedLayers();
	void plotStackedHistograms();

	//3D data plots
	Graph3D* openMatrixPlot3D(const QString& caption, const QString& matrix_name,
							 double xl,double xr,double yl,double yr,double zl,double zr);
	Graph3D* dataPlot3D(Table* table,const QString& colName);
	Graph3D* dataPlotXYZ(Table* table,const QString& zColName, int type);
		//when reading from .qti file
	Graph3D* dataPlot3D(const QString& caption,const QString& formula,
						double xl, double xr, double yl, double yr, double zl, double zr);
	Graph3D* dataPlotXYZ(const QString& caption,const QString& formula,
 						double xl, double xr, double yl, double yr, double zl, double zr);
	/*!
	* used when plotting from the wizard
	*/
	Graph3D* dataPlotXYZ(const QString& formula);
	Graph3D* dataPlot3D(const QString& formula);

	//surface plots
	Graph3D* newPlot3D(const QString& formula, double xl, double xr,
					   double yl, double yr, double zl, double zr);
	Graph3D* newPlot3D(const QString& caption,const QString& formula, 
					   double xl, double xr,double yl, double yr, double zl, double zr);
	Graph3D* copySurfacePlot();
	Graph3D* surfacePlot(const QString& name);
	void connectSurfacePlot(Graph3D *plot);
	void newSurfacePlot();
	void editSurfacePlot();
	void remove3DMatrixPlots(Matrix *m);
	void update3DMatrixPlots(QWidget *w);
	void add3DData();
	void change3DData();
	void change3DData(const QString& colName);
	void change3DMatrix();
	void change3DMatrix(const QString& matrix_name);
	void insertNew3DData(const QString& colName);
	void add3DMatrixPlot();
	void insert3DMatrixPlot(const QString& matrix_name);
	void initPlot3D(Graph3D *plot);
	void customPlot3D(Graph3D *plot);
	void setPlot3DOptions();

	//user-defined functions
	void newFunctionPlot();
	void newFunctionPlot(QString& type,QStringList &formulas,QStringList &vars,QValueList<double> &ranges,QValueList<int> &points);

	fDialog* functionDialog();
	void showFunctionDialog(const QString& function, int curve);
	void addFunctionCurve();
	void clearFunctionsList();
	void clearFitFunctionsList();
	void saveFitFunctionsList(const QStringList& l);
	void clearSurfaceFunctionsList();
	void clearLogInfo();
	void clearParamFunctionsList();
	void clearPolarFunctionsList();
	void updateFunctionLists(QString& type,QStringList &formulas);
	void updateSurfaceFuncList(const QString& s);

	Matrix* cloneMatrix();
	Matrix* newMatrix();
	Matrix* newMatrix(const QString& caption, int r, int c);
	Matrix* matrix(const QString& name);
	Matrix* createIntensityMatrix(const QPixmap& pic);
	Matrix* convertTableToMatrix();
	void initMatrix(Matrix* m, const QString& caption);
	void transposeMatrix();
	void invertMatrix();
	void matrixDeterminant();
	
	Table* newTable();
	Table* newTable(const QString& fname, const QString &sep, int lines, 
		            bool renameCols, bool stripSpaces, bool simplifySpaces);
	Table* newTable(const QString& caption,int r, int c);
	Table* newTable(const QString& caption, int r, int c, const QString& text);
	Table* newHiddenTable(const QString& caption, int r, int c, const QString& text);
	Table* table(const QString& name);
	Table* copyTable();
	Table* convertMatrixToTable();
	QWidgetList* tableList();

	void connectTable(Table* w);
	void newWrksheetPlot(const QString& caption,int r, int c, const QString& text);
	void showHistogramTable(const QString& caption, int r, int c, const QString& text);
	void initTable(Table* w, const QString& caption);
	void customTable(Table* w);
	void customizeTables(const QColor& bgColor,const QColor& textColor,
						const QColor& headerColor,const QFont& textFont, const QFont& headerFont);
	
	void customGraph(Graph* g);
	void setGraphDefaultSettings(bool autoscale,bool scaleFonts,bool resizeLayers);

	void plot3DWireframe();
	void plot3DHiddenLine();
	void plot3DPolygons();
	void plot3DWireSurface();

	void plot3DMatrix(int style);

	void plot3DRibbon();
	void plot3DScatter();
	void plot3DTrajectory();
	void plot3DBars();

	void plotL();
	void plotP();
	void plotLP();
	void plotPie();
	void plotVerticalBars();
	void plotHorizontalBars();
	void plotArea();
	void plotSteps();
	void plotSpline();
	void plotVerticalDropLines();
	void plotHistogram();
	void plotVectXYXY();
	void plotVectXYAM();
	void plotBoxDiagram();
	
	//image analysis
	void intensityTable();
	void pixelLineProfile();
	void loadImage();
	void loadImage(const QString& fn);
	void importImage();
	
    void loadASCII();
	void loadMultiple();
	void loadMultipleASCIIFiles(const QStringList& fileNames, int importFileAs);
	void exportAllTables(const QString& sep, bool colNames, bool expSelection);
	void exportASCII(const QString& tableName, const QString& sep, bool colNames, bool expSelection);

	void exportLayer();
    void exportGraph();
	void exportAllGraphs();
	void exportAllGraphs(const QString& dir, const QString& format, 
									int quality, bool transparency);
	void export2DPlotToFile(MultiLayer *plot, const QString& fileName, const QString& format, 
										int quality, bool transparency);
	void export3DPlotToFile(Graph3D *plot, const QString& fileName, 
										const QString& format);

	void saveProjectAs();
	bool saveProject();

	void readSettings();
	void saveSettings();
	void applyUserSettings();
	void modifiedProject();
	void modifiedProject(QWidget *w);
    void print();
	void print(QWidget* w);
	void printAllPlots();
	void printHelp();
	void setImportOptions(const QString& sep, int lines, bool rename, bool strip, bool simplify);
    
	void showExplorer();
	QStringList columnsList(Table::PlotDesignation plotType);
	
	void undo();
	void redo();
	
	myWidget* copyWindow();
	QWidget* window(QListViewItem *it);
	void rename();
	void renameWindow();
	void maximizeWindow(QListViewItem * lbi);
	bool minimized(QWidget* window);
	void checkMinimizedWindows();
	void minimizeWindow();
	void maximizeWindow();
	bool maximized(QWidget* window);
	bool hidden(QWidget* window);
	void resizedWindow(QWidget* window);
	void closeActiveWindow();
	void closeWindow(QWidget* window);
	void hideWindow(QWidget* window);
	void hideWindow();
	void hideActiveWindow();
	void activateWindow();
	void printWindow();
	void updateTable(const QString& caption,int row,const QString& text);
	void updateTableColumn(const QString& colName, double *dat, int rows);
    void about();
    void windowsMenuAboutToShow();
    void windowsMenuActivated( int id );
	void removeCurves(const QString& name);
	QStringList dependingPlots(const QString& caption);
	QStringList depending3DPlots(Matrix *m);
	QStringList multilayerDependencies(QWidget *w);

	void saveAsTemplate();
	void openTemplate();

	QString windowGeometryInfo(QWidget *w);
	void restoreWindowGeometry(ApplicationWindow *app, QWidget *w, const QString s);

	void resizeActiveWindow();
	void resizeWindow();
	
	// list view in project explorer
	void updateListView(const QString& caption);
	void setListView(const QString& caption,const QString& view);
	void renameListViewItem(const QString& oldName,const QString& newName);
	void setListViewDate(const QString& caption,const QString& date);
	QString listViewDate(const QString& caption);
	void setListViewSize(const QString& caption,const QString& size);
	void setListViewLabel(const QString& caption,const QString& label);
	
	void updateColNames(const QString& oldName, const QString& newName);
	void updateTableNames(const QString& oldName, const QString& newName);
	void updateCurves(const QString& name);
	
	void showTable(const QString& curve);
	void showTable(int i);

	void addColToTable();
	void cutSelection();
	void copySelection();
	void copyMarker();
	void pasteSelection();
	void clearSelection();
	void clearCellFromTable(const QString& name,double value);
	void copyActiveLayer();
	
	void newProject();
	void newGraph();

	Matrix* openMatrix(ApplicationWindow* app, const QStringList &flist);
	Table* openTable(ApplicationWindow* app, const QStringList &flist);
	Graph3D* openSurfacePlot(ApplicationWindow* app, const QStringList &lst);
	void openGraph(ApplicationWindow* app, MultiLayer *plot, const QStringList &list);

	void openRecentProject(int index);
	void insertTranslatedStrings();
	void translateActionsStrings();
	void init();
	void initGlobalConstants();
	void createActions();
	void initMainMenu();
	void initPlotMenu();
	void initTableMenu();
	void initTableAnalysisMenu();
	void initPlotDataMenu();
	void initToolBars();
	void initPlot3DToolBar();
	void disableActions();
	void hideToolbars();
	void customToolBars(QWidget* w);
	void customMenu(QWidget* w);
	void windowActivated(QWidget *w);

	//table tools
	void sortSelection();
	void sortActiveTable();
	void normalizeSelection();
	void normalizeActiveTable();
	void correlate();
	void convolute();
	void deconvolute();

	// plot tools 
	void newLegend();
	void addTimeStamp();
	void drawLine();
	void addText();
	void disableAddText();
	void addImage();
	void zoom();
	void unzoom();
	void showRangeSelectors();	
	void showCursor();
	void showScreenReader();
	void pickPointerCursor();
	void disableTools();
	void pickDataTool( QAction* action );
	
	void updateLog(const QString& result);

	//fitting
	void deleteFitTables();
	void fitLinear();
	void fitSigmoidal();
	void fitGauss();
	void fitLorentz();
	void fitMultiPeakGauss();
	void fitMultiPeakLorentz();
				 
	//calculus
	void differentiate();
	void analysis(const QString& whichFit);
	void analyzeCurve(const QString& whichFit, const QString& curveTitle);
	void showAnalysisDialog(const QString& whichFit);

	void addErrorBars();
	void defineErrorBars(const QString& name,int type,const QString& percent,int direction);
	void defineErrorBars(const QString& curveName,const QString& errColumnName, int direction);
	void movePoints();
	void removePoints();

	// event handlers 
	void closeEvent( QCloseEvent*);
	void showEvent ( QShowEvent *);
	void hideEvent ( QHideEvent *);
	void timerEvent ( QTimerEvent *e);
	void dragEnterEvent( QDragEnterEvent* e );
	void dropEvent( QDropEvent* e );

	//dialogs
	void showPlotDialog();
	void showPlotDialog(long curveKey);
	QDialog* showScaleDialog();
	QDialog* showPieDialog();
	QDialog* showPlot3dDialog();
	axesDialog* showScalePageFromAxisDialog(int axisPos);
	axesDialog* showAxisPageFromAxisDialog(int axisPos);
	void showAxisDialog();
	void showGridDialog();
	void showGeneralPlotDialog();
	void showResults(bool ok);
	void showResults(const QString& s);
	void showTextDialog();
	void showLineDialog();
	void showTitleDialog();
	void showExportASCIIDialog();
	void showCurvesDialog();
	void showPlotAssociations(int curve);

	void showXAxisTitleDialog();
	void showYAxisTitleDialog();
	void showRightAxisTitleDialog();
	void showTopAxisTitleDialog();
	void showColumnOptionsDialog();
	void showRowsDialog();
	void showColsDialog();
	void showColMenu(int c);
	void showColumnValuesDialog();	
	void showGraphContextMenu();
	void showWindowContextMenu();
	void showWindowPopupMenu(QListViewItem *it, const QPoint &p, int);
	void showMoreWindows();
	void showImportDialog();
	void showMarkerPopupMenu();
	void showHelp();
	void chooseHelpFolder();
	void showPlotWizard();
	void showFitPolynomDialog();
	void showIntDialog();
	void showInterpolationDialog();
	void showExpGrowthDialog();
	void showExpDecayDialog();
	void showExpDecayDialog(int type);
	void showTwoExpDecayDialog();	
	void showExpDecay3Dialog();
	void showRowStatistics();
	void showColStatistics();
	void showFitDialog();
	void showImageDialog();
	void showPlotGeometryDialog();
	void showLayerDialog();
	void showPreferencesDialog();
	void showMatrixDialog();
	void showMatrixSizeDialog();
	void showMatrixValuesDialog();
	void showSmoothSavGolDialog();
	void showSmoothFFTDialog();
	void showSmoothAverageDialog();
	void lowPassFilterDialog();
	void highPassFilterDialog();
	void bandPassFilterDialog();
	void bandBlockFilterDialog();
	void showFFTDialog();

	void translateCurveHor();
	void translateCurveVert();

	void setAscValues();
	void setRandomValues();
	void setXCol();
	void setYCol();
	void setZCol();
	void disregardCol();

	void updateConfirmOptions(bool askTables, bool askMatrixes, bool askPlots2D, bool askPlots3D, bool askNotes);
	void showAxis(int axis, int type, const QString& labelsColName, bool axisOn, int ticksType,
		bool labelsOn, const QColor& c, int format, int prec, int rotation, int baselineDist, const QString& formula);
	
	//plot3D tools	
	void setFramed3DPlot();
	void setBoxed3DPlot();
	void removeAxes3DPlot();
	void removeGrid3DPlot();
	void setHiddenLineGrid3DPlot();
	void setLineGrid3DPlot();
	void setPoints3DPlot();
	void setCrosses3DPlot();
	void setCones3DPlot();
	void setBars3DPlot();
	void setFilledMesh3DPlot();
	void setEmptyFloor3DPlot();
	void setFloorData3DPlot();
	void setFloorIso3DPlot();
	void setFloorGrid3DPlot(bool on);
	void setCeilGrid3DPlot(bool on);
	void setRightGrid3DPlot(bool on);
	void setLeftGrid3DPlot(bool on);
	void setFrontGrid3DPlot(bool on);
	void setBackGrid3DPlot(bool on);
	void pickPlotStyle( QAction* action );
	void pickCoordSystem( QAction* action);
	void pickFloorStyle( QAction* action);
	void custom3DActions(QWidget *w);
	void custom3DGrids(int grids);

	void updateRecentProjectsList();
	
	//open internet browser
	void checkUpdates();
	void checkUpdates(QNetworkOperation *op);
	void showDonationDialog();
	void showSupportPage();
	void showDonationsPage();
	void showHomePage();
	void downloadManual();
	void downloadTranslation();
	bool open_browser(QWidget* parent, const QString& rUrl);

	void parseCommandLineArgument(const QString& s, int args);
	void createLanguagesList();
	void switchToLanguage(int param);
	void switchToLanguage(const QString& locale);

	bool allreadyUsedName(const QString& label);

	Note* newNote(const QString& caption = QString::null);
	Note* openNote(ApplicationWindow* app, const QStringList &flist);
	void initNote(Note* m, const QString& caption);
	
signals:
	void windowClosed(const QString&);
	void modified();
	
public:
	enum {MaxRecentProjects = 5};
	int fileVersion;
	int majVersion, minVersion, patchVersion;
	QColor workspaceColor, panelsColor, panelsTextColor;
	QString appStyle, workingDir;
	bool smooth3DMesh, autoScaleFonts, autoResizeLayers, askForSupport;
	bool confirmCloseTable, confirmCloseMatrix, confirmClosePlot2D, confirmClosePlot3D, confirmCloseNotes;
	bool canvasFrameOn, titleOn, autoSave, drawBackbones, allAxesOn, autoscale2DPlots;
	int ticksStyle, legendFrameStyle, autoSaveTime, axesLineWidth, canvasFrameWidth;
	int majTicksLength, minTicksLength, defaultPlotMargin;
	int defaultCurveStyle, defaultCurveLineWidth, defaultSymbolSize;
	QFont appFont, plot3DTitleFont, plot3DNumbersFont, plot3DAxesFont;
	QFont tableTextFont, tableHeaderFont, plotAxesFont, plotLegendFont, plotNumbersFont, plotTitleFont;
	QColor tableBkgdColor, tableTextColor, tableHeaderColor;
	QString projectname,separator, helpFilePath, appLanguage;
	QString configFilePath, logInfo, fitPluginsPath;
	int logID,asciiID,closeID;
	int exportID, printAllID;
	int notes, graphs,tables, matrixes, fitNumber, ignoredLines, timerId, savingTimerId;
	bool renameColumns, copiedLayer, strip_spaces, simplify_spaces;
	QStringList plotWindows,tableWindows, recentProjects, plot3DWindows, matrixWindows, noteWindows;
	bool saved, showPlot3DProjection, showPlot3DLegend;
	int plot3DResolution;
	QStringList plot3DColors, locales;
	QStringList functions; //user-defined functions;
	QStringList xFunctions, yFunctions, rFunctions,tetaFunctions; // user functions for parametric and polar plots
	QStringList fitFunctions; //user-defined fit functions;
	QStringList surfaceFunc; //user-defined surface functions;
	QWidget *aw, *wmax; //active window and maximized window when loading a .qti file
	Graph::MarkerType copiedMarkerType;
	
//variables used when user copy/paste markers
	QString auxMrkText;
	QFont auxMrkFont;
	QColor auxMrkColor, auxMrkBkgColor;
	QPoint auxMrkStart,auxMrkEnd;
	Qt::PenStyle auxMrkStyle;
	QString auxMrkFileName;
	int auxMrkBkg,auxMrkWidth;
	bool startArrowOn, endArrowOn, fillArrowHead;
	int arrowHeadLength, arrowHeadAngle, specialPlotMenuID, statMenuID, panelMenuID, plot3dID;
	int plotMenuID, importMenuID, newMenuID, recentMenuID, setAsMenuID, fillMenuID, normMenuID;
	int translateMenuID, smoothMenuID, filterMenuID, fitExpMenuID, multiPeakMenuID; 

    QAction *actionNewProject, *actionNewNote, *actionNewTable, *actionNewFunctionPlot, *actionNewSurfacePlot, *actionNewMatrix, *actionNewGraph;
    QAction *actionOpen, *actionLoadImage, *actionSaveProject, *actionSaveProjectAs, *actionImportImage;
    QAction *actionLoad, *actionLoadMultiple, *actionUndo, *actionRedo;
    QAction *actionCopyWindow;
    QAction *actionCutSelection, *actionCopySelection, *actionPasteSelection, *actionClearSelection;
    QAction *actionShowExplorer, *actionShowLog, *actionAddLayer, *actionShowLayerDialog;

    QAction *actionExportGraph, *actionExportAllGraphs, *actionPrint, *actionPrintAllPlots, *actionShowExportASCIIDialog;
    QAction *actionShowImportDialog;
    QAction *actionCloseAllWindows, *actionClearLogInfo, *actionShowPlotWizard, *actionShowConfigureDialog;
    QAction *actionShowCurvesDialog, *actionAddErrorBars, *actionAddFunctionCurve, *actionUnzoom, *actionNewLegend, *actionAddImage, *actionAddText;
    QAction *actionPlotL, *actionPlotP, *actionPlotLP, *actionPlotVerticalDropLines, *actionPlotSpline, *actionPlotSteps, *actionPlotVerticalBars;
	QAction *actionPlotHorizontalBars, *actionPlotArea, *actionPlotPie, *actionPlotVectXYAM, *actionPlotVectXYXY;
    QAction *actionPlotHistogram, *actionPlotStackedHistograms, *actionPlot2VerticalLayers, *actionPlot2HorizontalLayers, *actionPlot4Layers, *actionPlotStackedLayers;
    QAction *actionPlot3DRibbon, *actionPlot3DBars, *actionPlot3DScatter, *actionPlot3DTrajectory;
    QAction *actionShowColStatistics, *actionShowRowStatistics, *actionShowIntDialog;
    QAction *actionDifferentiate, *actionFitLinear, *actionShowFitPolynomDialog;
    QAction *actionShowExpDecayDialog, *actionShowTwoExpDecayDialog, *actionShowExpDecay3Dialog;
    QAction *actionFitExpGrowth, *actionFitSigmoidal, *actionFitGauss, *actionFitLorentz, *actionShowFitDialog;
    QAction *actionShowLayoutDialog, *actionShowAxisDialog, *actionShowTitleDialog;
    QAction *actionShowColumnOptionsDialog, *actionShowColumnValuesDialog, *actionShowColsDialog, *actionShowRowsDialog;
    QAction *actionAbout, *actionShowHelp, *actionChooseHelpFolder;
    QAction *actionRename, *actionCloseWindow, *actionConvertTable;
    QAction *actionAddColToTable, *actionDeleteLayer, *actionInterpolate;
    QAction *actionPrintHelp, *actionResizeActiveWindow, *actionHideActiveWindow;
    QAction *actionShowMoreWindows, *actionPixelLineProfile, *actionIntensityTable;
    QAction *actionShowLineDialog, *actionShowImageDialog, *actionShowTextDialog;
    QAction *actionActivateWindow, *actionMinimizeWindow, *actionMaximizeWindow, *actionHideWindow, *actionResizeWindow, *actionPrintWindow;
    QAction *actionShowPlotGeometryDialog, *actionEditSurfacePlot, *actionAdd3DData;
	QAction *actionMatrixDeterminant, *actionSetMatrixProperties;
	QAction *actionSetMatrixDimensions, *actionConvertMatrix, *actionSetMatrixValues, *actionTransposeMatrix, *actionInvertMatrix;
	QAction *actionPlot3DWireFrame, *actionPlot3DHiddenLine, *actionPlot3DPolygons, *actionPlot3DWireSurface;
	QAction *actionDeleteFitTables, *actionShowGridDialog, *actionTimeStamp;
	QAction *actionSmoothSavGol, *actionSmoothFFT, *actionSmoothAverage, *actionFFT;
	QAction *actionLowPassFilter, *actionHighPassFilter, *actionBandPassFilter, *actionBandBlockFilter;
	QAction *actionSortTable, *actionSortSelection, *actionNormalizeSelection;
	QAction *actionNormalizeTable, *actionConvolute, *actionDeconvolute, *actionCorrelate;
	QAction *actionTranslateHor, *actionTranslateVert, *actionSetAscValues, *actionSetRandomValues;
	QAction *actionSetXCol, *actionSetYCol, *actionSetZCol, *actionDisregardCol;
	QAction *actionBoxPlot, *actionMultiPeakGauss, *actionMultiPeakLorentz, *actionCheckUpdates;
	QAction *actionDonate, *actionHomePage, *actionDownloadManual, *actionTechnicalSupport, *actionTranslations;
	QAction *actionShowPlotDialog, *actionShowScaleDialog, *actionOpenTemplate, *actionSaveTemplate;
};

#endif
