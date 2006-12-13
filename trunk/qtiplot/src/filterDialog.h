/***************************************************************************
    File                 : filterDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email                : ion_vasilief@yahoo.fr, thzs@gmx.net
    Description          : Filter options dialog
                           
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
#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>

class QPushButton;
class QLineEdit;
class QComboBox;
class QCheckBox;
class Graph;
class ColorBox;
	
//! Filter options dialog
class FilterDialog : public QDialog
{
    Q_OBJECT

public:
    FilterDialog(int type, QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0 );
    ~FilterDialog();

	enum FilterType{LowPass = 1, HighPass = 2, BandPass = 3, BandBlock = 4};

	QPushButton* buttonFilter;
	QPushButton* buttonCancel;
	QComboBox* boxName;
	QCheckBox* boxOffset;
	QLineEdit* boxStart;
	QLineEdit* boxEnd;
	ColorBox* boxColor;

public slots:
	void setGraph(Graph *g);
	void filter();

private:
	Graph *graph;
	int filter_type;
};

#endif



