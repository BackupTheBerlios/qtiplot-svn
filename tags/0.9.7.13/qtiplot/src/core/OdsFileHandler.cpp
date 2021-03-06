/***************************************************************************
    File                 : OdsFileHandler.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2009 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : An XML handler for parsing Open Document Format Spreadsheets (.ods)

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
#include "OdsFileHandler.h"
#include "ApplicationWindow.h"

OdsFileHandler::OdsFileHandler(ApplicationWindow *app, const QString& odsFileName) :
d_app(app),
d_ods_file_name(odsFileName)
 {
	d_rows = 0;
	d_col = 0;
	d_last_column = 0;
 }

 bool OdsFileHandler::startElement(const QString & /* namespaceURI */,
                                const QString & /* localName */,
                                const QString &qName,
                                const QXmlAttributes &attributes)
 {
	 if (qName == "table:table" && !attributes.value("table:name").isEmpty())
		d_sheet_names << attributes.value("table:name");

	if (qName == "table:table-row"){
		if (!attributes.value("table:number-rows-repeated").isEmpty())
			d_rows += attributes.value("table:number-rows-repeated").toInt();
		else
			d_rows++;
	}

	if (qName == "table:table-cell"){
		cell_data cell = {d_rows - 1, 0, 0.0, "", EmptyCell};
		QString type = attributes.value("office:value-type");
		if (type == QString("float")){
			cell.d = attributes.value("office:value").toDouble();
			cell.type = Float;
		} else if (type == QString("percentage")){
			cell.d = attributes.value("office:value").toDouble();
			cell.type = Percent;
		} else if (type == QString("currency")){
			cell.d = attributes.value("office:value").toDouble();
			cell.type = Currency;
		} else if (type == QString("boolean")){
			cell.d = attributes.value("office:value").toDouble();
			cell.type = Boolean;
		} else if (type == QString("date")){
			cell.type = Date;
		} else if (type == QString("time")){
			cell.type = Time;
		} else if (type == QString("string"))
			cell.type = String;

		cell.col = d_col;

		int repeatCols = 1;
		if (!attributes.value("table:number-columns-repeated").isEmpty())
			repeatCols = attributes.value("table:number-columns-repeated").toInt();

		int spannedCols = 1;
		if (!attributes.value("table:number-columns-spanned").isEmpty())
			spannedCols = attributes.value("table:number-columns-spanned").toInt();

		if (cell.type != EmptyCell){
			for (int i = 0; i < repeatCols; i++){
				cell.col = d_col + i;
				cells.push_back(cell);
			}
		}

		if (repeatCols > 1)
			d_col += repeatCols;
		else
			d_col++;

		if (spannedCols > 1)
			d_col += spannedCols - 1;
	}

	currentText.clear();
	return true;
 }

bool OdsFileHandler::endElement(const QString & /* namespaceURI */,
                              const QString & /* localName */,
                              const QString &qName)
{
	if (qName == "table:table-row"){
		int col = 0;
		if (!cells.empty())
			col = cells[cells.size() - 1].col;

		if (col > d_last_column)
			d_last_column = col;

		d_col = 0; //reset column index to 0
	}

	if (qName == "text:p")
		cells[cells.size() - 1].str = currentText;

	if (qName == "table:table"){
		if (!cells.empty() && (d_last_column > 1 || d_rows > 1)){
			Table *t = d_app->newTable(d_rows, d_last_column + 1, QString::null, d_ods_file_name + ", " + d_sheet_names.last());
			int n = cells.size();
			for (int i = 0; i < n; i++){
				cell_data cell = cells[i];
				if (cell.type == Float)
					t->setCell(cell.row, cell.col, cell.d);
				else
					t->setText(cell.row, cell.col, cell.str);
			}
			t->showNormal();
			d_tables << t;
		} else
			d_sheet_names.removeLast();

		d_last_column = 0;
		d_rows = 0;
		cells.clear();
	}
	return true;
 }

 bool OdsFileHandler::characters(const QString &str)
 {
     currentText += str;
     return true;
 }

 QString OdsFileHandler::errorString() const
 {
     return errorStr;
 }

 Table *OdsFileHandler::sheet(int index)
 {
	if (index >= 0 && index < d_tables.size())
		return d_tables[index];
	return NULL;
 }
