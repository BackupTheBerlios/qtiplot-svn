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
#include <ScriptEdit.h>

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
#include <QCompleter>
#include <QStringListModel>
#include <QInputDialog>
#include <QApplication>

FunctionDialog::FunctionDialog(ApplicationWindow* parent, bool standAlone, Qt::WFlags fl )
: QDialog( parent, fl ), d_app(parent), d_active_editor(0)
{
	QLocale locale = QLocale();
	int prec = 6;
	if (d_app){
		locale = d_app->locale();
		prec = d_app->d_decimal_digits;
	}

    setObjectName( "FunctionDialog" );
	QString recentTip = tr("Click here to select a recently typed expression");
	QString recentBtnText = tr("Rece&nt");

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

	QVBoxLayout *vl = new QVBoxLayout();
	vl->addWidget(new QLabel(tr( "f(x)= " )));

	buttonFunctionLog = new QPushButton(recentBtnText);
	buttonFunctionLog->setToolTip(recentTip);
	connect(buttonFunctionLog, SIGNAL(clicked()), this, SLOT(showFunctionLog()));
	vl->addWidget(buttonFunctionLog);
	vl->addStretch();

	QGridLayout *gl1 = new QGridLayout();
	gl1->addLayout(vl, 0, 0);

	boxFunction = new ScriptEdit(d_app->scriptingEnv());
	boxFunction->enableShortcuts();
	gl1->addWidget(boxFunction, 0, 1);

	gl1->addWidget(new QLabel(tr( "From x= " )), 1, 0);
	boxFrom = new DoubleSpinBox();
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

	int maxH = 80;
	gl2->addWidget(new QLabel(tr( "x = " )), 1, 0);
	boxXFunction = new ScriptEdit(d_app->scriptingEnv());
	boxXFunction->setMaximumHeight(maxH);
	boxXFunction->enableShortcuts();
	connect(boxXFunction, SIGNAL(activated(ScriptEdit *)), this, SLOT(setActiveEditor(ScriptEdit *)));
	gl2->addWidget(boxXFunction, 1, 1);

	buttonXParLog = new QPushButton(recentBtnText);
	buttonXParLog->setToolTip(recentTip);
	gl2->addWidget(buttonXParLog, 1, 2);
	connect(buttonXParLog, SIGNAL(clicked()), this, SLOT(showXParLog()));

	gl2->addWidget(new QLabel(tr( "y = " )), 2, 0);

	boxYFunction = new ScriptEdit(d_app->scriptingEnv());
	boxYFunction->setMaximumHeight(maxH);
	boxYFunction->enableShortcuts();
	connect(boxYFunction, SIGNAL(activated(ScriptEdit *)), this, SLOT(setActiveEditor(ScriptEdit *)));
	gl2->addWidget(boxYFunction, 2, 1);

	buttonYParLog = new QPushButton(recentBtnText);
	buttonYParLog->setToolTip(recentTip);
	gl2->addWidget(buttonYParLog, 2, 2);
	connect(buttonYParLog, SIGNAL(clicked()), this, SLOT(showYParLog()));

	gl2->addWidget(new QLabel(tr( "From" )), 3, 0);
	boxParFrom = new DoubleSpinBox();
	boxParFrom->setLocale(locale);
	boxParFrom->setDecimals(prec);
	gl2->addWidget(boxParFrom, 3, 1);

	gl2->addWidget(new QLabel(tr( "To" )), 4, 0);
	boxParTo = new DoubleSpinBox();
	boxParTo->setValue(1);
	boxParTo->setLocale(locale);
	boxParTo->setDecimals(prec);
	gl2->addWidget(boxParTo, 4, 1);

	gl2->addWidget(new QLabel(tr( "Points" )), 5, 0);
	boxParPoints = new QSpinBox();
	boxParPoints->setRange(2, INT_MAX);
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

	gl3->addWidget(new QLabel(tr( "R =" )), 1, 0);
	boxPolarRadius = new ScriptEdit(d_app->scriptingEnv());
	boxPolarRadius->setMaximumHeight(maxH);
	boxPolarRadius->enableShortcuts();
	connect(boxPolarRadius, SIGNAL(activated(ScriptEdit *)), this, SLOT(setActiveEditor(ScriptEdit *)));
	gl3->addWidget(boxPolarRadius, 1, 1);

	buttonPolarRadiusLog = new QPushButton(recentBtnText);
	buttonPolarRadiusLog->setToolTip(recentTip);
	gl3->addWidget(buttonPolarRadiusLog, 1, 2);
	connect(buttonPolarRadiusLog, SIGNAL(clicked()), this, SLOT(showPolarRadiusLog()));

	gl3->addWidget(new QLabel(tr( "Theta =" )), 2, 0);

	boxPolarTheta = new ScriptEdit(d_app->scriptingEnv());
	boxPolarTheta->setMaximumHeight(maxH);
	boxPolarTheta->enableShortcuts();
	connect(boxPolarTheta, SIGNAL(activated(ScriptEdit *)), this, SLOT(setActiveEditor(ScriptEdit *)));
	gl3->addWidget(boxPolarTheta, 2, 1);

	buttonPolarRThetaLog = new QPushButton(recentBtnText);
	buttonPolarRThetaLog->setToolTip(recentTip);
	gl3->addWidget(buttonPolarRThetaLog, 2, 2);
	connect(buttonPolarRThetaLog, SIGNAL(clicked()), this, SLOT(showPolarThetaLog()));

	gl3->addWidget(new QLabel(tr( "From" )), 3, 0);
	boxPolarFrom = new DoubleSpinBox();
	boxPolarFrom->setLocale(locale);
	boxPolarFrom->setDecimals(prec);
	gl3->addWidget(boxPolarFrom, 3, 1);

	gl3->addWidget(new QLabel(tr( "To" )), 4, 0);
	boxPolarTo = new DoubleSpinBox();
	boxPolarTo->setValue(M_PI);
	boxPolarTo->setLocale(locale);
	boxPolarTo->setDecimals(prec);
	gl3->addWidget(boxPolarTo, 4, 1);

	gl3->addWidget(new QLabel(tr( "Points" )), 5, 0);
	boxPolarPoints = new QSpinBox();
	boxPolarPoints->setRange(2, INT_MAX);
	boxPolarPoints->setSingleStep(100);
	boxPolarPoints->setValue(100);
	gl3->addWidget(boxPolarPoints, 5, 1);
	gl3->setRowStretch(6, 1);

	polarPage = new QWidget();
	polarPage->setLayout(gl3);
	optionStack->addWidget( polarPage );

	boxFunctionExplain = new QTextEdit;
	boxFunctionExplain->setReadOnly(true);
	boxFunctionExplain->setMaximumHeight(80);
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

	buttonClear = new QPushButton(tr( "Clea&r Function" ));
	connect( buttonClear, SIGNAL( clicked() ), this, SLOT(clearList()));
	vbox->addWidget(buttonClear);
	vbox->addStretch();

	QHBoxLayout *hbox3 = new QHBoxLayout();
	hbox3->addWidget(boxFunctionExplain);
	hbox3->addLayout(vbox);

	QVBoxLayout *vbox2 = new QVBoxLayout();
	vbox2->addLayout(hbox3);
	vbox2->addStretch();

	QVBoxLayout *vbox1 = new QVBoxLayout(this);
    vbox1->addLayout(hbox1);
	vbox1->addWidget(optionStack);
	vbox1->addLayout(vbox2);
	vbox1->addStretch();

	if (standAlone){
		buttonOk = new QPushButton(tr( "&Ok" ));
		buttonOk->setDefault(true);
		connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );

		buttonCancel = new QPushButton(tr( "&Close" ));
		buttonCancel->setAutoDefault(false);
		connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );

		QHBoxLayout *hbox2 = new QHBoxLayout();
		hbox2->addWidget(buttonOk);
		hbox2->addWidget(buttonCancel);
		hbox2->addStretch();

		vbox1->addLayout(hbox2);

		setSizeGripEnabled(true);
		setWindowTitle( tr( "QtiPlot - Add function curve" ) );
		setAttribute(Qt::WA_DeleteOnClose);
	}

	setFocusProxy (boxFunction);
	connect( boxType, SIGNAL( activated(int) ), this, SLOT( raiseWidget(int) ) );

	curveID = -1;
	graph = 0;
}

void FunctionDialog::raiseWidget(int index)
{
	optionStack->setCurrentIndex(index);
}

void FunctionDialog::setCurveToModify(FunctionCurve *c)
{
	Graph *g = (Graph *)c->plot();
	if (!g)
		return;

	setCurveToModify(g, g->curveIndex(c));
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
		QMapIterator<QString, double> i(constants);
		int row = 0;
 		while (i.hasNext()) {
     		i.next();
			boxConstants->setItem(row, 0, new QTableWidgetItem(i.key()));

			DoubleSpinBox *sb = new DoubleSpinBox();
			sb->setLocale(QLocale());
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
		boxPolarRadius->setText(formulas[0]);
		boxPolarTheta->setText(formulas[1]);
		boxPolarParameter->setText(c->variable());
		boxPolarFrom->setValue(c->startRange());
		boxPolarTo->setValue(c->endRange());
		boxPolarPoints->setValue(c->dataSize());
	} else if (c->functionType() == FunctionCurve::Parametric) {
		boxXFunction->setText(formulas[0]);
		boxYFunction->setText(formulas[1]);
		boxParameter->setText(c->variable());
		boxParFrom->setValue(c->startRange());
		boxParTo->setValue(c->endRange());
		boxParPoints->setValue(c->dataSize());
	}
	boxType->setCurrentIndex(c->functionType());
	optionStack->setCurrentIndex(c->functionType());
}

void FunctionDialog::clearList()
{
	int type = boxType->currentItem();
	switch (type)
	{
		case 0:
			boxFunction->clear();
			break;

		case 1:
			boxXFunction->clear();
			boxYFunction->clear();
			break;

		case 2:
			boxPolarTheta->clear();
			boxPolarRadius->clear();
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
	if (!error && d_app){
		d_app->updateFunctionLists(type, formulas);
		if (!graph){
			MultiLayer *plot = d_app->newFunctionPlot(formulas, start, end, boxPoints->value(), "x", type);
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
	double start = boxParFrom->value();
	double end = boxParTo->value();

	if (start >= end){
		QMessageBox::critical(this, tr("QtiPlot - Input error"),
				tr("Please enter parameter limits that satisfy: from < end!"));
		boxParTo->setFocus();
		return;
	}

	double parameter;
	QString xformula = boxXFunction->text().simplified();
	QString yformula = boxYFunction->text().simplified();
	bool error = false;

	try {
		MyParser parser;
		parser.DefineVar((boxParameter->text()).ascii(), &parameter);
		parser.SetExpr(xformula.ascii());

		parameter = start;
		parser.Eval();
		parameter = end;
		parser.Eval();
	} catch(mu::ParserError &e) {
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), QString::fromStdString(e.GetMsg()));
		boxXFunction->setFocus();
		error=true;
	}

	try {
		MyParser parser;
		parser.DefineVar((boxParameter->text()).ascii(), &parameter);
		parser.SetExpr(yformula.ascii());

		parameter=start;
		parser.Eval();
		parameter=end;
		parser.Eval();
	} catch(mu::ParserError &e) {
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), QString::fromStdString(e.GetMsg()));
		boxYFunction->setFocus();
		error=true;
	}
	// Collecting all the information
	int type = boxType->currentItem();
	QStringList formulas;
	formulas += xformula;
	formulas += yformula;
	if (!error && d_app){
		d_app->updateFunctionLists(type, formulas);
		if (!graph){
			MultiLayer *plot = d_app->newFunctionPlot(formulas, start, end, boxParPoints->value(), boxParameter->text(), type);
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
	double start = boxPolarFrom->value();
	double end = boxPolarTo->value();
	if (start >= end){
		QMessageBox::critical(this, tr("QtiPlot - Input error"),
				tr("Please enter parameter limits that satisfy: from < end!"));
		boxPolarTo->setFocus();
		return;
	}

	double parameter;
	QString rformula = boxPolarRadius->text().simplified();
	QString tformula = boxPolarTheta->text().simplified();
	bool error = false;

	try {
		MyParser parser;
		parser.DefineVar((boxPolarParameter->text()).ascii(), &parameter);
		parser.SetExpr(rformula.ascii());

		parameter = start;
		parser.Eval();
		parameter = end;
		parser.Eval();
	} catch(mu::ParserError &e) {
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), QString::fromStdString(e.GetMsg()));
		boxPolarRadius->setFocus();
		error = true;
	}

	try {
		MyParser parser;
		parser.DefineVar((boxPolarParameter->text()).ascii(), &parameter);
		parser.SetExpr(tformula.ascii());

		parameter = start;
		parser.Eval();
		parameter = end;
		parser.Eval();
	} catch(mu::ParserError &e) {
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), QString::fromStdString(e.GetMsg()));
		boxPolarTheta->setFocus();
		error = true;
	}
	// Collecting all the information
	int type = boxType->currentItem();
	QStringList formulas;
	formulas += rformula;
	formulas += tformula;
	if (!error && d_app){
		d_app->updateFunctionLists(type, formulas);

		if (!graph){
			MultiLayer *plot = d_app->newFunctionPlot(formulas, start, end, boxPolarPoints->value(), boxPolarParameter->text(), type);
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

void FunctionDialog::showFunctionLog()
{
	if (!d_app)
		return;

	if (d_app->d_recent_functions.isEmpty()){
		QMessageBox::information(this, tr("QtiPlot"), tr("Sorry, there are no recent expressions available!"));
		return;
	}

	bool ok;
	QString s = QInputDialog::getItem(this, tr("QtiPlot") + " - " + tr("Recent Functions"), tr("Please, choose a function:"), d_app->d_recent_functions, 0, false, &ok);
	if (ok && !s.isEmpty())
		boxFunction->setText(s);
}

void FunctionDialog::showXParLog()
{
	if (!d_app)
		return;

	if (d_app->xFunctions.isEmpty()){
		QMessageBox::information(this, tr("QtiPlot"), tr("Sorry, there are no recent expressions available!"));
		return;
	}

	bool ok;
	QString s = QInputDialog::getItem(this, tr("QtiPlot") + " - " + tr("Recent Functions"), tr("Please, choose a function:"), d_app->xFunctions, 0, false, &ok);
	if (ok && !s.isEmpty())
		boxXFunction->setText(s);
}


void FunctionDialog::showYParLog()
{
	if (!d_app)
		return;

	if (d_app->yFunctions.isEmpty()){
		QMessageBox::information(this, tr("QtiPlot"), tr("Sorry, there are no recent expressions available!"));
		return;
	}

	bool ok;
	QString s = QInputDialog::getItem(this, tr("QtiPlot") + " - " + tr("Recent Functions"), tr("Please, choose a function:"), d_app->yFunctions, 0, false, &ok);
	if (ok && !s.isEmpty())
		boxYFunction->setText(s);
}

void FunctionDialog::showPolarRadiusLog()
{
	if (!d_app)
		return;

	if (d_app->rFunctions.isEmpty()){
		QMessageBox::information(this, tr("QtiPlot"), tr("Sorry, there are no recent expressions available!"));
		return;
	}

	bool ok;
	QString s = QInputDialog::getItem(this, tr("QtiPlot") + " - " + tr("Recent Functions"), tr("Please, choose a function:"), d_app->rFunctions, 0, false, &ok);
	if (ok && !s.isEmpty())
		boxPolarRadius->setText(s);
}


void FunctionDialog::showPolarThetaLog()
{
	if (!d_app)
		return;

	if (d_app->thetaFunctions.isEmpty()){
		QMessageBox::information(this, tr("QtiPlot"), tr("Sorry, there are no recent expressions available!"));
		return;
	}

	bool ok;
	QString s = QInputDialog::getItem(this, tr("QtiPlot") + " - " + tr("Recent Functions"), tr("Please, choose a function:"), d_app->thetaFunctions, 0, false, &ok);
	if (ok && !s.isEmpty())
		boxPolarTheta->setText(s);
}

void FunctionDialog::insertFunction()
{
	QString fname = boxMathFunctions->currentText().remove("(").remove(")").remove(",").remove(";");
	if (optionStack->currentWidget () == functionPage){
		boxFunction->insertFunction(fname);
		boxFunction->setFocus();
	} else if (optionStack->currentWidget () == parametricPage){
		if (d_active_editor == boxYFunction){
			boxYFunction->insertFunction(fname);
			boxYFunction->setFocus();
		} else if (d_active_editor == boxXFunction){
			boxXFunction->insertFunction(fname);
			boxXFunction->setFocus();
		}
	} else if (optionStack->currentWidget () == polarPage){
		if (d_active_editor == boxPolarRadius){
			boxPolarRadius->insertFunction(fname);
			boxPolarRadius->setFocus();
		} else if (d_active_editor == boxPolarTheta){
			boxPolarTheta->insertFunction(fname);
			boxPolarTheta->setFocus();
		}
	}
}

void FunctionDialog::updateFunctionExplain(int index)
{
	boxFunctionExplain->setText(MyParser::explainFunction(index));
}
