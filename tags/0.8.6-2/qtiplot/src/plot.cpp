#include "plot.h"
#include "scales.h"

#include <qwt_plot.h>
#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_map.h>

#include <qapplication.h>
#include <qpixmap.h>
#include <qmessagebox.h>

#include "graph.h"

Plot::Plot(QWidget *parent, const char *name)
		: QwtPlot(parent)
{
marker_key = 0;
curve_key = 0;

minTickLength = 5;
majTickLength = 9;

movedGraph=FALSE;
graphToResize=FALSE;
ShiftButton=FALSE;

setGeometry(QRect(0,0,500,400));
setAxisTitle(QwtPlot::yLeft, tr("Y Axis Title"));
setAxisTitle(QwtPlot::xBottom, tr("X Axis Title"));	

// grid 
d_grid = new Grid;
d_grid->enableX(false);
d_grid->enableY(false);
d_grid->setMajPen(QPen(Qt::blue, 0, Qt::SolidLine));
d_grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
d_grid->attach(this);

//custom scale
for (int i= 0; i<QwtPlot::axisCnt; i++)
	{
	QwtScaleWidget *scale = (QwtScaleWidget *) axisWidget(i);
	if (scale)
		{
		scale->setMargin(0);

		//the axis title color must be initialized
		QwtText title = scale->title();
		title.setColor(Qt::black);
		scale->setTitle(title);

		ScaleDraw *sd = new ScaleDraw();
		sd->setTickLength  	(QwtScaleDiv::MinorTick, minTickLength); 
		sd->setTickLength  	(QwtScaleDiv::MediumTick, minTickLength);
		sd->setTickLength  	(QwtScaleDiv::MajorTick, majTickLength);

		setAxisScaleDraw (i, sd);
		}
	}
	
QwtPlotLayout *pLayout=plotLayout();
pLayout->setCanvasMargin(0);

QwtPlotCanvas* plCanvas = canvas();
plCanvas->setFocusPolicy(QWidget::StrongFocus);
plCanvas->setFocusIndicator(QwtPlotCanvas::ItemFocusIndicator);
plCanvas->setFocus();
plCanvas->setFrameShadow(QwtPlot::Plain);
plCanvas->setCursor(Qt::arrowCursor);
plCanvas->setLineWidth(0);

setFocusPolicy(QWidget::StrongFocus);
setFocusProxy(plCanvas);
setFrameShape (QFrame::Box);
setLineWidth(0);
}

QColor Plot::frameColor()
{
return palette().color(QPalette::Active, QColorGroup::Foreground);
}

void Plot::printFrame(QPainter *painter, const QRect &rect) const
{
painter->save();

int lw = lineWidth();
if (lw)
	{
	QColor color = palette().color(QPalette::Active, QColorGroup::Foreground);
	painter->setPen (QPen(color, lw, Qt::SolidLine));
	}
else
	painter->setPen(QPen(Qt::NoPen));

if (paletteBackgroundColor() != Qt::white)
	painter->setBrush(paletteBackgroundColor());
							
QwtPainter::drawRect(painter, rect.x(), rect.y(), rect.width(), rect.height());
painter->restore();
}

void Plot::printCanvas(QPainter *painter, const QRect &canvasRect,
    const QwtArray<QwtScaleMap> &map, const QwtPlotPrintFilter &pfilter) const
{
	const QwtPlotCanvas* plotCanvas=canvas();	
	QRect rect=canvasRect;
	int w=plotCanvas->lineWidth();

	if (w>0)
    	{
		QPalette pal = plotCanvas->palette();
		QColor color=pal.color(QPalette::Active, QColorGroup::Foreground);
		
		painter->save();
		painter->setPen (QPen(color,w,Qt::SolidLine));

		if (canvasBackground() != Qt::white)
			painter->setBrush(canvasBackground());
				
		//if (w == 1 && majorTicksType[QwtPlot::xBottom] == Plot::Out)
			rect.setHeight(canvasRect.height() + 1);	
						
		QwtPainter::drawRect(painter, rect.x(), rect.y(), rect.width(), rect.height());
		painter->restore();
   		}
	
	painter->setClipping(TRUE);
	rect = QRect(canvasRect.x()+1, canvasRect.y()+1, canvasRect.width(), canvasRect.height()-1);
	QwtPainter::setClipRect(painter, rect);

    drawItems(painter, canvasRect, map, pfilter);
}

void Plot::drawItems (QPainter *painter, const QRect &rect, 
							const QwtArray< QwtScaleMap > &map, const QwtPlotPrintFilter &pfilter) const
{
QwtPlot::drawItems(painter, rect, map, pfilter);
	
for (int i=0; i<QwtPlot::axisCnt; i++)
 	{
	if (!axisEnabled(i))
		continue;

	ScaleDraw *sd = (ScaleDraw *) axisScaleDraw (i);
	int majorTicksType = sd->majorTicksStyle();
	int minorTicksType = sd->minorTicksStyle();

	bool min = (minorTicksType == ScaleDraw::In || minorTicksType == ScaleDraw::Both);
	bool maj = (majorTicksType == ScaleDraw::In || majorTicksType == ScaleDraw::Both);

	if (min || maj)
		drawInwardTicks(painter, rect, map[i], i, min, maj);
 	}
}

void Plot::drawInwardTicks(QPainter *painter, const QRect &rect, 
							const QwtScaleMap &map, int axis, bool min, bool maj) const
{	
	int x1=rect.left();
	int x2=rect.right();
	int y1=rect.top();
	int y2=rect.bottom();
	
	QPalette pal=axisWidget(axis)->palette();
	QColor color=pal.color(QPalette::Active, QColorGroup::Foreground);
		
    painter->save();	
    painter->setPen(QPen(color, axesLinewidth(), QPainter::SolidLine));
		
	QwtScaleDiv *scDiv=(QwtScaleDiv *)axisScaleDiv(axis);
	const QwtValueList minTickList = scDiv->ticks(QwtScaleDiv::MinorTick);
	int minTicks = (int)minTickList.count();

	const QwtValueList medTickList = scDiv->ticks(QwtScaleDiv::MediumTick);
	int medTicks = (int)medTickList.count();

	const QwtValueList majTickList = scDiv->ticks(QwtScaleDiv::MajorTick);
	int majTicks = (int)majTickList.count();
	
int j, x, y, low,high;
switch (axis)
	{
	case QwtPlot::yLeft:
	x=x1;
	low=y1+majTickLength;
	high=y2-majTickLength;
	if (min)
	{
    for (j = 0; j < minTicks; j++)
        {
            y = map.transform(minTickList[j]);
			if (y>low && y< high)
            	QwtPainter::drawLine(painter, x, y, x+minTickLength, y);
        }
	for (j = 0; j < medTicks; j++)
        {
            y = map.transform(medTickList[j]);
			if (y>low && y< high)
            	QwtPainter::drawLine(painter, x, y, x+minTickLength, y);
        }
	}

	if (maj)
	{
	for (j = 0; j < majTicks; j++)
        {
            y = map.transform(majTickList[j]);
			if (y>low && y< high)
            	QwtPainter::drawLine(painter, x, y, x+majTickLength, y);
        }
	}
	break;
		
	case QwtPlot::yRight:
		{
		x=x2;
		low=y1+majTickLength;
		high=y2-majTickLength;
		if (min)
		{
     	for (j = 0; j < minTicks; j++)
       	 	{
            y = map.transform(minTickList[j]);
			if (y>low && y< high)
            	QwtPainter::drawLine(painter, x+1, y, x-minTickLength, y);
        	}
		for (j = 0; j < medTicks; j++)
       	 	{
            y = map.transform(medTickList[j]);
			if (y>low && y< high)
            	QwtPainter::drawLine(painter, x+1, y, x-minTickLength, y);
        	}
		}

		if (maj)
		{
		 for (j = 0; j <majTicks; j++)
        	{
            y = map.transform(majTickList[j]);
			if (y>low && y< high)
            	QwtPainter::drawLine(painter, x+1, y, x-majTickLength, y);
        	}
		}
		}
	  break;
		
	case QwtPlot::xBottom:
		y=y2;
		low=x1+majTickLength;
		high=x2-majTickLength;
		if (min)
		{
     	for (j = 0; j < minTicks; j++)
        	{
            x = map.transform(minTickList[j]);
			if (x>low && x<high)
            	QwtPainter::drawLine(painter, x, y+1, x, y-minTickLength);
       		 }
		for (j = 0; j < medTicks; j++)
        	{
            x = map.transform(medTickList[j]);
			if (x>low && x<high)
            	QwtPainter::drawLine(painter, x, y+1, x, y-minTickLength);
       		 }
		}

		if (maj)
		{
	 	for (j = 0; j < majTicks; j++)
        	{
            x = map.transform(majTickList[j]);
			if (x>low && x<high)
            	QwtPainter::drawLine(painter, x, y+1, x, y-majTickLength);
        	}
		}
		break;
	
	case QwtPlot::xTop:
		y=y1;
		low=x1+majTickLength;
		high=x2-majTickLength;

		if (min)
		{
    	for (j = 0; j < minTicks; j++)
       		{
             x = map.transform(minTickList[j]);
			if (x>low && x<high)
            	QwtPainter::drawLine(painter, x, y, x, y + minTickLength);
       	    }
		for (j = 0; j < medTicks; j++)
       		{
             x = map.transform(medTickList[j]);
			if (x>low && x<high)
            	QwtPainter::drawLine(painter, x, y, x, y + minTickLength);
       	    }
		}

		if (maj)
		{
	 	for (j = 0; j <majTicks; j++)
        	{
            x = map.transform(majTickList[j]);
			if (x>low && x<high)
            	QwtPainter::drawLine(painter, x, y, x, y + majTickLength);
        	}
		}
	break;
	}
painter->restore();
}

void Plot::setAxesLinewidth(int width)
{
for (int i=0; i<QwtPlot::axisCnt; i++)
	{
	QwtScaleWidget *scale=(QwtScaleWidget*) this->axisWidget(i);
	if (scale)
		{
		scale->setPenWidth(width);
		scale->repaint();
		}
	}
}

int Plot::axesLinewidth() const
{
for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
	{
	const QwtScaleWidget *scale = this->axisWidget(axis);
    if (scale)
		return scale->penWidth();
	}
return 0;
}

int Plot::minorTickLength() const
{
return minTickLength;
}

int Plot::majorTickLength() const
{
return majTickLength;
}

void Plot::setTickLength (int minLength, int majLength)
{
if (majTickLength == majLength &&
	minTickLength == minLength)
	return;

majTickLength = majLength;
minTickLength = minLength;
}

void Plot::mousePressEvent ( QMouseEvent * e )
{
if(e->state()==Qt::ShiftButton)
		ShiftButton=TRUE;
	
presspos = e->pos();
emit selectPlot();
}

void Plot::mouseMoveEvent ( QMouseEvent * e )
{
if(ShiftButton)
	{
	graphToResize=TRUE;
	emit resizeGraph(e->pos());	
	}				
/*else if ((presspos - e->pos()).manhattanLength() > QApplication::startDragDistance())
	{
	if (!((Graph *)parent())->zoomOn())
		{
		movedGraph=TRUE;
		emit moveGraph(e->pos());
		}
	}*/
}

void Plot::mouseReleaseEvent ( QMouseEvent *)
{
if (movedGraph)
	{
	emit releasedGraph();
	movedGraph=FALSE;
	}

if (graphToResize)
	{
	emit resizedGraph();
	graphToResize=FALSE;
	ShiftButton=FALSE;
	}
}

void Plot::print(QPainter *painter, const QRect &plotRect,
        const QwtPlotPrintFilter &pfilter) const
{
printFrame(painter, plotRect);
QwtPlot::print(painter, plotRect, pfilter);
}

int Plot::closestCurve(int xpos, int ypos, int &dist, int &point)
{
QwtScaleMap map[QwtPlot::axisCnt];
for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
	map[axis] = canvasMap(axis);

double dmin = 1.0e10;
int key = -1;
for (QMapIterator<int, QwtPlotCurve *> it = d_curves.begin(); it != d_curves.end(); ++it ) 
	{
	QwtPlotCurve *c = (QwtPlotCurve *)it.data();
	if (!c)
		continue;

	for (int i=0; i<c->dataSize(); i++)
		{
		double cx = map[c->xAxis()].xTransform(c->x(i)) - double(xpos);
		double cy = map[c->yAxis()].xTransform(c->y(i)) - double(ypos);
		double f = qwtSqr(cx) + qwtSqr(cy);
		if (f < dmin)
			{
			dmin = f;
			key = it.key();
			point = i;
			}
		 }
	}
dist = int(sqrt(dmin));
return key;
}

void Plot::removeMarker(int index)
{
QwtPlotMarker *m = d_markers[index];
m->detach();
d_markers.remove (index);
}

int Plot::insertMarker(QwtPlotMarker *m)
{
marker_key++;
d_markers.insert (marker_key, m, false );
m->attach(((QwtPlot *)this));
return marker_key;
}

int Plot::insertCurve(QwtPlotCurve *c)
{
curve_key++;
d_curves.insert (curve_key, c, false);
c->attach(this);
return curve_key;
}

void Plot::removeCurve(int index)
{
QwtPlotCurve *c = d_curves[index];
c->detach();
d_curves.remove (index);
}

QValueList <int> Plot::getMajorTicksType()
{
QValueList <int> majorTicksType;
for (int axis=0; axis<QwtPlot::axisCnt; axis++)
	{
	if (axisEnabled(axis))
		{
		ScaleDraw *sd = (ScaleDraw *) axisScaleDraw (axis);
		majorTicksType << sd->majorTicksStyle();
		}
	else
		majorTicksType << ScaleDraw::Out;
	}
return majorTicksType;
}

void Plot::setMajorTicksType(int axis, int type)
{
ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (axis);
if (sd)
	sd->setMajorTicksStyle ((ScaleDraw::TicksStyle)type);
}

QValueList <int> Plot::getMinorTicksType()
{
QValueList <int> minorTicksType;
for (int axis=0; axis<QwtPlot::axisCnt; axis++)
	{
	if (axisEnabled(axis))
		{
		ScaleDraw *sd = (ScaleDraw *) axisScaleDraw (axis);
		minorTicksType << sd->minorTicksStyle();
		}
	else
		minorTicksType << ScaleDraw::Out;
	}
return minorTicksType;
}

void Plot::setMinorTicksType(int axis, int type)
{
ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (axis);
if (sd)
	sd->setMinorTicksStyle((ScaleDraw::TicksStyle)type);
}

int Plot::axisLabelFormat(int axis)
{
if (axisValid(axis))
	{
	int prec;
	char format;

	ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (axis);
    sd->labelFormat(format, prec);

	if (format == 'g')
		return Automatic;
	else if (format == 'e')
		return Scientific;
	else if (format == 'f')
		return Decimal;
	else
		return Superscripts;
	}

return 0; 
}

int Plot::axisLabelPrecision(int axis)
{
if (axisValid(axis))
	{
	ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (axis);
    return sd->labelNumericPrecision();
	}

//for a bad call we return the default values
return 4;
}

/*!
  \return the number format for the major scale labels of a specified axis
  \param axis axis index
  \retval f format character
  \retval prec precision
*/
void Plot::axisLabelFormat(int axis, char &f, int &prec) const
{
if (axisValid(axis))
	{
	ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (axis);
    sd->labelFormat(f, prec);
	}
else
    {
    //for a bad call we return the default values
    f = 'g'; 
    prec = 4;
    }
}

/*!
  Change the number format for the major scale of a selected axis
  \param axis axis index
  \param f format
  \param prec precision
*/
void Plot::setAxisLabelFormat(int axis, char f, int prec)
{
    if (axisValid(axis))
		{
		ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (axis);
        sd->setLabelFormat(f, prec);
		}
}

QwtDoubleRect Plot::boundingRect ()
{
QMapIterator<int, QwtPlotCurve *> it = d_curves.begin();
QwtPlotCurve *c = (QwtPlotCurve *)it.data();

double minX = c->minXValue();
double minY = c->minYValue();
double maxX = c->maxXValue();
double maxY = c->maxYValue();

it++;

for (it; it != d_curves.end(); ++it) 
	{
	QwtPlotCurve *c = (QwtPlotCurve *)it.data();
	if (!c)
		continue;

	minX = (c->minXValue() < minX) ? c->minXValue() : minX;
	maxX = (c->maxXValue() > maxX) ? c->maxXValue() : maxX;
	minY = (c->minYValue() < minY) ? c->minYValue() : minY;
	maxY = (c->maxYValue() > maxY) ? c->maxYValue() : maxY;
	}

QwtDoubleRect r;
r.setLeft(minX);
r.setRight(maxX);
r.setTop(minY);
r.setBottom(maxY);
return r;
}

/*!
  \brief Paint a scale into a given rectangle.
  Paint the scale into a given rectangle. Reimplemented from Qwt5

  \param painter Painter
  \param axisId Axis
  \param startDist Start border distance
  \param endDist End border distance
  \param baseDist Base distance
  \param rect Bounding rectangle
*/

void Plot::printScale(QPainter *painter, int axisId, int startDist, int endDist, 
					  int baseDist, const QRect &rect) const
{
    if (!axisEnabled(axisId))
        return;

    const QwtScaleWidget *scaleWidget = axisWidget(axisId);
    if ( scaleWidget->isColorBarEnabled() 
        && scaleWidget->colorBarWidth() > 0)
    {
        const QwtMetricsMap map = QwtPainter::metricsMap();

        const QRect r = map.layoutToScreen(rect);
        scaleWidget->drawColorBar(painter, scaleWidget->colorBarRect(r));

        const int off = scaleWidget->colorBarWidth() + scaleWidget->spacing();
        if ( scaleWidget->scaleDraw()->orientation() == Qt::Horizontal )
            baseDist += map.screenToLayoutY(off);
        else
            baseDist += map.screenToLayoutX(off);
    }

    QwtScaleDraw::Alignment align;
    int x, y, w;

	const int bw2 = scaleWidget->penWidth() / 2;

    switch(axisId)
    {
        case yLeft:
        {
            x = rect.right() - baseDist + 1 - bw2;
            y = rect.y() + startDist;
            w = rect.height() - startDist - endDist;
            align = QwtScaleDraw::LeftScale;
            break;
        }
        case yRight:
        {
            x = rect.left() + baseDist;
			if (scaleWidget->penWidth() % 2 == 0)
				x += 1;

            y = rect.y() + startDist;
            w = rect.height() - startDist - endDist + bw2;
            align = QwtScaleDraw::RightScale;
            break;
        }
        case xTop:
        {
            x = rect.left() + startDist;
            y = rect.bottom() - baseDist + 1;
			if (scaleWidget->penWidth() % 2 == 0)
				y -= bw2;
            w = rect.width() - startDist - endDist;
            align = QwtScaleDraw::TopScale;
            break;
        }
        case xBottom:
        {
            x = rect.left() + startDist;
            y = rect.top() + baseDist;

			if (scaleWidget->penWidth() % 2 == 1)
				y += bw2;

            w = rect.width() - startDist - endDist + bw2;
            align = QwtScaleDraw::BottomScale;
            break;
        }
        default:
            return;
    }

    scaleWidget->drawTitle(painter, align, rect);

    painter->save();
    painter->setFont(scaleWidget->font());

    QPen pen = painter->pen();
    pen.setWidth(scaleWidget->penWidth());
    painter->setPen(pen);

    QwtScaleDraw *sd = (QwtScaleDraw *)scaleWidget->scaleDraw();
    const QPoint sdPos = sd->pos();
    const int sdLength = sd->length();

    sd->move(x, y);
    sd->setLength(w);

#if QT_VERSION < 0x040000
    sd->draw(painter, scaleWidget->palette().active());
#else
    QPalette palette = scaleWidget->palette();
    palette.setCurrentColorGroup(QPalette::Active);
    sd->draw(painter, palette);
#endif
    // reset previous values
    sd->move(sdPos); 
    sd->setLength(sdLength); 

    painter->restore();
}

/*!
  \brief Draw the grid
  
  The grid is drawn into the bounding rectangle such that 
  gridlines begin and end at the rectangle's borders. The X and Y
  maps are used to map the scale divisions into the drawing region
  screen.
  \param painter  Painter
  \param mx X axis map
  \param my Y axis 
  \param r Contents rect of the plot canvas
*/
void Grid::draw(QPainter *painter, 
    const QwtScaleMap &mx, const QwtScaleMap &my,
    const QRect &r) const
{
    //  draw minor gridlines
    painter->setPen(minPen ());
    
    if (xEnabled() && xMinEnabled())
    {
        drawLines(painter, r, Qt::Vertical, mx, 
				xScaleDiv().ticks(QwtScaleDiv::MinorTick));
        drawLines(painter, r, Qt::Vertical, mx, 
				xScaleDiv().ticks(QwtScaleDiv::MediumTick));
    }

    if (yEnabled() && yMinEnabled())
    {
        drawLines(painter, r, Qt::Horizontal, my, 
				yScaleDiv().ticks(QwtScaleDiv::MinorTick));
        drawLines(painter, r, Qt::Horizontal, my, 
				yScaleDiv().ticks(QwtScaleDiv::MediumTick));
    }

    //  draw major gridlines
    painter->setPen(majPen());
    
    if (xEnabled())
    {
        drawLines(painter, r, Qt::Vertical, mx, 
			xScaleDiv().ticks (QwtScaleDiv::MajorTick));
    }

    if (yEnabled())
    {
        drawLines(painter, r, Qt::Horizontal, my, 
			yScaleDiv().ticks (QwtScaleDiv::MajorTick));
    }
}

void Grid::drawLines(QPainter *painter, const QRect &rect,
    Qt::Orientation orientation, const QwtScaleMap &map, 
    const QwtValueList &values) const
{
    const int x1 = rect.left();
    const int x2 = rect.right() + 1;
    const int y1 = rect.top();
    const int y2 = rect.bottom() + 1;
	const int margin = 10;

    for (uint i = 0; i < (uint)values.count(); i++)
    {
        const int value = map.transform(values[i]);
        if ( orientation == Qt::Horizontal )
        {
            if ((value >= y1 + margin) && (value <= y2 - margin))
                QwtPainter::drawLine(painter, x1, value, x2, value);
        }
        else
        {
            if ((value >= x1 + margin) && (value <= x2 - margin))
                QwtPainter::drawLine(painter, value, y1, value, y2);
        }
    }
}