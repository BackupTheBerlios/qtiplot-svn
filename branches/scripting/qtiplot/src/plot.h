#ifndef PLOT_H
#define PLOT_H

#include <qobject.h>
#include <qmap.h>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>

class Grid;

class Plot: public QwtPlot
{	
    Q_OBJECT

public:	
	Plot(QWidget *parent = 0, const char *name = 0);
	
	enum LabelFormat{Automatic, Decimal, Scientific, Superscripts};
	
	QwtPlotGrid *grid(){return (QwtPlotGrid *)d_grid;};
	QValueList<int> curveKeys(){return d_curves.keys();};

	int insertCurve(QwtPlotCurve *c);
	void removeCurve(int index);

	int closestCurve(int xpos, int ypos, int &dist, int &point);
	QwtPlotCurve* curve(int index){return d_curves[index];};
	QMap<int, QwtPlotCurve*> curves(){return d_curves;};

	QwtDoubleRect boundingRect();

	QwtPlotMarker* marker(int index){return d_markers[index];};
	QValueList<int> markerKeys(){return d_markers.keys();};
	int insertMarker(QwtPlotMarker *m);
	void removeMarker(int index);

	QValueList <int> getMajorTicksType();
	void setMajorTicksType(int axis, int type);

	QValueList <int> getMinorTicksType();
	void setMinorTicksType(int axis, int type);

	int minorTickLength() const;
	int majorTickLength() const;
	void setTickLength (int minLength, int majLength);

	int axesLinewidth() const;
	void setAxesLinewidth(int width);

	void setAxisLabelFormat(int axis, char f, int prec);
    void axisLabelFormat(int axis, char &f, int &prec) const;

	int axisLabelFormat(int axis);
	int axisLabelPrecision(int axis);

	void printFrame(QPainter *painter, const QRect &rect) const;

	QColor frameColor();

	void mousePressEvent ( QMouseEvent * e );
	void mouseReleaseEvent ( QMouseEvent * e );
	void mouseMoveEvent ( QMouseEvent * e );

	virtual void print(QPainter *, const QRect &rect,
        const QwtPlotPrintFilter & = QwtPlotPrintFilter()) const;
	
protected:
	void printScale(QPainter *painter, int axisId, int startDist, int endDist, 
						  int baseDist, const QRect &rect) const;
	void printCanvas(QPainter *painter, const QRect &canvasRect,
   			 const QwtArray<QwtScaleMap> &map, const QwtPlotPrintFilter &pfilter) const;

	virtual void drawItems (QPainter *painter, const QRect &rect,
			const QwtArray< QwtScaleMap > &map, const QwtPlotPrintFilter &pfilter) const;

	void drawInwardTicks(QPainter *painter, const QRect &rect, 
							const QwtScaleMap&map, int axis, bool min, bool maj) const;

signals:
	void selectPlot();
	void moveGraph(const QPoint&);
	void releasedGraph();
	void resizeGraph(const QPoint&);
	void resizedGraph();

protected:
	Grid *d_grid;
	QMap<int, QwtPlotCurve*> d_curves;
	QMap<int, QwtPlotMarker*> d_markers;

	int minTickLength, majTickLength;
	bool movedGraph, ShiftButton, graphToResize;
	QPoint presspos;
	int marker_key;
	int curve_key;
};

class Grid : public QwtPlotGrid
{
public:
    Grid(){};

void draw (QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRect &rect) const;
void drawLines(QPainter *painter, const QRect &rect, Qt::Orientation orientation, const QwtScaleMap &map, 
    const QwtValueList &values) const;
};

#endif
