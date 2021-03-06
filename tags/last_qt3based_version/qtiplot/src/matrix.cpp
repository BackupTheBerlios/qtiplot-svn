#include "matrix.h"
#include "nrutil.h"

#include <qdatetime.h>
#include <qlayout.h>
#include <qaccel.h>
#include <qregexp.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qdragobject.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qsimplerichtext.h>

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_math.h>

Matrix::Matrix(ScriptingEnv *env, int r, int c, const QString& label, QWidget* parent, const char* name, WFlags f)
				: myWidget(label, parent, name, f), scripted(env)
{
init(r, c);	
}

void Matrix::init(int rows, int cols)
{
formula_str = "";
selectedCol=0;
LeftButton=FALSE;
txt_format = 'f';
num_precision = 6;
x_start = 1.0;
x_end = 10.0;
y_start = 1.0; 
y_end = 10.0;
dMatrix = 0;

QDateTime dt = QDateTime::currentDateTime ();
setBirthDate(dt.toString(Qt::LocalDate));

d_table = new QTable (rows, cols, this, "d_table");
d_table->setFocusPolicy(QWidget::StrongFocus);
d_table->setFocus();
d_table->setSelectionMode (QTable::Single);
d_table->setRowMovingEnabled(true);

/*
//!TODO: enable column moving, right now it doesn't work because of the event filter
//installed on hHeader
d_table->setColumnMovingEnabled(true);
connect(hHeader, SIGNAL(indexChange (int, int, int)), this, SLOT(notifyChanges()));
*/

QColor background = QColor(255, 255, 128);
d_table->setPaletteBackgroundColor(background);
d_table->setBackgroundColor(background);

QVBoxLayout* hlayout = new QVBoxLayout(this,0,0);
hlayout->addWidget(d_table);

QHeader* hHeader=(QHeader*)d_table->horizontalHeader();
hHeader->installEventFilter(this);
hHeader->setMouseTracking(TRUE);

QHeader* vHeader=(QHeader*)d_table->verticalHeader();
vHeader->setResizeEnabled (false);
vHeader->installEventFilter(this);

int w, h;
if (cols>3)
	w=3*hHeader->sectionSize(0);
else
	w=cols*hHeader->sectionSize(0);

if (rows>11)
	h=11*vHeader->sectionSize(0);
else
	h=(rows+1)*vHeader->sectionSize(0);
setGeometry(50,50,w+55,h);

QAccel *accel = new QAccel(this);
accel->connectItem( accel->insertItem( Key_Tab ),
                            this, SLOT(moveCurrentCell()));
accel->connectItem( accel->insertItem( CTRL+Key_A ),
                            this, SLOT(selectAll()));

connect(d_table, SIGNAL(valueChanged(int,int)), this, SLOT(cellEdited(int,int)));
connect(vHeader, SIGNAL(indexChange (int, int, int)), this, SLOT(notifyChanges()));
}

void Matrix::selectAll()
{	
d_table->addSelection (QTableSelection( 0, 0, d_table->numRows(), d_table->numCols() ));
}

void Matrix::moveCurrentCell()
{
int cols=d_table->numCols();
int row=d_table->currentRow();
int col=d_table->currentColumn();
d_table->clearSelection (TRUE);
	
if (col+1 < cols)
	{
	d_table->setCurrentCell (row, col+1);
	d_table->selectCells(row, col+1, row, col+1);
	}
else
	{
	d_table->setCurrentCell (row+1, 0);
	d_table->selectCells(row+1, 0, row+1, 0);	
	}
}

void Matrix::cellEdited(int row,int col)
{
QString text = d_table->text(row,col).replace(",", ".");
bool ok = false;
double res = text.toDouble(&ok);
if (!text.isEmpty() && ok)
	d_table->setText(row, col, QString::number(res, txt_format, num_precision));
else
	{
Script *script = scriptEnv->newScript(d_table->text(row,col),this,QString("<%1_%2_%3>").arg(name()).arg(row+1).arg(col+1));
connect(script, SIGNAL(error(const QString&,const QString&,int)), scriptEnv, SIGNAL(error(const QString&,const QString&,int)));

script->setInt(row+1, "row");
script->setInt(row+1, "i");
script->setInt(col+1, "col");
script->setInt(col+1, "j");
QVariant ret = script->eval();
if(ret.type()==QVariant::Int || ret.type()==QVariant::UInt || ret.type()==QVariant::LongLong || ret.type()==QVariant::ULongLong)
	d_table->setText(row, col, ret.toString());
else if(ret.canCast(QVariant::Double))
	d_table->setText(row, col, QString::number(ret.toDouble(), txt_format, num_precision));
else
	d_table->setText(row, col, "");
	}

emit modifiedWindow(this);
}

QString Matrix::text (int row, int col)
{
  if(dMatrix)
    return QString::number(dMatrix[row][col], txt_format, num_precision);
  else
    return d_table->text(row, col); 
}

void Matrix::setText (int row, int col, const QString & text )
{
d_table->setText(row, col, text); 
}

bool Matrix::isEmptyRow(int row)
{
bool empty=TRUE;
int cols=d_table->numCols();
for (int i=0;i<cols;i++)
	{
	QString text=d_table->text(row,i);
	if (!text.isEmpty())
		{
		empty=FALSE;
		break;
		}
	}	
return empty;
}

void Matrix::setCoordinates(double xs, double xe, double ys, double ye)
{
if (x_start == xs && x_end == xe &&	y_start == ys && y_end == ye)
	return;

x_start = xs;
x_end = xe;
y_start = ys;
y_end = ye;

emit modifiedWindow(this);
}

QString Matrix::saveToString(const QString &info)
{
QString s= "<matrix>\n";
s+= QString(name()) + "\t";
s+= QString::number(d_table->numRows())+"\t";
s+= QString::number(d_table->numCols())+"\t";
s+= birthDate() + "\n";
s+= info;
s+= "ColWidth\t" + QString::number(d_table->columnWidth(0))+"\n";
s+= "<formula>\n" + formula_str + "\n</formula>\n";
s+= "TextFormat\t" + QString(txt_format) + "\t" + QString::number(num_precision) + "\n";
s+= "WindowLabel\t" + windowLabel() + "\t" + QString::number(captionPolicy()) + "\n";
s+= "Coordinates\t" + QString::number(x_start,'g',15) + "\t" +QString::number(x_end,'g',15) + "\t";
s+= QString::number(y_start,'g',15) + "\t" + QString::number(y_end,'g',15) + "\n";
s+= saveText();
s+="</matrix>\n";
return s;
}

QString Matrix::saveAsTemplate(const QString &info)
{
QString s= "<matrix>\t";
s+= QString::number(d_table->numRows())+"\t";
s+= QString::number(d_table->numCols())+"\n";
s+= info;
s+= "ColWidth\t" + QString::number(d_table->columnWidth(0))+"\n";
s+= "<formula>\n" + formula_str + "\n</formula>\n";
s+= "TextFormat\t" + QString(txt_format) + "\t" + QString::number(num_precision) + "\n";
s+= "Coordinates\t" + QString::number(x_start,'g',15) + "\t" +QString::number(x_end,'g',15) + "\t";
s+= QString::number(y_start,'g',15) + "\t" + QString::number(y_end,'g',15) + "\n";
return s;
}

void Matrix::restore(const QStringList &lst)
{
QStringList l;
QStringList::const_iterator i=lst.begin();

l= QStringList::split ("\t", *i++, true);
setColumnsWidth(l[1].toInt());

l= QStringList::split ("\t", *i++, true);
if (l[0] == "Formula")
  formula_str = l[1];
else if (l[0] == "<formula>")
{
  for (formula_str=""; i != lst.end() && *i != "</formula>"; i++)
    formula_str += *i + "\n";
  formula_str.truncate(formula_str.length()-1);
  i++;
}

l= QStringList::split ("\t", *i++, true);
if (l[1] == "f")
	setTextFormat('f', l[2].toInt());
else
	setTextFormat('e', l[2].toInt());

l= QStringList::split ("\t", *i++, true);
x_start = l[1].toDouble();
x_end = l[2].toDouble();
y_start = l[3].toDouble();
y_end = l[4].toDouble();
}

QString Matrix::saveText()
{
QString text = "<data>\n";
int cols=d_table->numCols();
int rows=d_table->numRows();
	
for (int i=0; i<rows; i++)
	{
	if (!isEmptyRow(i))
		{
		text+=QString::number(i)+"\t";
		for (int j=0; j<cols-1; j++)
			text+=d_table->text(i,j) + "\t";
		
		text+=d_table->text(i,cols-1)+"\n";
		}
	}	
return text + "</data>\n";
}

void Matrix::setFormula(const QString &s)
{
if (formula_str == s)
	return;

formula_str = s;
}

void Matrix::setNumericFormat(const QChar& f, int prec)
{
if (txt_format == f && num_precision == prec)
	return;

QApplication::setOverrideCursor(waitCursor);

txt_format = f;
num_precision = prec;

int rows=d_table->numRows();
int cols=d_table->numCols();
for (int i=0; i<rows; i++)
	{
	for (int j=0; j<cols; j++)
		{
		QString t = d_table->text(i, j);
		if (!t.isEmpty())
			{
			double val = dMatrix[i][j];
			t = t.setNum(val, txt_format, num_precision);
			d_table->setText(i, j, t);
			}
		}		
	}

emit modifiedWindow(this);
QApplication::restoreOverrideCursor();
}

void Matrix::setTextFormat(const QChar &format, int precision)
{
if (txt_format == format && num_precision == precision)
	return;

txt_format = format;
num_precision = precision;
}

int Matrix::columnsWidth()
{
return d_table->columnWidth(0);	
}

void Matrix::setColumnsWidth(int width)
{
if (width == columnsWidth())
	return;

for (int i=0; i<d_table->numCols(); i++)
	d_table->setColumnWidth (i, width);

emit modifiedWindow(this);	
}

void Matrix::setMatrixDimensions(int rows, int cols)
{
int r = d_table->numRows();
int c = d_table->numCols();

if (r == rows && c == cols)
	return;

if (rows < r || cols < c)
	{
	QString text="Deleting rows/columns from the matrix!";
	text+="<p>Do you really want to continue?";
	switch( QMessageBox::information(0,"QtiPlot", tr(text),tr("Yes"), tr("Cancel"), 0, 1 ) ) 
		{
		case 0:
		QApplication::setOverrideCursor(waitCursor);
		if (cols != c)
			d_table->setNumCols(cols);
		if (rows != r)
			d_table->setNumRows(rows);
		QApplication::restoreOverrideCursor();
		emit modifiedWindow(this);
		break;
		
   		case 1:
		    return;
		break;
		}
	}
else
	{
	QApplication::setOverrideCursor(waitCursor);
	if (cols != c)
		d_table->setNumCols(cols);
	if (rows != r)
		d_table->setNumRows(rows);
	QApplication::restoreOverrideCursor();
	emit modifiedWindow(this);
	}
}

int Matrix::numRows()
{
return d_table->numRows();
}

int Matrix::numCols()
{
return d_table->numCols();
}

double Matrix::determinant()
{
int rows = d_table->numRows();
int cols = d_table->numCols();

if (rows != cols)
	{
	QMessageBox::critical(0,tr("QtiPlot - Error"),
		tr("Calculation failed, the matrix is not square!"));
	return GSL_POSINF;
	}

QApplication::setOverrideCursor(waitCursor);

gsl_matrix *A = gsl_matrix_alloc (rows, cols);
int i, j;
for (i=0; i<rows; i++)
	{
	for (j=0; j<cols; j++)
		{
		QString s = d_table->text(i,j);
		gsl_matrix_set (A, i, j, s.toDouble());
		}
	}

int s;
gsl_permutation * p = gsl_permutation_alloc (rows);
gsl_linalg_LU_decomp (A, p, &s);

double det = gsl_linalg_LU_det (A, s);

gsl_matrix_free (A);
gsl_permutation_free (p);

QApplication::restoreOverrideCursor();
return det;
}

void Matrix::invert()
{
int rows = d_table->numRows();
int cols = d_table->numCols();

if (rows != cols)
	{
	QMessageBox::critical(0,tr("QtiPlot - Error"),
		tr("Inversion failed, the matrix is not square!"));
	return;
	}

QApplication::setOverrideCursor(waitCursor);

gsl_matrix *A = gsl_matrix_alloc (rows, cols);
int i, j;
for (i=0; i<rows; i++)
	{
	for (j=0; j<cols; j++)
		{
		QString s = d_table->text(i,j);
		gsl_matrix_set (A, i, j, s.toDouble());
		}
	}

int s;
gsl_permutation * p = gsl_permutation_alloc (cols);
gsl_linalg_LU_decomp (A, p, &s);

gsl_matrix *inverse = gsl_matrix_alloc (rows, cols);
gsl_linalg_LU_invert (A, p, inverse);

gsl_matrix_free (A);
gsl_permutation_free (p);

for (i=0; i<rows; i++)
	{
	for (j=0; j<cols; j++)
		{
		double val = gsl_matrix_get (inverse, i, j);
		d_table->setText(i, j, QString::number(val));
		}
	}

gsl_matrix_free (inverse);
QApplication::restoreOverrideCursor();
}

void Matrix::transpose()
{
int i, j;
int rows = d_table->numRows();
int cols = d_table->numCols();

QTable* t = new QTable(rows, cols);
for (i = 0; i<rows; i++)
	{
	for (j = 0; j<cols; j++)
		t->setText(i, j, d_table->text(i,j));
	}

d_table->setNumCols(rows);
d_table->setNumRows(cols);	

for (i = 0; i<cols; i++)
	{
	for (j = 0; j<rows; j++)
		d_table->setText(i, j, t->text(j,i));
	}

delete t;
}

void Matrix::saveCellsToMemory()
{
int rows=d_table->numRows();
int cols=d_table->numCols();
dMatrix=matrix(0,rows-1,0,cols-1);
for (int i=0; i<rows; i++)
	{
	for (int j=0; j<cols; j++)
		{
		QString s = d_table->text(i, j);
		dMatrix[i][j]=s.toDouble();
		}
	}
}

void Matrix::forgetSavedCells()
{
int rows=d_table->numRows();
int cols=d_table->numCols();
free_matrix (dMatrix, 0, rows-1, 0, cols-1);
dMatrix = 0;
}

bool Matrix::calculate(int startRow, int endRow, int startCol, int endCol)
{
  QApplication::setOverrideCursor(waitCursor);

  Script *script = scriptEnv->newScript(formula_str, this, QString("<%1>").arg(name()));
  connect(script, SIGNAL(error(const QString&,const QString&,int)), scriptEnv, SIGNAL(error(const QString&,const QString&,int))); 
  connect(script, SIGNAL(print(const QString&)), parent()->parent()->parent(), SLOT(scriptPrint(const QString&)));
  if (!script->compile())
  {
    QApplication::restoreOverrideCursor();
    return false;
  }

  int rows=d_table->numRows();
  int cols=d_table->numCols();
  if (endCol >= cols)
    d_table->setNumCols(endCol+1);
  if (endRow >= rows)
    d_table->setNumRows(endRow+1);

  QVariant ret;
  saveCellsToMemory();
  double dx = fabs(x_end-x_start)/(double)(d_table->numRows()-1);
  double dy = fabs(y_end-y_start)/(double)(d_table->numCols()-1);
  for (int row=startRow; row<=endRow; row++)
    for (int col=startCol; col<=endCol; col++)
    {
      script->setInt(row+1, "i");
      script->setInt(row+1, "row");
	  script->setDouble(y_start+row*dy, "y");
      script->setInt(col+1, "j");
      script->setInt(col+1, "col");
	  script->setDouble(x_start+col*dx, "x");
      ret = script->eval();
      if (ret.type()==QVariant::Int || ret.type()==QVariant::UInt || ret.type()==QVariant::LongLong || ret.type()==QVariant::ULongLong)
	d_table->setText(row, col, ret.toString());
      else if(ret.canCast(QVariant::Double))
	d_table->setText(row,col,QString::number(ret.toDouble(),txt_format, num_precision));
      else {
	d_table->setText(row,col,"");
	QApplication::restoreOverrideCursor();
	return false;
      }
    }
  forgetSavedCells();
  
  emit modifiedWindow(this);
  QApplication::restoreOverrideCursor();
  return true;
}

void Matrix::clearSelection()
{
bool colSelection = false;
for (int i=0; i<d_table->numCols(); i++)
	{
	if(d_table->isColumnSelected (i,TRUE))
		{
		colSelection = true;
		for (int j=0; j<d_table->numRows(); j++)
			d_table->setText(j, i, "");
		}
	}

if (colSelection)
	{
	emit modifiedWindow(this);
	return;
	}
else
	{
	QTableSelection sel=d_table->selection(d_table->currentSelection());	
	for (int i= sel.topRow();i<=sel.bottomRow();i++)
		{
		for (int j= sel.leftCol();j<= sel.rightCol();j++)
			d_table->setText(i, j, "");
		}
	emit modifiedWindow(this);
	}
}

void Matrix::copySelection()
{
QString text;
int i,j;
int rows=d_table->numRows();
int cols=d_table->numCols();

QMemArray<int> selection(1);
int c=0;	
for (i=0;i<cols;i++)
	{
	if (d_table->isColumnSelected(i,TRUE))
		{
		c++;
		selection.resize(c);
		selection[c-1]=i;			
		}
	}
if (c>0)
	{	
	for (i=0; i<rows; i++)
		{
		for (j=0;j<c-1;j++)
			text+=d_table->text(i,selection[j])+"\t";
		text+=d_table->text(i,selection[c-1])+"\n";
		}	
	}
else
	{
	QTableSelection sel=d_table->selection(d_table->currentSelection());
	int top=sel.topRow();
	int bottom=sel.bottomRow();
	int left=sel.leftCol();
	int right=sel.rightCol();
	for (i=top; i<=bottom; i++)
		{
		for (j=left; j<right; j++)
			text+=d_table->text(i,j)+"\t";
		text+=d_table->text(i,right)+"\n";
		}
	}		
	
// Copy text into the clipboard
QApplication::clipboard()->setData(new QTextDrag(text,d_table,0));
}

void Matrix::cutSelection()
{
copySelection();
clearSelection();
}

bool Matrix::rowsSelected()
{
QTableSelection sel=d_table->selection(d_table->currentSelection());	
for (int i=sel.topRow(); i<=sel.bottomRow(); i++)
	{
	if (!d_table->isRowSelected (i, true))
		return false;
	}
return true;
}

void Matrix::deleteSelectedRows()
{
QTableSelection sel=d_table->selection(d_table->currentSelection());
int top=sel.topRow();
int bottom=sel.bottomRow();

QMemArray<int> rows(bottom-top+1);
for (int i=top; i<=bottom; i++)
	rows[i-top]= i;

d_table->removeRows(rows);
emit modifiedWindow(this);
}

void Matrix::insertColumn()
{
int cr = d_table->currentColumn();
if (d_table->isColumnSelected (cr, true))
	{
	d_table->insertColumns(cr,1);
	emit modifiedWindow(this);
	}
}

bool Matrix::columnsSelected()
{
for (int i=0; i<d_table->numCols(); i++)
	{
	if (d_table->isColumnSelected (i, true))
		return true;
	}
return false;
}

void Matrix::deleteSelectedColumns()
{
QMemArray<int> cols;
int n=0;
for (int i=0; i<d_table->numCols(); i++)
	{
	if (d_table->isColumnSelected (i, true))
		{
		n++;
		cols.resize(n);
		cols[n-1]= i;
		}
	}
d_table->removeColumns(cols);
emit modifiedWindow(this);
}

void Matrix::insertRow()
{
int cr = d_table->currentRow();
if (d_table->isRowSelected (cr, true))
	{
	d_table->insertRows(cr,1);
	emit modifiedWindow(this);
	}
}

// Paste text from the clipboard
void Matrix::pasteSelection()
{
QApplication::setOverrideCursor(waitCursor);
	
QString text;		
if ( !QTextDrag::decode(QApplication::clipboard()->data(), text) || text.isNull())
	{
	QApplication::restoreOverrideCursor();
	return;
	}
	
QTextStream ts( &text, IO_ReadOnly );
QString s = ts.readLine(); 
QStringList cellTexts = QStringList::split ("\t", s, TRUE);
int cols=int(cellTexts.count());
int rows= 1;
while(!ts.atEnd()) 
	{
	rows++;
	s = ts.readLine(); 
	}
ts.reset();
	
int i, j, top, bottom, right, left, firstCol;
QTableSelection sel=d_table->selection(d_table->currentSelection());
if (!sel.isEmpty())
	{// not columns but only cells are selected
	top=sel.topRow();
	bottom=sel.bottomRow();
	left=sel.leftCol();
	right=sel.rightCol();
	}
else
	{
	top=0;
	bottom=d_table->numRows() - 1;	
	left=0;
	right=d_table->numCols() - 1;

	firstCol = -1;
	for (i=0; i<d_table->numCols(); i++)
		{
		if (d_table->isColumnSelected(i, TRUE))
			{
			firstCol = i;
			break;
			}
		}
	if (firstCol >= 0)
		{// columns are selected
		left=firstCol;
		int selectedColsNumber = 0;
		for (i=0; i<d_table->numCols(); i++)
			{
			if (d_table->isColumnSelected(i, TRUE))
				selectedColsNumber++;
			}
		right=firstCol + selectedColsNumber - 1;
		}
	}

QTextStream ts2( &text, IO_ReadOnly );	
int r = bottom-top+1;
int c = right-left+1;

QApplication::restoreOverrideCursor();
if (rows>r || cols>c)
	{
	switch( QMessageBox::information(0,"QtiPlot",
		   tr("The text in the clipboard is larger than your current selection!\
	        \nDo you want to insert cells?"),
				      tr("Yes"), tr("No"), tr("Cancel"), 0, 0) ) 
		{
		case 0:	
			if(cols > c )
				d_table->insertColumns(left, cols - c);

			if(rows > r)
				{
				if (firstCol >= 0)
					d_table->insertRows(top, rows - r);
				else
					d_table->insertRows(top, rows - r + 1);
				}
		break;		
    	case 1:
			rows = r;
			cols = c;		
		break;
    	case 2:
			return;
		break;
		}
	}

QApplication::setOverrideCursor(waitCursor);	
bool numeric;
double value;
for (i=top; i<top+rows; i++)
	{
	s = ts2.readLine();
	cellTexts=QStringList::split ("\t", s, TRUE);
	for (j=left; j<left+cols; j++)					
		{
		value = cellTexts[j-left].toDouble(&numeric);
	  	if (numeric)
			d_table->setText(i, j, QString::number(value, txt_format, num_precision));
		else
			d_table->setText(i, j, cellTexts[j-left]);
		}
	}

emit modifiedWindow(this);
QApplication::restoreOverrideCursor();
}

void Matrix::contextMenuEvent(QContextMenuEvent *e)
{
emit showContextMenu();
e->accept();
}

void Matrix::customEvent(QCustomEvent *e)
{
  if (e->type() == SCRIPTING_CHANGE_EVENT)
    scriptingChangeEvent((ScriptingChangeEvent*)e);
}

bool Matrix::eventFilter(QObject *object, QEvent *e)
{
QHeader *hheader = d_table->horizontalHeader();
QHeader *vheader = d_table->verticalHeader();

if (object == (QObject*)hheader) switch(e->type())
    {
        case QEvent::MouseButtonDblClick:
			{
			const QMouseEvent *me = (const QMouseEvent *)e;
			selectedCol = hheader->sectionAt (me->pos().x()+hheader->offset());
			return TRUE; 
			}

		case QEvent::MouseButtonPress:
			{
			const QMouseEvent *me = (const QMouseEvent *)e;
			int col = hheader->sectionAt (me->pos().x() + hheader->offset());
			if (me->button() == QMouseEvent::RightButton && !d_table->isColumnSelected(col,true))
				{
				selectedCol=col;
				d_table->clearSelection();
				d_table->selectColumn(selectedCol);
				d_table->setCurrentCell (0, selectedCol);
				}
			
			if (me->button() == QMouseEvent::LeftButton)	
				{
				LeftButton=TRUE;
				
				if (me->state ()==Qt::ControlButton)
					{
					int current=d_table->currentSelection();
					QTableSelection sel=d_table->selection(current);
					if (sel.topRow() != 0 || sel.bottomRow() != (d_table->numRows() - 1))
						//select only full columns
						d_table->removeSelection(sel);						
					}
				else
					d_table->clearSelection (TRUE);
				
				selectedCol=hheader->sectionAt (me->pos().x()+hheader->offset());
				lastSelectedCol=selectedCol;
				d_table->selectColumn (selectedCol);
				d_table->setCurrentCell (0, selectedCol);
				}		
			return TRUE; 
			}
		
		case QEvent::MouseMove:
			{
			if(LeftButton)
				{
				const QMouseEvent *me = (const QMouseEvent *)e;
				selectedCol=hheader->sectionAt (me->pos().x() + hheader->offset());

				if(selectedCol != lastSelectedCol)
					{// This means that we are in the next column
					if(d_table->isColumnSelected(selectedCol,TRUE))
						{//Since this column is selected, deselect it
						d_table->removeSelection(QTableSelection (0,lastSelectedCol,d_table->numRows()-1,lastSelectedCol));
						}
					else
						d_table->selectColumn (selectedCol);
					}
				lastSelectedCol=selectedCol;
				d_table->setCurrentCell (0, selectedCol);
				}
			return TRUE;
			}
			
		case QEvent::MouseButtonRelease:
			{
			LeftButton=FALSE;
			return TRUE;
			}
			
			default:
				;
    }
else if (e->type() == QEvent::MouseButtonPress && object == (QObject*)vheader)
	{
	const QMouseEvent *me = (const QMouseEvent *)e;
	int offset = vheader->offset();
	int row = vheader->sectionAt(me->pos().y()+offset);
	if (me->button() == QMouseEvent::RightButton && !d_table->isRowSelected(row,true))
		{
		d_table->clearSelection();
		d_table->selectRow (row);
		d_table->setCurrentCell (row,0);
		}
	}
else if (e->type()==QEvent::ContextMenu && object == titleBar)
	{
	emit showTitleBarMenu();
	((QContextMenuEvent*)e)->accept();
	return true;
	}
return QObject::eventFilter(object, e);
}

void Matrix::print()
{
QPrinter printer;
printer.setColorMode (QPrinter::GrayScale);
if (printer.setup()) 
	{
        printer.setFullPage( TRUE );
        QPainter p;
        if ( !p.begin(&printer ) )
            return; // paint on printer
        QPaintDeviceMetrics metrics( p.device() );
        int dpiy = metrics.logicalDpiY();
        const int margin = (int) ( (1/2.54)*dpiy ); // 1 cm margins
		
		QHeader *hHeader = d_table->horizontalHeader();
		QHeader *vHeader = d_table->verticalHeader();

		int rows=d_table->numRows();
		int cols=d_table->numCols();
		int height=margin;
		int i,vertHeaderWidth=vHeader->width();
		int right = margin + vertHeaderWidth;
		
		// print header
		p.setFont(QFont());
		QRect br=p.boundingRect(br,Qt::AlignCenter,	hHeader->label(0),-1,0);
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
		height+=tr.height();	
		p.drawLine(margin,height,right-1,height);		
		
		// print d_table values
		for (i=0;i<rows;i++)
			{
			right=margin;
			QString text=vHeader->label(i)+"\t";
			tr=p.boundingRect(tr,Qt::AlignCenter,text,-1,0);
			p.drawLine(right,height,right,height+tr.height());

			br.setTopLeft(QPoint(right,height));	
			br.setWidth(vertHeaderWidth);	
			br.setHeight(tr.height());
			p.drawText(br,Qt::AlignCenter,text,-1);
			right+=vertHeaderWidth;
			p.drawLine(right,height,right,height+tr.height());

			for (int j=0;j<cols;j++)
				{
				int w=d_table->columnWidth (j);
				text=d_table->text(i,j)+"\t";
				tr=p.boundingRect(tr,Qt::AlignCenter,text,-1,0);
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
}

void Matrix::range(double *min, double *max)
{
double d_min = d_table->text(0, 0).toDouble();
double d_max = d_min;
for (int i=0; i<d_table->numRows(); i++)
	{
	for (int j=0; j<d_table->numCols(); j++)
		{
		double aux = d_table->text(i, j).replace(",", ".").toDouble();
		if (aux <= d_min)
			d_min = aux;
		
		if (aux >= d_max)
			d_max = aux;
		}		
	}

*min = d_min;
*max = d_max;
}
