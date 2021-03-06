/***************************************************************************
    File                 : QwtErrorPlotCurve.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Error bars curve

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "QwtErrorPlotCurve.h"
#include "QwtBarCurve.h"
#include "ScaleEngine.h"
#include "Graph.h"

#include <qwt_painter.h>
#include <qwt_symbol.h>

#include <QPainter>

QwtErrorPlotCurve::QwtErrorPlotCurve(int orientation, Table *t, const QString& name):
	DataCurve(t, QString(), name),
	d_master_curve(NULL)
{
	type = orientation;
	init();
}

QwtErrorPlotCurve::QwtErrorPlotCurve(Table *t, const QString& name):
	DataCurve(t, QString(), name),
	d_master_curve(NULL)
{
	type = Vertical;
	init();
}

void QwtErrorPlotCurve::init()
{
	plus = true;
	minus = true;
	through = false;
	setType(Graph::ErrorBars);
	setPlotStyle(Graph::ErrorBars);
    setStyle(QwtPlotCurve::UserCurve);
}

void QwtErrorPlotCurve::copy(const QwtErrorPlotCurve *e)
{
	d_cap_length = e->d_cap_length;
	type = e->type;
	plus = e->plus;
	minus = e->minus;
	through = e->through;
	setPen(e->pen());
	err = e->err;
}

void QwtErrorPlotCurve::draw(QPainter *painter,
		const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const
{
	if ( !painter || dataSize() <= 0 )
		return;

	if (to < 0)
		to = dataSize() - 1;

  painter->save();
  QPen p = QwtPainter::scaledPen(pen());
  p.setCapStyle(Qt::FlatCap);
  p.setJoinStyle(Qt::MiterJoin);
  painter->setPen(p);
  drawErrorBars(painter, xMap, yMap, from, to);
  painter->restore();
}

void QwtErrorPlotCurve::drawErrorBars(QPainter *painter,
		const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const
{
    int sh2 = 0, sw2 = 0;
    double x_factor = (double)painter->device()->logicalDpiX()/(double)plot()->logicalDpiX();
    double y_factor = (double)painter->device()->logicalDpiY()/(double)plot()->logicalDpiY();
    const QwtSymbol symbol = d_master_curve->symbol();
    if (symbol.style() != QwtSymbol::NoSymbol){
        sh2 = int(0.5*y_factor*symbol.size().height());
        sw2 = int(0.5*x_factor*symbol.size().width());
    }

	double d_xOffset = 0.0;
	double d_yOffset = 0.0;
	if (d_master_curve->type() == Graph::VerticalBars)
		d_xOffset = ((QwtBarCurve *)d_master_curve)->dataOffset();
	else if (d_master_curve->type() == Graph::HorizontalBars)
		d_yOffset = ((QwtBarCurve *)d_master_curve)->dataOffset();

	ScaleEngine *yScaleEngine = (ScaleEngine *)plot()->axisScaleEngine(yAxis());
	bool logYScale = (yScaleEngine->type() == ScaleTransformation::Log10) ? true : false;

	int skipPoints = d_master_curve->skipSymbolsCount() + d_skip_symbols;
	if (d_skip_symbols > 0)
		skipPoints--;
	if (skipPoints == 0)
		skipPoints = 1;

	for (int i = from; i <= to; i += skipPoints){
		const int xi = xMap.transform(x(i) + d_xOffset);
		const int yi = yMap.transform(y(i) + d_yOffset);

		double error = err[i];
		if (error == 0.0)
			continue;

		if (type == Vertical){
			const int yh = yMap.transform(y(i) + error);
			const int yl = yMap.transform(y(i) - error);
			const int yhl = yi - sh2;
			const int ylh = yi + sh2;
			const int cap2 = qRound(d_cap_length*0.5*x_factor);

			if (plus){
				QwtPainter::drawLine(painter, xi, yhl, xi, yh);
				QwtPainter::drawLine(painter, xi - cap2, yh, xi + cap2, yh);
			}
			if (minus && (!logYScale || (logYScale && yl > 0))){
				QwtPainter::drawLine(painter, xi, ylh, xi, yl);
				QwtPainter::drawLine(painter, xi - cap2, yl, xi + cap2, yl);
			}
			if (through && (plus || minus))
				QwtPainter::drawLine(painter, xi, yhl, xi, ylh);
		} else if (type == Horizontal) {
			const int xp = xMap.transform(x(i) + error);
			const int xm = xMap.transform(x(i) - error);
  			const int xpm = xi + sw2;
  	        const int xmp = xi - sw2;
			const int cap2 = qRound(d_cap_length*0.5*y_factor);

			if (plus){
				QwtPainter::drawLine(painter, xp, yi, xpm, yi);
				QwtPainter::drawLine(painter, xp, yi - cap2, xp, yi + cap2);
			}
			if (minus){
				QwtPainter::drawLine(painter, xm, yi, xmp, yi);
				QwtPainter::drawLine(painter, xm, yi - cap2, xm, yi + cap2);
			}
			if (through && (plus || minus))
				QwtPainter::drawLine(painter, xmp, yi, xpm, yi);
		}
	}
}

double QwtErrorPlotCurve::errorValue(int i)
{
	if (i >= 0 && i < dataSize())
		return err[i];
	else
		return 0.0;
}

bool QwtErrorPlotCurve::xErrors()
{
	bool x = false;
	if (type == Horizontal)
		x = true;

	return x;
}

void QwtErrorPlotCurve::setXErrors(bool yes)
{
	if (yes)
		type = Horizontal;
	else
		type = Vertical;
}

void QwtErrorPlotCurve::setWidth(double w)
{
	QPen p = pen();
 	p.setWidthF(w);
  	setPen(p);
}

void QwtErrorPlotCurve::setColor(const QColor& c)
{
	QPen p = pen();
  	p.setColor(c);
  	setPen(p);
}

QwtDoubleRect QwtErrorPlotCurve::boundingRect() const
{
	QwtDoubleRect rect = QwtPlotCurve::boundingRect();

	int size = dataSize();

	QwtArray <double> X(size), Y(size), min(size), max(size);
	for (int i=0; i<size; i++){
		X[i]=x(i);
		Y[i]=y(i);
		if (type == Vertical){
			min[i] = y(i) - err[i];
			max[i] = y(i) + err[i];
		} else {
			min[i] = x(i) - err[i];
			max[i] = x(i) + err[i];
		}
	}

	QwtArrayData *erMin, *erMax;
	if (type == Vertical) {
		erMin=new QwtArrayData(X, min);
		erMax=new QwtArrayData(X, max);
	} else {
		erMin=new QwtArrayData(min, Y);
		erMax=new QwtArrayData(max, Y);
	}

	QwtDoubleRect minrect = erMin->boundingRect();
	QwtDoubleRect maxrect = erMax->boundingRect();

	rect.setTop(QMIN(minrect.top(), maxrect.top()));
	rect.setBottom(QMAX(minrect.bottom(), maxrect.bottom()));
	rect.setLeft(QMIN(minrect.left(), maxrect.left()));
	rect.setRight(QMAX(minrect.right(), maxrect.right()));

	delete erMin;
	delete erMax;

	return rect;
}

void QwtErrorPlotCurve::setMasterCurve(DataCurve *c)
{
	if (!c || d_master_curve == c)
		return;

	d_master_curve = c;
	setAxis(c->xAxis(), c->yAxis());
	d_start_row = c->startRow();
	d_end_row = c->endRow();
	c->addErrorBars(this);

	loadData();
}

void QwtErrorPlotCurve::loadData()
{
	if (!d_master_curve)
		return;

    if (!plot())
        return;

	Table *mt = d_master_curve->table();
	if (!mt)
		return;

	int xcol = mt->colIndex(d_master_curve->xColumnName());
	int ycol = mt->colIndex(d_master_curve->title().text());
	int errcol = d_table->colIndex(title().text());
	if (xcol<0 || ycol<0 || errcol<0)
		return;

	d_start_row = d_master_curve->startRow();
	d_end_row = d_master_curve->endRow();
    int r = abs(d_end_row - d_start_row) + 1;
	QVector<double> X(r), Y(r), err(r);
    int data_size = 0;
    QLocale locale = d_table->locale();
	for (int i = d_start_row; i <= d_end_row; i++){
		QString xval = mt->text(i, xcol);
		QString yval = mt->text(i, ycol);
		QString errval = d_table->text(i, errcol);
		if (!xval.isEmpty() && !yval.isEmpty()){
			X[data_size] = d_master_curve->x(data_size);
			Y[data_size] = d_master_curve->y(data_size);

			if (!errval.isEmpty())
				err[data_size] = locale.toDouble(errval);
			else
				err[data_size] = 0.0;

			data_size++;
		}
	}

	if (!data_size)
		remove();

    X.resize(data_size);
	Y.resize(data_size);
	err.resize(data_size);

	setData(X.data(), Y.data(), data_size);
	setErrors(err);
}

QString QwtErrorPlotCurve::plotAssociation()
{
    if (!d_master_curve)
		return QString();

    QString base = d_master_curve->xColumnName() + "(X)," + d_master_curve->title().text() + "(Y)," + title().text();
    if (type == Horizontal)
        return base + "(xErr)";
    else
        return base + "(yErr)";
}

bool QwtErrorPlotCurve::updateData(Table *t, const QString& colName)
{
	if (d_table != t || colName != title().text())
		return false;

	loadData();
	return true;
}
