/***************************************************************************
    File                 : FunctionDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2009 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Function dialog

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
#include "FunctionDialog.h"
#include <MyParser.h>
#include <ApplicationWindow.h>
#include <MultiLayer.h>
#include <FunctionCurve.h>
#include <DoubleSpinBox.h>

#include <QTextEdit>
#include <QLineEdit>
#include <QLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QStackedWidget>
#include <QWidget>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>

FunctionDialog::FunctionDialog( QWidget* parent, Qt::WFlags fl )
: QDialog( parent, fl )
{
	ApplicationWindow *app = (ApplicationWindow *)parent;
	QLocale locale = QLocale();
	int prec = 6;
	if (app){
		locale = app->locale();
		prec = app->d_decimal_digits;
	}

    setObjectName( "FunctionDialog" );
	setWindowTitle( tr( "QtiPlot - Add function curve" ) );
	setSizeGripEnabled(true);
	setAttribute(Qt::WA_DeleteOnClose);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->addWidget(new QLabel(tr( "Curve type " )));
	boxType = new QComboBox();
	boxType->addItem( tr( "Function" ) );
	boxType->addItem( tr( "Parametric plot" ) );
	boxType->addItem( tr( "Polar plot" ) );
	hbox1->addWidget(boxType);

	optionStack = new QStackedWidget();
	optionStack->setFrameShape( QFrame::StyledPanel );
	optionStack->setFrameShadow( QStackedWidget::Plain );

	QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(new QLabel(tr( "f(x)= " )), 0, 0);
	boxFunction = new QTextEdit();
	boxFunction->setMinimumWidth(350);
	gl1->addWidget(boxFunction, 0, 1);
	gl1->addWidget(new QLabel(tr( "From x= " )), 1, 0);
	boxFrom = new DoubleSpinBox();
	boxFrom->setValue(0);
	boxFrom->setLocale(locale);
	boxFrom->setDecimals(prec);
	gl1->addWidget(boxFrom, 1, 1);
	gl1->addWidget(new QLabel(tr( "To x= " )), 2, 0);
	boxTo = new DoubleSpinBox();
	boxTo->setValue(1);
	boxTo->setLocale(locale);
	boxTo->setDecimals(prec);
	gl1->addWidget(boxTo, 2, 1);
	gl1->addWidget(new QLabel(tr( "Points" )), 3, 0);
	boxPoints = new QSpinBox();
	boxPoints->setRange(2, INT_MAX);
	boxPoints->setSingleStep(100);
	boxPoints->setValue(100);
	gl1->addWidget(boxPoints, 3, 1);

	boxConstants = new QTableWidget();
    boxConstants->setColumnCount(2);
    boxConstants->horizontalHeader()->setClickable(false);
    boxConstants->horizontalHeader()->setResizeMode (0, QHeaderView::ResizeToContents);
	boxConstants->horizontalHeader()->setResizeMode (1, QHeaderView::Stretch);
    QStringList header = QStringList() << tr("Constant") << tr("Value");
    boxConstants->setHorizontalHeaderLabels(header);
    boxConstants->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    boxConstants->verticalHeader()->hide();
	boxConstants->setMinimumWidth(200);
	boxConstants->hide();

	functionPage = new QWidget();

	QHBoxLayout *hb = new QHBoxLayout(functionPage);
	hb->addLayout(gl1);
	hb->addWidget(boxConstants);

	optionStack->addWidget( functionPage );

	QGridLayout *gl2 = new QGridLayout();
    gl2->addWidget(new QLabel(tr( "Parameter" )), 0, 0);
	boxParameter = new QLineEdit();
	boxParameter->setText("m");
	gl2->addWidget(boxParameter, 0, 1);
	gl2->addWidget(new QLabel(tr( "From" )), 1, 0);
	boxParFrom = new QLineEdit();
	boxParFrom->setText("0");
	gl2->addWidget(boxParFrom, 1, 1);
	gl2->addWidget(new QLabel(tr( "To" )), 2, 0);
	boxParTo = new QLineEdit();
	boxParTo->setText("1");
	gl2->addWidget(boxParTo, 2, 1);
	gl2->addWidget(new QLabel(tr( "x = " )), 3, 0);
	boxXFunction = new QComboBox( );
	boxXFunction->setEditable ( true );
	gl2->addWidget(boxXFunction, 3, 1);
	gl2->addWidget(new QLabel(tr( "y = " )), 4, 0);
	boxYFunction = new QComboBox( );
	boxYFunction->setEditable ( true );
	gl2->addWidget(boxYFunction, 4, 1);
	gl2->addWidget(new QLabel(tr( "Points" )), 5, 0);
	boxParPoints = new QSpinBox();
	boxParPoints->setRange(2, 1000000);
	boxParPoints->setSingleStep(100);
	boxParPoints->setValue(100);
	gl2->addWidget(boxParPoints, 5, 1);
	gl2->setRowStretch(6, 1);

	parametricPage = new QWidget();
	parametricPage->setLayout(gl2);
	optionStack->addWidget( parametricPage );

	QGridLayout *gl3 = new QGridLayout();
    gl3->addWidget(new QLabel(tr( "Parameter" )), 0, 0);
	boxPolarParameter = new QLineEdit();
	boxPolarParameter->setText ("t");
	gl3->addWidget(boxPolarParameter, 0, 1);
	gl3->addWidget(new QLabel(tr( "From" )), 2, 0);
	boxPolarFrom = new QLineEdit();
	boxPolarFrom->setText("0");
	gl3->addWidget(boxPolarFrom, 2, 1);
	gl3->addWidget(new QLabel(tr( "To" )), 3, 0);
	boxPolarTo = new QLineEdit();
	boxPolarTo->setText("pi");
	gl3->addWidget(boxPolarTo, 3, 1);
	gl3->addWidget(new QLabel(tr( "R =" )), 4, 0);
	boxPolarRadius = new QComboBox();
	boxPolarRadius->setEditable ( true );
	gl3->addWidget(boxPolarRadius, 4, 1);
	gl3->addWidget(new QLabel(tr( "Theta =" )), 5, 0);
	boxPolarTheta = new QComboBox();
	boxPolarTheta->setEditable ( true );
	gl3->addWidget(boxPolarTheta, 5, 1);
	gl3->addWidget(new QLabel(tr( "Points" )), 6, 0);
	boxPolarPoints = new QSpinBox();
	boxPolarPoints->setRange(2, 1000000);
	boxPolarPoints->setSingleStep(100);
	boxPolarPoints->setValue(100);
	gl3->addWidget(boxPolarPoints, 6, 1);
	gl3->setRowStretch(7, 1);

	polarPage = new QWidget();
	polarPage->setLayout(gl3);
	optionStack->addWidget( polarPage );

	boxFunctionExplain = new QTextEdit;
	boxFunctionExplain->setReadOnly(true);
	boxFunctionExplain->setMaximumHeight(50);
	QPalette palette = boxFunctionExplain->palette();
	palette.setColor(QPalette::Active, QPalette::Base, Qt::lightGray);
	boxFunctionExplain->setPalette(palette);

	addFunctionBtn = new QPushButton(tr( "&Add Function" ));
	addFunctionBtn->setAutoDefault(false);
	connect(addFunctionBtn, SIGNAL(clicked()), this, SLOT(insertFunction()));

	boxMathFunctions = new QComboBox();
	boxMathFunctions->addItems(MyParser::functionsList());
	connect(boxMathFunctions, SIGNAL(activated(int)), this, SLOT(updateFunctionExplain(int)));
	updateFunctionExplain(0);

	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->addWidget(boxMathFunctions);
	vbox->addWidget(addFunctionBtn);

	QHBoxLayout *hbox3 = new QHBoxLayout();
	hbox3->addWidget(boxFunctionExplain);
	hbox3->addLayout(vbox);

	buttonClear = new QPushButton(tr( "Clea&r Function" ));
	buttonClear->setAutoDefault(false);
	buttonOk = new QPushButton(tr( "&Ok" ));
	buttonOk->setDefault(true);
	buttonCancel = new QPushButton(tr( "&Close" ));
	buttonCancel->setAutoDefault(false);

	QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->addWidget(buttonClear);
	hbox2->addWidget(buttonOk);
	hbox2->addWidget(buttonCancel);
	hbox2->addStretch();

	QVBoxLayout *vbox1 = new QVBoxLayout();
    vbox1->addLayout(hbox1);
	vbox1->addWidget(optionStack);
	vbox1->addLayout(hbox3);
	vbox1->addLayout(hbox2);

	setLayout(vbox1);
	setFocusProxy (boxFunction);

	connect( boxType, SIGNAL( activated(int) ), this, SLOT( raiseWidget(int) ) );
	connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
	connect( buttonClear, SIGNAL( clicked() ), this, SLOT(clearList() ) );

	curveID = -1;
	graph = 0;
}

void FunctionDialog::raiseWidget(int index)
{
	if (index)
		buttonClear->setText( tr( "Clear list" ) );
	else
		buttonClear->setText( tr( "Clear Function" ) );

	optionStack->setCurrentIndex(index);
}

void FunctionDialog::setCurveToModify(Graph *g, int curve)
{
	if (!g)
		return;

	graph = g;

	FunctionCurve *c = (FunctionCurve *)graph->curve(curve);
	if (!c)
		return;

	curveID = curve;
	QStringList formulas = c->formulas();

	QMap<QString, double> constants = c->constants();
	if (!constants.isEmpty()){
		boxConstants->clearContents();
		boxConstants->setRowCount(constants.size());
		boxConstants->show();
		ApplicationWindow *app = (ApplicationWindow *)parent();
		QMapIterator<QString, double> i(constants);
		int row = 0;
 		while (i.hasNext()) {
     		i.next();
			boxConstants->setItem(row, 0, new QTableWidgetItem(i.key()));

			DoubleSpinBox *sb = new DoubleSpinBox();
			sb->setLocale(app->locale());
			sb->setDecimals(app->fit_output_precision);
			sb->setValue(i.value());
        	boxConstants->setCellWidget(row, 1, sb);
			connect(sb, SIGNAL(valueChanged(double)), this, SLOT(acceptFunction()));
			row++;
 		}
	}

	if (c->functionType() == FunctionCurve::Normal){
		boxFunction->setText(formulas[0]);
		boxFrom->setValue(c->startRange());
		boxTo->setValue(c->endRange());
		boxPoints->setValue(c->dataSize());
	} else if (c->functionType() == FunctionCurve::Polar) {
		optionStack->setCurrentIndex(2);
		boxType->setCurrentItem(2);

		boxPolarRadius->setCurrentText(formulas[0]);
		boxPolarTheta->setCurrentText(formulas[1]);
		boxPolarParameter->setText(c->variable());
		boxPolarFrom->setText(QString::number(c->startRange(), 'g', 15));
		boxPolarTo->setText(QString::number(c->endRange(), 'g', 15));
		boxPolarPoints->setValue(c->dataSize());
	} else if (c->functionType() == FunctionCurve::Parametric) {
		boxType->setCurrentItem(1);
		optionStack->setCurrentIndex(1);

		boxXFunction->setCurrentText(formulas[0]);
		boxYFunction->setCurrentText(formulas[1]);
		boxParameter->setText(c->variable());
		boxParFrom->setText(QString::number(c->startRange(), 'g', 15));
		boxParTo->setText(QString::number(c->endRange(), 'g', 15));
		boxParPoints->setValue(c->dataSize());
	}
}

void FunctionDialog::clearList()
{
	int type=boxType->currentItem();
	switch (type)
	{
		case 0:
			boxFunction->clear();
			break;

		case 1:
			boxXFunction->clear();
			boxYFunction->clear();
			emit clearParamFunctionsList();
			break;

		case 2:
			boxPolarTheta->clear();
			boxPolarRadius->clear();
			emit clearPolarFunctionsList();
			break;
	}
}

void FunctionDialog::acceptFunction()
{
	double start = boxFrom->value();
	double end = boxTo->value();
	if (start >= end){
		QMessageBox::critical(this, tr("QtiPlot - Input error"), tr("Please enter x limits that satisfy: from < end!"));
		boxTo->setFocus();
		return;
	}

	QMap<QString, double> constants;

	QString formula = boxFunction->text().simplified();
	bool error = false;
	try {
		double x = start;
		MyParser parser;
		parser.DefineVar("x", &x);
		for (int i = 0; i < boxConstants->rowCount(); i++){
			double val = ((DoubleSpinBox*)boxConstants->cellWidget(i, 1))->value();
			QString constName = boxConstants->item(i, 0)->text();
			if (!constName.isEmpty()){
				constants.insert(constName, val);
				parser.DefineConst(constName.ascii(), val);
			}
		}
		parser.SetExpr(formula.ascii());

		parser.Eval();
		x = end;
		parser.Eval();
	} catch(mu::ParserError &e) {
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), QString::fromStdString(e.GetMsg()));
		boxFunction->setFocus();
		error=true;
	}

	// Collecting all the information
	int type = boxType->currentItem();
	QStringList formulas;
	formulas += formula;
	if (!error){
		ApplicationWindow *app = (ApplicationWindow *)this->parent();
		app->updateFunctionLists(type,formulas);
		if (!graph){
			MultiLayer *plot = app->newFunctionPlot(formulas, start, end, boxPoints->value(), "x", type);
			if (plot)
				graph = plot->activeLayer();
		} else {
			if (curveID >= 0)
				graph->modifyFunctionCurve(curveID, type, formulas, "x", start, end, boxPoints->value(), constants);
			else
				graph->addFunction(formulas, start, end, boxPoints->value(), "x", type);
		}
	}

}
void FunctionDialog::acceptParametric()
{
	QString from=boxParFrom->text().lower();
	QString to=boxParTo->text().lower();
	QString points=boxParPoints->text().lower();

	double start = 0.0, end = 0.0;
	try
	{
		MyParser parser;
		parser.SetExpr(from.ascii());
		start = parser.Eval();
	}
	catch(mu::ParserError &e)
	{
		QMessageBox::critical(this, tr("QtiPlot - Start limit error"), QString::fromStdString(e.GetMsg()));
		boxParFrom->setFocus();
		return;
	}

	try
	{
		MyParser parser;
		parser.SetExpr(to.ascii());
		end=parser.Eval();
	}
	catch(mu::ParserError &e)
	{
		QMessageBox::critical(this, tr("QtiPlot - End limit error"), QString::fromStdString(e.GetMsg()));
		boxParTo->setFocus();
		return;
	}

	if (start>=end)
	{
		QMessageBox::critical(this, tr("QtiPlot - Input error"),
				tr("Please enter parameter limits that satisfy: from < end!"));
		boxParTo->setFocus();
		return;
	}

	double parameter;
	QString xformula=boxXFunction->currentText();
	QString yformula=boxYFunction->currentText();
	bool error=false;

	try
	{
		MyParser parser;
		parser.DefineVar((boxParameter->text()).ascii(), &parameter);
		parser.SetExpr(xformula.ascii());

		parameter=start;
		parser.Eval();
		parameter=end;
		parser.Eval();
	}
	catch(mu::ParserError &e)
	{
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), QString::fromStdString(e.GetMsg()));
		boxXFunction->setFocus();
		error=true;
	}
	try
	{
		MyParser parser;
		parser.DefineVar((boxParameter->text()).ascii(), &parameter);
		parser.SetExpr(yformula.ascii());

		parameter=start;
		parser.Eval();
		parameter=end;
		parser.Eval();
	}
	catch(mu::ParserError &e)
	{
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), QString::fromStdString(e.GetMsg()));
		boxYFunction->setFocus();
		error=true;
	}
	// Collecting all the information
	int type = boxType->currentItem();
	QStringList formulas;
	formulas += xformula;
	formulas += yformula;
	if (!error){
		ApplicationWindow *app = (ApplicationWindow *)this->parent();
		app->updateFunctionLists(type,formulas);
		if (!graph){
			MultiLayer *plot = app->newFunctionPlot(formulas, start, end, boxParPoints->value(), boxParameter->text(), type);
			if (plot)
				graph = plot->activeLayer();
		} else {
			if (curveID >= 0)
				graph->modifyFunctionCurve(curveID, type, formulas, boxParameter->text(), start, end, boxParPoints->value(), QMap<QString, double>());
			else
				graph->addFunction(formulas, start, end, boxParPoints->value(), boxParameter->text(), type);
		}
	}
}

void FunctionDialog::acceptPolar()
{
	QString from=boxPolarFrom->text().lower();
	QString to=boxPolarTo->text().lower();
	QString points=boxPolarPoints->text().lower();

	double start = 0, end = 0;
	try
	{
		MyParser parser;
		parser.SetExpr(from.ascii());
		start=parser.Eval();
	}
	catch(mu::ParserError &e)
	{
		QMessageBox::critical(this, tr("QtiPlot - Start limit error"), QString::fromStdString(e.GetMsg()));
		boxPolarFrom->setFocus();
		return;
	}

	try
	{
		MyParser parser;
		parser.SetExpr(to.ascii());
		end=parser.Eval();
	}
	catch(mu::ParserError &e)
	{
		QMessageBox::critical(this, tr("QtiPlot - End limit error"), QString::fromStdString(e.GetMsg()));
		boxPolarTo->setFocus();
		return;
	}

	if (start>=end)
	{
		QMessageBox::critical(this, tr("QtiPlot - Input error"),
				tr("Please enter parameter limits that satisfy: from < end!"));
		boxPolarTo->setFocus();
		return;
	}

	double parameter;
	QString rformula=boxPolarRadius->currentText();
	QString tformula=boxPolarTheta->currentText();
	bool error=false;

	try
	{
		MyParser parser;
		parser.DefineVar((boxPolarParameter->text()).ascii(), &parameter);
		parser.SetExpr(rformula.ascii());

		parameter=start;
		parser.Eval();
		parameter=end;
		parser.Eval();
	}
	catch(mu::ParserError &e)
	{
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), QString::fromStdString(e.GetMsg()));
		boxPolarRadius->setFocus();
		error=true;
	}
	try
	{
		MyParser parser;
		parser.DefineVar((boxPolarParameter->text()).ascii(), &parameter);
		parser.SetExpr(tformula.ascii());

		parameter=start;
		parser.Eval();
		parameter=end;
		parser.Eval();
	}
	catch(mu::ParserError &e)
	{
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), QString::fromStdString(e.GetMsg()));
		boxPolarTheta->setFocus();
		error=true;
	}
	// Collecting all the information
	int type = boxType->currentItem();
	QStringList formulas;
	formulas+=rformula;
	formulas+=tformula;
	if (!error){
		ApplicationWindow *app = (ApplicationWindow *)this->parent();
		app->updateFunctionLists(type,formulas);

		if (!graph){
			MultiLayer *plot = app->newFunctionPlot(formulas, start, end, boxPolarPoints->value(), boxPolarParameter->text(), type);
			if (plot)
				graph = plot->activeLayer();
		} else {
			if (curveID >= 0)
				graph->modifyFunctionCurve(curveID, type, formulas, boxPolarParameter->text(), start, end, boxPolarPoints->value(), QMap<QString, double>());
			else
				graph->addFunction(formulas, start, end, boxPolarPoints->value(), boxPolarParameter->text(), type);
		}
	}
}

void FunctionDialog::accept()
{
	switch (boxType->currentIndex())
	{
		case 0:
			acceptFunction();
			break;

		case 1:
			acceptParametric();
			break;

		case 2:
			acceptPolar();
			break;
	}
}

void FunctionDialog::insertParamFunctionsList(const QStringList& xList, const QStringList& yList)
{
	boxXFunction->insertItems (0, xList);
	boxYFunction->insertItems (0, yList);
}

void FunctionDialog::insertPolarFunctionsList(const QStringList& rList, const QStringList& thetaList)
{
	boxPolarRadius->insertItems (0, rList);
	boxPolarTheta->insertItems (0, thetaList);
}

void FunctionDialog::insertFunction()
{
	QString fname = boxMathFunctions->currentText();
	if (optionStack->currentWidget () == functionPage){
		QTextCursor cursor = boxFunction->textCursor();
		QString markedText = cursor.selectedText();
		if(markedText.isEmpty()){
			cursor.insertText(fname);
			cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor,1);
		} else
			cursor.insertText(fname.remove(")").remove(",") + markedText + ")");

		boxFunction->setTextCursor(cursor);
	} else if (optionStack->currentWidget () == parametricPage){
		if (boxYFunction->lineEdit()->hasFocus())
			boxYFunction->lineEdit()->insert(fname);
		else
			boxXFunction->lineEdit()->insert(fname);
	} else if (optionStack->currentWidget () == polarPage){
		if (boxPolarRadius->lineEdit()->hasFocus())
			boxPolarRadius->lineEdit()->insert(fname);
		else
			boxPolarTheta->lineEdit()->insert(fname);
	}
}

void FunctionDialog::updateFunctionExplain(int index)
{
	boxFunctionExplain->setText(MyParser::explainFunction(index));
}
