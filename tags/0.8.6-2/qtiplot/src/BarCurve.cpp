#include "BarCurve.h"
#include <qpainter.h>

QwtBarCurve::QwtBarCurve(QwtPlot *parent, const char *name):
    QwtPlotCurve(name)
{
bar_offset=0;
bar_gap=0;
bar_style=Vertical;

setPen(QPen(Qt::black,1,Qt::SolidLine));
setBrush(QBrush(Qt::red));
}

QwtBarCurve::QwtBarCurve(BarStyle style, QwtPlot *parent, const char *name):
    QwtPlotCurve(name)
{
bar_offset=0;
bar_gap=0;
bar_style=style;

setPen(QPen(Qt::black,1,Qt::SolidLine));
setBrush(QBrush(Qt::red));
}

void QwtBarCurve::copy(const QwtBarCurve *b)
{
bar_gap = b->bar_gap;
bar_offset = b->bar_offset;
bar_style = b->bar_style;
	
setTitle(b->title());
}

void QwtBarCurve::draw(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const
{
   if ( !painter || dataSize() <= 0 )
        return;

    if (to < 0)
        to = dataSize() - 1;

    if ( verifyRange(from, to) > 0 )
		{
        painter->save();
        painter->setPen(QwtPlotCurve::pen());
        painter->setBrush(QwtPlotCurve::brush());
			
		int dx,dy,ref,bar_width;
			
		if (bar_style == Vertical)
			ref= yMap.transform(1e-100); //smalest positive value for log scales
		else
			ref= xMap.transform(1e-100);	
				
		int i;
		if (bar_style == Vertical)
			{
			dx = abs(xMap.transform(x(from+1))-xMap.transform(x(from)));
			for (i=from+2; i<to; i++)
				{
				int min = abs(xMap.transform(x(i+1))-xMap.transform(x(i)));
				if (min <= dx)
					dx=min;
				}
			bar_width=int(dx*(1-bar_gap*0.01));
			}
		else
			{
			dy = abs(yMap.transform(y(from+1))-yMap.transform(y(from)));
			for (i=from+2; i<to; i++)
				{
				int min = abs(yMap.transform(y(i+1))-yMap.transform(y(i)));
				if (min <= dy)
					dy=min;
				}
			bar_width=int(dy*(1-bar_gap*0.01));
			}
		
		const int half_width = int((0.5-bar_offset*0.01)*bar_width);
		const int bw1 = bar_width+1;
        for (i=from; i<=to; i++)
			{
            const int px = xMap.transform(x(i));
            const int py = yMap.transform(y(i));
			
			if (bar_style == Vertical)
				{
				if (y(i) < 0)
					painter->drawRect(px-half_width, ref, bw1, (py-ref));
				else
					painter->drawRect(px-half_width, py, bw1, (ref-py+1));	
				}
			else
				{
				if (x(i) < 0)
					painter->drawRect(px, py-half_width, (ref-px), bw1);
				else
					painter->drawRect(ref, py-half_width,(px-ref), bw1);
				}	
			}
		painter->restore();
		}
}

QwtDoubleRect QwtBarCurve::boundingRect() const
{
QwtDoubleRect rect = QwtPlotCurve::boundingRect();
double n= (double)dataSize();

if (bar_style == Vertical)
	{	
	double dx=(rect.right()-rect.left())/n;
	rect.setLeft(rect.left()-dx);
	rect.setRight(rect.right()+dx);
	}
else
	{	
	double dy=(rect.bottom()-rect.top())/n;
	rect.setTop(rect.top()-dy);
	rect.setBottom(rect.bottom()+dy);
	}
	
return rect;
}

void QwtBarCurve::setGap (int gap)   
{
if (bar_gap == gap)
	return;

bar_gap =gap;
}

void QwtBarCurve::setOffset(int offset)   
{
if (bar_offset == offset)
	return;

bar_offset = offset;
}

double QwtBarCurve::dataOffset()   
{
if (bar_style == Vertical)
	{
	const QwtScaleMap &xMap = plot()->canvasMap(xAxis());

	int dx = abs(xMap.transform(x(1))-xMap.transform(x(0)));
	for (int i = 2; i<dataSize(); i++)
		{
		int min = abs(xMap.transform(x(i+1))-xMap.transform(x(i)));
		if (min <= dx)
			dx=min;
		}
	int bar_width=int(dx*(1-bar_gap*0.01));
	int x1 = xMap.transform(minXValue()) + int(bar_offset*0.01*bar_width);	
	return xMap.invTransform(x1) - minXValue();
	}
else
	{
	const QwtScaleMap &yMap = plot()->canvasMap(yAxis());

	int dy = abs(yMap.transform(y(1))-yMap.transform(y(0)));
	for (int i=2; i<dataSize(); i++)
		{
		int min = abs(yMap.transform(y(i+1))-yMap.transform(y(i)));
		if (min <= dy)
			dy=min;
		}

	int bar_width = int(dy*(1-bar_gap*0.01));
	int y1 = yMap.transform(minYValue()) + int(bar_offset*0.01*bar_width);	
	return yMap.invTransform(y1) - minYValue();
	}
return 0;
}