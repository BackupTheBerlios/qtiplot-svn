/***************************************************************************
	File                 : MatrixModel.cpp
	Project              : QtiPlot
--------------------------------------------------------------------
	Copyright            : (C) 2007 by Ion Vasilief
	Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : QtiPlot's matrix model

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
#include <QtGui>
#include <QFile>
#include <QTextStream>

#include "Matrix.h"
#include "MatrixModel.h"
#include "MatrixCommand.h"

#include <gsl/gsl_math.h>
#include <qwt_color_map.h>

MatrixModel::MatrixModel(int rows, int cols, QObject *parent)
     : QAbstractTableModel(parent),
     d_rows(rows),
     d_cols(cols),
	 d_data(d_rows*d_cols, GSL_NAN),
	 d_matrix((Matrix*)parent),
	 d_txt_format('g'),
	 d_num_precision(6),
	 d_locale(QLocale())
{
	if (d_matrix){
		d_txt_format = d_matrix->textFormat().toAscii();
		d_num_precision = d_matrix->precision();
		d_locale = d_matrix->locale();
	}
}

MatrixModel::MatrixModel(const QImage& image, QObject *parent)
     : QAbstractTableModel(parent),
	 d_matrix((Matrix*)parent),
	 d_txt_format('g'),
	 d_num_precision(6),
	 d_locale(QLocale())
{
    d_rows = image.height();
    d_cols = image.width();
    d_data.resize(d_rows*d_cols);

    for (int i=0; i<d_rows; i++ ){
		for (int j=0; j<d_cols; j++){
		    int index = i*d_cols + j;
            d_data[index] = qGray(image.pixel (j, i));
		}
    }
}

MatrixModel* MatrixModel::copy()
{
	MatrixModel* m = new MatrixModel(d_rows, d_cols, d_matrix);
    double *data = m->dataVector();
    if (data){
        int size = d_rows*d_cols;
        for (int i = 0; i < size; i++)
            data[i] = d_data[i];
    }
	return m;
}

Qt::ItemFlags MatrixModel::flags(const QModelIndex & index ) const
{
	if (index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	else
		return Qt::ItemIsEnabled;
}

int MatrixModel::rowCount(const QModelIndex & /* parent */) const
{
    return d_rows;
}

int MatrixModel::columnCount(const QModelIndex & /* parent */) const
{
    return d_cols;
}

void MatrixModel::setRowCount(int rows)
{
	if (d_rows == rows)
		return;

   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	int diff = abs(rows - d_rows);
	if (rows > d_rows )
		insertRows(d_rows, diff);
    else if (rows < d_rows )
		removeRows(rows, diff);

	QApplication::restoreOverrideCursor();

}

void MatrixModel::setColumnCount(int cols)
{
	if (d_cols == cols)
		return;

   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	int diff = abs(cols - d_cols);
	if (cols > d_cols )
		insertColumns(d_cols, diff);
    else if (cols < d_cols )
		removeColumns(cols, diff);

	QApplication::restoreOverrideCursor();
}

double MatrixModel::cell(int row, int col)
{
    int i = d_cols*row + col;
    if (i < 0 || i>= d_data.size() || gsl_isnan (d_data[i]))
        return 0.0;

	return d_data[i];
}

void MatrixModel::setCell(int row, int col, double val)
{
	int i = d_cols*row + col;
    if (i < 0 || i>= d_data.size())
		return;

    d_data[i] = val;
}

QString MatrixModel::text(int row, int col)
{
	int i = d_cols*row + col;
    if (i < 0 || i>= d_data.size() || gsl_isnan (d_data[i]))
        return "";

	if (d_matrix){
		QLocale locale = d_matrix->locale();
		return locale.toString(d_data[i], d_matrix->textFormat().toAscii(), d_matrix->precision());
	}
	return d_locale.toString(d_data[i], d_txt_format, d_num_precision);
}

void MatrixModel::setText(int row, int col, const QString& text)
{
	int i = d_cols*row + col;
    if (i < 0 || i>= d_data.size())
		return;

 	if (text.isEmpty())
		d_data[i] = GSL_NAN;
	else {
		if (d_matrix)
			d_data[i] = d_matrix->locale().toDouble(text);
		else
			d_data[i] = d_locale.toDouble(text);
	}
}

double* MatrixModel::dataVector()
{
    return (double *)d_data.data();
}

double MatrixModel::data(int row, int col) const
{
    int i = d_cols*row + col;
    if (i < 0 || i>= d_data.size())
        return 0.0;

	return d_data[i];
}

double MatrixModel::x(int col) const
{
	if (col < 0 || col >= d_cols)
        return 0.0;

	double start = d_matrix->xStart();
	double end = d_matrix->xEnd();
	if (start < end)
		return start + col*d_matrix->dx();
	else
		return start - col*d_matrix->dx();

	return 0.0;
}

double MatrixModel::y(int row) const
{
	if (row < 0 || row >= d_rows)
        return 0.0;

	double start = d_matrix->yStart();
	double end = d_matrix->yEnd();
	if (start < end)
		return start + row*d_matrix->dy();
	else
		return start - row*d_matrix->dy();

	return 0.0;
}

QVariant MatrixModel::headerData ( int section, Qt::Orientation orientation, int role) const
{
    if (!d_matrix || d_matrix->headerViewType() == Matrix::ColumnRow)
        return QAbstractItemModel::headerData(section, orientation, role);

    QLocale locale = d_locale;
	int prec = d_num_precision;
	char fmt = d_txt_format;
	if (d_matrix){
		locale = d_matrix->locale();
		fmt = d_matrix->textFormat().toAscii();
		prec = d_matrix->precision();
	}

    if (role == Qt::DisplayRole || role == Qt::EditRole){
        if (orientation == Qt::Horizontal){
            double start = d_matrix->xStart();
            double end = d_matrix->xEnd();
            double dx = d_matrix->dx();
            if (start < end)
                return QVariant(locale.toString(start + section*dx, fmt, prec));
            else
                return QVariant(locale.toString(start - section*dx, fmt, prec));
        } else if (orientation == Qt::Vertical){
            double start = d_matrix->yStart();
            double end = d_matrix->yEnd();
            double dy = d_matrix->dy();
            if (start < end)
                return QVariant(locale.toString(start + section*dy, fmt, prec));
            else
                return QVariant(locale.toString(start - section*dy, fmt, prec));
        }
    }
	return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant MatrixModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
		return QVariant();

    int i = d_cols*index.row() + index.column();
    if (gsl_isnan (d_data[i]))
        return QVariant();

	if (role == Qt::DisplayRole || role == Qt::EditRole){
		if (d_matrix)
			return QVariant(d_matrix->locale().toString(d_data[i], d_matrix->textFormat().toAscii(), d_matrix->precision()));
		else
			return QVariant(d_locale.toString(d_data[i], d_txt_format, d_num_precision));
	} else
		return QVariant();
}

bool MatrixModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
	if (!index.isValid())
		return false;

	int i = d_cols*index.row() + index.column();
	double valBefore = d_data[i];
 	if(role == Qt::EditRole){
		if (value.toString().isEmpty())
			d_data[i] = GSL_NAN;
		else
			d_data[i] = value.toDouble();
	}

	if(index.row() + 1 >= d_rows){
        insertRows(d_rows, 1);
		d_matrix->resetView();
	}

    d_matrix->undoStack()->push(new MatrixEditCellCommand(this, index, valBefore, d_data[i],
                                tr("Edited cell") + " (" + QString::number(index.row() + 1) + "," +
                                QString::number(index.column() + 1) + ")"));
    d_matrix->notifyChanges();
	d_matrix->moveCell(index);
	return false;
}

bool MatrixModel::removeColumns(int column, int count, const QModelIndex & parent)
{
	beginRemoveColumns(parent, column, column + count - 1);

	int new_cols = d_cols - count;
	QVector <double> d_data_new(d_rows*new_cols);
	for (int i=0; i <d_rows; i++){
		int aux1 = d_cols * i;
		int aux2 = new_cols * i;
		for (int j=0; j < column; j++)
			d_data_new[aux2 + j] = d_data[aux1 + j];

		aux1 += count;
		for (int j = column; j < new_cols; j++)
			d_data_new[aux2 + j] = d_data[aux1 + j];
	}

	d_data.resize(0);
	d_data = d_data_new;
	d_cols = new_cols;

	endRemoveColumns();
	return true;
}

bool MatrixModel::insertColumns(int column, int count, const QModelIndex & parent)
{
	beginInsertColumns(parent, column, column + count - 1);

	int new_cols = d_cols + count;
	QVector <double> d_data_new(d_rows*new_cols);
	d_data_new.fill(GSL_NAN);

	for (int i=0; i <d_rows; i++){
		int aux1 = d_cols * i;
		int aux2 = new_cols * i;
		for (int j=0; j < column; j++)
			d_data_new[aux2 + j] = d_data[aux1 + j];

		aux2 += count;
		for (int j = column; j < d_cols; j++)
			d_data_new[aux2 + j] = d_data[aux1 + j];
	}

	d_data.resize(0);
	d_data = d_data_new;
	d_cols = new_cols;

	endInsertColumns();
	return true;
}

bool MatrixModel::insertRows(int row, int count, const QModelIndex & parent)
{
	beginInsertRows(parent, row, row + count - 1);

	d_data.insert(row * d_cols, count*d_cols, GSL_NAN);
	d_rows += count;

	endInsertRows();
	return true;
}

bool MatrixModel::removeRows(int row, int count, const QModelIndex & parent)
{
	beginRemoveRows(parent, row, row + count - 1);

	d_data.remove(row * d_cols, count*d_cols);
	d_rows -= count;

	endRemoveRows();
	return true;
}

QString MatrixModel::saveToString()
{
	QString s = "<data>\n";
	int cols = d_cols - 1;
	for(int i = 0; i < d_rows; i++){
		int aux = d_cols*i;
		bool emptyRow = true;
		for(int j = 0; j < d_cols; j++){
			if (gsl_finite(d_data[aux + j])){
				emptyRow = false;
				break;
			}
		}
		if (emptyRow)
			continue;

		s += QString::number(i) + "\t";
		for(int j = 0; j < cols; j++){
			double val = d_data[aux + j];
			if (gsl_finite(val))
				s += QString::number(val, 'e', 16);
			s += "\t";
		}
		double val = d_data[aux + cols];
		if (gsl_finite(val))
			s += QString::number(val, 'e', 16);
		s += "\n";
	}
	return s + "</data>\n";
}

QImage MatrixModel::renderImage()
{
	int size = QMAX(d_cols, d_rows);
	QImage image(QSize(size, size), QImage::Format_RGB32);
	image.fill(0);
	QwtLinearColorMap color_map = d_matrix->colorMap();

	double minValue = 0.0, maxValue = 0.0;
	d_matrix->range(&minValue, &maxValue);
    const QwtDoubleInterval intensityRange = QwtDoubleInterval (minValue, maxValue);
    for ( int i = 0; i < d_rows; i++ ){
    	QRgb *line = (QRgb *)image.scanLine(i);
		for ( int j = 0; j < d_cols; j++){
			if(fabs(d_data[i*d_cols + j]) < HUGE_VAL)
				*line++ = color_map.rgb(intensityRange, d_data[i*d_cols + j]);
		}
     }
	 return image;
}

void MatrixModel::setDataVector(const QVector<double>& data)
{
	d_data = data;
}

bool MatrixModel::importASCII(const QString &fname, const QString &sep, int ignoredLines,
    bool stripSpaces, bool simplifySpaces, const QString& commentString, int importAs,
	const QLocale& locale, int endLineChar, int maxRows)
{
	int rows = 0;
	QString name = MdiSubWindow::parseAsciiFile(fname, commentString, endLineChar, ignoredLines, maxRows, rows);
	if (name.isEmpty())
		return false;
	QFile f(name);
	if (!f.open(QIODevice::ReadOnly))
		return false;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QTextStream t(&f);
	QLocale l = d_locale;
	if (d_matrix)
		l = d_matrix->locale();
	bool updateDecimalSeparators = (l != locale) ? true : false;

	QString s = t.readLine();
	if (simplifySpaces)
		s = s.simplifyWhiteSpace();
	else if (stripSpaces)
		s = s.stripWhiteSpace();

	QStringList line = s.split(sep);
	int cols = line.size();

	int startRow = 1, startCol = 0;
	switch(importAs){
		case Matrix::Overwrite:
			if (d_rows != rows)
				setRowCount(rows);
			if (d_cols != cols)
				setColumnCount(cols);
		break;
		case Matrix::NewColumns:
			startCol = d_cols;
			setColumnCount(d_cols + cols);
			if (d_rows < rows)
				setRowCount(rows);
		break;
		case Matrix::NewRows:
			startRow = d_rows;
			if (d_cols < cols)
				setColumnCount(cols);
			setRowCount(d_rows + rows);
		break;
	}

	for (int j = startCol; j<d_cols; j++){
		int aux = j - startCol;
		if (cols > aux){
			if (updateDecimalSeparators)
				setCell(0, j, locale.toDouble(line[aux]));
			else
				setText(0, j, line[aux]);
		}
	}

	qApp->processEvents(QEventLoop::ExcludeUserInput);
	for (int i = startRow; i<d_rows; i++){
		s = t.readLine();
		if (simplifySpaces)
			s = s.simplifyWhiteSpace();
		else if (stripSpaces)
			s = s.stripWhiteSpace();
		line = s.split(sep);
		int lc = line.size();
		if (lc > cols)
			setColumnCount(d_cols + lc - cols);

		for (int j = startCol; j<d_cols; j++){
			int aux = j - startCol;
		    if (lc > aux){
				if (updateDecimalSeparators)
					setCell(i, j, locale.toDouble(line[aux]));
				else
					setText(i, j, line[aux]);
			}
		}
	}
    f.remove();	//remove temp file
	QApplication::restoreOverrideCursor();
	return true;
}

void MatrixModel::setNumericFormat(char f, int prec)
{
	if (d_txt_format == f && d_num_precision == prec)
		return;

	d_txt_format = f;
	d_num_precision = prec;
}
