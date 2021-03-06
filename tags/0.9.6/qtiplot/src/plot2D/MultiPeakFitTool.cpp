/***************************************************************************
    File                 : MultiPeakFitTool.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : Plot tool for doing multi-peak fitting.

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
#include "MultiPeakFitTool.h"
#include "RangeSelectorTool.h"
#include "../ApplicationWindow.h"
#include "DataPickerTool.h"
#include "../cursors.h"

#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <QApplication>

#include <gsl/gsl_statistics.h>

MultiPeakFitTool::MultiPeakFitTool(Graph *graph, ApplicationWindow *app, MultiPeakFit::PeakProfile profile, int num_peaks, const QObject *status_target, const char *status_slot)
	: PlotToolInterface(graph),
	d_profile(profile),
	d_num_peaks(num_peaks)
{
	d_selected_peaks = 0;
	d_curve = 0;

	d_fit = new MultiPeakFit(app, d_graph, d_profile, d_num_peaks);
	d_fit->enablePeakCurves(app->generatePeakCurves);
	d_fit->setPeakCurvesColor(app->peakCurvesColor);
	d_fit->generateFunction(app->generateUniformFitPoints, app->fitPoints);

	if (status_target)
		connect(this, SIGNAL(statusText(const QString&)), status_target, status_slot);
	d_picker_tool = new DataPickerTool(d_graph, app, DataPickerTool::Display, this, SIGNAL(statusText(const QString&)));
    d_graph->canvas()->setCursor(QCursor(QPixmap(cursor_xpm), -1, -1));

	connect(d_picker_tool, SIGNAL(selected(QwtPlotCurve*,int)), this, SLOT(selectPeak(QwtPlotCurve*,int)));
	d_graph->canvas()->grabMouse();

	emit statusText(tr("Move cursor and click to select a point and double-click/press 'Enter' to set the position of a peak!"));
}

MultiPeakFitTool::~MultiPeakFitTool()
{
	if (d_picker_tool)
		delete d_picker_tool;
	if (d_fit)
		delete d_fit;
}

void MultiPeakFitTool::selectPeak(QwtPlotCurve *curve, int point_index)
{
	// TODO: warn user
	if (!curve || (d_curve && d_curve != curve))
		return;
	d_curve = curve;

	d_fit->setInitialGuess(3*d_selected_peaks, curve->y(point_index));
	d_fit->setInitialGuess(3*d_selected_peaks+1, curve->x(point_index));

	QwtPlotMarker *m = new QwtPlotMarker();
	m->setLineStyle(QwtPlotMarker::VLine);
	m->setLinePen(QPen(Qt::green, 2, Qt::DashLine));
	m->setXValue(curve->x(point_index));
	d_graph->insertMarker(m);
	d_lines.append(m);
	d_graph->replot();

	d_selected_peaks++;
	if (d_selected_peaks == d_num_peaks)
		finalize();
	else
		emit statusText(
				tr("Peak %1 selected! Click to select a point and double-click/press 'Enter' to set the position of the next peak!")
				.arg(QString::number(d_selected_peaks)));
}

void MultiPeakFitTool::finalize()
{
	delete d_picker_tool; d_picker_tool = NULL;
	d_graph->canvas()->releaseMouse();

	if (d_fit->setDataFromCurve(d_curve->title().text())){
		QApplication::setOverrideCursor(Qt::WaitCursor);

		double *y = d_fit->y();
		int n = d_fit->dataSize();

		size_t imin, imax;
		gsl_stats_minmax_index(&imin, &imax, y, 1, n);
		double temp[n];
		for (int i = 0; i < n; i++)
			temp[i] = fabs(y[i]);
		size_t imax_temp = gsl_stats_max_index(temp, 1, n);
        double offset = 0.0;
		if (imax_temp == imax)
			offset = y[imin];
		else
            offset = y[imax];
        d_fit->setInitialGuess(3*d_selected_peaks, offset);

		double w = 2*gsl_stats_sd(d_fit->x(), 1, n)/(double)d_selected_peaks;
		for (int i = 0; i < d_selected_peaks; i++){
		    int aux = 3*i;
			d_fit->setInitialGuess(aux + 2, w);
			double yc = d_fit->initialGuess(aux);
			if (d_profile == MultiPeakFit::Lorentz)
                d_fit->setInitialGuess(aux, (yc - offset)*M_PI_2*w);
            else
                d_fit->setInitialGuess(aux, (yc - offset)*sqrt(M_PI_2)*w);
		}

		d_fit->fit();
		delete d_fit; d_fit = NULL;
		QApplication::restoreOverrideCursor();
	}

	//remove peak line markers
	foreach(QwtPlotMarker *m, d_lines)
		m->detach();

	d_graph->replot();
    if (d_graph->activeTool() && d_graph->activeTool()->rtti() == PlotToolInterface::Rtti_RangeSelector){
        ((RangeSelectorTool *)d_graph->activeTool())->setEnabled();
    } else
        d_graph->canvas()->unsetCursor();
    delete this;
}
