#ifndef GRAPH3D_H
#define GRAPH3D_H

#include <qtimer.h> 

#include <qwt3d_surfaceplot.h>
#include <qwt3d_function.h> 

#include "worksheet.h"
#include "matrix.h"

using namespace Qwt3D;

class UserFunction;

class Graph3D: public myWidget
{
	Q_OBJECT

public:
	Graph3D (const QString& label, QWidget* parent, const char* name, WFlags f);
	~Graph3D();

	enum PlotType{Scatter=0, Trajectory = 1, Bars = 2};
	enum PointStyle{None=0, Dots=1, VerticalBars=2, HairCross=3, Cones=4};

	Qwt3D::SurfacePlot* sp;
	UserFunction *func;
	
public slots:
	void initPlot();
	void initCoord();
	void addFunction(const QString& s,double xl,double xr,double yl,
						  double yr,double zl,double zr);
	void insertFunction(const QString& s,double xl,double xr,double yl,
						  double yr,double zl,double zr);
	void insertNewData(Table* table, const QString& colName);

	Matrix * getMatrix(){return matrix_;};
	void addMatrixData(Matrix* m);//used to plot matrixes
	void addMatrixData(Matrix* m,double xl,double xr,double yl,double yr,double zl,double zr);
	void updateMatrixData(Matrix* m);

	void addData(Table* table, const QString& colName);
/*!
* used when creating a ribbon plot from the plot wizard
*/
	void addData(Table* table, int xcol, int ycol);
	void addData(Table* table,const QString& xColName,const QString& yColName);
	void addData(Table* table,const QString& xColName,const QString& yColName,
							double xl, double xr, double yl, double yr, double zl, double zr);
	void addData(Table* table, int xCol,int yCol,int zCol, int type);
	void addData(Table* table, int xCol,int yCol,int zCol, 
							double xl, double xr, double yl, double yr, double zl, double zr);

	void clearData();
	bool hasData(){return sp->hasData();};

	void updateData(Table* table);
	void updateDataXY(Table* table, int xCol, int yCol);
	void updateDataXYZ(Table* table, int xCol, int yCol, int zCol);

	void changeMatrix(Matrix* m);
	void changeDataColumn(Table* table, const QString& colName);

	//user function
	UserFunction* userFunction();
	QString formula();

	//event handlers
	bool eventFilter(QObject *object, QEvent *e);
	void resizeEvent ( QResizeEvent *);
	void contextMenuEvent(QContextMenuEvent *e);
	void scaleFonts(double factor);
	void setIgnoreFonts(bool ok){ignoreFonts = ok;};

	//axes
	void setFramed();
	void setBoxed();
	void setNoAxes();

	bool isOrthogonal(){return sp->ortho();};
	void setOrtho(bool on = true){sp->setOrtho(on);};

	//mesh
	void setNoGrid();
	void setHiddenLineGrid();
	void setLineGrid();
	void setFilledMesh();
	void setPointsMesh();
	void setBarsPlot();
	void setFloorData();
	void setFloorIsolines();
	void setEmptyFloor();

	//grid
	int grids();
	void setGrid(Qwt3D::SIDE s, bool b);
	void setGrid(int grids);

	void setLeftGrid(bool b);
	void setRightGrid(bool b);
	void setCeilGrid(bool b);
	void setFloorGrid(bool b);
	void setFrontGrid(bool b);
	void setBackGrid(bool b);

	void setStyle(Qwt3D::COORDSTYLE coord,Qwt3D::FLOORSTYLE floor,
							Qwt3D::PLOTSTYLE plot, Graph3D::PointStyle point);
	void setStyle(const QStringList& st);
	void customPlotStyle(int style);
	void resetNonEmptyStyle();

	void setRotation(double  xVal,double  yVal,double  zVal);
	void setScale(double  xVal,double  yVal,double  zVal);
	void setShift(double  xVal,double  yVal,double  zVal);
	void updateScaling(double  xVal,double  yVal,double  zVal);
	
	double xRotation(){return sp->xRotation();};
	double yRotation(){return sp->yRotation();};
	double zRotation(){return sp->zRotation();};
	
	double xScale(){return sp->xScale();};
	double yScale(){return sp->yScale();};
	double zScale(){return sp->zScale();};
	
	double xShift(){return sp->xShift();};
	double yShift(){return sp->yShift();};
	double zShift(){return sp->zShift();};
	
	double zoom(){return sp->zoom();};
	void setZoom(double  val);
	void updateZoom(double  val);
	
	Qwt3D::PLOTSTYLE plotStyle();
	Qwt3D::FLOORSTYLE floorStyle();
	Qwt3D::COORDSTYLE coordStyle();

	void print();
	void copyImage();
	void saveImageToFile(const QString& fname, const QString& format);
	void saveImage();
	QString saveToString(const QString& geometry);
	QString saveAsTemplate(const QString& geometryInfo);

	void zoomChanged(double);
	void rotationChanged(double, double, double);
	void scaleChanged(double, double, double);
	void shiftChanged(double, double, double);

	//mesh
	void setMeshLineWidth(int lw);
	double meshLineWidth(){return sp->meshLineWidth();};

	//colors
	void setDataColors(const QColor& cMax, const QColor& cMin);

	void updateColors(const QColor& meshColor,const QColor& axesColor,const QColor& numColor,
						   const QColor& labelColor,const QColor& bgColor,const QColor& gridColor);
	void changeTransparency(double t);
	void setTransparency(double t);
	double transparency(){return alpha;};
		
	QColor minDataColor();
	QColor maxDataColor();
	QColor meshColor(){return meshCol;};
	QColor axesColor(){return axesCol;};
	QColor labelColor(){return labelsCol;};
	QColor numColor(){return numCol;};
	QColor bgColor(){return bgCol;};
	QColor gridColor(){return gridCol;};

	QString colorMap(){return color_map;};
	void setDataColorMap(const QString& fileName);
	bool openColorMap(ColorVector& cv, QString fname);

	void setColors(const QStringList& colors);
	void setColors(const QColor& meshColor,const QColor& axesColor,const QColor& numColor,
						   const QColor& labelColor,const QColor& bgColor,const QColor& gridColor);

	//title
	void updateTitle(const QString& s,const QColor& color,const QFont& font);
	QFont titleFont(){return titleFnt;};
	void setTitleFont(const QFont& font);
	QString plotTitle(){return title;};
	QColor titleColor(){return titleCol;};
	void setTitle(const QStringList& lst);
	void setTitle(const QString& s,const QColor& color,const QFont& font);

	//resolution
	void setResolution(int r);
	int resolution(){return sp->resolution();};
	
	//legend
	void showColorLegend(bool show);
	bool isLegendOn(){return legendOn;};

	//axes
	QStringList axesLabels(){return labels;};
	void updateLabel(int axis,const QString& label, const QFont& f);
	void setAxesLabels(const QStringList& lst);
	void resetAxesLabels();

	QFont xAxisLabelFont();
	QFont yAxisLabelFont();
	QFont zAxisLabelFont();

	void setXAxisLabelFont(const QFont& fnt);
	void setYAxisLabelFont(const QFont& fnt);
	void setZAxisLabelFont(const QFont& fnt);
	
	void setXAxisLabelFont(const QStringList& lst);
	void setYAxisLabelFont(const QStringList& lst);
	void setZAxisLabelFont(const QStringList& lst);

	QFont numbersFont();
	void setNumbersFont(const QFont& font);
	void setNumbersFont(const QStringList& lst);

	double xStart();
	double xStop();
	double yStart();
	double yStop();
	double zStart();
	double zStop();
	QStringList scaleLimits();
	void updateScale(int axis,const QStringList& options);
	void updateScales(double xl, double xr, double yl, double yr, double zl, double zr);
	void updateScales(double xl, double xr, double yl, double yr, 
				  		double zl, double zr, int xcol, int ycol);
	void updateScales(double xl, double xr, double yl, double yr, 
				  		double zl, double zr, int xCol, int yCol, int zCol);
	void updateScalesFromMatrix(double xl,double xr,double yl,double yr,double zl,double zr);

	QStringList scaleTicks();
	void setTicks(const QStringList& options);
	
	void updateTickLength(int, double majorLength, double minorLength);
	void adjustLabels(int val);
	int labelsDistance(){return labelsDist;};

	QStringList axisTickLengths();
	void setTickLengths(const QStringList& lst);

	void setOptions(bool legend, int r, int dist);
	void setOptions(const QStringList& lst);
	void update();

	Qwt3D::Triple** allocateData(int columns, int rows);
	void deleteData(Qwt3D::Triple **data, int columns);

	// bars
	double barsRadius();
	void setBarsRadius(double rad);
	void updateBars(double rad);
	
	// scatter plots
	double pointsSize(){return pointSize;};
	bool smoothPoints(){return smooth;};
	void updatePoints(double size, bool sm);
	
	bool smoothCrossHair(){return crossHairSmooth;};
	bool boxedCrossHair(){return crossHairBoxed;};
	double crossHairRadius(){return crossHairRad;};
	double crossHairLinewidth(){return crossHairLineWidth;};
	void updateCross(double rad, double linewidth, bool smooth, bool boxed);
	void setCrossOptions(double rad, double linewidth, bool smooth, bool boxed);
	void setCrossMesh();
	
	double coneRadius(){return conesRad;};
	int coneQuality(){return conesQuality;};
	void updateCones(double rad, int quality);
	void setConesOptions(double rad, int quality);
	void setConesMesh();
	
	PointStyle pointType(){return pointStyle;};
	void setPointOptions(double size, bool s);
	
	Table* getTable(){return worksheet;};
	void showWorksheet();
	void setPlotAssociation(const QString& s){plotAssociation = s;};
	void setSmoothMesh(bool smooth);

	//! Used for the animation: rotates the scene with 1/360 degrees
	void rotate();
	void animate(bool on = true);
	bool isAnimated(){return d_timer->isActive();};

	void findBestLayout();

signals:   
	void showContextMenu();
	void showOptionsDialog();
	void modified();
	void custom3DActions(QWidget*);
	
private:
	// Wait this many msecs before redraw 3D plot (used for animations)
	int animation_redraw_wait; 

	//! File name of the color map used for the data (if any)
	QString color_map;

	QTimer *d_timer;
	QString title, plotAssociation;
	QStringList labels;
	QFont titleFnt;
	bool legendOn, smoothMesh;
	QMemArray<int> scaleType;
	QColor axesCol,labelsCol,titleCol,meshCol,bgCol,numCol,gridCol;
	QColor fromColor, toColor;//custom data colors
	int labelsDist, legendMajorTicks;
	bool ignoreFonts; 
	Qwt3D::StandardColor* col_;
	double barsRad, alpha, pointSize, crossHairRad, crossHairLineWidth, conesRad;
	bool smooth; // draw 3D points with smoothed angles
	bool crossHairSmooth, crossHairBoxed;
    int conesQuality;
	PointStyle pointStyle;
	Table *worksheet;
	Matrix *matrix_;
	Qwt3D::PLOTSTYLE style_;
};

class UserFunction : public Function
{
public:

    UserFunction(const QString& s, SurfacePlot& pw);
	~UserFunction();
    double operator()(double x, double y);
	QString function(){return formula;};

private:
	  QString formula;
};

#endif // Plot3D_H
