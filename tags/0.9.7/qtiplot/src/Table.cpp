/***************************************************************************
    File                 : Table.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief,
                           Tilman Hoener zu Siederdissen,
                           Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
                           knut.franke*gmx.de
    Description          : Table worksheet class

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
#include "Table.h"
#include "SortDialog.h"
#include "ImportASCIIDialog.h"
#include "muParserScript.h"
#include "ApplicationWindow.h"

#include <QMessageBox>
#include <QDateTime>
#include <QTextStream>
#include <QClipboard>
#include <QApplication>
#include <QPainter>
#include <QEvent>
#include <QLayout>
#include <QPrintDialog>
#include <QLocale>
#include <QShortcut>
#include <QProgressDialog>
#include <QFile>

#include <q3paintdevicemetrics.h>
#include <q3dragobject.h>
#include <Q3TableSelection>
#include <Q3MemArray>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_sort_vector.h>

#ifdef Q_CC_MSVC
#include <ctime>
#endif

Table::Table(ScriptingEnv *env, int r, int c, const QString& label, ApplicationWindow* parent, const QString& name, Qt::WFlags f)
: MdiSubWindow(label,parent,name,f), scripted(env)
{
	init(r,c);
}

void Table::init(int rows, int cols)
{
	selectedCol=-1;
	d_saved_cells = 0;
	d_show_comments = false;
	d_numeric_precision = 13;

	d_table = new MyTable(rows, cols, this, "table");
	d_table->setSelectionMode (Q3Table::Single);
	d_table->setRowMovingEnabled(true);
	d_table->setColumnMovingEnabled(true);
	d_table->setCurrentCell(-1, -1);

	connect(d_table->verticalHeader(), SIGNAL(indexChange(int, int, int)),
			this, SLOT(notifyChanges()));
    connect(d_table->horizontalHeader(), SIGNAL(indexChange(int, int, int)),
			this, SLOT(moveColumn(int, int, int)));

	setFocusPolicy(Qt::StrongFocus);
	setFocus();

	for (int i=0; i<cols; i++){
		commands << "";
		colTypes << Numeric;
		col_format << "0/16";
		comments << "";
		col_label << QString::number(i+1);
		col_plot_type << Y;
	}

	Q3Header* head=(Q3Header*)d_table->horizontalHeader();
	head->setMouseTracking(true);
	head->setResizeEnabled(true);
	head->installEventFilter(this);
	connect(head, SIGNAL(sizeChange(int, int, int)), this, SLOT(colWidthModified(int, int, int)));

	col_plot_type[0] = X;
	setHeaderColType();

	int w = 4*(d_table->horizontalHeader())->sectionSize(0);
	int h = 11*(d_table->verticalHeader())->sectionSize(0);
	setGeometry(50, 50, w + 45, h);

	d_table->verticalHeader()->setResizeEnabled(false);
	d_table->verticalHeader()->installEventFilter(this);

	setWidget(d_table);

	QShortcut *accelTab = new QShortcut(QKeySequence(Qt::Key_Tab), this);
	connect(accelTab, SIGNAL(activated()), this, SLOT(moveCurrentCell()));

	QShortcut *accelAll = new QShortcut(QKeySequence(Qt::CTRL+Qt::Key_A), this);
	connect(accelAll, SIGNAL(activated()), this, SLOT(selectAllTable()));

	connect(d_table, SIGNAL(valueChanged(int, int)), this, SLOT(cellEdited(int, int)));

	setAutoUpdateValues(applicationWindow()->autoUpdateTableValues());
}

void Table::setAutoUpdateValues(bool on)
{
	if (on){
		connect(this, SIGNAL(modifiedData(Table *, const QString&)),
            	this, SLOT(updateValues(Table*, const QString&)));
	} else {
		disconnect(this, SIGNAL(modifiedData(Table *, const QString&)),
            	this, SLOT(updateValues(Table*, const QString&)));
	}
}

void Table::colWidthModified(int, int, int)
{
	emit modifiedWindow(this);
	setHeaderColType();
}


void Table::setBackgroundColor(const QColor& col)
{
	d_table->setPaletteBackgroundColor ( col );
}

void Table::setTextColor(const QColor& col)
{
	d_table->setPaletteForegroundColor (col);
}

void Table::setTextFont(const QFont& fnt)
{
	d_table->setFont (fnt);
	QFontMetrics fm(fnt);
	int lm = fm.width( QString::number(10*d_table->numRows()));
	d_table->setLeftMargin( lm );
}

void Table::setHeaderColor(const QColor& col)
{
	d_table->horizontalHeader()->setPaletteForegroundColor (col);
}

void Table::setHeaderFont(const QFont& fnt)
{
	d_table->horizontalHeader()->setFont (fnt);
}

void Table::exportPDF(const QString& fileName)
{
	print(fileName);
}

void Table::print()
{
    print(QString());
}

void Table::print(const QString& fileName)
{
	QPrinter printer;
	printer.setColorMode (QPrinter::GrayScale);
	if (!fileName.isEmpty()){
	    printer.setCreator("QtiPlot");
	    printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
	} else {
        QPrintDialog printDialog(&printer, applicationWindow());
        if (printDialog.exec() != QDialog::Accepted)
            return;
    }

	printer.setFullPage( true );
    QPainter p;
    if ( !p.begin(&printer ) )
        return; // paint on printer

		Q3PaintDeviceMetrics metrics( p.device() );
		int dpiy = metrics.logicalDpiY();
		const int margin = (int) ( (1/2.54)*dpiy ); // 2 cm margins

		Q3Header *hHeader = d_table->horizontalHeader();
		Q3Header *vHeader = d_table->verticalHeader();

		int rows=d_table->numRows();
		int cols=d_table->numCols();
		int height=margin;
		int i,vertHeaderWidth=vHeader->width();
		int right = margin + vertHeaderWidth;

		// print header
		p.setFont(hHeader->font());
		QRect br=p.boundingRect(br,Qt::AlignCenter,	hHeader->label(0));
		p.drawLine(right,height,right,height+br.height());
		QRect tr(br);

		for (i=0;i<cols;i++)
		{
			int w=d_table->columnWidth (i);
			tr.setTopLeft(QPoint(right,height));
			tr.setWidth(w);
			tr.setHeight(br.height());
			p.drawText(tr,Qt::AlignCenter,hHeader->label(i),-1);
			right+=w;
			p.drawLine(right,height,right,height+tr.height());

			if (right >= metrics.width()-2*margin )
				break;
		}
		p.drawLine(margin + vertHeaderWidth, height, right-1, height);//first horizontal line
		height += tr.height();
		p.drawLine(margin,height,right-1,height);

		// print table values
		for (i=0;i<rows;i++)
		{
			right = margin;
			QString text = vHeader->label(i)+"\t";
			tr = p.boundingRect(tr,Qt::AlignCenter,text);
			p.drawLine(right,height,right,height+tr.height());

			br.setTopLeft(QPoint(right,height));
			br.setWidth(vertHeaderWidth);
			br.setHeight(tr.height());
			p.drawText(br,Qt::AlignCenter,text,-1);
			right += vertHeaderWidth;
			p.drawLine(right,height,right,height+tr.height());

			for (int j=0;j<cols;j++)
			{
				int w=d_table->columnWidth (j);
				text=d_table->text(i,j)+"\t";
				tr=p.boundingRect(tr,Qt::AlignCenter,text);
				br.setTopLeft(QPoint(right,height));
				br.setWidth(w);
				br.setHeight(tr.height());
				p.drawText(br,Qt::AlignCenter,text,-1);
				right+=w;
				p.drawLine(right,height,right,height+tr.height());

				if (right >= metrics.width()-2*margin )
					break;
			}
			height+=br.height();
			p.drawLine(margin,height,right-1,height);

			if (height >= metrics.height()-margin )
			{
				printer.newPage();
				height=margin;
				p.drawLine(margin,height,right,height);
			}
		}
}

void Table::cellEdited(int row, int col)
{
	QString text = d_table->text(row,col).remove(QRegExp("\\s"));
	if (columnType(col) != Numeric || text.isEmpty()){
		emit modifiedData(this, colName(col));
		emit modifiedWindow(this);
		return;
	}

	char f;
	int precision;
  	columnNumericFormat(col, &f, &precision);
  	bool ok = true;
  	double res = locale().toDouble(text, &ok);
  	if (ok)
  		d_table->setText(row, col, locale().toString(res, f, precision));
  	else {
  		Script *script = scriptEnv->newScript(d_table->text(row,col),this,QString("<%1_%2_%3>").arg(objectName()).arg(row+1).arg(col+1));
  		connect(script, SIGNAL(error(const QString&,const QString&,int)), scriptEnv, SIGNAL(error(const QString&,const QString&,int)));

  		script->setInt(row+1, "i");
  		script->setInt(col+1, "j");
  		QVariant ret = script->eval();
  		if(ret.type()==QVariant::Int || ret.type()==QVariant::UInt || ret.type()==QVariant::LongLong || ret.type()==QVariant::ULongLong)
  			d_table->setText(row, col, ret.toString());
  		else if(ret.canCast(QVariant::Double))
  			d_table->setText(row, col, locale().toString(ret.toDouble(), f, precision));
  		else
  			d_table->setText(row, col, "");
  	}

  	emit modifiedData(this, colName(col));
	emit modifiedWindow(this);
}

int Table::colX(int col)
{
	int i, xcol = -1;
	for(i=col-1; i>=0; i--)
	{
		if (col_plot_type[i] == X)
			return i;
	}
	for(i=col+1; i<(int)d_table->numCols(); i++)
	{
		if (col_plot_type[i] == X)
			return i;
	}
	return xcol;
}

int Table::colY(int col)
{
	int i, yCol = -1;
	for(i=col-1; i>=0; i--)
	{
		if (col_plot_type[i] == Y)
			return i;
	}
	for(i=col+1; i<(int)d_table->numCols(); i++)
	{
		if (col_plot_type[i] == Y)
			return i;
	}
	return yCol;
}

void Table::setPlotDesignation(PlotDesignation pd, bool rightColumns)
{
	if (rightColumns){
	     int cols = d_table->numCols();
	     for (int i = selectedCol; i<cols; i++){
            col_plot_type[i] = pd;
            if (pd == Label)
                colTypes[i] = Text;
            else if (pd != None)
                colTypes[i] = Numeric;
        }
	} else {
        QStringList list = selectedColumns();
        for (int i=0; i<(int) list.count(); i++){
            int col = colIndex(list[i]);
            col_plot_type[col] = pd;
            if (pd == Label)
                colTypes[col] = Text;
            else if (pd != None)
                colTypes[col] = Numeric;
        }
	}

	setHeaderColType();
	emit modifiedWindow(this);
}

void Table::setColPlotDesignation(int col, PlotDesignation pd)
{
	if (col < 0 || col >= d_table->numCols() || col_plot_type[col] == pd)
        return;

	col_plot_type[col] = pd;
	if (pd == Label)
        colTypes[col] = Text;
}

void Table::columnNumericFormat(int col, int *f, int *precision)
{
	QStringList format = col_format[col].split("/", QString::KeepEmptyParts);
	if (format.count() == 2)
	{
		*f = format[0].toInt();
		*precision = format[1].toInt();
	}
	else
	{
		*f = 0;
		*precision = 14;
	}
}

void Table::columnNumericFormat(int col, char *f, int *precision)
{
	QStringList format = col_format[col].split("/", QString::KeepEmptyParts);
	if (format.count() == 2)
	{
		switch(format[0].toInt())
		{
			case 0:
			*f = 'g';
			break;

			case 1:
			*f = 'f';
			break;

			case 2:
			*f = 'e';
			break;
		}
		*precision = format[1].toInt();
	}
	else
	{
		*f = 'g';
		*precision = 14;
	}
}

int Table::columnWidth(int col)
{
	return d_table->columnWidth(col);
}

QStringList Table::columnWidths()
{
	QStringList widths;
	for (int i=0;i<d_table->numCols();i++)
		widths<<QString::number(d_table->columnWidth(i));

	return widths;
}

void Table::setColWidths(const QStringList& widths)
{
	for (int i=0; i<(int)widths.count(); i++)
		d_table->setColumnWidth(i, widths[i].toInt());
}

void Table::setColumnTypes(const QStringList& ctl)
{
	int n = QMIN((int)ctl.count(), numCols());
	for (int i=0; i<n; i++)
	{
		QStringList l = ctl[i].split(";");
		colTypes[i] = l[0].toInt();

		if ((int)l.count() == 2 && !l[1].isEmpty())
			col_format[i] = l[1];
		else
			col_format[i] = "0/6";
	}
}

void Table::setCommands(const QStringList& com)
{
	commands.clear();
	for(int i=0; i<(int)com.size() && i<numCols(); i++)
		commands << com[i].stripWhiteSpace();
}

void Table::setCommand(int col, const QString& com)
{
	if(col<(int)commands.size())
		commands[col] = com.stripWhiteSpace();
}

void Table::setCommands(const QString& com)
{
	QStringList lst = com.split("\t");
	lst.pop_front();
	setCommands(lst);
}

bool Table::calculate()
{
	Q3TableSelection sel = getSelection();
	bool success = true;
	for (int col=sel.leftCol(); col<=sel.rightCol(); col++)
		if (!calculate(col, sel.topRow(), sel.bottomRow()))
			success = false;
	return success;
}

bool Table::muParserCalculate(int col, int startRow, int endRow, bool notifyChanges)
{
	if (startRow < 0)
		startRow = 0;
	if (endRow >= numRows())
		resizeRows(endRow + 1);

	QString cmd = commands[col];
	if (cmd.isEmpty() || colTypes[col] != Numeric){
		for (int i = startRow; i <= endRow; i++)
			d_table->setText(i, col, cmd);
        if (notifyChanges)
            emit modifiedData(this, colName(col));
        emit modifiedWindow(this);
        return true;
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    muParserScript *mup = new muParserScript(scriptEnv, cmd, this,  QString("<%1>").arg(colName(col)));
	connect(mup, SIGNAL(error(const QString&,const QString&,int)), scriptEnv, SIGNAL(error(const QString&,const QString&,int)));
	connect(mup, SIGNAL(print(const QString&)), scriptEnv, SIGNAL(print(const QString&)));

    double *r = mup->defineVariable("i");
    mup->defineVariable("j", (double)col);
    mup->defineVariable("sr", startRow + 1.0);
    mup->defineVariable("er", endRow + 1.0);

	if (!mup->compile()){
		QApplication::restoreOverrideCursor();
		return false;
	}

    QLocale loc = locale();
    int prec;
    char f;
    columnNumericFormat(col, &f, &prec);

    if (mup->codeLines() == 1){
        for (int i = startRow; i <= endRow; i++){
            *r = i + 1.0;
            d_table->setText(i, col, mup->evalSingleLineToString(loc, f, prec));
        }
	} else {
        QVariant ret;
        for (int i = startRow; i <= endRow; i++){
            *r = i + 1.0;
            ret = mup->eval();
            if(ret.type() == QVariant::Double) {
                d_table->setText(i, col, loc.toString(ret.toDouble(), f, prec));
            } else if(ret.canConvert(QVariant::String))
                d_table->setText(i, col, ret.toString());
            else {
                QApplication::restoreOverrideCursor();
                return false;
            }
		}
	}
	if (notifyChanges)
        emit modifiedData(this, colName(col));
	emit modifiedWindow(this);
	QApplication::restoreOverrideCursor();
	return true;
}

bool Table::calculate(int col, int startRow, int endRow, bool forceMuParser, bool notifyChanges)
{
	if (col < 0 || col >= d_table->numCols())
		return false;

    if (d_table->isColumnReadOnly(col)){
        QMessageBox::warning(this, tr("QtiPlot - Error"),
        tr("Column '%1' is read only!").arg(col_label[col]));
        return false;
    }

	if (QString(scriptEnv->name()) == "muParser" || forceMuParser)
		return muParserCalculate(col, startRow, endRow, notifyChanges);

	if (startRow < 0)
		startRow = 0;
	if (endRow >= numRows())
		resizeRows(endRow + 1);

	QString cmd = commands[col];
	if (cmd.isEmpty() || colTypes[col] != Numeric){
		for (int i=startRow; i<=endRow; i++)
			d_table->setText(i, col, cmd);
        if (notifyChanges)
            emit modifiedData(this, colName(col));
        emit modifiedWindow(this);
        return true;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	Script *colscript = scriptEnv->newScript(cmd, this,  QString("<%1>").arg(colName(col)));
	connect(colscript, SIGNAL(error(const QString&,const QString&,int)), scriptEnv, SIGNAL(error(const QString&,const QString&,int)));
	connect(colscript, SIGNAL(print(const QString&)), scriptEnv, SIGNAL(print(const QString&)));

	if (!colscript->compile()){
		QApplication::restoreOverrideCursor();
		return false;
	}

    QLocale loc = locale();
    int prec;
    char f;
    columnNumericFormat(col, &f, &prec);

	colscript->setDouble(col + 1.0, "j");
	colscript->setDouble(startRow + 1.0, "sr");
	colscript->setDouble(endRow + 1.0, "er");
	QVariant ret;
	for (int i = startRow; i <= endRow; i++){
		colscript->setDouble(i + 1.0, "i");
		ret = colscript->eval();
		if(ret.type() == QVariant::Double)
			d_table->setText(i, col, loc.toString(ret.toDouble(), f, prec));
		else if(ret.canConvert(QVariant::String))
			d_table->setText(i, col, ret.toString());
		else {
			QApplication::restoreOverrideCursor();
			return false;
		}
	}
	if (notifyChanges)
        emit modifiedData(this, colName(col));
	emit modifiedWindow(this);
	QApplication::restoreOverrideCursor();
	return true;
}

void Table::updateValues(Table* t, const QString& columnName)
{
    if (!t || t != this)
        return;

    QString colLabel = columnName;
    colLabel.remove(this->objectName()).remove("_");

	int cols = numCols();
	int endRow = numRows() - 1;
    for (int i = 0; i < cols; i++){
		QString cmd = commands[i];
        if (cmd.isEmpty() || colTypes[i] != Numeric || !cmd.contains("\"" + colLabel + "\"") ||
			cmd.contains("\"" + col_label[i] + "\""))
            continue;

        calculate(i, 0, endRow, false, false);
	}
}

Q3TableSelection Table::getSelection()
{
	Q3TableSelection sel;
	if (d_table->numSelections() == 0){
		sel.init(d_table->currentRow(), d_table->currentColumn());
		sel.expandTo(d_table->currentRow(), d_table->currentColumn());
	} else if (d_table->currentSelection()>0)
		sel = d_table->selection(d_table->currentSelection());
	else
		sel = d_table->selection(0);
	return sel;
}

QString Table::saveColumnWidths()
{
        QString s="ColWidth\t";
        for (int i=0;i<d_table->numCols();i++)
                s+=QString::number(d_table->columnWidth (i))+"\t";
        return s+"\n";
}

QString Table::saveColumnTypes()
{
        QString s="ColType";
        for (int i=0; i<d_table->numCols(); i++)
                s += "\t"+QString::number(colTypes[i])+";"+col_format[i];
        return s+"\n";
}

QString Table::saveCommands()
{
        QString s="<com>\n";
        for (int col=0; col<numCols(); col++)
                if (!commands[col].isEmpty()){
                        s += "<col nr=\""+QString::number(col)+"\">\n";
                        s += commands[col];
                        s += "\n</col>\n";
                }
        s += "</com>\n";
        return s;
}

QString Table::saveComments()
{
        QString s = "Comments\t";
        for (int i=0; i<d_table->numCols(); i++){
                if (comments.count() > i)
                        s += comments[i] + "\t";
                else
                        s += "\t";
        }
        return s + "\n";
}

QString Table::saveHeader()
{
        QString s = "header";
        for (int j=0; j<d_table->numCols(); j++){
                if (col_plot_type[j] == X)
                        s += "\t" + colLabel(j) + "[X]";
                else if (col_plot_type[j] == Y)
                        s += "\t" + colLabel(j) + "[Y]";
                else if (col_plot_type[j] == Z)
                        s += "\t" + colLabel(j) + "[Z]";
                else if (col_plot_type[j] == xErr)
                        s += "\t" + colLabel(j) + "[xEr]";
                else if (col_plot_type[j] == yErr)
                        s += "\t" + colLabel(j) + "[yEr]";
                else if (col_plot_type[j] == Label)
                        s += "\t" + colLabel(j) + "[L]";
                else
                        s += "\t" + colLabel(j);
        }
        return s += "\n";
}

QString Table::saveReadOnlyInfo()
{
        QString s = "ReadOnlyColumn";
        for (int i=0; i<d_table->numCols(); i++)
                s += "\t" + QString::number(d_table->isColumnReadOnly(i));
        return s += "\n";
}

QString Table::saveHiddenColumnsInfo()
{
        QString s = "HiddenColumn";
        for (int i=0; i<d_table->numCols(); i++)
                s += "\t" + QString::number(d_table->isColumnHidden(i));
        return s += "\n";
}

void Table::save(const QString& fn, const QString& geometry, bool saveAsTemplate)
{
	QFile f(fn);
	if (!f.isOpen()){
		if (!f.open(QIODevice::Append))
			return;
	}
	QTextStream t( &f );
	t.setEncoding(QTextStream::UnicodeUTF8);
	t << "<table>";
	if (saveAsTemplate){
	    t << "\t" + QString::number(d_table->numRows()) + "\t";
        t << QString::number(d_table->numCols()) + "\n";
	} else {
	    t << "\n" + QString(objectName()) + "\t";
        t << QString::number(d_table->numRows()) + "\t";
        t << QString::number(d_table->numCols()) + "\t";
        t << birthDate() + "\n";
	}
	t << geometry;
	t << saveHeader();
	t << saveColumnWidths();
	t << saveCommands();
	t << saveColumnTypes();
	t << saveReadOnlyInfo();
	t << saveHiddenColumnsInfo();
	t << saveComments();

	if (!saveAsTemplate){
        t << "WindowLabel\t" + windowLabel() + "\t" + QString::number(captionPolicy()) + "\n";
        t << "<data>\n";
		int cols = d_table->numCols() - 1;
		int rows = d_table->numRows();
		for (int i=0; i<rows; i++){
			if (!isEmptyRow(i)){
				t << QString::number(i) + "\t";
				for (int j=0; j<cols; j++){
			    	if (colTypes[j] == Numeric && !d_table->text(i, j).isEmpty())
                    	t << QString::number(cell(i, j), 'e', 14) + "\t";
					else
						t << d_table->text(i, j) + "\t";
				}
            	if (colTypes[cols] == Numeric && !d_table->text(i, cols).isEmpty())
                	t << QString::number(cell(i, cols), 'e', 14) + "\n";
				else
					t << d_table->text(i, cols) + "\n";
			}
		}
		t << "</data>\n";
	}
	t << "</table>\n";
}

int Table::firstXCol()
{
	int xcol = -1;
	for (int j=0; j<d_table->numCols(); j++)
	{
		if (col_plot_type[j] == X)
			return j;
	}
	return xcol;
}

QString Table::comment(int col)
{
	if (col < 0 || col >= d_table->numCols())
		return QString::null;
	
	return comments[col];
}

void Table::setColComment(int col, const QString& s)
{
	if (col < 0 || col >= d_table->numCols())
		return;

	if (comments[col] == s)
		return;

	comments[col] = s;

	if (d_show_comments)
		setHeaderColType();
}

void Table::setColumnWidth(int width, bool allCols)
{
	int cols=d_table->numCols();
	if (allCols)
	{
		for (int i=0;i<cols;i++)
			d_table->setColumnWidth (i, width);

		emit modifiedWindow(this);
	}
	else
	{
		if (d_table->columnWidth(selectedCol) == width)
			return;

		d_table->setColumnWidth (selectedCol, width);
		emit modifiedWindow(this);
	}
}

void Table::setColumnWidth(int col, int width)
{

	if (d_table->columnWidth(col) == width)
		return;

	d_table->setColumnWidth (col, width);
	emit modifiedWindow(this);
}

QString Table::colName(int col)
{//returns the table name + horizontal header text
	if (col<0 || col >= col_label.count())
		return QString();

	return QString(this->objectName())+"_"+col_label[col];
}

void Table::setColName(int col, const QString& text, bool enumerateRight)
{
    if (text.isEmpty() || col<0 || col >= d_table->numCols())
		return;

    if (col_label[col] == text && !enumerateRight)
        return;

	QString caption = objectName();
	QString oldLabel = col_label[col];
	int cols = col + 1;
	if (enumerateRight)
        cols = (int)d_table->numCols();

    int n = 1;
	for (int i = col; i<cols; i++){
		QString newLabel = text;
		if (enumerateRight)
            newLabel += QString::number(n);

        if (col_label.contains(newLabel) > 0){
            QMessageBox::critical(0,tr("QtiPlot - Error"),
            tr("There is already a column called : <b>"+newLabel+"</b> in table <b>"+caption+"</b>!<p>Please choose another name!"));
            return;
        }
		n++;
	}

    n = 1;
    caption += "_";
	for (int i = col; i<cols; i++){
		QString newLabel = text;
		if (enumerateRight)
            newLabel += QString::number(n);

        commands.replaceInStrings("\"" + col_label[i] + "\"", "\"" + newLabel + "\"");
		emit changedColHeader(caption + col_label[i], caption + newLabel);
        col_label[i] = newLabel;
		n++;
	}

    setHeaderColType();
	emit modifiedWindow(this);
}

QStringList Table::selectedColumns()
{
	QStringList names;
	for (int i=0; i<d_table->numCols(); i++){
		if(d_table->isColumnSelected (i, true))
			names << QString(name()) + "_" + col_label[i];
	}
	return names;
}

QStringList Table::YColumns()
{
	QStringList names;
	for (int i=0;i<d_table->numCols();i++)
	{
		if(col_plot_type[i] == Y)
			names<<QString(name())+"_"+col_label[i];
	}
	return names;
}

QStringList Table::selectedYColumns()
{
	QStringList names;
	for (int i=0;i<d_table->numCols();i++)
	{
	if(d_table->isColumnSelected (i) && col_plot_type[i] == Y)
  		names<<QString(name())+"_"+col_label[i];
  	}
  	return names;
}

QStringList Table::selectedErrColumns()
{
  	QStringList names;
  	for (int i=0;i<d_table->numCols();i++)
  		{
  	    if(d_table->isColumnSelected (i,true) &&
  	       (col_plot_type[i] == yErr || col_plot_type[i] == xErr))
  	       	names<<QString(objectName())+"_"+col_label[i];
  	    }
  	return names;
}

QStringList Table::drawableColumnSelection()
{
  	QStringList names;
  	for (int i=0; i<d_table->numCols(); i++)
  	{
	if(d_table->isColumnSelected (i) && col_plot_type[i] == Y)
		names << QString(objectName()) + "_" + col_label[i];
    }

  	for (int i=0; i<d_table->numCols(); i++)
  	{
  	 	if(d_table->isColumnSelected (i) &&
			(col_plot_type[i] == yErr || col_plot_type[i] == xErr || col_plot_type[i] == Label))
  	    	names << QString(objectName()) + "_" + col_label[i];
  	}
	return names;
}

QStringList Table::selectedYLabels()
{
	QStringList names;
	for (int i=0;i<d_table->numCols();i++)
	{
		if(d_table->isColumnSelected (i) && col_plot_type[i] == Y)
			names<<col_label[i];
	}
	return names;
}

QStringList Table::columnsList()
{
	QStringList names;
	for (int i=0;i<d_table->numCols();i++)
		names<<QString(objectName())+"_"+col_label[i];

	return names;
}

int Table::firstSelectedColumn()
{
	for (int i=0;i<d_table->numCols();i++)
	{
		if(d_table->isColumnSelected (i,true))
			return i;
	}
	return -1;
}

int Table::numSelectedRows()
{
	int r=0;
	for (int i=0;i<d_table->numRows();i++)
	{
		if(d_table->isRowSelected (i,true))
			r++;
	}
	return r;
}

int Table::selectedColsNumber()
{
	int c=0;
	for (int i=0;i<d_table->numCols(); i++){
		if(d_table->isColumnSelected (i,true))
			c++;
	}
	return c;
}

QVarLengthArray<double> Table::col(int c)
{
	int rows = d_table->numRows();
	QVarLengthArray<double> Y(rows);
	if (c >= 0 && c <= d_table->numCols()){
        char format;
        int prec;
        columnNumericFormat(c, &format, &prec);
        QLocale l = locale();
		for (int i=0; i<rows; i++)
			Y[i] = l.toDouble(d_table->text(i, c));
	}
	return Y;
}

void Table::insertCols(int start, int count)
{
    if (start < 0)
        start = 0;

	int max = 0;
	for (int i = 0; i<d_table->numCols(); i++){
		if (!col_label[i].contains(QRegExp ("\\D"))){
			int id = col_label[i].toInt();
			if (id > max)
				max = id;
		}
	}
    max++;

    d_table->insertColumns(start, count);

	for(int i = 0; i<count; i++ ){
        int j = start + i;
		commands.insert(j, QString());
		col_format.insert(j, "0/" + QString::number(d_numeric_precision));
		comments.insert(j, QString());
		col_label.insert(j, QString::number(max + i));
		colTypes.insert(j, Numeric);
		col_plot_type.insert(j, Y);
	}
	setHeaderColType();
	emit modifiedWindow(this);
}

void Table::insertCol()
{
	insertCols(selectedCol, 1);
}

void Table::insertRow()
{
	int cr = d_table->currentRow();
	if (d_table->isRowSelected (cr, true))
	{
		d_table->insertRows(cr, 1);
		emit modifiedWindow(this);
	}
}

void Table::addCol(PlotDesignation pd)
{
	d_table->clearSelection();
	int index, max=0, cols=d_table->numCols();
	for (int i=0; i<cols; i++){
		if (!col_label[i].contains(QRegExp ("\\D"))){
			index = col_label[i].toInt();
			if (index > max)
				max = index;
		}
	}
	d_table->insertColumns(cols);
	d_table->ensureCellVisible ( 0, cols );

	comments << QString();
	commands << "";
	colTypes << Numeric;
	col_format << "0/" + QString::number(d_numeric_precision);
	col_label << QString::number(max+1);
	col_plot_type << pd;

	setHeaderColType();
	emit modifiedWindow(this);
}

void Table::addColumns(int c)
{
	int max=0, cols=d_table->numCols();
	for (int i=0; i<cols; i++){
		if (!col_label[i].contains(QRegExp ("\\D"))){
			int index=col_label[i].toInt();
			if (index>max)
				max=index;
		}
	}
	max++;
	d_table->insertColumns(cols, c);
	for (int i=0; i<c; i++){
		comments << QString();
		commands<<"";
		colTypes<<Numeric;
		col_format<<"0/" + QString::number(d_numeric_precision);
		col_label<< QString::number(max+i);
		col_plot_type << Y;
	}
}

void Table::clearCol()
{
	if (d_table->isColumnReadOnly(selectedCol))
		return;

	for (int i=0; i<d_table->numRows(); i++){
		if (d_table->isSelected (i, selectedCol))
			d_table->setText(i, selectedCol, "");
	}

	emit modifiedData(this, colName(selectedCol));
}

void Table::clearCell(int row, int col)
{
	if (col < 0 || col >= d_table->numCols())
        return;

	if (d_table->isColumnReadOnly(col)){
		QMessageBox::warning(this, tr("QtiPlot - Error"), tr("Column '%1' is read only!").arg(colName(col)));
        return;
	}

	d_table->setText(row, col, "");

	emit modifiedData(this, colName(col));
	emit modifiedWindow(this);
}

void Table::deleteSelectedRows()
{
	Q3TableSelection sel = d_table->selection(0);
	deleteRows(sel.topRow() + 1, sel.bottomRow() + 1);
}

void Table::deleteRows(int startRow, int endRow)
{
	for(int i=0; i<d_table->numCols(); i++){
        if (d_table->isColumnReadOnly(i)){
			QMessageBox::warning(this, tr("QtiPlot - Error"),
        	tr("The table '%1' contains read-only columns! Operation aborted!").arg(objectName()));
			return;
		}
	}

    int start = QMIN(startRow, endRow);
    int end = QMAX(startRow, endRow);

    start--;
    end--;
    if (start < 0)
        start = 0;
    if (end >= d_table->numRows())
        end = d_table->numRows() - 1;

	int rows = abs(end - start) + 1;
	Q3MemArray<int> rowsToDelete(rows);
	for (int i=0; i<rows; i++)
		rowsToDelete[i] = start + i;

	d_table->removeRows(rowsToDelete);
	notifyChanges();
}

void Table::cutSelection()
{
	copySelection();
	clearSelection();
}

void Table::selectAllTable()
{
	d_table->addSelection (Q3TableSelection( 0, 0, d_table->numRows(), d_table->numCols() ));
}

void Table::deselect()
{
	d_table->clearSelection();
}

void Table::clearSelection()
{
	QStringList list=selectedColumns();
	int n = int(list.count());

	if (n>0){
		QStringList lstReadOnly;
		for (int i=0; i<list.count(); i++){
			QString name = list[i];
			int col = colIndex(name);
			if (d_table->isColumnReadOnly(col))
				lstReadOnly << name;
		}
		if (lstReadOnly.count() > 0){
			QMessageBox::warning(this, tr("QtiPlot - Error"),
        	tr("The folowing columns")+":\n"+ lstReadOnly.join("\n") + "\n"+ tr("are read only!"));
    	}

		for (int i=0; i<n; i++){
			QString name = list[i];
			selectedCol = colIndex(name);
			clearCol();
		}
	} else {
		Q3TableSelection sel=d_table->selection(0);
		int top=sel.topRow();
		int bottom=sel.bottomRow();
		int left=sel.leftCol();
		int right=sel.rightCol();

		if (sel.isEmpty ()){
			int col = d_table->currentColumn();
			QString name = colName(col);
			if (d_table->isColumnReadOnly(col)){
				QMessageBox::warning(this, tr("QtiPlot - Error"),
       			tr("Column '%1' is read only!").arg(name));
				return;
    		}
			d_table->setText(d_table->currentRow (), col, "");
			emit modifiedData(this, name);
		} else {
			QStringList lstReadOnly;
			for (int i=left; i<=right; i++){
				QString name = col_label[i];
				if (d_table->isColumnReadOnly(i))
					lstReadOnly << name;
			}
			if (lstReadOnly.count() > 0){
				QMessageBox::warning(this, tr("QtiPlot - Error"),
        		tr("The folowing columns")+":\n"+ lstReadOnly.join("\n") + "\n"+ tr("are read only!"));
    		}

			for (int i=left; i<=right; i++){
				if (d_table->isColumnReadOnly(i))
					continue;

				for (int j=top; j<=bottom; j++)
					d_table->setText(j, i, "");

				QString name = colName(i);
				emit modifiedData(this, name);
			}
		}
	}
	emit modifiedWindow(this);
}

void Table::copySelection()
{
	QString text;
	int rows = d_table->numRows();
	int cols = d_table->numCols();
	QString eol = applicationWindow()->endOfLine();

	QVarLengthArray<int> selection(1);
	int c = 0;
	for (int i = 0; i<cols; i++){
		if (d_table->isColumnSelected(i, true)){
			c++;
			selection.resize(c);
			selection[c-1] = i;
		}
	}
	if (c > 0){
		for (int i = 0; i<rows; i++){
			for (int j = 0; j<c-1; j++)
				text += d_table->text(i, selection[j]) + "\t";
			text += d_table->text(i, selection[c-1]) + eol;
		}
	} else {
		Q3TableSelection sel = d_table->selection(0);
		int right = sel.rightCol();
		int bottom = sel.bottomRow();
		for (int i = sel.topRow(); i<bottom; i++){
			for (int j = sel.leftCol(); j<right; j++)
				text += d_table->text(i, j) + "\t";
			text += d_table->text(i, right) + eol;
		}
		for (int j = sel.leftCol(); j<right; j++)
				text += d_table->text(bottom, j) + "\t";
			text += d_table->text(bottom, right);
	}

	// Copy text into the clipboard
	QApplication::clipboard()->setText(text);
}

// Paste text from the clipboard
void Table::pasteSelection()
{
	QString text = QApplication::clipboard()->text();
	if (text.isEmpty())
		return;

	QStringList linesList = text.split(applicationWindow()->endOfLine());
	int rows = linesList.size();
	if (rows < 1)
		return;

	int cols = linesList[0].split("\t").count();
	for (int i = 1; i < rows; i++){
		int aux = linesList[i].split("\t").count();
		if (aux > cols)
            cols = aux;
	}

	int top, left, firstCol = firstSelectedColumn();
	Q3TableSelection sel = d_table->selection(0);
	if (!sel.isEmpty()){// not columns but only cells are selected
		top = sel.topRow();
		left = sel.leftCol();
	} else if(cols == 1 && rows == 1){
		top = d_table->currentRow();
		left = d_table->currentColumn();
	} else {
		top = 0;
		left = 0;
		if (firstCol >= 0)// columns are selected
			left = firstCol;
	}

    if (top + rows > d_table->numRows())
        d_table->setNumRows(top + rows);
    if (left + cols > d_table->numCols()){
        addColumns(left + cols - d_table->numCols());
        setHeaderColType();
    }

	QStringList lstReadOnly;
	for (int i = left; i < left + cols; i++){
		QString name = colName(i);
		if (d_table->isColumnReadOnly(i))
			lstReadOnly << name;
	}
	if (lstReadOnly.count() > 0){
		QMessageBox::warning(this, tr("QtiPlot - Error"),
        tr("The folowing columns")+":\n"+ lstReadOnly.join("\n") + "\n"+ tr("are read only!"));
    }

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QLocale l = locale();
	QLocale clipboardLocale = applicationWindow()->clipboardLocale();
	for (int i=0; i < rows; i++){
		int row = top + i;
		QStringList cells = linesList[i].split("\t");
		for (int j = left; j< left + cols; j++){
			if (d_table->isColumnReadOnly(j))
				continue;

            int colIndex = j-left;
            if (colIndex >= cells.count())
                break;

			bool numeric = false;
			double value = clipboardLocale.toDouble(cells[colIndex], &numeric);
			if (numeric){
			    int prec;
                char f;
				columnNumericFormat(j, &f, &prec);
				d_table->setText(row, j, l.toString(value, f, prec));
			} else
				d_table->setText(row, j, cells[colIndex]);
		}
	}

	for (int i = left; i< left + cols; i++){
		if (!d_table->isColumnReadOnly(i))
			emit modifiedData(this, colName(i));
	}
	emit modifiedWindow(this);
	QApplication::restoreOverrideCursor();
}

void Table::removeCol()
{
	QStringList list=selectedColumns();
	removeCol(list);
}

void Table::removeCol(const QStringList& list)
{
	QStringList lstReadOnly;
	for (int i=0; i<list.count(); i++){
		QString name = list[i];
		int col = colIndex(name);
		if (d_table->isColumnReadOnly(col))
			lstReadOnly << name;
	}

	if (lstReadOnly.count() > 0){
		QMessageBox::warning(this, tr("QtiPlot - Error"),
        tr("The folowing columns")+":\n"+ lstReadOnly.join("\n") + "\n"+ tr("are read only!"));
    }

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	for (int i=0; i<list.count(); i++){
		QString name = list[i];
		int id = colIndex(name);
		if (id >= 0){
			if (d_table->isColumnReadOnly(id))
				continue;

			if ( id < commands.size() )
				commands.removeAt(id);

			if ( id < col_label.size() )
				col_label.removeAt(id);

			if ( id < col_format.size() )
				col_format.removeAt(id);

			if ( id < comments.size() )
				comments.removeAt(id);

			if ( id < colTypes.size() )
				colTypes.removeAt(id);

			if ( id < col_plot_type.size() )
				col_plot_type.removeAt(id);

			d_table->removeColumn(id);
			emit removedCol(name);
		}
	}
	emit modifiedWindow(this);
	QApplication::restoreOverrideCursor();
}

void Table::normalizeSelection()
{
	QStringList s = selectedColumns();
	QStringList lstReadOnly;
	for (int i=0; i<(int)s.count(); i++){
		int col = colIndex(s[i]);
		if (d_table->isColumnReadOnly(col))
			lstReadOnly << col_label[col];
	}

	if (lstReadOnly.count() > 0){
		QMessageBox::warning(this, tr("QtiPlot - Error"),
        tr("The folowing columns")+":\n"+ lstReadOnly.join("\n") + "\n"+ tr("are read only!"));
    }

	for (int i=0; i<(int)s.count(); i++)
		normalizeCol(colIndex(s[i]));

	emit modifiedWindow(this);
}

void Table::normalize()
{
	QStringList lstReadOnly;
	for (int i=0; i<d_table->numCols(); i++){
		if (d_table->isColumnReadOnly(i))
			lstReadOnly << col_label[i];
	}

	if (lstReadOnly.count() > 0){
		QMessageBox::warning(this, tr("QtiPlot - Error"),
        tr("The folowing columns")+":\n"+ lstReadOnly.join("\n") + "\n"+ tr("are read only!"));
    }

	for (int i=0; i<d_table->numCols(); i++)
		normalizeCol(i);

	emit modifiedWindow(this);
}

void Table::normalizeCol(int col)
{
	if (col<0) col = selectedCol;
	if (d_table->isColumnReadOnly(col) || colTypes[col] == Table::Text)
	    return;

	int rows = d_table->numRows();
	gsl_vector *data = gsl_vector_alloc(rows);
	for (int i=0; i<rows; i++)
		gsl_vector_set(data, i, cell(i, col));

	double max = gsl_vector_max(data);
	if (max == 1.0)
		return;

    int prec;
    char f;
    columnNumericFormat(col, &f, &prec);

	for (int i=0; i<rows; i++){
		if ( !d_table->text(i, col).isEmpty() )
			d_table->setText(i, col, locale().toString(gsl_vector_get(data, i)/max, f, prec));
	}

	gsl_vector_free(data);
	emit modifiedData(this, colName(col));
}

void Table::sortColumnsDialog()
{
	SortDialog *sortd = new SortDialog((QWidget*)applicationWindow());
	sortd->setAttribute(Qt::WA_DeleteOnClose);
	connect (sortd, SIGNAL(sort(int, int, const QString&)), this, SLOT(sortColumns(int, int, const QString&)));
	sortd->insertColumnsList(selectedColumns());
	sortd->exec();
}

void Table::sortTableDialog()
{
	SortDialog *sortd = new SortDialog((QWidget*)applicationWindow());
	sortd->setAttribute(Qt::WA_DeleteOnClose);
	connect (sortd, SIGNAL(sort(int, int, const QString&)), this, SLOT(sort(int, int, const QString&)));
	sortd->insertColumnsList(colNames());
	sortd->exec();
}

void Table::sort(int type, int order, const QString& leadCol)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	sortColumns(col_label, type, order, leadCol);
    QApplication::restoreOverrideCursor();
}

void Table::sortColumns(int type, int order, const QString& leadCol)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	sortColumns(selectedColumns(), type, order, leadCol);
    QApplication::restoreOverrideCursor();
}

void Table::sortColumns(const QStringList&s, int type, int order, const QString& leadCol)
{
	int cols=s.count();
	if(!type){//Sort columns separately
		for(int i=0;i<cols;i++)
			sortColumn(colIndex(s[i]), order);
	}else{
		int leadcol = colIndex(leadCol);
		if (leadcol < 0){
			QMessageBox::critical(this, tr("QtiPlot - Error"),
			tr("Please indicate the name of the leading column!"));
			return;
		}
		if (columnType(leadcol) == Table::Text){
			QMessageBox::critical(this, tr("QtiPlot - Error"),
			tr("The leading column has the type set to 'Text'! Operation aborted!"));
			return;
		}

		int rows=d_table->numRows();
		int non_empty_cells = 0;
		QVarLengthArray<int> valid_cell(rows);
		QVarLengthArray<double> data_double(rows);
		for (int j = 0; j <rows; j++){
			if (!d_table->text(j, leadcol).isEmpty()){
				data_double[non_empty_cells] = cell(j, leadcol);
				valid_cell[non_empty_cells] = j;
				non_empty_cells++;
			}
		}

		if (!non_empty_cells){
			QMessageBox::critical(this, tr("QtiPlot - Error"),
			tr("The leading column is empty! Operation aborted!"));
			return;
		}

		data_double.resize(non_empty_cells);
		valid_cell.resize(non_empty_cells);
		QVarLengthArray<QString> data_string(non_empty_cells);
		size_t *p= new size_t[non_empty_cells];

		// Find the permutation index for the lead col
		gsl_sort_index(p, data_double.data(), 1, non_empty_cells);

		for(int i=0;i<cols;i++){// Since we have the permutation index, sort all the columns
            int col=colIndex(s[i]);
            if (d_table->isColumnReadOnly(col))
                continue;

            if (columnType(col) == Text){
                for (int j=0; j<non_empty_cells; j++)
                    data_string[j] = text(valid_cell[j], col);
                if(!order)
                    for (int j=0; j<non_empty_cells; j++)
                        d_table->setText(valid_cell[j], col, data_string[p[j]]);
                else
                    for (int j=0; j<non_empty_cells; j++)
                        d_table->setText(valid_cell[j], col, data_string[p[non_empty_cells-j-1]]);
            }else{
                for (int j = 0; j<non_empty_cells; j++)
                    data_double[j] = cell(valid_cell[j], col);
                int prec;
                char f;
                columnNumericFormat(col, &f, &prec);
                if(!order)
                    for (int j=0; j<non_empty_cells; j++)
                        d_table->setText(valid_cell[j], col, locale().toString(data_double[p[j]], f, prec));
                else
                    for (int j=0; j<non_empty_cells; j++)
                        d_table->setText(valid_cell[j], col, locale().toString(data_double[p[non_empty_cells-j-1]], f, prec));
            }
            emit modifiedData(this, colName(col));
        }
        delete[] p;
    }
	emit modifiedWindow(this);
}

void Table::sortColumn(int col, int order)
{
	if (col < 0)
		col = d_table->currentColumn();

    if (d_table->isColumnReadOnly(col))
        return;

	int rows=d_table->numRows();
	int non_empty_cells = 0;
	QVarLengthArray<int> valid_cell(rows);
	QVarLengthArray<double> r(rows);
    QStringList text_cells;
	for (int i = 0; i <rows; i++){
		if (!d_table->text(i, col).isEmpty()){
            if (columnType(col) == Table::Text)
                text_cells << d_table->text(i, col);
            else
			    r[non_empty_cells] = cell(i, col);
			valid_cell[non_empty_cells] = i;
			non_empty_cells++;
		}
	}

	if (!non_empty_cells)
		return;

	valid_cell.resize(non_empty_cells);
    if (columnType(col) == Table::Text){
        r.clear();
        text_cells.sort();
    } else {
        r.resize(non_empty_cells);
        gsl_sort(r.data(), 1, non_empty_cells);
    }

    if (columnType(col) == Table::Text){
        if (!order){
            for (int i=0; i<non_empty_cells; i++)
                d_table->setText(valid_cell[i], col, text_cells[i]);
        } else {
            for (int i=0; i<non_empty_cells; i++)
                d_table->setText(valid_cell[i], col, text_cells[non_empty_cells-i-1]);
        }
    } else {
	   int prec;
	   char f;
	   columnNumericFormat(col, &f, &prec);
       if (!order) {
	       for (int i=0; i<non_empty_cells; i++)
                d_table->setText(valid_cell[i], col, locale().toString(r[i], f, prec));
        } else {
            for (int i=0; i<non_empty_cells; i++)
                d_table->setText(valid_cell[i], col, locale().toString(r[non_empty_cells-i-1], f, prec));
        }
    }
	emit modifiedData(this, colName(col));
	emit modifiedWindow(this);
}

void Table::sortColAsc()
{
sortColumn(d_table->currentColumn ());
}

void Table::sortColDesc()
{
sortColumn(d_table->currentColumn(), 1);
}

int Table::numRows()
{
	return d_table->numRows();
}

int Table::numCols()
{
	return d_table->numCols();
}

bool Table::isEmptyRow(int row)
{
	for (int i=0; i<d_table->numCols(); i++)
	{
		QString text = d_table->text(row,i);
		if (!text.isEmpty())
			return false;
	}
	return true;
}

bool Table::isEmptyColumn(int col)
{
	for (int i=0; i<d_table->numRows(); i++)
	{
		QString text=d_table->text(i,col);
		if (!text.isEmpty())
			return false;
	}
	return true;
}

int Table::nonEmptyRows()
{
	int r=0;
	for (int i=0;i<d_table->numRows();i++){
		if (!isEmptyRow(i))
			r++;
	}
	return r;
}

double Table::cell(int row, int col)
{
	return locale().toDouble(d_table->text(row, col));
}

void Table::setCell(int row, int col, double val)
{
	char format;
    int prec;
    columnNumericFormat(col, &format, &prec);
    d_table->setText(row, col, locale().toString(val, format, prec));
}

QString Table::text(int row, int col)
{
	return d_table->text(row, col);
}

void Table::setText (int row, int col, const QString & text )
{
	d_table->setText(row, col, text);
}

void Table::saveToMemory()
{
    int rows = d_table->numRows();
    int cols = d_table->numCols();

    d_saved_cells = new double* [cols];
	for ( int i = 0; i < cols; ++i)
		d_saved_cells[i] = new double [rows];

    for (int col = 0; col < cols; col++){// initialize the matrix to zero
        for (int row = 0; row < rows; row++)
            d_saved_cells[col][row] = 0.0;}

	for (int col = 0; col < cols; col++){
		if (colTypes[col] == Time){
			QTime ref = QTime(0, 0);
			for (int row = 0; row <rows; row++){
				QTime t = QTime::fromString(d_table->text(row, col), col_format[col]);
                d_saved_cells[col][row] = ref.msecsTo(t);
				}
            }
		else if (colTypes[col] == Date){
			QTime ref = QTime(0, 0);
			for (int row = 0; row < rows; row++){
				QDateTime dt = QDateTime::fromString(d_table->text(row, col), col_format[col]);
				d_saved_cells[col][row] = dt.date().toJulianDay() - 1 + (double)ref.msecsTo(dt.time())/864.0e5;
				}
            }
        }

    QLocale l = locale();
	for (int col = 0; col < cols; col++){
	    if (colTypes[col] == Numeric){
            for (int row=0; row < rows; row++){
                QString s = d_table->text(row, col);
                if (!s.isEmpty())
                    d_saved_cells[col][row] = l.toDouble(s);
            }
	    }
	}
}

void Table::freeMemory()
{
    for ( int i = 0; i < d_table->numCols(); i++)
        delete[] d_saved_cells[i];

	delete[] d_saved_cells;
	d_saved_cells = 0;
}

void Table::setTextFormat(int col)
{
	if (col >= 0 && col < colTypes.count())
		colTypes[col] = Text;
}

void Table::setColNumericFormat(int f, int prec, int col, bool updateCells)
{
	if (colTypes[col] == Numeric){
		int old_f, old_prec;
		columnNumericFormat(col, &old_f, &old_prec);
		if (old_f == f && old_prec == prec)
			return;
	}

	colTypes[col] = Numeric;
	col_format[col] = QString::number(f)+"/"+QString::number(prec);

    if (!updateCells)
        return;

    char format = 'g';
	for (int i=0; i<d_table->numRows(); i++) {
		QString t = text(i, col);
		if (!t.isEmpty()) {
			if (!f)
				prec = 6;
			else if (f == 1)
                format = 'f';
			else if (f == 2)
                format = 'e';

			if (d_saved_cells)
				setText(i, col, locale().toString(d_saved_cells[col][i], format, prec));
			else
				setText(i, col, locale().toString(locale().toDouble(t), format, prec));
		}
	}
}

void Table::setColumnsFormat(const QStringList& lst)
{
	if (col_format == lst)
		return;

	col_format = lst;
}

bool Table::setDateFormat(const QString& format, int col, bool updateCells)
{
	if (colTypes[col] == Date && col_format[col] == format)
		return true;

	bool first_time = false;
    if (updateCells){
	for (int i=0; i<d_table->numRows(); i++){
		QString s = d_table->text(i,col);
		if (!s.isEmpty()){
		    QDateTime d = QDateTime::fromString (s, format);
			if (colTypes[col] != Date && d.isValid()){
			//This might be the first time the user assigns a date format.
            //If Qt understands the format we break the loop, assign it to the column and return true!
				first_time = true;
                break;
			}

		    if (d_saved_cells){
                d = QDateTime(QDate::fromJulianDay((int)d_saved_cells[col][i] + 1));
                double msecs = (d_saved_cells[col][i] - floor(d_saved_cells[col][i]))*864e5;
                d.setTime(d.time().addMSecs(qRound(msecs)));
				if (d.isValid())
					d_table->setText(i, col, d.toString(format));
		    }
		}
	}
    }
	colTypes[col] = Date;
	col_format[col] = format;
	QTime ref = QTime(0, 0);
	if (first_time){//update d_saved_cells in case the user changes the time format before pressing OK in the column dialog
		for (int i=0; i<d_table->numRows(); i++){
			QDateTime dt = QDateTime::fromString(d_table->text(i, col), format);
			d_saved_cells[col][i] = dt.date().toJulianDay() - 1 + (double)ref.msecsTo(dt.time())/864.0e5;
		}
	}
	return true;
}

bool Table::setTimeFormat(const QString& format, int col, bool updateCells)
{
	if (colTypes[col] == Time && col_format[col] == format)
		return true;

	QTime ref = QTime(0, 0);
	bool first_time = false;
    if (updateCells){
	for (int i=0; i<d_table->numRows(); i++){
		QString s = d_table->text(i,col);
		if (!s.isEmpty()){
			QTime t = QTime::fromString (s, format);
			if (colTypes[col] != Time && t.isValid()){
			//This is the first time the user assigns a time format.
            //If Qt understands the format we break the loop, assign it to the column and return true!
            	first_time = true;
				break;
			}

		    if (d_saved_cells){
				if (d_saved_cells[col][i] < 1)// import of Origin files
                	t = ref.addMSecs(int(d_saved_cells[col][i]*86400000));
				else
					t = ref.addMSecs((int)d_saved_cells[col][i]);

				if (t.isValid())
					d_table->setText(i, col, t.toString(format));
			}
		}
	}
    }
	colTypes[col] = Time;
	col_format[col] = format;
	if (first_time){//update d_saved_cells in case the user changes the time format before pressing OK in the column dialog
		for (int i=0; i<d_table->numRows(); i++){
			QTime t = QTime::fromString(d_table->text(i, col), format);
			d_saved_cells[col][i] = ref.msecsTo(t);
		}
	}
	return true;
}

void Table::setMonthFormat(const QString& format, int col, bool updateCells)
{
    if (colTypes[col] == Month && col_format[col] == format)
        return;

	colTypes[col] = Month;
	col_format[col] = format;

	if (!updateCells)
        return;

    for (int i=0; i<numRows(); i++){
        QString t = d_table->text(i,col);
        if (!t.isEmpty()){
            int day;
            if (d_saved_cells)
                day = int(d_saved_cells[col][i]) % 12;
            else
                day = t.toInt() % 12;
            if (!day)
                day = 12;

            if (format == "M")
                d_table->setText(i, col, QDate::shortMonthName(day).left(1));
            else if (format == "MMM")
                d_table->setText(i, col, QDate::shortMonthName(day));
            else if (format == "MMMM")
                d_table->setText(i, col, QDate::longMonthName(day));
        }
    }
}

void Table::setDayFormat(const QString& format, int col, bool updateCells)
{
    if (colTypes[col] == Day && col_format[col] == format)
        return;

	colTypes[col] = Day;
	col_format[col] = format;

	if (!updateCells)
        return;

	for (int i=0; i<numRows(); i++){
        QString t = d_table->text(i,col);
        if (!t.isEmpty()){
            int day;
            if (d_saved_cells)
                day = int(d_saved_cells[col][i]) % 7;
            else
                day = t.toInt() % 7;
            if (!day)
                day = 7;

            if (format == "d")
                d_table->setText(i, col, QDate::shortDayName(day).left(1));
            else if (format == "ddd")
                d_table->setText(i, col, QDate::shortDayName(day));
            else if (format == "dddd")
                d_table->setText(i, col, QDate::longDayName(day));
        }
    }
}

void Table::setRandomValues()
{
	QStringList list=selectedColumns();
	QStringList lstReadOnly;
	for (int i=0; i<list.count(); i++){
		QString name = list[i];
		int col = colIndex(name);
		if (d_table->isColumnReadOnly(col))
			lstReadOnly << name;
	}

	if (lstReadOnly.count() > 0){
		QMessageBox::warning(this, tr("QtiPlot - Error"),
        tr("The folowing columns")+":\n"+ lstReadOnly.join("\n") + "\n"+ tr("are read only!"));
    }

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    time_t tmp;
    srand(time(&tmp));
	int rows=d_table->numRows();
	for (int j=0; j<(int) list.count(); j++){
		QString name=list[j];
		selectedCol=colIndex(name);
        if (d_table->isColumnReadOnly(selectedCol))
            continue;

		int prec;
		char f;
		columnNumericFormat(selectedCol, &f, &prec);

		for (int i=0; i<rows; i++)
			d_table->setText(i, selectedCol, locale().toString(double(rand())/double(RAND_MAX), f, prec));

		emit modifiedData(this, name);
	}

	emit modifiedWindow(this);
	QApplication::restoreOverrideCursor();
}

void Table::loadHeader(QStringList header)
{
	col_label = QStringList();
	col_plot_type = QList <int>();
	for (int i=0; i<header.count();i++)
	{
		if (header[i].isEmpty())
			continue;

		QString s = header[i].replace("_","-");
		if (s.contains("[X]"))
		{
			col_label << s.remove("[X]");
			col_plot_type << X;
		}
		else if (s.contains("[Y]"))
		{
			col_label << s.remove("[Y]");
			col_plot_type << Y;
		}
		else if (s.contains("[Z]"))
		{
			col_label << s.remove("[Z]");
			col_plot_type << Z;
		}
		else if (s.contains("[xEr]"))
		{
			col_label << s.remove("[xEr]");
			col_plot_type << xErr;
		}
		else if (s.contains("[yEr]"))
		{
			col_label << s.remove("[yEr]");
			col_plot_type << yErr;
		}
		else if (s.contains("[L]"))
		{
			col_label << s.remove("[L]");
			col_plot_type << Label;
		}
		else
		{
			col_label << s;
			col_plot_type << None;
		}
	}
	setHeaderColType();
}

void Table::setHeader(QStringList header)
{
	col_label = header;
	setHeaderColType();
}

int Table::colIndex(const QString& name)
{
	int pos = name.find("_",false);
	QString label = name.right(name.length()-pos-1);
	return col_label.findIndex(label);
}

void Table::setHeaderColType()
{
	int xcols = 0;
	for (int j=0; j < d_table->numCols(); j++){
		if (col_plot_type[j] == X)
			xcols++;
	}

	if (xcols > 1){
		xcols = 0;
		for (int i=0; i < d_table->numCols(); i++){
			if (col_plot_type[i] == X)
				setColumnHeader(i, col_label[i]+"[X" + QString::number(++xcols) +"]");
			else if (col_plot_type[i] == Y){
				if(xcols>0)
					setColumnHeader(i, col_label[i]+"[Y"+ QString::number(xcols) +"]");
				else
					setColumnHeader(i, col_label[i]+"[Y]");
			} else if (col_plot_type[i] == Z){
				if(xcols>0)
					setColumnHeader(i, col_label[i]+"[Z"+ QString::number(xcols) +"]");
				else
					setColumnHeader(i, col_label[i]+"[Z]");
			} else if (col_plot_type[i] == xErr)
				setColumnHeader(i, col_label[i]+"[xEr]");
			else if (col_plot_type[i] == yErr)
				setColumnHeader(i, col_label[i]+"[yEr]");
			else if (col_plot_type[i] == Label)
				setColumnHeader(i, col_label[i]+"[L]");
			else
				setColumnHeader(i, col_label[i]);
		}
	} else {
		for (int i=0; i < d_table->numCols(); i++){
			if (col_plot_type[i] == X)
				setColumnHeader(i, col_label[i]+"[X]");
			else if (col_plot_type[i] == Y)
				setColumnHeader(i, col_label[i]+"[Y]");
			else if (col_plot_type[i] == Z)
				setColumnHeader(i, col_label[i]+"[Z]");
			else if (col_plot_type[i] == xErr)
				setColumnHeader(i, col_label[i]+"[xEr]");
			else if (col_plot_type[i] == yErr)
				setColumnHeader(i, col_label[i]+"[yEr]");
			else if (col_plot_type[i] == Label)
				setColumnHeader(i, col_label[i]+"[L]");
			else
				setColumnHeader(i, col_label[i]);
		}
	}
}

void Table::setAscValues()
{
	QStringList list=selectedColumns();
	QStringList lstReadOnly;
	for (int i=0; i<list.count(); i++){
		QString name = list[i];
		int col = colIndex(name);
		if (d_table->isColumnReadOnly(col))
			lstReadOnly << name;
	}

	if (lstReadOnly.count() > 0){
		QMessageBox::warning(this, tr("QtiPlot - Error"),
        tr("The folowing columns")+":\n"+ lstReadOnly.join("\n") + "\n"+ tr("are read only!"));
    }

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	int rows = d_table->numRows();
	for (int j=0; j<(int) list.count(); j++){
		QString name = list[j];
		selectedCol = colIndex(name);

        if (d_table->isColumnReadOnly(selectedCol))
            continue;

		if (columnType(selectedCol) != Numeric){
			colTypes[selectedCol] = Numeric;
			col_format[selectedCol] = "0/6";
		}

		int prec;
		char f;
		columnNumericFormat(selectedCol, &f, &prec);

		for (int i=0; i<rows; i++)
			setText(i, selectedCol, QString::number(i+1, f, prec));

		emit modifiedData(this, name);
	}
	emit modifiedWindow(this);
	QApplication::restoreOverrideCursor();
}

bool Table::noXColumn()
{
	bool notSet = true;
	for (int i=0; i<d_table->numCols(); i++){
		if (col_plot_type[i] == X)
			return false;
	}
	return notSet;
}

bool Table::noYColumn()
{
	bool notSet = true;
	for (int i=0; i<d_table->numCols(); i++){
		if (col_plot_type[i] == Y)
			return false;
	}
	return notSet;
}

void Table::importASCII(const QString &fname, const QString &sep, int ignoredLines, bool renameCols,
    bool stripSpaces, bool simplifySpaces, bool importComments, const QString& commentString,
    bool readOnly, ImportMode importAs, int endLine, int maxRows)
{
	int rows;
	QString name = MdiSubWindow::parseAsciiFile(fname, commentString, endLine, ignoredLines, maxRows, rows);
	if (name.isEmpty())
		return;

	QFile f(name);
	if (f.open(QIODevice::ReadOnly)){
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        QTextStream t(&f);
		QString s = t.readLine();//read first line
		if (simplifySpaces)
			s = s.simplifyWhiteSpace();
		else if (stripSpaces)
			s = s.stripWhiteSpace();

		QStringList line = s.split(sep);
		int cols = line.size();

		bool allNumbers = true;
		for (int i=0; i<cols; i++)
		{//verify if the strings in the line used to rename the columns are not all numbers
			locale().toDouble(line[i], &allNumbers);
			if (!allNumbers)
				break;
		}
        if (renameCols && !allNumbers){
            rows--;
            if (importComments)
                rows--;
        }

		QStringList oldHeader;
        int startRow = 0, startCol = 0;
        int c = d_table->numCols();
        int r = d_table->numRows();
		switch(importAs){
			case Overwrite:
                if (d_table->numRows() != rows)
                    d_table->setNumRows(rows);

                oldHeader = col_label;
                if (c != cols){
                    if (c < cols)
                        addColumns(cols - c);
                    else {
                        d_table->setNumCols(cols);
                        for (int i = c-1; i>=cols; i--){
                            emit removedCol(QString(objectName()) + "_" + oldHeader[i]);
                            commands.removeLast();
                            comments.removeLast();
                            col_format.removeLast();
                            col_label.removeLast();
                            colTypes.removeLast();
                            col_plot_type.removeLast();
                        }
                    }
                }
			break;
			case NewColumns:
                startCol = c;
                addColumns(cols);
                if (r < rows)
                    d_table->setNumRows(rows);
			break;
			case NewRows:
                startRow = r;
                if (c < cols)
                    addColumns(cols - c);
                d_table->setNumRows(r + rows);
			break;
		}

		if (renameCols && !allNumbers){//use first line to set the table header
			for (int i = 0; i<cols; i++){
			    int aux = i + startCol;
                col_label[aux] = QString::null;
			    if (!importComments)
                    comments[aux] = line[i];
				s = line[i].replace("-","_").remove(QRegExp("\\W")).replace("_","-");
				int n = col_label.count(s);
				if(n){//avoid identical col names
					while (col_label.contains(s+QString::number(n)))
						n++;
					s += QString::number(n);
				}
				col_label[aux] = s;
			}

            if (importComments){//import comments
                s = t.readLine();//read 2nd line
                if (simplifySpaces)
                    s = s.simplifyWhiteSpace();
                else if (stripSpaces)
                    s = s.stripWhiteSpace();
                line = s.split(sep, QString::SkipEmptyParts);
				for (int i=0; i<line.size(); i++){
					int aux = startCol + i;
					if (aux < comments.size())
                    	comments[aux] = line[i];
				}
                qApp->processEvents(QEventLoop::ExcludeUserInput);
				showComments(true);
            }
        } else if (rows > 0){//put values in the first line of the table
            for (int i = 0; i<cols; i++)
				d_table->setText(startRow, startCol + i, line[i]);
            startRow++;
        }

        d_table->blockSignals(true);
		setHeaderColType();

        int steps = rows/100 + 1;
		QProgressDialog progress((QWidget *)applicationWindow());
		progress.setWindowTitle(tr("Qtiplot") + " - " + tr("Reading file..."));
		progress.setLabelText(fname);
		progress.setActiveWindow();
		progress.setAutoClose(true);
		progress.setAutoReset(true);
		progress.setRange(0, steps);

        QApplication::restoreOverrideCursor();

		int l = 0;
		int row = startRow;
		rows = d_table->numRows();
		while (!t.atEnd() && row < rows){
		    if (progress.wasCanceled()){
				f.close();
				return;
			}
		    s = t.readLine();
			if (simplifySpaces)
				s = s.simplifyWhiteSpace();
			else if (stripSpaces)
				s = s.stripWhiteSpace();
			line = s.split(sep);
			int lc = line.size();
			if (lc > cols) {
				addColumns(lc - cols);
				cols = lc;
			}
			for (int j=0; j<cols && j<lc; j++)
				d_table->setText(row, startCol + j, line[j]);

            l++;
            row++;
            if (l%100 == 0)
                progress.setValue(l/100);
            qApp->processEvents();
		}

		d_table->blockSignals(false);
		f.remove();

        if (readOnly){
            for (int i = 0; i<cols; i++)
				d_table->setColumnReadOnly(startCol + i, true);
        }

		if (importAs == Overwrite || importAs == NewRows){
			if (cols > c)
				cols = c;
			for (int i=0; i<cols; i++){
				emit modifiedData(this, colName(i));
				if (colLabel(i) != oldHeader[i])
					emit changedColHeader(QString(objectName()) + "_" + oldHeader[i], QString(objectName())+"_"+colLabel(i));
			}
		}
	}
}

bool Table::exportASCII(const QString& fname, const QString& separator,
		bool withLabels, bool exportComments, bool exportSelection)
{
	QFile f(fname);
	if ( !f.open( QIODevice::WriteOnly ) ){
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(0, tr("QtiPlot - ASCII Export Error"),
				tr("Could not write to file: <br><h4>"+fname+ "</h4><p>Please verify that you have the right to write to this location!").arg(fname));
		return false;
	}

	QString text;
	QString eol = applicationWindow()->endOfLine();
	int rows=d_table->numRows();
	int cols=d_table->numCols();
	int selectedCols = 0;
	int topRow = 0, bottomRow = 0;
	int *sCols = 0;
	if (exportSelection){
		for (int i=0; i<cols; i++){
			if (d_table->isColumnSelected(i))
				selectedCols++;
		}

		sCols = new int[selectedCols];
		int aux = 0;
		for (int i=0; i<cols; i++){
			if (d_table->isColumnSelected(i)){
				sCols[aux] = i;
				aux++;
			}
		}

		for (int i=0; i<rows; i++){
			if (d_table->isRowSelected(i)){
				topRow = i;
				break;
			}
		}

		for (int i=rows - 1; i>0; i--){
			if (d_table->isRowSelected(i)){
				bottomRow = i;
				break;
			}
		}
	}

	int aux = selectedCols-1;
	if (withLabels){
		QStringList header = colNames();
		QStringList ls = header.grep ( QRegExp ("\\D"));
		if (exportSelection){
			for (int i=0; i<aux; i++){
				if (ls.count()>0)
					text += header[sCols[i]] + separator;
				else
					text += "C" + header[sCols[i]] + separator;
			}

			if (aux >= 0){
				if (ls.count()>0)
					text += header[sCols[aux]] + eol;
				else
					text += "C" + header[sCols[aux]] + eol;
			}
		} else {
			if (ls.count()>0){
				for (int j=0; j<cols-1; j++)
					text += header[j] + separator;
				text += header[cols-1] + eol;
			} else {
				for (int j=0; j<cols-1; j++)
					text += "C" + header[j] + separator;
				text += "C" + header[cols-1] + eol;
			}
		}
	}// finished writting labels

	if (exportComments){
		if (exportSelection){
			for (int i=0; i<aux; i++)
                text += comments[sCols[i]] + separator;
			if (aux >= 0)
				text += comments[sCols[aux]] + eol;
		} else {
            for (int i=0; i<cols-1; i++)
                text += comments[i] + separator;
            text += comments[cols-1] + eol;
        }
	}

	if (exportSelection){
		for (int i=topRow; i<=bottomRow; i++){
			for (int j=0; j<aux; j++)
				text += d_table->text(i, sCols[j]) + separator;
			if (aux >= 0)
				text += d_table->text(i, sCols[aux]) + eol;
		}
		delete [] sCols;
	} else {
		for (int i=0;i<rows;i++) {
			for (int j=0; j<cols-1; j++)
				text += d_table->text(i,j) + separator;
			text += d_table->text(i, cols-1) + eol;
		}
	}
	QTextStream t( &f );
	t << text;
	f.close();
	return true;
}

void Table::moveCurrentCell()
{
	int cols=d_table->numCols();
	int row=d_table->currentRow();
	int col=d_table->currentColumn();
	d_table->clearSelection (true);

	if (col+1<cols)
	{
		d_table->setCurrentCell(row, col+1);
		d_table->selectCells(row, col+1, row, col+1);
	}
	else
	{
        if(row+1 >= numRows())
            d_table->setNumRows(row + 11);

		d_table->setCurrentCell (row+1, 0);
		d_table->selectCells(row+1, 0, row+1, 0);
	}
}

bool Table::eventFilter(QObject *object, QEvent *e)
{
	Q3Header *hheader = d_table->horizontalHeader();
	Q3Header *vheader = d_table->verticalHeader();

	if (e->type() == QEvent::MouseButtonDblClick && object == (QObject*)hheader) {
		const QMouseEvent *me = (const QMouseEvent *)e;
		selectedCol = hheader->sectionAt (me->pos().x() + hheader->offset());

		QRect rect = hheader->sectionRect (selectedCol);
		rect.setLeft(rect.right() - 2);
		rect.setWidth(4);

		if (rect.contains (me->pos())) {
			d_table->adjustColumn(selectedCol);
			emit modifiedWindow(this);
		} else
			emit optionsDialog();
        setActiveWindow();
		return true;
	} else if (e->type() == QEvent::MouseButtonPress && object == (QObject*)hheader) {
		const QMouseEvent *me = (const QMouseEvent *)e;
		if (me->button() == Qt::LeftButton && me->state() == Qt::ControlButton) {
			int col = hheader->sectionAt (me->pos().x() + hheader->offset());
			if (!d_table->isColumnSelected(col, true)){
			    selectedCol = col;
                d_table->selectColumn (col);
                d_table->setCurrentCell (0, col);
			} else {//deselect already selected column: dirty hack to be modified when porting Table to Qt4
			    QVector<int> sel;
			    int cols = 0;
			    for (int i = 0; i < d_table->numCols(); i++){
                    if(d_table->isColumnSelected (i, true) && i != col){
                        sel << i;
                        cols++;
                    }
                }
                sel.resize(cols);
                d_table->clearSelection();
                for (int i = 0; i < cols; i++)
                    d_table->selectColumn (sel[i]);
			}
			setActiveWindow();
			return true;
		} else if (selectedColsNumber() <= 1) {
			selectedCol = hheader->sectionAt (me->pos().x() + hheader->offset());
			d_table->clearSelection();
			d_table->selectColumn (selectedCol);
			d_table->setCurrentCell (0, selectedCol);
			setActiveWindow();
			return false;
		}
	} else if (e->type() == QEvent::MouseButtonPress && object == (QObject*)vheader) {
		const QMouseEvent *me = (const QMouseEvent *)e;
		if (me->button() == Qt::RightButton && numSelectedRows() <= 1) {
			d_table->clearSelection();
			int row = vheader->sectionAt(me->pos().y() + vheader->offset());
			d_table->selectRow (row);
			d_table->setCurrentCell (row, 0);
			setActiveWindow();
		}
	} else if (e->type() == QEvent::ContextMenu && object == (QObject*)d_table){
        const QContextMenuEvent *ce = (const QContextMenuEvent *)e;
        QRect r = d_table->horizontalHeader()->sectionRect(d_table->numCols()-1);
        setFocus();
        if (ce->pos().x() > r.right() + d_table->verticalHeader()->width())
            emit showContextMenu(false);
        else if (d_table->numCols() > 0 && d_table->numRows() > 0)
            emit showContextMenu(true);
    }

	return MdiSubWindow::eventFilter(object, e);
}

void Table::customEvent(QEvent *e)
{
	if (e->type() == SCRIPTING_CHANGE_EVENT)
		scriptingChangeEvent((ScriptingChangeEvent*)e);
}

void Table::setNumRows(int rows)
{
	d_table->setNumRows(rows);
}

void Table::setNumCols(int c)
{
	int cols = d_table->numCols();
	if (cols == c)
		return;

	if (cols > c){
		d_table->setNumCols(c);
		for (int i = cols - 1; i >= c; i--){
			commands.removeLast();
			comments.removeLast();
			col_format.removeLast();
			col_label.removeLast();
			colTypes.removeLast();
			col_plot_type.removeLast();
		}
	} else {
		addColumns(c - cols);
		setHeaderColType();
	}
}

void Table::resizeRows(int r)
{
	int rows = d_table->numRows();
	if (rows == r)
		return;

	if (rows > r){
		QString text= tr("Rows will be deleted from the table!");
		text+="<p>"+tr("Do you really want to continue?");
		int i,cols = d_table->numCols();
		switch( QMessageBox::information(this,tr("QtiPlot"), text, tr("Yes"), tr("Cancel"), 0, 1 ) )
		{
			case 0:
				QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
				d_table->setNumRows(r);
				for (i=0; i<cols; i++)
					emit modifiedData(this, colName(i));

				QApplication::restoreOverrideCursor();
				break;

			case 1:
				return;
				break;
		}
	} else {
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		d_table->setNumRows(r);
		QApplication::restoreOverrideCursor();
	}

	emit modifiedWindow(this);
}

void Table::resizeCols(int c)
{
	int cols = d_table->numCols();
	if (cols == c)
		return;

	if (cols > c){
		QString text= tr("Columns will be deleted from the table!");
		text+="<p>"+tr("Do you really want to continue?");
		switch( QMessageBox::information(this,tr("QtiPlot"), text, tr("Yes"), tr("Cancel"), 0, 1 ) ){
			case 0: {
				QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                Q3MemArray<int> columns(cols-c);
				for (int i=cols-1; i>=c; i--){
					QString name = colName(i);
					emit removedCol(name);
					columns[i-c]=i;

					commands.removeLast();
					comments.removeLast();
					col_format.removeLast();
					col_label.removeLast();
					colTypes.removeLast();
					col_plot_type.removeLast();
				}

				d_table->removeColumns(columns);
				QApplication::restoreOverrideCursor();
				break;
			}

			case 1:
				return;
				break;
		}
	}
	else{
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		addColumns(c-cols);
		setHeaderColType();
		QApplication::restoreOverrideCursor();
	}
	emit modifiedWindow(this);
}

void Table::copy(Table *m)
{
	int rows = d_table->numRows();
	int cols = d_table->numCols();
	for (int i=0; i<rows; i++){
		for (int j=0; j<cols; j++)
			d_table->setText(i, j, m->text(i, j));
	}

	for (int i=0; i<cols; i++){
		d_table->setColumnReadOnly(i, m->isReadOnlyColumn(i));
		d_table->setColumnWidth(i, m->columnWidth(i));
		if (m->isColumnHidden(i))
			d_table->hideColumn(i);
	}

	col_label = m->colNames();
	col_plot_type = m->plotDesignations();
	d_show_comments = m->commentsEnabled();
    comments = m->colComments();
	setHeaderColType();

	commands = m->getCommands();
	setColumnTypes(m->columnTypes());
	col_format = m->getColumnsFormat();
}

void Table::restore(const QStringList& lst)
{
	QStringList l;
	QStringList::const_iterator i=lst.begin();

	l= (*i++).split("\t");
	l.remove(l.first());
	loadHeader(l);

	setColWidths((*i).right((*i).length()-9).split("\t", QString::SkipEmptyParts));
	i++;

	l = (*i++).split("\t");
	if (l[0] == "com")
	{
		l.remove(l.first());
		setCommands(l);
	} else if (l[0] == "<com>") {
		commands.clear();
		for (int col=0; col<numCols(); col++)
			commands << "";
		for (; i != lst.end() && *i != "</com>"; i++)
		{
			int col = (*i).mid(9,(*i).length()-11).toInt();
			QString formula;
			for (i++; i!=lst.end() && *i != "</col>"; i++)
				formula += *i + "\n";
			formula.truncate(formula.length()-1);
			commands[col] = formula;
		}
		i++;
	}

	l = (*i++).split("\t");
	l.remove(l.first());
	setColumnTypes(l);

	l = (*i++).split("\t");
	l.remove(l.first());
	setColComments(l);
}

void Table::notifyChanges()
{
	for (int i=0; i<d_table->numCols(); i++)
		emit modifiedData(this, colName(i));

	emit modifiedWindow(this);
}

void Table::clear()
{
	for (int i=0; i<d_table->numCols(); i++)
	{
		for (int j=0; j<d_table->numRows(); j++)
			d_table->setText(j, i, QString::null);

		emit modifiedData(this, colName(i));
	}

	emit modifiedWindow(this);
}

void Table::goToRow(int row)
{
	if( (row < 1) || (row > numRows()) ) return;

	d_table->ensureCellVisible ( row-1, 0 );
	d_table->selectRow(row-1);
}

void Table::goToColumn(int col)
{
	if( (col < 1) || (col > numCols()) ) return;

	d_table->ensureCellVisible (0, col - 1);
	d_table->selectColumn(col - 1);
}

void Table::setColumnHeader(int index, const QString& label)
{
	Q3Header *head = d_table->horizontalHeader();
	if (d_show_comments){
		QString s = label;
		int lines = d_table->columnWidth(index)/head->fontMetrics().averageCharWidth();
		head->setLabel(index, s.remove("\n") + "\n" + QString(lines, '_') + "\n" + comments[index]);
	} else
		head->setLabel(index, label);
}

void Table::showComments(bool on)
{
	if (d_show_comments == on)
		return;

	d_show_comments = on;
	setHeaderColType();

	if(!on)
		d_table->setTopMargin (d_table->horizontalHeader()->height()/2);
}

void Table::setNumericPrecision(int prec)
{
	d_numeric_precision = prec;
	for (int i=0; i<d_table->numCols(); i++){
		if (colTypes[i] == Numeric)
        	col_format[i] = "0/" + QString::number(prec);
	}
}

void Table::updateDecimalSeparators(const QLocale& oldSeparators)
{
    QLocale l = locale();
    if (l == oldSeparators)
        return;

    int rows = d_table->numRows();
	for (int i=0; i<d_table->numCols(); i++){
	    if (colTypes[i] != Numeric)
            continue;

        char format;
        int prec;
        columnNumericFormat(i, &format, &prec);

        for (int j = 0; j<rows; j++){
            QString s = d_table->text(j, i);
            if (!s.isEmpty()){
				double val = oldSeparators.toDouble(s);
                d_table->setText(j, i, l.toString(val, format, prec));
			}
		}
	}
}

bool Table::isReadOnlyColumn(int col)
{
    if (col < 0 || col >= d_table->numCols())
        return false;

    return d_table->isColumnReadOnly(col);
}

void Table::setReadOnlyColumn(int col, bool on)
{
    if (col < 0 || col >= d_table->numCols())
        return;

	d_table->setColumnReadOnly(col, on);
}

void Table::moveColumn(int, int fromIndex, int toIndex)
{
    int to = toIndex;
    if (fromIndex < toIndex)
        to = toIndex - 1;

    col_label.move(fromIndex, to);
    comments.move(fromIndex, to);
	commands.move(fromIndex, to);
	colTypes.move(fromIndex, to);
	col_format.move(fromIndex, to);
	col_plot_type.move(fromIndex, to);
	setHeaderColType();
}

void Table::swapColumns(int col1, int col2)
{
    if (col1 < 0 || col1 >= d_table->numCols() || col2 < 0 || col2 >= d_table->numCols())
        return;

    int width1 = d_table->columnWidth(col1);
    int width2 = d_table->columnWidth(col2);

    d_table->swapColumns(col1, col2);
    col_label.swap (col1, col2);
    comments.swap (col1, col2);
    commands.swap (col1, col2);
    colTypes.swap (col1, col2);
    col_format.swap (col1, col2);
    col_plot_type.swap (col1, col2);

    d_table->setColumnWidth(col1, width2);
    d_table->setColumnWidth(col2, width1);
    setHeaderColType();
}

void Table::moveColumnBy(int cols)
{
	int oldPos = selectedCol;
	int newPos = oldPos + cols;
	if (newPos < 0)
		newPos = 0;
	else if	(newPos >= d_table->numCols())
		newPos = d_table->numCols() - 1;

	if (abs(cols) > 1){
		d_table->insertColumns(newPos);
		if (cols < 0)
			d_table->swapColumns(oldPos + 1, newPos);
		else
			d_table->swapColumns(oldPos, newPos + 1);

		d_table->removeColumn(oldPos);

		col_label.move(oldPos, newPos);
    	comments.move(oldPos, newPos);
		commands.move(oldPos, newPos);
		colTypes.move(oldPos, newPos);
		col_format.move(oldPos, newPos);
		col_plot_type.move(oldPos, newPos);
	} else
		swapColumns(oldPos, newPos);

	setHeaderColType();

	setSelectedCol(newPos);
    d_table->clearSelection();
    d_table->selectColumn(newPos);
}

void Table::hideColumn(int col, bool hide)
{
	if(hide)
		d_table->hideColumn(col);
	else
		d_table->showColumn(col);
}

void Table::hideSelectedColumns()
{
	for(int i = 0; i<d_table->numCols(); i++){
		if (d_table->isColumnSelected(i, true))
			d_table->hideColumn(i);
	}
}

void Table::showAllColumns()
{
	for(int i = 0; i<d_table->numCols(); i++){
		if (d_table->isColumnHidden(i))
			d_table->showColumn(i);
	}
}

QString Table::sizeToString()
{
	int size = d_table->numRows() * d_table->numCols();
	return QString::number((sizeof(Table) + size*sizeof(Q3TableItem))/1024.0, 'f', 1) + " " + tr("kB");
}

/*****************************************************************************
 *
 * Class MyTable
 *
 *****************************************************************************/

MyTable::MyTable(QWidget * parent, const char * name)
:Q3Table(parent, name)
{}

MyTable::MyTable(int numRows, int numCols, QWidget * parent, const char * name)
:Q3Table(numRows, numCols, parent, name)
{}

void MyTable::activateNextCell()
{
	int row = currentRow();
	int col = currentColumn();

	clearSelection (true);

    if(row+1 >= numRows())
        setNumRows(row + 11);

	setCurrentCell (row + 1, col);
    selectCells(row+1, col, row+1, col);
}
