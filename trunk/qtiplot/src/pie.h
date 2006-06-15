/***************************************************************************
    File                 : pie.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email                : ion_vasilief@yahoo.fr, thzs@gmx.net
    Description          : Pie plot class
                           
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
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

//! Pie plot class
class QwtPieCurve: public QwtPlotCurve
{
public:
	QwtPieCurve(QwtPlot *parent, const char *name=0);

	virtual void draw(QPainter *painter,const QwtScaleMap &xMap, 
		const QwtScaleMap &yMap, int from, int to);

	virtual void drawPie(QPainter *painter, const QwtScaleMap &xMap, 
		const QwtScaleMap &yMap, int from, int to);

public slots:
	QColor color(int i);

	int radius(){return pieRadius;};
	void setRadius(int size){pieRadius=size;};

	Qt::BrushStyle pattern(){return QwtPlotCurve::brush().style();};
	void setBrushStyle(const Qt::BrushStyle& style);

	void setFirstColor(int index){firstColor=index;};
	int first(){return firstColor;};
	
private:
	int pieRadius,firstColor;
};
