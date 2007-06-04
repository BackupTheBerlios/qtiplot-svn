/***************************************************************************
    File                 : DoubleColumnData.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Hoener zu Siederdissen,
    Email (use @ for *)  : thzs*gmx.net
    Description          : Data source that stores a vector of doubles (implementation)

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

#include "DoubleColumnData.h"
#include <QObject>
#include <QLocale>

DoubleColumnData::DoubleColumnData(int size)
	: QVector<double>(size), d_numeric_format('e'), d_displayed_digits(6)
{
}

bool DoubleColumnData::copy(const AbstractDataSource * other)
{
	emit dataAboutToChange(this);
	if(other->inherits("AbstractDoubleDataSource"))
	{
		d_numeric_format = static_cast< const AbstractDoubleDataSource* >(other)->numericFormat();
		d_displayed_digits = static_cast< const AbstractDoubleDataSource* >(other)->displayedDigits();
	}

	int end = other->numRows();
	setNumRows(end);
	double * ptr = data();
	for(int i=0; i<end; i++)
		ptr[i] = other->valueAt(i);
	emit dataChanged(this);
	return true;
}
	
int DoubleColumnData::numRows() const 
{ 
	return size(); 
}

QString DoubleColumnData::label() const
{ 
	return d_label;
}

QString DoubleColumnData::comment() const
{ 
	return d_comment;
}

AbstractDataSource::PlotDesignation DoubleColumnData::plotDesignation() const
{ 
	return d_plot_designation;
}

void DoubleColumnData::setLabel(const QString& label) 
{ 
	emit descriptionAboutToChange(this);
	d_label = label; 
	emit descriptionChanged(this);
}

void DoubleColumnData::setComment(const QString& comment) 
{ 
	emit descriptionAboutToChange(this);
	d_comment = comment; 
	emit descriptionChanged(this);
}

void DoubleColumnData::setPlotDesignation(AbstractDataSource::PlotDesignation pd) 
{ 
	emit plotDesignationAboutToChange(this);
	d_plot_designation = pd; 
	emit plotDesignationChanged(this);
}

void DoubleColumnData::notifyReplacement(AbstractDataSource * replacement)
{
	emit aboutToBeReplaced(this, replacement); 
}

void DoubleColumnData::setRowFromString(int row, const QString& string)
{
	emit dataAboutToChange(this);
    data()[row] = QLocale().toDouble(string);
	emit dataChanged(this);
}

void DoubleColumnData::setNumRows(int new_size)
{
	if(new_size == numRows())
		return;
	emit dataAboutToChange(this);
	QVector<double>::resize(new_size);
	emit dataChanged(this);
}

void DoubleColumnData::insertEmptyRows(int before, int count)
{
	emit dataAboutToChange(this);
	insert(before, count, 0.0);
	emit dataChanged(this);
}

void DoubleColumnData::removeRows(int first, int count)
{
	emit dataAboutToChange(this);
	remove(first, count);
	emit dataChanged(this);
}

void DoubleColumnData::setNumericFormat(char format) 
{ 
	emit formatAboutToChange(this);
	d_numeric_format = format; 
	emit formatChanged(this);
}

void DoubleColumnData::setDisplayedDigits(int digits) 
{ 
	emit formatAboutToChange(this);
	d_displayed_digits = digits; 
	emit formatChanged(this);
}

char DoubleColumnData::numericFormat() const
{ 
	return d_numeric_format; 
}

int DoubleColumnData::displayedDigits() const 
{ 
	return d_displayed_digits; 
}

QString DoubleColumnData::textAt(int row) const
{ 
		return QLocale().toString(value(row), d_numeric_format, d_displayed_digits);
}

double DoubleColumnData::valueAt(int row) const 
{ 
	return value(row); 
}

const double * DoubleColumnData::constDataPointer() const 
{ 
	return data(); 
}
