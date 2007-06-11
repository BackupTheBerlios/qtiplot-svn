/***************************************************************************
    File                 : TableItemDelegate.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Hoener zu Siederdissen,
    Email (use @ for *)  : thzs*gmx.net
    Description          : Item delegate for TableView

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

#include <QItemDelegate>

//TODO: Unless there is another purpose for this than handling
// [Return]/[Enter], this class can be deleted

//! Item delegate for TableView
/**
 * This class does only one thing at the moment:
 * It advances the current cell in the view by
 * one row if [Return] or [Enter] was pressed.
 */ 
class TableItemDelegate : public QItemDelegate
{
    Q_OBJECT

signals:
	//! User pressed [Return] or [Enter] after editing a cell
	void returnPressed();

protected:
	//! Wrapper of QItemDelegate::eventFilter that handles [Return] and [Enter]
	virtual bool eventFilter( QObject * editor, QEvent * event );
};
