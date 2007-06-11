/***************************************************************************
    File                 : Double2DayOfWeekFilter.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke
    Email (use @ for *)  : knut.franke*gmx.de
    Description          : Conversion filter double -> QDateTime, interpreting
                           the input numbers as days of the week (1 -> Monday).
                           
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
#ifndef DOUBLE2DAY_OF_WEEK_FILTER_H
#define DOUBLE2DAY_OF_WEEK_FILTER_H

#include "AbstractSimpleFilter.h"
#include <QDateTime>

//! Conversion filter double -> QDateTime, interpreting the input numbers as days of the week (1 = Monday).
class Double2DayOfWeekFilter : public AbstractSimpleFilter<QDateTime>
{
	Q_OBJECT
	public:
		virtual QDate dateAt(int row) const {
			if (!d_inputs.value(0)) return QDate();
			// Julian day 0 is a Monday, but we want to have 1 -> Monday,
			// so our reference date is Julian day -1.
			return QDate::fromJulianDay(qRound(doubleInput()->valueAt(row) - 1.0));
		}
		virtual QTime timeAt(int row) const {
			Q_UNUSED(row)
			return QTime(0,0,0,0);
			/*
			if (!d_inputs.value(0)) return QTime();
			double input_value = doubleInput()->valueAt(row);
			return QTime(12,0,0,0).addMSecs(int( (input_value - int(input_value)) * 86400000.0 ));
			*/
		}
		virtual QDateTime dateTimeAt(int row) const {
			return QDateTime(dateAt(row), timeAt(row));
		}

	protected:
		//! Using typed ports: only double inputs are accepted.
		virtual bool inputAcceptable(int, AbstractDataSource *source) {
			return source->inherits("AbstractDoubleDataSource");
		}
};

#endif // ifndef DOUBLE2DAY_OF_WEEK_FILTER_H
