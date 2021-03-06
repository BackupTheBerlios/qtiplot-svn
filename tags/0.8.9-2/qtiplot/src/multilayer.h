#ifndef MULTILAYER_H
#define MULTILAYER_H

#include "widget.h"
#include "graph.h"

#include <qwidget.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qhbox.h>
#include <qptrlist.h>
#include <qprinter.h>
#include <qvaluevector.h>

#include <gsl/gsl_vector.h>

class QWidget;
class QLabel;
class QPushButton;
class QWidget;
class Graph;
class Table;
class LayerButton;
	
class MultiLayer: public myWidget
{
	Q_OBJECT

public:
    MultiLayer (const QString& label, QWidget* parent=0, const char* name=0, WFlags f=0);
	QWidgetList* graphPtrs(){return graphsList;};
	Graph *layer(int num);
	LayerButton* addLayerButton();	

	enum HorAlignement{HCenter, Left, Right};
	enum VertAlignement{VCenter, Top, Bottom};

	//event handlers
	void mousePressEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void contextMenuEvent(QContextMenuEvent *);
	void wheelEvent(QWheelEvent *);
	void keyPressEvent(QKeyEvent *);
	bool eventFilter(QObject *object, QEvent *);
	void releaseLayer();
	
	QWidgetList *buttonsList, *graphsList;
	QHBox  *hbox1;
	QWidget *canvas;

public slots:
	void resizeLayers (const QResizeEvent *re);

	Graph* insertFirstLayer();
	Graph* addLayer();
	Graph* addLayerToOrigin();
	Graph* addLayer(int x, int y, int width, int height);
	void setLayersNumber(int n);

	bool isEmpty();
    void removeLayer();
	void confirmRemoveLayer();

	void addTextLayer(int f, const QFont& font, const QColor& textCol, const QColor& backgroundCol);
	void addTextLayer(const QPoint& pos);

	Graph* activeGraph(){return active_graph;};
	void setActiveGraph(Graph* g);
	void activateGraph(LayerButton* button);
	
	void moveGraph(Graph* g, const QPoint& pos);
	void releaseGraph(Graph* g);
	
	void setGraphOrigin(const QPoint& pos);
	void setGraphGeometry(int x, int y, int w, int h);

	void findBestLayout(int &rows, int &cols);
	
	QSize arrangeLayers(bool userSize);
	void arrangeLayers(bool fit, bool userSize);
	
	QSize maxSize();
	
	int getRows(){return rows;};
	void setRows(int r);
	
	int getCols(){return cols;};
	void setCols(int c);
	
	int colsSpacing(){return colsSpace;};
	int rowsSpacing(){return rowsSpace;};
	void setSpacing (int rgap, int cgap);

	int leftMargin(){return left_margin;};
	int rightMargin(){return right_margin;};
	int topMargin(){return top_margin;};
	int bottomMargin(){return bottom_margin;};
	void setMargins (int lm, int rm, int tm, int bm);

	QSize layerCanvasSize(){return QSize(l_canvas_width, l_canvas_height);};
	void setLayerCanvasSize (int w, int h);

	int horizontalAlignement(){return hor_align;};
	int verticalAlignement(){return vert_align;};
	void setAlignement (int ha, int va);

	int layers(){return graphs;};
	
	// print and export
	QPixmap canvasPixmap();

	void exportImage(const QString& fileName,const QString& fileType, int quality, bool transparent);
	void exportToSVG(const QString& fname);
	void exportToEPS(const QString& fname);
	void exportToEPS(const QString& fname, int res, QPrinter::Orientation o, 
					QPrinter::PageSize pageSize, QPrinter::ColorMode col);

	void copyAllLayers();
	void print();
	void printAllLayers(QPainter *painter);
	void printActiveLayer();
	
	void setFonts(const QFont& titleFnt, const QFont& scaleFnt,
							const QFont& numbersFnt, const QFont& legendFnt);
	void makeTransparentLayer(Graph *g);
	void updateLayerTransparency(Graph *g);
	void updateTransparency();
	void connectLayer(Graph *g);
	bool overlapsLayers(Graph *g);
	bool hasOverlapingLayers();
	bool allLayersTransparent();

	void highlightLayer(Graph*g);
	void drawLayerFocusRect(const QRect& fr);
	void showLayers(bool ok);

	QString saveToString(const QString& geometry);
	QString saveAsTemplate(const QString& geometryInfo);

	int layerButtonHeight();
	void ignoreResizeEvent(bool ignore){ignore_resize = ignore;};

signals:   
	void showTextDialog();
	void showPlotDialog(int);
	void showAxisDialog(int);
	void showScaleDialog(int);
	void showGraphContextMenu();
	void showCurveContextMenu(int);
	void showWindowContextMenu();
	void showCurvesDialog();
	void drawTextOff();
	void drawLineEnded(bool);
	void showXAxisTitleDialog();
	void showYAxisTitleDialog();
	void showTopAxisTitleDialog();
	void showRightAxisTitleDialog();
	void showMarkerPopupMenu();
	void modifiedPlot();
	void cursorInfo(const QString&);
	void showImageDialog();
	void showPieDialog();
	void showLineDialog();
	void viewTitleDialog();
	void createTablePlot(const QString&,int,int,const QString&);
	void createTable(const QString&,int,int,const QString&);
	void createHiddenTable(const QString&,int,int,const QString&);
	void createHistogramTable(const QString&,int,int,const QString&);
	void updateTable(const QString&,int,const QString&);
	void updateTableColumn(const QString&, double *, int);
	void clearCell(const QString&,double);	
	void showGeometryDialog();
	void pasteMarker();
	void createIntensityTable(const QPixmap&);
	void setPointerCursor();
	void resizeCanvas(const QResizeEvent *);
	
private:
	Graph* active_graph;
	QRect aux_rect;//used for resizing of layers
	QPixmap cache_pix;
	int graphs, cols, rows, graph_width, graph_height, colsSpace, rowsSpace;
	int left_margin, right_margin, top_margin, bottom_margin;
	int l_canvas_width, l_canvas_height, hor_align, vert_align;
	int xMouse, yMouse, xActiveGraph, yActiveGraph;//used for moving layers
	bool movedGraph, addTextOn, highlightedLayer, ignore_resize, mousePressed;

	//! Used when adding text markers on new layers
	int defaultTextMarkerFrame;
	QFont defaultTextMarkerFont;
	QColor defaultTextMarkerColor, defaultTextMarkerBackground;

};

	
class LayerButton: public QWidget
{
	Q_OBJECT

public:
    LayerButton (const QString& text = QString::null, QWidget* parent = 0, const char* name = 0);
	~LayerButton();

	QPushButton  *btn;

	bool eventFilter(QObject *object, QEvent *e);

public slots:
	 void setText(const QString& text);
	 void setOn(bool on);
	 bool isOn(){return btn->isOn();};

signals:
	void showLayerMenu();
	void showCurvesDialog();	
	void clicked(LayerButton*);
};

#endif
