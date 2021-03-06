/***************************************************************************
    File                 : DataPickerTool.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : Plot tool for selecting points on curves.

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
#include "DataPickerTool.h"
#include "Graph.h"
#include <cursors.h>
#include "FunctionCurve.h"
#include "PlotCurve.h"
#include "QwtErrorPlotCurve.h"
#include <ApplicationWindow.h>
#include <MultiLayer.h>
#include <QClipboard>

#include <qwt_symbol.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_curve.h>
#include <QMessageBox>
#include <QLocale>
#include <QApplication>
#include <QTextStream>

DataPickerTool::DataPickerTool(Graph *graph, ApplicationWindow *app, Mode mode, const QObject *status_target, const char *status_slot) :
	QwtPlotPicker(graph->canvas()),
	PlotToolInterface(graph),
	d_app(app),
	d_mode(mode),
	d_move_mode(Free)
{
	d_selected_curve = NULL;

	//d_selection_marker.setSymbol(QwtSymbol(QwtSymbol::Ellipse, QBrush(QColor(255,255,0,128)), QPen(Qt::black,2), QSize(20,20)));
	d_selection_marker.setLineStyle(QwtPlotMarker::Cross);
	d_selection_marker.setLinePen(QPen(Qt::red,1));

	setTrackerMode(QwtPicker::AlwaysOn);
	if (d_mode == Move) {
		setSelectionFlags(QwtPicker::PointSelection | QwtPicker::DragSelection);
		d_graph->canvas()->setCursor(Qt::pointingHandCursor);
	} else {
		setSelectionFlags(QwtPicker::PointSelection | QwtPicker::ClickSelection);
		d_graph->canvas()->setCursor(QCursor(QPixmap(vizor_xpm), -1, -1));
	}

	if (status_target)
		connect(this, SIGNAL(statusText(const QString&)), status_target, status_slot);
	switch(d_mode) {
		case Display:
			emit statusText(tr("Click on plot or move cursor to display coordinates!"));
			break;
		case Move:
			emit statusText(tr("Please, click on plot and move cursor!"));
			break;
		case Remove:
			emit statusText(tr("Select point and double click to remove it!"));
			break;
	}
}

DataPickerTool::~DataPickerTool()
{
	d_selection_marker.detach();
	d_graph->canvas()->unsetCursor();
}

void DataPickerTool::append(const QPoint &pos)
{
	int dist, point_index;
	PlotCurve *c = d_graph->closestCurve(pos.x(), pos.y(), dist, point_index);
	if (!c || dist >= 5) { // 5 pixels tolerance
		setSelection(NULL, 0);
		return;
	}
	setSelection((QwtPlotCurve *)c, point_index);
	if (!d_selected_curve) return;

	QwtPlotPicker::append(transform(QwtDoublePoint(d_selected_curve->x(d_selected_point),
					d_selected_curve->y(d_selected_point))));
}

void DataPickerTool::setSelection(QwtPlotCurve *curve, int point_index)
{
	if (curve == d_selected_curve && point_index == d_selected_point)
		return;

	d_selected_curve = curve;
	d_selected_point = point_index;

	if (!d_selected_curve) {
		d_selection_marker.detach();
		d_graph->replot();
		return;
	}

	d_selection_marker.setAxis(d_selected_curve->xAxis(), d_selected_curve->yAxis());
	setAxis(d_selected_curve->xAxis(), d_selected_curve->yAxis());

    d_restricted_move_pos = QPoint(plot()->transform(xAxis(), d_selected_curve->x(d_selected_point)),
                                   plot()->transform(yAxis(), d_selected_curve->y(d_selected_point)));

    if (((PlotCurve *)d_selected_curve)->type() == Graph::Function) {
         QLocale locale = d_app->locale();
         emit statusText(QString("%1[%2]: x=%3; y=%4")
			.arg(d_selected_curve->title().text())
			.arg(d_selected_point + 1)
			.arg(locale.toString(d_selected_curve->x(d_selected_point), 'G', d_app->d_decimal_digits))
			.arg(locale.toString(d_selected_curve->y(d_selected_point), 'G', d_app->d_decimal_digits)));
    } else {
        int row = ((DataCurve*)d_selected_curve)->tableRow(d_selected_point);

        Table *t = ((DataCurve*)d_selected_curve)->table();
        int xCol = t->colIndex(((DataCurve*)d_selected_curve)->xColumnName());
        int yCol = t->colIndex(d_selected_curve->title().text());

        emit statusText(QString("%1[%2]: x=%3; y=%4")
			.arg(d_selected_curve->title().text())
			.arg(row + 1)
			.arg(t->text(row, xCol))
			.arg(t->text(row, yCol)));
    }

	QwtDoublePoint selected_point_value(d_selected_curve->x(d_selected_point), d_selected_curve->y(d_selected_point));
	d_selection_marker.setValue(selected_point_value);
	if (d_selection_marker.plot() == NULL)
		d_selection_marker.attach(d_graph);
	d_graph->replot();
}

bool DataPickerTool::eventFilter(QObject *obj, QEvent *event)
{
	switch(event->type()) {
		case QEvent::MouseButtonDblClick:
			switch(d_mode) {
				case Remove:
					removePoint();
					return true;
				default:
					if (d_selected_curve)
						emit selected(d_selected_curve, d_selected_point);
					return true;
			}
		break;

        case QEvent::MouseMove:
            if (((QMouseEvent *)event)->modifiers() == Qt::ControlModifier)
                d_move_mode = Vertical;
            else if (((QMouseEvent *)event)->modifiers() == Qt::AltModifier)
                d_move_mode = Horizontal;
            else
                d_move_mode = Free;
		break;

		case QEvent::KeyPress:
			if (keyEventFilter((QKeyEvent*)event))
				return true;
			break;
		default:
			break;
	}
	return QwtPlotPicker::eventFilter(obj, event);
}

bool DataPickerTool::keyEventFilter(QKeyEvent *ke)
{
	const int delta = 5;
	switch(ke->key()) {
		case Qt::Key_Escape:
			d_graph->setActiveTool(NULL);
			d_graph->multiLayer()->applicationWindow()->pickPointerCursor();
			return true;

		case Qt::Key_Enter:
		case Qt::Key_Return:
			emit selected(d_selected_curve, d_selected_point);
			return true;

        case Qt::Key_Home:
			setSelection(d_selected_curve, 0);
			return true;

		case Qt::Key_End:
			setSelection(d_selected_curve, d_selected_curve->dataSize() - 1);
			return true;

		case Qt::Key_Up:
			{
				int n_curves = d_graph->curveCount();
				int start = d_graph->curveIndex(d_selected_curve) + 1;
				for (int i = start; i < start + n_curves; ++i)
					if (d_graph->curve(i % n_curves)->dataSize() > 0) {
					    QwtPlotCurve *c = d_graph->curve(i % n_curves);
					    if (c)
                            setSelection(c, findClosestPoint(c, d_selected_curve->x(d_selected_point), true));
						break;
					}
				d_graph->replot();
				return true;
			}

		case Qt::Key_Down:
			{
				int n_curves = d_graph->curveCount();
				int start = d_graph->curveIndex(d_selected_curve) + n_curves - 1;
				for (int i = start; i > start - n_curves; --i)
					if (d_graph->curve(i % n_curves)->dataSize() > 0) {
						QwtPlotCurve *c = d_graph->curve(i % n_curves);
					    if (c)
                            setSelection(c, findClosestPoint(c, d_selected_curve->x(d_selected_point), false));

						break;
					}
				d_graph->replot();
				return true;
			}

		case Qt::Key_Right:
		case Qt::Key_Plus:
			if (d_selected_curve) {
				int n_points = d_selected_curve->dataSize();
				if (ke->modifiers () == Qt::ShiftModifier)
                    setSelection(d_selected_curve, (d_selected_point + 10) % n_points);
                else if (ke->modifiers () == Qt::NoModifier)
                    setSelection(d_selected_curve, (d_selected_point + 1) % n_points);
				d_graph->replot();
			} else
				setSelection(d_graph->curve(0), 0);
			return true;

		case Qt::Key_Left:
		case Qt::Key_Minus:
			if (d_selected_curve) {
				int n_points = d_selected_curve->dataSize();
				if (ke->modifiers () == Qt::ShiftModifier)
                    setSelection(d_selected_curve, (d_selected_point - 10 + n_points) % n_points);
                else if (ke->modifiers () == Qt::NoModifier)
                    setSelection(d_selected_curve, (d_selected_point - 1 + n_points) % n_points);
				d_graph->replot();
			} else
				setSelection(d_graph->curve(d_graph->curveCount()-1), 0);
			return true;

		// The following keys represent a direction, they are
		// organized on the keyboard.
		case Qt::Key_1:
			if (d_mode == Move) {
				moveBy(-delta, delta);
				return true;
			}
			break;
		case Qt::Key_2:
			if (d_mode == Move) {
				moveBy(0, delta);
				return true;
			}
			break;
		case Qt::Key_3:
			if (d_mode == Move) {
				moveBy(delta, delta);
				return true;
			}
			break;
		case Qt::Key_4:
			if (d_mode == Move) {
				moveBy(-delta, 0);
				return true;
			}
			break;
		case Qt::Key_6:
			if (d_mode == Move) {
				moveBy(delta, 0);
				return true;
			}
			break;
		case Qt::Key_7:
			if (d_mode == Move) {
				moveBy(-delta, -delta);
				return true;
			}
			break;
		case Qt::Key_8:
			if (d_mode == Move) {
				moveBy(0, -delta);
				return true;
			}
			break;
		case Qt::Key_9:
			if (d_mode == Move) {
				moveBy(delta, -delta);
				return true;
			}
			break;
		default:
			break;
	}
	return false;
}

void DataPickerTool::removePoint()
{
	if ( !d_selected_curve )
		return;
	if (((PlotCurve *)d_selected_curve)->type() == Graph::Function){
		QMessageBox::critical(d_graph, tr("QtiPlot - Remove point error"),
				tr("Sorry, but removing points of a function is not possible."));
		return;
	}

	Table *t = ((DataCurve *)d_selected_curve)->table();
	if (!t)
		return;

	int col = t->colIndex(d_selected_curve->title().text());
	if (t->columnType(col) == Table::Numeric)
		t->clearCell(((DataCurve *)d_selected_curve)->tableRow(d_selected_point), col);
	else {
		QMessageBox::warning(d_graph, tr("QtiPlot - Warning"),
					tr("This operation cannot be performed on curves plotted from columns having a non-numerical format."));
	}

	d_selection_marker.detach();
	d_graph->replot();
	d_graph->setFocus();
	d_selected_curve = NULL;
}

void DataPickerTool::movePoint(const QPoint &pos)
{
	if ( !d_selected_curve )
		return;
	if ( ((PlotCurve *)d_selected_curve)->type() == Graph::Function){
		QMessageBox::critical(d_graph, tr("QtiPlot - Move point error"),
				tr("Sorry, but moving points of a function is not possible."));
		return;
	}
	Table *t = ((DataCurve *)d_selected_curve)->table();
	if (!t)
		return;

	if (t->isReadOnlyColumn(t->colIndex(((DataCurve *)d_selected_curve)->xColumnName()))){
    	QMessageBox::warning(d_app, tr("QtiPlot - Warning"),
        tr("The column '%1' is read-only! Please choose another curve!").arg(((DataCurve *)d_selected_curve)->xColumnName()));
		return;
	} else if (t->isReadOnlyColumn(t->colIndex(d_selected_curve->title().text()))){
    	QMessageBox::warning(d_app, tr("QtiPlot - Warning"),
		tr("The column '%1' is read-only! Please choose another curve!").arg(d_selected_curve->title().text()));
		return;
    }

	double new_x_val = d_graph->invTransform(d_selected_curve->xAxis(), pos.x());
	double new_y_val = d_graph->invTransform(d_selected_curve->yAxis(), pos.y());

	switch (d_move_mode){
        case Free:
            d_restricted_move_pos = pos;
        break;
        case Vertical:
            d_restricted_move_pos.setY(pos.y());
        break;
        case Horizontal:
            d_restricted_move_pos.setX(pos.x());
        break;
    }

    d_selection_marker.setValue(new_x_val, new_y_val);
	if (d_selection_marker.plot() == NULL)
		d_selection_marker.attach(d_graph);

    QLocale locale = d_app->locale();
	int row = ((DataCurve *)d_selected_curve)->tableRow(d_selected_point);
	int xcol = t->colIndex(((DataCurve *)d_selected_curve)->xColumnName());
	int ycol = t->colIndex(d_selected_curve->title().text());
	if (t->columnType(xcol) == Table::Numeric && t->columnType(ycol) == Table::Numeric) {
		t->setText(row, xcol, locale.toString(new_x_val));
		t->setText(row, ycol, locale.toString(new_y_val));
		d_app->updateCurves(t, d_selected_curve->title().text());
		d_app->modifiedProject();
	} else
		QMessageBox::warning(d_graph, tr("QtiPlot - Warning"),
        tr("This operation cannot be performed on curves plotted from columns having a non-numerical format."));


	emit statusText(QString("%1[%2]: x=%3; y=%4")
			.arg(d_selected_curve->title().text())
			.arg(row + 1)
			.arg(locale.toString(new_x_val, 'G', d_app->d_decimal_digits))
			.arg(locale.toString(new_y_val, 'G', d_app->d_decimal_digits)) );
}

void DataPickerTool::move(const QPoint &point)
{
	QPoint p = point;
	if (d_mode == Move){
	    switch (d_move_mode){
	        case Free:
                movePoint(point);
            break;
            case Vertical:
                p = QPoint(d_restricted_move_pos.x(), point.y());
                movePoint(p);
            break;
            case Horizontal:
                p = QPoint(point.x(), d_restricted_move_pos.y());
                movePoint(p);
            break;
	    }
	}

	QwtPlotPicker::move(p);
}

bool DataPickerTool::end(bool ok)
{
	if (d_mode == Move)
		d_selected_curve = NULL;
	return QwtPlotPicker::end(ok);
}

void DataPickerTool::moveBy(int dx, int dy)
{
	if ( !d_selected_curve )
		return;
	movePoint(transform(QwtDoublePoint(d_selected_curve->x(d_selected_point),
					d_selected_curve->y(d_selected_point))) + QPoint(dx, dy));
}

void DataPickerTool::cutSelection()
{
    copySelection();
    removePoint();
}

void DataPickerTool::copySelection()
{
    if (!d_selected_curve)
        return;

    QString text = d_app->locale().toString(d_selected_curve->x(d_selected_point), 'G', 16) + "\t";
    text += d_app->locale().toString(d_selected_curve->y(d_selected_point), 'G', 16) + "\n";

	QApplication::clipboard()->setText(text);
}

void DataPickerTool::pasteSelection()
{
	QString text = QApplication::clipboard()->text();
	if (text.isEmpty())
		return;

    if (((PlotCurve *)d_selected_curve)->type() == Graph::Function)
        return;

    Table *t = ((DataCurve*)d_selected_curve)->table();
    if (!t)
        return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QTextStream ts( &text, QIODevice::ReadOnly );
    int row = ((DataCurve*)d_selected_curve)->tableRow(d_selected_point);
    int col = t->colIndex(d_selected_curve->title().text());

    int prec; char f;
    t->columnNumericFormat(col, &f, &prec);
    QLocale locale = d_app->locale();
    QStringList cellTexts = ts.readLine().split("\t");
    if (cellTexts.count() >= 2){
        bool numeric;
        double value = locale.toDouble(cellTexts[1], &numeric);
        if (numeric){
            t->setText(row, col, locale.toString(value, f, prec));

            double x_val = d_selected_curve->x(d_selected_point);
            d_selection_marker.setValue(x_val, value);
            if (d_selection_marker.plot() == NULL)
                d_selection_marker.attach(d_graph);

            t->notifyChanges();

            emit statusText(QString("%1[%2]: x=%3; y=%4")
                            .arg(d_selected_curve->title().text())
                            .arg(row + 1)
                            .arg(locale.toString(x_val, 'G', d_app->d_decimal_digits))
                            .arg(cellTexts[1]));
        }
    }

	QApplication::restoreOverrideCursor();
}

void DataPickerTool::selectTableRow()
{
    if (!d_selected_curve)
        return;

    if (((PlotCurve *)d_selected_curve)->type() == Graph::Function)
        return;

    Table *t = ((DataCurve*)d_selected_curve)->table();
    if (!t)
        return;

    int row = ((DataCurve*)d_selected_curve)->tableRow(d_selected_point);
    t->goToRow(row + 1);
}

int DataPickerTool::findClosestPoint(QwtPlotCurve *c, double x, bool up)
{
    if (!c)
        return -1;

    if (c->minXValue() > x)
        return 0;

    if (c->maxXValue() < x)
        return c->dataSize() - 1;

    int index_left = 0;
    int index_right = c->dataSize() - 1;
    while ((index_right - index_left)>1) {
        int middle = (index_right + index_left)/2;
        if (c->x(index_left) < x && c->x(middle) > x)
            index_right = middle;
        else
            index_left = middle;
    }
    if (up)
        return index_left;
    else
        return index_right;
}
