/***************************************************************************
    File                 : PlotDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2006-2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Custom curves dialog

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
#include "PlotDialog.h"
#include <ApplicationWindow.h>
#include <Table.h>
#include <MyParser.h>
#include <QwtHistogram.h>
#include <VectorCurve.h>
#include <QwtErrorPlotCurve.h>
#include <BoxCurve.h>
#include <FunctionCurve.h>
#include <Spectrogram.h>
#include <QwtPieCurve.h>
#include <Folder.h>
#include <ColorMapEditor.h>
#include <DoubleSpinBox.h>
#include <PenStyleBox.h>
#include <ColorBox.h>
#include <ColorButton.h>
#include <PatternBox.h>
#include <SymbolBox.h>
#include <ContourLinesEditor.h>
#include <FunctionDialog.h>

#include <QTreeWidget>
#include <QLineEdit>
#include <QLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QWidget>
#include <QMessageBox>
#include <QComboBox>
#include <QWidgetList>
#include <QFileDialog>
#include <QGroupBox>
#include <QFontDialog>
#include <QShortcut>
#include <QKeySequence>
#include <QDoubleSpinBox>
#include <QMenu>
#include <QDateTime>
#include <QSlider>

#include <qwt_plot_canvas.h>

PlotDialog::PlotDialog(bool showExtended, QWidget* parent, Qt::WFlags fl )
: QDialog(parent, fl),
  d_ml(0)
{
    setName( "PlotDialog" );
	setWindowTitle( tr( "QtiPlot - Plot details" ) );
	setModal(true);
	setSizeGripEnabled(true);
    setAttribute(Qt::WA_DeleteOnClose);

	listBox = new QTreeWidget();
    listBox->setColumnCount(1);
	listBox->header()->hide();
    listBox->setIndentation(15);

    QGridLayout *gl = new QGridLayout(this);
    gl->addWidget(listBox, 0, 0);

	privateTabWidget = new QTabWidget();
    gl->addWidget(privateTabWidget, 0, 1);

    curvePlotTypeBox = new QWidget();
    QHBoxLayout *hb1 = new QHBoxLayout(curvePlotTypeBox);
    hb1->addWidget(new QLabel(tr("Plot type")));
    boxPlotType = new QComboBox();
    boxPlotType->setEditable(false);
    hb1->addWidget(boxPlotType);
    gl->addWidget(curvePlotTypeBox, 1, 0);

	initAxesPage();
	initLinePage();
	initSymbolsPage();
	initHistogramPage();
	initErrorsPage();
	initSpacingPage();
	initVectPage();
	initBoxPage();
	initPercentilePage();
	initSpectrogramValuesPage();
	initSpectrogramPage();
	initContourLinesPage();
	initPiePage();
	initPieGeometryPage();
	initPieLabelsPage();
	initLayerPage();
	initLayerGeometryPage();
	initLayerSpeedPage();
	initFontsPage();
	initPrintPage();
	initMiscPage();
	initLabelsPage();
	initFunctionPage();
	initPlotGeometryPage();
	clearTabWidget();

    QHBoxLayout* hb2 = new QHBoxLayout();
	btnMore = new QPushButton("&<<");
	btnMore->setFixedWidth(30);
	btnMore->setCheckable(true);
	if (showExtended)
		btnMore->toggle ();
    hb2->addWidget(btnMore);
	btnWorksheet = new QPushButton(tr( "&Worksheet" ) );
    hb2->addWidget(btnWorksheet);
	buttonOk = new QPushButton(tr( "&OK" ));
	buttonOk->setDefault( true );
    hb2->addWidget(buttonOk);
	buttonCancel = new QPushButton(tr( "&Cancel" ));
    hb2->addWidget(buttonCancel);
    buttonApply = new QPushButton(tr( "&Apply" ));
    hb2->addWidget(buttonApply);
	btnEditCurve = new QPushButton(tr("&Plot Associations..."));
    hb2->addWidget(btnEditCurve);
    hb2->addStretch();
    gl->addLayout(hb2, 1, 1);

	connect(btnMore, SIGNAL(toggled(bool)), this, SLOT(showAll(bool)));

	connect( buttonOk, SIGNAL(clicked()), this, SLOT(quit()));
	connect( buttonCancel, SIGNAL(clicked()), this, SLOT(close()));
	connect( buttonApply, SIGNAL(clicked() ), this, SLOT(acceptParams()));
	connect( btnWorksheet, SIGNAL(clicked()), this, SLOT(showWorksheet()));
	connect( btnEditCurve, SIGNAL(clicked()), this, SLOT(editCurve()));
	connect(listBox, SIGNAL(itemDoubleClicked( QTreeWidgetItem *, int)),
            this, SLOT(showPlotAssociations( QTreeWidgetItem *, int)));
	connect(listBox, SIGNAL(currentItemChanged (QTreeWidgetItem *, QTreeWidgetItem *)),
            this, SLOT(updateTabWindow(QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(listBox, SIGNAL(itemCollapsed(QTreeWidgetItem *)), this, SLOT(updateTreeWidgetItem(QTreeWidgetItem *)));
    connect(listBox, SIGNAL(itemExpanded(QTreeWidgetItem *)), this, SLOT(updateTreeWidgetItem(QTreeWidgetItem *)));
	connect(boxPlotType, SIGNAL(currentIndexChanged(int)), this, SLOT(changePlotType(int)));

	QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(removeSelectedCurve()));
}

void PlotDialog::showAll(bool all)
{
	if(all){
		listBox->show();
		listBox->setFocus();

		QTreeWidgetItem *item = listBox->currentItem();
    	if (item->type() == CurveTreeItem::PlotCurveTreeItem)
        	curvePlotTypeBox->show();

		btnMore->setText("&>>");
	} else {
		listBox->hide();
		curvePlotTypeBox->hide();
		btnMore->setText("&<<");
	}
}

void PlotDialog::showPlotAssociations(QTreeWidgetItem *item, int)
{
	if (!item)
		return;

	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
		return;

    if (item->type() != CurveTreeItem::PlotCurveTreeItem)
        return;

    QwtPlotItem *it = (QwtPlotItem *)((CurveTreeItem *)item)->plotItem();
    if (!it)
        return;

    if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
    {
  	    Spectrogram *sp = (Spectrogram *)it;
  	    if (sp->matrix())
  	    	sp->matrix()->showMaximized();
        return;
    }

	hide();
	if (((PlotCurve *)it)->type() != Graph::Function){
        AssociationsDialog* ad = app->showPlotAssociations(((CurveTreeItem *)item)->plotItemIndex());
		if (ad)
			connect((QObject *)ad, SIGNAL(destroyed()), this, SLOT(show()));
	}
}

void PlotDialog::editCurve()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();

	CurveTreeItem *item = (CurveTreeItem *)listBox->currentItem();
    if (!item)
        return;
    if (item->type() != CurveTreeItem::PlotCurveTreeItem)
        return;

	int index = item->plotItemIndex();
	int curveType = ((PlotCurve *)item->plotItem())->type();

	hide();

	if (app){
		if (curveType == Graph::Function){
			FunctionDialog *fd = app->showFunctionDialog(item->graph(), index);
			if (fd)
				connect((QObject *)fd, SIGNAL(destroyed()), this, SLOT(show()));
		} else {
			AssociationsDialog* ad = app->showPlotAssociations(index);
			if (ad)
				connect((QObject *)ad, SIGNAL(destroyed()), this, SLOT(show()));
		}
	}
}

void PlotDialog::changePlotType(int plotType)
{
    if (boxPlotType->count() == 1)
		return;

    CurveTreeItem *item = (CurveTreeItem *)listBox->currentItem();
    if (!item)
        return;
    if (item->type() != CurveTreeItem::PlotCurveTreeItem)
        return;
    Graph *graph = item->graph();
    if (!graph)
        return;

	int curveType = item->plotItemStyle();
	if (curveType == Graph::ColorMap || curveType == Graph::Contour || curveType == Graph::GrayScale)
  		clearTabWidget();
  	else if (curveType == Graph::VectXYAM || curveType == Graph::VectXYXY){
		if ((plotType && curveType == Graph::VectXYAM) ||
            (!plotType && curveType == Graph::VectXYXY))
			return;

		clearTabWidget();
		insertTabs(curveType);

		VectorCurve *v = (VectorCurve*)item->plotItem();
		if (plotType){
			v->setVectorStyle(VectorCurve::XYAM);
			v->setPlotStyle(Graph::VectXYAM);
		} else {
			v->setVectorStyle(VectorCurve::XYXY);
			v->setPlotStyle(Graph::VectXYXY);
		}
		customVectorsPage(plotType);
	} else {
		clearTabWidget();
		insertTabs(plotType);

		boxConnect->setCurrentIndex(1);//show line for Line and LineSymbol plots

		int size = 2*boxSymbolSize->value() + 1;
		QBrush br = QBrush(boxFillColor->color(), Qt::SolidPattern);
		if (!boxFillSymbol->isChecked())
			br = QBrush();
		QPen pen = QPen(boxSymbolColor->color(), boxPenWidth->value(), Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
		pen.setCosmetic(true);
		QwtSymbol s = QwtSymbol(boxSymbolStyle->selectedSymbol(), br, pen, QSize(size, size));
		if (s.style() == QwtSymbol::NoSymbol){
			s.setStyle(QwtSymbol::Ellipse);
			boxSymbolStyle->setCurrentIndex(1);
		}

		if (plotType == Graph::Line)
			s.setStyle(QwtSymbol::NoSymbol);
		else if (plotType == Graph::Scatter)
			graph->setCurveStyle(item->plotItemIndex(), QwtPlotCurve::NoCurve);
		else if (plotType == Graph::LineSymbols)
			graph->setCurveStyle(item->plotItemIndex(), QwtPlotCurve::Lines);

		PlotCurve *c = (PlotCurve*)item->plotItem();
        c->setSymbol(s);
        c->setPlotStyle(plotType);
	}
	acceptParams();
}

void PlotDialog::initMiscPage()
{
	miscPage = new QWidget();
	QGridLayout *gl = new QGridLayout(miscPage);

	boxLinkXAxes = new QCheckBox(tr( "Link &X axes"));
	gl->addWidget(boxLinkXAxes, 0, 0);

	QLabel *l = new QLabel(tr("Apply &to..."));

	QHBoxLayout *hl = new QHBoxLayout();
	hl->addWidget(l);

	boxLinkAllXAxes = new QComboBox();
	boxLinkAllXAxes->addItem(tr("Window"));
	boxLinkAllXAxes->addItem(tr("All Windows"));
	hl->addWidget(boxLinkAllXAxes);
	hl->addStretch();

	l->setBuddy(boxLinkAllXAxes);

	gl->addLayout(hl, 0, 1);
	gl->setRowStretch(1, 1);

	privateTabWidget->addTab(miscPage, tr("Miscellaneous"));
}

void PlotDialog::initFontsPage()
{
	QGroupBox *boxFonts = new QGroupBox();
	QGridLayout *fl = new QGridLayout(boxFonts);

	btnTitle = new QPushButton(tr("Titles"));
	btnAxesLabels = new QPushButton(tr("Axes Labels"));
	btnAxesNumbers = new QPushButton(tr("Axes Numbers"));
	btnLegend = new QPushButton(tr("Legends"));

	fl->addWidget(btnTitle, 0, 0);
	fl->addWidget(btnAxesLabels, 0, 1);
	fl->addWidget(btnAxesNumbers, 0, 2);
	fl->addWidget(btnLegend, 0, 3);
	fl->setRowStretch(1, 1);
	fl->setColumnStretch(4, 1);

	fontsPage = new QWidget();
	QVBoxLayout *vl = new QVBoxLayout(fontsPage);
	vl->addWidget(boxFonts);

	privateTabWidget->addTab(fontsPage, tr( "Fonts"));

	connect( btnTitle, SIGNAL( clicked() ), this, SLOT( setTitlesFont() ) );
	connect( btnAxesLabels, SIGNAL( clicked() ), this, SLOT( setAxesLabelsFont() ) );
	connect( btnAxesNumbers, SIGNAL( clicked() ), this, SLOT( setAxesNumbersFont() ) );
	connect( btnLegend, SIGNAL( clicked() ), this, SLOT( setLegendsFont() ) );
}

void PlotDialog::initLayerPage()
{
    layerPage = new QWidget();

    QGroupBox * boxBkg = new QGroupBox();
    QGridLayout * boxBkgLayout = new QGridLayout( boxBkg );

    boxBkgLayout->addWidget( new QLabel(tr( "Background Color" )), 0, 0 );
    boxBackgroundColor = new ColorButton();
    boxBkgLayout->addWidget( boxBackgroundColor, 0, 1 );
    boxBkgLayout->addWidget( new QLabel(tr( "Opacity" )), 0, 2 );
    boxBackgroundTransparency = new QSpinBox();
    boxBackgroundTransparency->setRange(0, 255);
    boxBackgroundTransparency->setSingleStep(5);
    boxBackgroundTransparency->setWrapping(true);
    boxBackgroundTransparency->setSpecialValueText(tr("Transparent"));
    boxBkgLayout->addWidget( boxBackgroundTransparency, 0, 3 );

    boxBkgLayout->addWidget( new QLabel(tr("Canvas Color" )), 1, 0);
    boxCanvasColor = new ColorButton();
    boxBkgLayout->addWidget( boxCanvasColor, 1, 1 );
    boxBkgLayout->addWidget( new QLabel(tr( "Opacity" )), 1, 2 );
    boxCanvasTransparency = new QSpinBox();
    boxCanvasTransparency->setRange(0, 255);
    boxCanvasTransparency->setSingleStep(5);
    boxCanvasTransparency->setWrapping(true);
    boxCanvasTransparency->setSpecialValueText(tr("Transparent"));
    boxBkgLayout->addWidget( boxCanvasTransparency, 1, 3 );

    boxBkgLayout->addWidget( new QLabel(tr("Border Color" )), 2, 0);
    boxBorderColor = new ColorButton();
    boxBkgLayout->addWidget( boxBorderColor, 2, 1);

    boxBkgLayout->addWidget( new QLabel(tr( "Width" )), 2, 2);
    boxBorderWidth = new QSpinBox();
    boxBkgLayout->addWidget( boxBorderWidth, 2, 3);

	boxAntialiasing = new QCheckBox(tr("Antialiasing"));
    boxBkgLayout->addWidget( boxAntialiasing, 3, 0);

	boxBkgLayout->addWidget( new QLabel(tr( "Margin" )), 3, 2);
	boxMargin = new QSpinBox();
    boxMargin->setRange( 0, 1000 );
    boxMargin->setSingleStep(5);
    boxBkgLayout->addWidget( boxMargin, 3, 3);

	layerScaleFonts = new QCheckBox(tr("Scale &Fonts"));
	boxBkgLayout->addWidget(layerScaleFonts, 4, 0);

	boxBkgLayout->setRowStretch(5, 1);

    QVBoxLayout *vl = new QVBoxLayout();

    layerDefaultBtn = new QPushButton(tr("Set As &Default"));
    connect(layerDefaultBtn, SIGNAL(clicked()), this, SLOT(setLayerDefaultValues()));
	vl->addWidget(layerDefaultBtn);

	QLabel *l = new QLabel(tr("Apply &to..."));
	vl->addWidget(l);

	backgroundApplyToBox = new QComboBox();
	backgroundApplyToBox->insertItem(tr("Layer"));
    backgroundApplyToBox->insertItem(tr("Window"));
    backgroundApplyToBox->insertItem(tr("All Windows"));
	vl->addWidget(backgroundApplyToBox);
	vl->addStretch();

	l->setBuddy(backgroundApplyToBox);

    QHBoxLayout * hl = new QHBoxLayout( layerPage );
    hl->addWidget(boxBkg);
    hl->addLayout(vl);

    privateTabWidget->addTab(layerPage, tr("Layer"));

	connect(boxBackgroundTransparency, SIGNAL(valueChanged(int)), this, SLOT(updateBackgroundTransparency(int)));
	connect(boxCanvasTransparency, SIGNAL(valueChanged(int)), this, SLOT(updateCanvasTransparency(int)));
	connect(boxAntialiasing, SIGNAL(toggled(bool)), this, SLOT(applyLayerFormat()));
	connect(boxMargin, SIGNAL(valueChanged (int)), this, SLOT(applyLayerFormat()));
	connect(boxBorderColor, SIGNAL(colorChanged()), this, SLOT(applyLayerFormat()));
	connect(boxBackgroundColor, SIGNAL(colorChanged()), this, SLOT(applyLayerFormat()));
	connect(boxCanvasColor, SIGNAL(colorChanged()), this, SLOT(applyLayerFormat()));
	connect(boxBorderWidth, SIGNAL(valueChanged (int)), this, SLOT(applyLayerFormat()));
}


void PlotDialog::initPlotGeometryPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale locale = QLocale();
	if (app)
		locale = app->locale();

	plotUnitBox = new QComboBox();
	plotUnitBox->insertItem(tr("inch"));
	plotUnitBox->insertItem(tr("mm"));
	plotUnitBox->insertItem(tr("cm"));
	plotUnitBox->insertItem(tr("point"));
	plotUnitBox->insertItem(tr("pixel"));
	plotUnitBox->setCurrentIndex(app->d_layer_geometry_unit);

	QHBoxLayout *bl1 = new QHBoxLayout();
	bl1->addWidget(new QLabel(tr( "Unit" )));
	bl1->addWidget(plotUnitBox);

	QGroupBox *gb1 = new QGroupBox(tr("Origin"));
	boxPlotX = new DoubleSpinBox();
	boxPlotX->setLocale(locale);
	boxPlotX->setDecimals(6);

	boxPlotY = new DoubleSpinBox();
	boxPlotY->setLocale(locale);
	boxPlotY->setDecimals(6);

	QGridLayout *gl1 = new QGridLayout(gb1);
	gl1->addWidget(new QLabel( tr("X= ")), 0, 0);
	gl1->addWidget(boxPlotX, 0, 1);
	gl1->addWidget(new QLabel(tr("Y= ")), 1, 0);
	gl1->addWidget(boxPlotY, 1, 1);
	gl1->setRowStretch(2, 1);

	QGroupBox *gb2 = new QGroupBox(tr("Size"));
	boxPlotWidth = new DoubleSpinBox();
	boxPlotWidth->setMinimum(0);
	boxPlotWidth->setLocale(locale);
	boxPlotWidth->setDecimals(6);

	boxPlotHeight = new DoubleSpinBox();
	boxPlotHeight->setMinimum(0);
	boxPlotHeight->setLocale(locale);
	boxPlotHeight->setDecimals(6);

	QGridLayout *gl2 = new QGridLayout(gb2);
	gl2->addWidget(new QLabel( tr("width= ")), 0, 0);
	gl2->addWidget(boxPlotWidth, 0, 1);

	gl2->addWidget(new QLabel(tr("height= ")), 2, 0);
	gl2->addWidget(boxPlotHeight, 2, 1);

	keepPlotRatioBox = new QCheckBox(tr("Keep aspect ratio"));
	keepPlotRatioBox->setChecked(app->d_keep_aspect_ration);
	gl2->addWidget(keepRatioBox, 3, 1);

	gl2->setRowStretch(4, 1);

	QHBoxLayout *bl2 = new QHBoxLayout();
	bl2->addWidget(gb1);
	bl2->addWidget(gb2);

	plotGeometryPage = new QWidget();
	QVBoxLayout * vl = new QVBoxLayout(plotGeometryPage);
	vl->addLayout(bl1);
	vl->addLayout(bl2);

	boxResizeLayers = new QCheckBox(tr("Do not &resize layers when window size changes"));
	vl->addWidget(boxResizeLayers);

	privateTabWidget->addTab(plotGeometryPage, tr("Dimensions"));

	connect(boxPlotWidth, SIGNAL(valueChanged (double)), this, SLOT(adjustPlotHeight(double)));
	connect(boxPlotHeight, SIGNAL(valueChanged (double)), this, SLOT(adjustPlotWidth(double)));
	connect(plotUnitBox, SIGNAL(activated(int)), this, SLOT(displayPlotCoordinates(int)));
}

void PlotDialog::initLayerGeometryPage()
{
	layerGeometryPage = new QWidget();

	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale locale = QLocale();
	if (app)
		locale = app->locale();

	unitBox = new QComboBox();
	unitBox->insertItem(tr("inch"));
	unitBox->insertItem(tr("mm"));
	unitBox->insertItem(tr("cm"));
	unitBox->insertItem(tr("point"));
	unitBox->insertItem(tr("pixel"));

	QBoxLayout *bl1 = new QBoxLayout (QBoxLayout::LeftToRight);
	bl1->addWidget(new QLabel(tr( "Unit" )));
	bl1->addWidget(unitBox);

	QGroupBox *gb1 = new QGroupBox(tr("Origin"));
	boxX = new DoubleSpinBox();
	boxX->setLocale(locale);
	boxX->setDecimals(6);

	boxY = new DoubleSpinBox();
	boxY->setLocale(locale);
	boxY->setDecimals(6);

	QGridLayout *gl1 = new QGridLayout(gb1);
	gl1->addWidget(new QLabel( tr("X= ")), 0, 0);
	gl1->addWidget(boxX, 0, 1);
	gl1->addWidget(new QLabel(tr("Y= ")), 1, 0);
	gl1->addWidget(boxY, 1, 1);
	gl1->setRowStretch(2, 1);

	QGroupBox *gb2 = new QGroupBox(tr("Size"));
	boxLayerWidth = new DoubleSpinBox();
	boxLayerWidth->setMinimum(0);
	boxLayerWidth->setLocale(locale);
	boxLayerWidth->setDecimals(6);

	boxLayerHeight = new DoubleSpinBox();
	boxLayerHeight->setMinimum(0);
	boxLayerHeight->setLocale(locale);
	boxLayerHeight->setDecimals(6);

	QGridLayout *gl2 = new QGridLayout(gb2);
	gl2->addWidget(new QLabel( tr("width= ")), 0, 0);
	gl2->addWidget(boxLayerWidth, 0, 1);

	gl2->addWidget(new QLabel(tr("height= ")), 2, 0);
	gl2->addWidget(boxLayerHeight, 2, 1);

	keepRatioBox = new QCheckBox(tr("Keep aspect ratio"));
	keepRatioBox->setChecked(app->d_keep_aspect_ration);
	gl2->addWidget(keepRatioBox, 3, 1);

	QLabel *l = new QLabel(tr("Apply &to..."));
	gl2->addWidget(l, 4, 0);

	sizeApplyToBox = new QComboBox();
	sizeApplyToBox->insertItem(tr("Layer"));
	sizeApplyToBox->insertItem(tr("Window"));
	sizeApplyToBox->insertItem(tr("All Windows"));
	gl2->addWidget(sizeApplyToBox, 4, 1);

	l->setBuddy(sizeApplyToBox);

	gl2->setRowStretch(5, 1);

	QBoxLayout *bl2 = new QBoxLayout (QBoxLayout::LeftToRight);
	bl2->addWidget(gb1);
	bl2->addWidget(gb2);

	QVBoxLayout * vl = new QVBoxLayout( layerGeometryPage );
	vl->addLayout(bl1);
	vl->addLayout(bl2);

	privateTabWidget->addTab(layerGeometryPage, tr("Geometry"));

	connect(boxLayerWidth, SIGNAL(valueChanged (double)), this, SLOT(adjustLayerHeight(double)));
	connect(boxLayerHeight, SIGNAL(valueChanged (double)), this, SLOT(adjustLayerWidth(double)));
	connect(unitBox, SIGNAL(activated(int)), this, SLOT(displayCoordinates(int)));
	unitBox->setCurrentIndex(app->d_layer_geometry_unit);
}

void PlotDialog::initLayerSpeedPage()
{
	speedPage = new QWidget();

	speedModeBox = new QGroupBox(tr("&Speed Mode, Skip Points if needed"));
	speedModeBox->setCheckable(true);
	speedModeBox->setChecked(false);

	boxMaxPoints = new QSpinBox();
	boxMaxPoints->setMaximum(INT_MAX);
	boxMaxPoints->setSuffix(" " + tr("data points"));
	boxMaxPoints->setValue(3000);

    QGridLayout *gl1 = new QGridLayout(speedModeBox);
	gl1->addWidget(new QLabel(tr("Apply to curves with more than:")), 0, 0);
    gl1->addWidget(boxMaxPoints, 0, 1);

	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale locale = QLocale();
	if (app)
		locale = app->locale();

	boxDouglasPeukerTolerance = new DoubleSpinBox();
	boxDouglasPeukerTolerance->setLocale(locale);
	boxDouglasPeukerTolerance->setMinimum(0.0);
	boxDouglasPeukerTolerance->setSpecialValueText(tr("0 (all data points)"));

	gl1->addWidget(new QLabel(tr("Tolerance (Douglas-Peucker algorithm)")), 1, 0);
	gl1->addWidget(boxDouglasPeukerTolerance, 1, 1);
    gl1->setRowStretch(2, 1);

    QVBoxLayout * vl = new QVBoxLayout( speedPage );
    vl->addWidget(speedModeBox);

    privateTabWidget->addTab(speedPage, tr("Speed"));
	connect(speedModeBox, SIGNAL(toggled(bool)), this, SLOT(acceptParams()));
	connect(boxDouglasPeukerTolerance, SIGNAL(valueChanged(double)), this, SLOT(acceptParams()));
}

void PlotDialog::initPiePage()
{
	piePage = new QWidget();

	QGridLayout *gl1 = new QGridLayout();
	gl1->addWidget(new QLabel( tr( "Color" )), 0, 0);

	boxPieLineColor = new ColorButton();
	gl1->addWidget(boxPieLineColor, 0, 1);

	gl1->addWidget(new QLabel(tr( "Style" )), 1, 0);
	boxPieLineStyle = new PenStyleBox();
	gl1->addWidget(boxPieLineStyle);

	gl1->addWidget(new QLabel(tr( "Width")), 2, 0);
	boxPieLineWidth = new DoubleSpinBox('f');
	boxPieLineWidth->setSingleStep(0.1);
	boxPieLineWidth->setMinimum(0.0);
	boxPieLineWidth->setLocale(((ApplicationWindow *)this->parent())->locale());
	gl1->addWidget(boxPieLineWidth, 2, 1);
	gl1->setRowStretch(3,1);

	QGroupBox *gb1 = new QGroupBox(tr( "Border" ));
	gb1->setLayout(gl1);

	QGridLayout *gl2 = new QGridLayout();
	gl2->addWidget(new QLabel( tr( "First color" )), 0, 0);

	boxFirstColor = new ColorBox();
	gl2->addWidget(boxFirstColor, 0, 1);

	gl2->addWidget(new QLabel( tr( "Pattern" )), 1, 0);
	boxPiePattern = new PatternBox(false);
	gl2->addWidget(boxPiePattern, 1, 1);
	gl2->setRowStretch(2, 1);

	QGroupBox *gb2 = new QGroupBox(tr( "Fill" ));
	gb2->setLayout(gl2);

	QHBoxLayout* hl = new QHBoxLayout();
	hl->addWidget(gb1);
	hl->addWidget(gb2);
	piePage->setLayout(hl);

	privateTabWidget->addTab(piePage, tr( "Pattern" ) );
}

void PlotDialog::initPieGeometryPage()
{
    pieGeometryPage = new QWidget();

	QLocale locale = ((ApplicationWindow *)this->parent())->locale();

	QGroupBox *gb3 = new QGroupBox(tr( "3D View" ));
	QGridLayout *gl3 = new QGridLayout(gb3);
	gl3->addWidget(new QLabel( tr( "View Angle (deg)" )), 0, 0);
	boxPieViewAngle = new DoubleSpinBox('f');
	boxPieViewAngle->setWrapping(true);
	boxPieViewAngle->setRange(0.0, 90.0);
	boxPieViewAngle->setLocale(locale);
	gl3->addWidget(boxPieViewAngle, 0, 1);

    gl3->addWidget(new QLabel( tr( "Thickness (% of radius)" )), 1, 0);
	boxPieThickness = new DoubleSpinBox('f');
	boxPieThickness->setLocale(locale);
	boxPieThickness->setRange(0.0, 300.0);
	gl3->addWidget(boxPieThickness, 1, 1);
	gl3->setRowStretch(2, 1);

	QGroupBox *gb1 = new QGroupBox(tr( "Rotation" ));
	QGridLayout *gl1 = new QGridLayout(gb1);
	gl1->addWidget(new QLabel( tr( "Starting Azimuth (deg)" )), 0, 0);
	boxPieStartAzimuth = new DoubleSpinBox('f');
	boxPieStartAzimuth->setRange(0.0, 360.0);
	boxPieStartAzimuth->setWrapping(true);
	boxPieStartAzimuth->setSingleStep(10.0);
	boxPieStartAzimuth->setLocale(locale);
	gl1->addWidget(boxPieStartAzimuth, 0, 1);

    boxPieConterClockwise = new QCheckBox(tr("Counter cloc&kwise"));
	gl1->addWidget(boxPieConterClockwise, 1, 0);
	gl1->setRowStretch(2, 1);

	QGroupBox *gb2 = new QGroupBox(tr( "Radius/Center" ));
	QGridLayout *gl2 = new QGridLayout(gb2);
	gl2->addWidget(new QLabel( tr( "Radius (% of frame)" )), 0, 0);
	boxRadius = new QSpinBox();
	boxRadius->setRange(0, 300);
	boxRadius->setSingleStep(5);
	gl2->addWidget(boxRadius, 0, 1);
    gl2->addWidget(new QLabel( tr( "Horizontal Offset (% of frame)" )), 1, 0);
    boxPieOffset = new QSpinBox();
	boxPieOffset->setRange(-100, 100);
	gl2->addWidget(boxPieOffset, 1, 1);
	gl2->setRowStretch(2, 1);

	QVBoxLayout* vl = new QVBoxLayout(pieGeometryPage);
	vl->addWidget(gb3);
	vl->addWidget(gb1);
	vl->addWidget(gb2);

	privateTabWidget->addTab(pieGeometryPage, tr( "Pie Geometry" ) );

    connect(boxPieConterClockwise, SIGNAL(toggled(bool)), this, SLOT(acceptParams()));
    connect(boxPieViewAngle, SIGNAL(valueChanged(double)), this, SLOT(acceptParams()));
    connect(boxPieThickness, SIGNAL(valueChanged(double)), this, SLOT(acceptParams()));
    connect(boxPieStartAzimuth, SIGNAL(valueChanged(double)), this, SLOT(acceptParams()));
    connect(boxRadius, SIGNAL(valueChanged(int)), this, SLOT(acceptParams()));
    connect(boxPieOffset, SIGNAL(valueChanged(int)), this, SLOT(acceptParams()));
}

void PlotDialog::initPieLabelsPage()
{
	pieLabelsPage = new QWidget();

    pieAutoLabelsBox = new QGroupBox(tr("Automatic &Format"));
	pieAutoLabelsBox->setCheckable(true);

	QGridLayout *gl1 = new QGridLayout(pieAutoLabelsBox);
	boxPieValues = new QCheckBox(tr("&Values"));
	gl1->addWidget(boxPieValues, 0, 0);

	boxPiePercentages = new QCheckBox(tr("&Percentages"));
	gl1->addWidget(boxPiePercentages, 1, 0);

	boxPieCategories = new QCheckBox(tr("Categories/&Rows"));
	gl1->addWidget(boxPieCategories, 2, 0);

	gl1->setRowStretch(3, 1);

    boxPieWedge = new QGroupBox(tr( "Associate Position with &Wedge" ));
	boxPieWedge->setCheckable(true);

	QGridLayout *gl2 = new QGridLayout(boxPieWedge);
	gl2->addWidget(new QLabel( tr( "Dist. from Pie Edge" )), 0, 0);
	boxPieEdgeDist = new DoubleSpinBox('f');
	boxPieEdgeDist->setRange(-100, 100);
	boxPieEdgeDist->setLocale(((ApplicationWindow *)this->parent())->locale());
	gl2->addWidget(boxPieEdgeDist, 0, 1);
	gl2->setRowStretch(1, 1);

	QVBoxLayout* vl = new QVBoxLayout(pieLabelsPage);
	vl->addWidget(pieAutoLabelsBox);
	vl->addWidget(boxPieWedge);

	privateTabWidget->addTab(pieLabelsPage, tr( "Labels" ) );

    connect(boxPieEdgeDist, SIGNAL(valueChanged(double)), this, SLOT(acceptParams()));
}

void PlotDialog::initPrintPage()
{
	QGroupBox *gb = new QGroupBox();
    QVBoxLayout *vl = new QVBoxLayout(gb);
	boxScaleLayers = new QCheckBox(tr("&Scale layers to paper size"));
	vl->addWidget(boxScaleLayers);
	boxPrintCrops = new QCheckBox(tr("Print Crop&marks"));
	vl->addWidget(boxPrintCrops);
    vl->addStretch();

	printPage = new QWidget();
	QHBoxLayout* hlayout = new QHBoxLayout(printPage);
	hlayout->addWidget(gb);
	privateTabWidget->addTab(printPage, tr( "Print" ) );
}

void PlotDialog::initFunctionPage()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
		return;

	functionEdit = new FunctionDialog(app, false);//in order to have correct locale settings, parent must be app window

	functionPage = new QWidget();
	QVBoxLayout* vl = new QVBoxLayout(functionPage);
	vl->addWidget(functionEdit);
	vl->addStretch();
}

void PlotDialog::initLabelsPage()
{
    labelsGroupBox = new QGroupBox(tr("&Show"));
    labelsGroupBox->setCheckable (true);

    QGridLayout *gl = new QGridLayout(labelsGroupBox);
    labelsColumnLbl = new QLabel(tr( "Column" ));
    gl->addWidget(labelsColumnLbl, 0, 0);
    boxLabelsColumn = new QComboBox();
    gl->addWidget(boxLabelsColumn, 0, 1);

    gl->addWidget(new QLabel(tr( "Color" )), 1, 0);
    boxLabelsColor = new ColorButton();
    gl->addWidget(boxLabelsColor, 1, 1);
    boxLabelsWhiteOut = new QCheckBox(tr("White O&ut"));
    gl->addWidget(boxLabelsWhiteOut, 1, 2);

	justifyLabelsLbl = new QLabel(tr( "Justify" ));
    gl->addWidget(justifyLabelsLbl, 2, 0);
    boxLabelsAlign = new QComboBox();
    boxLabelsAlign->addItem( tr( "Center" ) );
    boxLabelsAlign->addItem( tr( "Left" ) );
    boxLabelsAlign->addItem( tr( "Right" ) );
    gl->addWidget(boxLabelsAlign, 2, 1);

    gl->addWidget(new QLabel(tr( "Rotate (deg)" )), 3, 0);
    boxLabelsAngle = new DoubleSpinBox('f');
    boxLabelsAngle->setDecimals(1);
    boxLabelsAngle->setLocale(((ApplicationWindow *)parent())->locale());
    boxLabelsAngle->setRange(0, 180);
    gl->addWidget(boxLabelsAngle, 3, 1);
    btnLabelsFont = new QPushButton(tr("&Font"));
    gl->addWidget(btnLabelsFont, 3, 2);

    gl->addWidget(new QLabel(tr("X Offset (font height %)")), 4, 0);
    boxLabelsXOffset = new QSpinBox();
    boxLabelsXOffset->setRange(-INT_MAX, INT_MAX);
    boxLabelsXOffset->setSingleStep(10);
    gl->addWidget(boxLabelsXOffset, 4, 1);

    gl->addWidget(new QLabel(tr("Y Offset (font height %)")), 5, 0);
    boxLabelsYOffset = new QSpinBox();
    boxLabelsYOffset->setRange(-INT_MAX, INT_MAX);
    boxLabelsYOffset->setSingleStep(10);
    gl->addWidget(boxLabelsYOffset, 5, 1);
    gl->setRowStretch (6, 1);
    gl->setColumnStretch (3, 1);

	labelsPage = new QWidget();
	QHBoxLayout* hlayout = new QHBoxLayout(labelsPage);
	hlayout->addWidget(labelsGroupBox);
	privateTabWidget->addTab(labelsPage, tr("Labels"));

    connect(labelsGroupBox, SIGNAL(toggled(bool)), this, SLOT(acceptParams()));
    connect(boxLabelsColumn, SIGNAL(activated(int)), this, SLOT(acceptParams()));
    connect(boxLabelsAlign, SIGNAL(activated(int)), this, SLOT(acceptParams()));
    connect(boxLabelsWhiteOut, SIGNAL(toggled(bool)), this, SLOT(acceptParams()));
    connect(boxLabelsColor, SIGNAL(colorChanged()), this, SLOT(acceptParams()));
    connect(boxLabelsXOffset, SIGNAL(valueChanged(int)), this, SLOT(acceptParams()));
    connect(boxLabelsYOffset, SIGNAL(valueChanged(int)), this, SLOT(acceptParams()));
    connect(boxLabelsAngle, SIGNAL(valueChanged(double)), this, SLOT(acceptParams()));
    connect(btnLabelsFont, SIGNAL(clicked()), this, SLOT(chooseLabelsFont()));
}

void PlotDialog::initAxesPage()
{
	QGroupBox *gb = new QGroupBox(tr( "Attach curve to: "));
    QGridLayout *gl = new QGridLayout(gb);
    gl->addWidget(new QLabel( tr( "x Axis" )), 0, 0);
	boxXAxis = new QComboBox();
	boxXAxis->setEditable(false);
	boxXAxis->addItem(tr("Bottom"));
	boxXAxis->addItem(tr("Top"));
	gl->addWidget(boxXAxis, 0, 1);

	gl->addWidget(new QLabel( tr( "y Axis" )), 1, 0);
	boxYAxis = new QComboBox();
	boxYAxis->setEditable(false);
	boxYAxis->addItem(tr("Left"));
	boxYAxis->addItem(tr("Right"));
	gl->addWidget(boxYAxis, 1, 1);
    gl->setRowStretch (2, 1);

	axesPage = new QWidget();
	QHBoxLayout* hlayout = new QHBoxLayout(axesPage);
	hlayout->addWidget(gb);
	privateTabWidget->addTab(axesPage, tr( "Axes" ) );
}

void PlotDialog::initLinePage()
{
	QGroupBox *gb = new QGroupBox();
	QGridLayout *gl1 = new QGridLayout(gb);
    gl1->addWidget(new QLabel( tr( "Connect" )), 0, 0);

	boxConnect = new QComboBox();
	boxConnect->setEditable(false);
	boxConnect->addItem(tr("No line"));
	boxConnect->addItem(tr("Lines"));
	boxConnect->addItem(tr("Sticks"));
	boxConnect->addItem(tr("Horizontal Steps"));
	boxConnect->addItem(tr("Dots"));
	boxConnect->addItem(tr("Spline"));
	boxConnect->addItem(tr("Vertical Steps"));
	gl1->addWidget(boxConnect, 0, 1);

	gl1->addWidget(new QLabel(tr( "Style" )), 1, 0);
	boxLineStyle = new PenStyleBox();
	gl1->addWidget(boxLineStyle, 1, 1);

	gl1->addWidget(new QLabel(tr( "Width" )), 2, 0);
	boxLineWidth = new DoubleSpinBox('f');
	boxLineWidth->setLocale(((ApplicationWindow *)this->parent())->locale());
	boxLineWidth->setSingleStep(0.1);
	boxLineWidth->setMinimum(0.1);
	boxLineWidth->setValue( 1 );
	gl1->addWidget(boxLineWidth, 2, 1);

	gl1->addWidget(new QLabel(tr( "Color" )), 3, 0);
	boxLineColor = new ColorButton();
	gl1->addWidget(boxLineColor, 3, 1);

	QLabel *l = new QLabel(tr("Apply Format &to"));
	gl1->addWidget(l, 4, 0);

	lineFormatApplyToBox = new QComboBox();
	lineFormatApplyToBox->insertItem(tr("Selected Curve"));
	lineFormatApplyToBox->insertItem(tr("Layer"));
    lineFormatApplyToBox->insertItem(tr("Window"));
    lineFormatApplyToBox->insertItem(tr("All Windows"));
	gl1->addWidget(lineFormatApplyToBox, 4, 1);
	l->setBuddy(lineFormatApplyToBox);

    gl1->setRowStretch (5, 1);

	fillGroupBox = new QGroupBox(tr( "Fill area under curve" ));
	fillGroupBox->setCheckable(true);
	QGridLayout *gl2 = new QGridLayout(fillGroupBox);
    gl2->addWidget(new QLabel(tr( "Fill color" )), 0, 0);
	boxAreaColor = new ColorButton();
	gl2->addWidget(boxAreaColor, 0, 1);

	gl2->addWidget(new QLabel(tr( "Opacity" )), 1, 0);
	boxCurveOpacity = new QSpinBox();
	boxCurveOpacity->setRange(0, 255);
	boxCurveOpacity->setSingleStep(5);
	boxCurveOpacity->setWrapping(true);
	boxCurveOpacity->setSpecialValueText(tr("Transparent"));
	gl2->addWidget(boxCurveOpacity, 1, 1);

	curveOpacitySlider = new QSlider();
	curveOpacitySlider->setOrientation(Qt::Horizontal);
	curveOpacitySlider->setRange(0, 255);
	gl2->addWidget(curveOpacitySlider, 1, 2);

	gl2->addWidget(new QLabel(tr( "Pattern" )), 2, 0);
	boxPattern = new PatternBox(false);
	gl2->addWidget(boxPattern, 2, 1);
	gl2->setRowStretch (3, 1);

	linePage = new QWidget();
	QHBoxLayout* hlayout = new QHBoxLayout(linePage);
	hlayout->addWidget(gb);
	hlayout->addWidget(fillGroupBox);
	privateTabWidget->addTab( linePage, tr( "Line" ) );

	connect(boxLineWidth, SIGNAL(valueChanged(double)), this, SLOT(acceptParams()));
	connect(boxLineColor, SIGNAL(colorChanged()), this, SLOT(acceptParams()));
	connect(boxConnect, SIGNAL(activated(int)), this, SLOT(acceptParams()));
	connect(boxLineStyle, SIGNAL(activated(int)), this, SLOT(acceptParams()));
	connect(boxAreaColor, SIGNAL(colorChanged()), this, SLOT(acceptParams()));
	connect(boxPattern, SIGNAL(activated(int)), this, SLOT(acceptParams()));
	connect(fillGroupBox, SIGNAL(toggled(bool)), this, SLOT(showAreaColor(bool)));
	connect(fillGroupBox, SIGNAL(clicked()), this, SLOT(acceptParams()));
	connect(boxCurveOpacity, SIGNAL(valueChanged(int)), this, SLOT(acceptParams()));
	connect(curveOpacitySlider, SIGNAL(valueChanged(int)), boxCurveOpacity, SLOT(setValue(int)));
	connect(boxCurveOpacity, SIGNAL(valueChanged(int)), curveOpacitySlider, SLOT(setValue(int)));
}

void PlotDialog::initSymbolsPage()
{
	QGroupBox *gb = new QGroupBox();
    QGridLayout *gl = new QGridLayout(gb);
    gl->addWidget(new QLabel(tr( "Style" )), 0, 0);
	boxSymbolStyle = new SymbolBox();
    gl->addWidget(boxSymbolStyle, 0, 1);
    gl->addWidget(new QLabel(tr( "Size" )), 1, 0);
	boxSymbolSize = new QSpinBox();
    boxSymbolSize->setRange(1, 100);
	boxSymbolSize->setValue(5);
    gl->addWidget(boxSymbolSize, 1, 1);
	boxFillSymbol = new QCheckBox( tr( "Fill Color" ));
    gl->addWidget(boxFillSymbol, 2, 0);
	boxFillColor = new ColorButton();
    gl->addWidget(boxFillColor, 2, 1);
    gl->addWidget(new QLabel(tr( "Edge Color" )), 3, 0);
	boxSymbolColor = new ColorButton();
    gl->addWidget(boxSymbolColor, 3, 1);
    gl->addWidget(new QLabel(tr( "Edge Width" )), 4, 0);
	boxPenWidth = new DoubleSpinBox('f');
	boxPenWidth->setLocale(((ApplicationWindow *)this->parent())->locale());
	boxPenWidth->setSingleStep(0.1);
    boxPenWidth->setRange(0.1, 100);
    gl->addWidget(boxPenWidth, 4, 1);

	gl->addWidget(new QLabel(tr( "Skip Points" )), 5, 0);
    boxSkipSymbols = new QSpinBox();
    boxSkipSymbols->setMinimum(1);
    boxSkipSymbols->setWrapping(true);
    boxSkipSymbols->setSpecialValueText(tr("None"));
    gl->addWidget(boxSkipSymbols, 5, 1);

    QLabel *l = new QLabel(tr("Apply Format &to"));
	gl->addWidget(l, 6, 0);

	symbolsFormatApplyToBox = new QComboBox();
	symbolsFormatApplyToBox->insertItem(tr("Selected Curve"));
	symbolsFormatApplyToBox->insertItem(tr("Layer"));
    symbolsFormatApplyToBox->insertItem(tr("Window"));
    symbolsFormatApplyToBox->insertItem(tr("All Windows"));
	gl->addWidget(symbolsFormatApplyToBox, 6, 1);
	l->setBuddy(symbolsFormatApplyToBox);

    gl->setRowStretch (7, 1);

    symbolPage = new QWidget();
	QHBoxLayout* hl = new QHBoxLayout(symbolPage);
	hl->addWidget(gb);

	privateTabWidget->insertTab(symbolPage, tr( "Symbol" ));

	connect(boxSymbolColor, SIGNAL(colorChanged()), this, SLOT(acceptParams()));
	connect(boxSymbolStyle, SIGNAL(activated(int)), this, SLOT(acceptParams()));
	connect(boxFillColor, SIGNAL(colorChanged()), this, SLOT(acceptParams()));
	connect(boxFillSymbol, SIGNAL(clicked()), this, SLOT(fillSymbols()));
	connect(boxSkipSymbols, SIGNAL(valueChanged(int)), this, SLOT(acceptParams()));
}

void PlotDialog::initBoxPage()
{
	QGroupBox *gb1 = new QGroupBox(tr( "Box" ));
    QGridLayout *gl1 = new QGridLayout(gb1);
    gl1->addWidget(new QLabel(tr( "Type" )), 0, 0);

	boxType = new QComboBox();
    boxType->setEditable(false);
	boxType->addItem(tr("No Box"));
	boxType->addItem(tr("Rectangle"));
	boxType->addItem(tr("Diamond"));
	boxType->addItem(tr("Perc 10, 25, 75, 90"));
	boxType->addItem(tr("Notch"));
    gl1->addWidget(boxType, 0, 1);

	boxRangeLabel = new QLabel(tr( "Range" ));
    gl1->addWidget(boxRangeLabel, 1, 0);
	boxRange = new QComboBox();
    boxRange->setEditable(false);
	boxRange->addItem(tr("Standard Deviation"));
	boxRange->addItem(tr("Standard Error"));
	boxRange->addItem(tr("Perc 25, 75"));
	boxRange->addItem(tr("Perc 10, 90"));
	boxRange->addItem(tr("Perc 5, 95"));
	boxRange->addItem(tr("Perc 1, 99"));
	boxRange->addItem(tr("Max-Min"));
	boxRange->addItem(tr("Constant"));
    gl1->addWidget(boxRange, 1, 1);

	boxCoeffLabel = new QLabel(tr( "Percentile (%)" ));
    gl1->addWidget(boxCoeffLabel, 2, 0);
	boxCoef = new QSpinBox();
    boxCoef->setRange(50, 100);
    boxCoef->setSingleStep(5);
    gl1->addWidget(boxCoef, 2, 1);

	boxCntLabel = new QLabel(tr( "Coefficient" ));
    gl1->addWidget(boxCntLabel, 3, 0);
	boxCnt = new QDoubleSpinBox();
	boxCnt->setRange(0.0, 100.0);
    boxCnt->setSingleStep(0.01);
	boxCnt->setValue(1.0);
    gl1->addWidget(boxCnt, 3, 1);

    gl1->addWidget(new QLabel(tr( "Box Width" )), 4, 0);
	boxWidth = new QSpinBox();
    boxWidth->setRange(0, 100);
    boxWidth->setSingleStep(5);
    gl1->addWidget(boxWidth, 4, 1);

	QGroupBox *gb2 = new QGroupBox(tr( "Whiskers" ));
    QGridLayout *gl2 = new QGridLayout(gb2);
    whiskerRangeLabel = new QLabel(tr( "Range" ));
    gl2->addWidget(whiskerRangeLabel, 0, 0);

	boxWhiskersRange = new QComboBox();
    boxWhiskersRange->setEditable(false);
	boxWhiskersRange->addItem(tr("No Whiskers"));
	boxWhiskersRange->addItem(tr("Standard Deviation"));
	boxWhiskersRange->addItem(tr("Standard Error"));
	boxWhiskersRange->addItem(tr("75-25"));
	boxWhiskersRange->addItem(tr("90-10"));
	boxWhiskersRange->addItem(tr("95-5"));
	boxWhiskersRange->addItem(tr("99-1"));
	boxWhiskersRange->addItem(tr("Max-Min"));
	boxWhiskersRange->addItem(tr("Constant"));
    gl2->addWidget(boxWhiskersRange, 0, 1);

	whiskerCoeffLabel = new QLabel(tr( "Percentile (%)" ));
    gl2->addWidget(whiskerCoeffLabel, 1, 0);
	boxWhiskersCoef = new QSpinBox();
    boxWhiskersCoef->setRange(50, 100);
    boxWhiskersCoef->setSingleStep(5);
    gl2->addWidget(boxWhiskersCoef, 1, 1);

	whiskerCntLabel = new QLabel(tr( "Coef" ));
    gl2->addWidget(whiskerCntLabel, 2, 0);
	whiskerCnt = new QDoubleSpinBox();
	whiskerCnt->setRange(0.0, 100.0);
    whiskerCnt->setSingleStep(0.01);
	whiskerCnt->setValue(1.0);
    gl2->addWidget(whiskerCnt, 2, 1);

    QVBoxLayout *vl1 = new QVBoxLayout();
    vl1->addWidget(gb1);
    vl1->addStretch();

    QVBoxLayout *vl2 = new QVBoxLayout();
    vl2->addWidget(gb2);
    vl2->addStretch();

    boxPage = new QWidget();
	QHBoxLayout* hl = new QHBoxLayout(boxPage);
	hl->addLayout(vl1);
	hl->addLayout(vl2);
    privateTabWidget->insertTab(boxPage, tr( "Box/Whiskers" ) );

	connect(boxType, SIGNAL(activated(int)), this, SLOT(setBoxType(int)));
	connect(boxRange, SIGNAL(activated(int)), this, SLOT(setBoxRangeType(int)));
	connect(boxWhiskersRange, SIGNAL(activated(int)), this, SLOT(setWhiskersRange(int)));
}

void PlotDialog::initPercentilePage()
{
	QGroupBox *gb1 = new QGroupBox(tr( "Type" ) );
    QGridLayout *gl1 = new QGridLayout(gb1);
    gl1->addWidget(new QLabel(tr( "Max" )), 0, 0);

	boxMaxStyle = new SymbolBox();
    gl1->addWidget(boxMaxStyle, 0, 1);

    gl1->addWidget(new QLabel(tr( "99%" )), 1, 0);
	box99Style = new SymbolBox();
    gl1->addWidget(box99Style, 1, 1);

    gl1->addWidget(new QLabel(tr( "Mean" )), 2, 0);
	boxMeanStyle = new SymbolBox();
    gl1->addWidget(boxMeanStyle, 2, 1);

    gl1->addWidget(new QLabel(tr( "1%" )), 3, 0);
	box1Style = new SymbolBox();
    gl1->addWidget(box1Style, 3, 1);

    gl1->addWidget(new QLabel(tr( "Min" )), 4, 0);
	boxMinStyle = new SymbolBox();
    gl1->addWidget(boxMinStyle, 4, 1);
    gl1->setRowStretch(5, 1);

	QGroupBox *gb2 = new QGroupBox(tr( "Symbol" ));
    QGridLayout *gl2 = new QGridLayout(gb2);
    gl2->addWidget(new QLabel(tr( "Size" )), 0, 0);

	boxPercSize = new QSpinBox();
	boxPercSize->setMinValue( 1 );
    gl2->addWidget(boxPercSize, 0, 1);

    boxFillSymbols = new QCheckBox(tr( "Fill Color" ));
    gl2->addWidget(boxFillSymbols, 1, 0);
	boxPercFillColor = new ColorButton();
    gl2->addWidget(boxPercFillColor, 1, 1);

    gl2->addWidget(new QLabel(tr( "Edge Color" )), 2, 0);
	boxEdgeColor = new ColorButton();
    gl2->addWidget(boxEdgeColor, 2, 1);

    gl2->addWidget(new QLabel(tr( "Edge Width" )), 3, 0);
	boxEdgeWidth = new DoubleSpinBox('f');
	boxEdgeWidth->setLocale(((ApplicationWindow *)parent())->locale());
	boxEdgeWidth->setSingleStep(0.1);
    boxEdgeWidth->setRange(0, 100);
    gl2->addWidget(boxEdgeWidth, 3, 1);
    gl2->setRowStretch(4, 1);

    percentilePage = new QWidget();
	QHBoxLayout* hl = new QHBoxLayout(percentilePage);
	hl->addWidget(gb1);
	hl->addWidget(gb2);
    privateTabWidget->insertTab(percentilePage, tr( "Percentile" ) );

	connect(boxMeanStyle, SIGNAL(activated(int)), this, SLOT(acceptParams()));
	connect(boxMinStyle, SIGNAL(activated(int)), this, SLOT(acceptParams()));
	connect(boxMaxStyle, SIGNAL(activated(int)), this, SLOT(acceptParams()));
	connect(box99Style, SIGNAL(activated(int)), this, SLOT(acceptParams()));
	connect(box1Style, SIGNAL(activated(int)), this, SLOT(acceptParams()));
	connect(box1Style, SIGNAL(activated(int)), this, SLOT(acceptParams()));
	connect(boxEdgeColor, SIGNAL(colorChanged()), this, SLOT(acceptParams()));
	connect(boxPercFillColor, SIGNAL(colorChanged()), this, SLOT(acceptParams()));
	connect(boxFillSymbols, SIGNAL(clicked()), this, SLOT(fillBoxSymbols()));
}

void PlotDialog::initSpectrogramValuesPage()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
		return;

	QGroupBox *gb = new QGroupBox();
    QGridLayout *gl = new QGridLayout(gb);
    gl->addWidget(new QLabel( tr( "Matrix" )), 0, 0);
	boxSpectroMatrix = new QComboBox();
	boxSpectroMatrix->setEditable(false);
	boxSpectroMatrix->addItems(app->matrixNames());

	gl->addWidget(boxSpectroMatrix, 0, 1);

	boxUseMatrixFormula = new QCheckBox(tr("Use matrix formula to calculate values"));
	gl->addWidget(boxUseMatrixFormula, 1, 1);

    gl->setRowStretch (2, 1);

	spectroValuesPage = new QWidget();
	QHBoxLayout* hlayout = new QHBoxLayout(spectroValuesPage);
	hlayout->addWidget(gb);
	privateTabWidget->insertTab(spectroValuesPage, tr( "Values"));
}

void PlotDialog::initSpectrogramPage()
{
  	spectrogramPage = new QWidget();

  	imageGroupBox = new QGroupBox(tr( "Image" ));
  	imageGroupBox->setCheckable (true);

	QVBoxLayout *vl = new QVBoxLayout();
  	grayScaleBox = new QRadioButton(tr("&Gray Scale"));
	connect(grayScaleBox, SIGNAL(toggled(bool)), this, SLOT(showColorMapEditor(bool)));
    vl->addWidget(grayScaleBox);
  	defaultScaleBox = new QRadioButton(tr("&Default Color Map"));
	connect(defaultScaleBox, SIGNAL(toggled(bool)), this, SLOT(showColorMapEditor(bool)));
    vl->addWidget(defaultScaleBox);
  	customScaleBox = new QRadioButton(tr("&Custom Color Map"));
	connect(customScaleBox, SIGNAL(toggled(bool)), this, SLOT(showColorMapEditor(bool)));
    vl->addWidget(customScaleBox);

    QHBoxLayout *hl = new QHBoxLayout(imageGroupBox);
	colorMapEditor = new ColorMapEditor(((ApplicationWindow*)parent())->locale());
    hl->addLayout(vl);
	hl->addWidget(colorMapEditor);

  	axisScaleBox = new QGroupBox(tr( "Color Bar Scale" ));
  	axisScaleBox->setCheckable (true);

    QGridLayout *gl2 = new QGridLayout(axisScaleBox);
    gl2->addWidget(new QLabel(tr( "Axis" )), 0, 0);

  	colorScaleBox = new QComboBox();
  	colorScaleBox->addItem(tr("Left"));
  	colorScaleBox->addItem(tr("Right"));
  	colorScaleBox->addItem(tr("Bottom"));
  	colorScaleBox->addItem(tr("Top"));
    gl2->addWidget(colorScaleBox, 0, 1);

    gl2->addWidget(new QLabel(tr( "Width" )), 1, 0);
  	colorScaleWidthBox = new QSpinBox();
  	colorScaleWidthBox->setRange(2, 10000);
    gl2->addWidget(colorScaleWidthBox, 1, 1);

  	QVBoxLayout* vl2 = new QVBoxLayout(spectrogramPage);
  	vl2->addWidget(imageGroupBox);
  	vl2->addWidget(axisScaleBox);
    vl2->addStretch();

  	privateTabWidget->insertTab(spectrogramPage, tr("Colors"));
}

void PlotDialog::initContourLinesPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale locale = QLocale();
	if (app)
		locale = app->locale();

  	contourLinesPage = new QWidget();

  	levelsGroupBox = new QGroupBox(tr("&Show Contour Lines"));
  	levelsGroupBox->setCheckable(true);
  	QHBoxLayout *hl0 = new QHBoxLayout();

	QGroupBox *gb1 = new QGroupBox(tr("Set Equidistant Levels"));
    QGridLayout *hl1 = new QGridLayout(gb1);

	hl1->addWidget(new QLabel(tr("Levels")), 0, 0);
  	levelsBox = new QSpinBox();
  	levelsBox->setRange(0, 1000);
	hl1->addWidget(levelsBox, 0, 1);

    hl1->addWidget(new QLabel(tr("Start")), 1, 0);
  	firstContourLineBox = new DoubleSpinBox();
  	firstContourLineBox->setLocale(locale);
	firstContourLineBox->setDecimals(6);
	hl1->addWidget(firstContourLineBox, 1, 1);

    hl1->addWidget(new QLabel(tr("Step")), 2, 0);
    contourLinesDistanceBox = new DoubleSpinBox();
    contourLinesDistanceBox->setLocale(locale);
	contourLinesDistanceBox->setDecimals(6);
	hl1->addWidget(contourLinesDistanceBox, 2, 1);

	btnSetEquidistantLevels = new QPushButton(tr("Set &Levels"));
	connect(btnSetEquidistantLevels, SIGNAL(clicked()), this, SLOT(setEquidistantLevels()));
	hl1->addWidget(btnSetEquidistantLevels, 3, 1);

	hl1->setColumnStretch(1, 10);
	hl1->setRowStretch(4, 1);

	contourLinesEditor = new ContourLinesEditor(app->locale());
	hl0->addWidget(contourLinesEditor);
	hl0->addWidget(gb1);

	QGroupBox *penGroupBox = new QGroupBox(tr("Pen"));
	QHBoxLayout *hl2 = new QHBoxLayout(penGroupBox);

    QVBoxLayout *vl1 = new QVBoxLayout;
  	autoContourBox = new QRadioButton(tr("Use &Color Map"));
  	connect(autoContourBox, SIGNAL(toggled(bool)), this, SLOT(showDefaultContourLinesBox(bool)));
    vl1->addWidget(autoContourBox);

  	defaultContourBox = new QRadioButton(tr("Use Default &Pen"));
  	connect(defaultContourBox, SIGNAL(toggled(bool)), this, SLOT(showDefaultContourLinesBox(bool)));
    vl1->addWidget(defaultContourBox);

    customPenBtn = new QRadioButton(tr("Use &Table Custom Pen"));
  	connect(customPenBtn, SIGNAL(toggled(bool)), this, SLOT(showCustomPenColumn(bool)));
    vl1->addWidget(customPenBtn);

	hl2->addLayout(vl1);

  	defaultPenBox = new QGroupBox();
    QGridLayout *gl1 = new QGridLayout(defaultPenBox);
    gl1->addWidget(new QLabel(tr( "Color" )), 0, 0);

  	levelsColorBox = new ColorButton(defaultPenBox);
    gl1->addWidget(levelsColorBox, 0, 1);

    gl1->addWidget(new QLabel(tr( "Width" )), 1, 0);
  	contourWidthBox = new DoubleSpinBox('f');
	contourWidthBox->setLocale(locale);
	contourWidthBox->setSingleStep(0.1);
    contourWidthBox->setRange(0, 100);
    gl1->addWidget(contourWidthBox, 1, 1);

    gl1->addWidget(new QLabel(tr( "Style" )), 2, 0);
  	boxContourStyle = new PenStyleBox();
    gl1->addWidget(boxContourStyle, 2, 1);
    hl2->addWidget(defaultPenBox);

	QVBoxLayout *vl0 = new QVBoxLayout(levelsGroupBox);
	vl0->addLayout(hl0);
	vl0->addWidget(penGroupBox);
	vl0->addStretch();

  	QVBoxLayout* vl2 = new QVBoxLayout(contourLinesPage);
  	vl2->addWidget(levelsGroupBox);

  	privateTabWidget->insertTab(contourLinesPage, tr("Contour Lines"));
}

void PlotDialog::fillBoxSymbols()
{
	boxPercFillColor->setEnabled(boxFillSymbols->isChecked());
	acceptParams();
}

void PlotDialog::fillSymbols()
{
	boxFillColor->setEnabled(boxFillSymbol->isChecked());
	acceptParams();
}

void PlotDialog::initErrorsPage()
{
	QGroupBox *gb1 = new QGroupBox(tr( "Direction" ));

    QVBoxLayout* vl = new QVBoxLayout(gb1);
	plusBox = new QCheckBox(tr( "Plus" ));
    vl->addWidget(plusBox);
	minusBox = new QCheckBox(tr( "Minus" ));
    vl->addWidget(minusBox);
	xBox = new QCheckBox(tr( "&X Error Bar" ));
    vl->addWidget(xBox);
    vl->addWidget(xBox);
    vl->addStretch();

	QGroupBox *gb2 = new QGroupBox(tr( "Style" ));
    QGridLayout *gl = new QGridLayout(gb2);
    gl->addWidget(new QLabel(tr( "Color" )), 0, 0);

	colorBox = new ColorButton();
    gl->addWidget(colorBox, 0, 1);

    gl->addWidget(new QLabel(tr( "Line Width" )), 1, 0);
	widthBox = new DoubleSpinBox('f');
	widthBox->setLocale(((ApplicationWindow *)parent())->locale());
	widthBox->setSingleStep(0.1);
    widthBox->setRange(0, 100);
    gl->addWidget(widthBox, 1, 1);

    gl->addWidget(new QLabel(tr( "Cap Width" )), 2, 0);
	capBox = new QComboBox();
	capBox->addItem( tr( "8" ) );
	capBox->addItem( tr( "10" ) );
	capBox->addItem( tr( "12" ) );
	capBox->addItem( tr( "16" ) );
	capBox->addItem( tr( "20" ) );
	capBox->setEditable (true);
    gl->addWidget(capBox, 2, 1);

	throughBox = new QCheckBox(tr( "Through Symbol" ));
    gl->addWidget(throughBox, 3, 0);
	gl->setRowStretch (4, 1);


	QGroupBox *gb3 = new QGroupBox();
	QGridLayout *gl3 = new QGridLayout(gb3);

	gl3->addWidget(new QLabel(tr( "Skip Points" )), 0, 0);
	boxSkipErrorBars = new QSpinBox;
	boxSkipErrorBars->setMinimum(1);
	boxSkipErrorBars->setWrapping(true);
	boxSkipErrorBars->setSpecialValueText(tr("None"));
	gl3->addWidget(boxSkipErrorBars, 0, 1);

	errorBarsFormatApplyToBox = new QComboBox;

	QLabel *l = new QLabel(tr("Apply Format &to"));
	gl3->addWidget(l, 2, 0);

	errorBarsFormatApplyToBox->insertItem(tr("Selected Curve"));
	errorBarsFormatApplyToBox->insertItem(tr("Layer"));
	errorBarsFormatApplyToBox->insertItem(tr("Window"));
	errorBarsFormatApplyToBox->insertItem(tr("All Windows"));
	gl3->addWidget(errorBarsFormatApplyToBox, 2, 1);
	l->setBuddy(errorBarsFormatApplyToBox);


    errorsPage = new QWidget();
	QHBoxLayout* hl = new QHBoxLayout();
	hl->addWidget(gb1);
	hl->addWidget(gb2);

	QVBoxLayout* vl0 = new QVBoxLayout(errorsPage);
	vl0->addLayout(hl);
	vl0->addWidget(gb3);

    privateTabWidget->insertTab( errorsPage, tr( "Error Bars" ) );

	connect(boxSkipErrorBars, SIGNAL(valueChanged(int)), this, SLOT(acceptParams()));
	connect(capBox, SIGNAL(activated(int)), this, SLOT(acceptParams()));
	connect(widthBox, SIGNAL(valueChanged(double)), this, SLOT(acceptParams()));
	connect(colorBox, SIGNAL(colorChanged()), this, SLOT(pickErrorBarsColor()));
	connect(xBox, SIGNAL(clicked()), this, SLOT(acceptParams()));
	connect(plusBox, SIGNAL(clicked()), this, SLOT(acceptParams()));
	connect(minusBox, SIGNAL(clicked()), this, SLOT(acceptParams()));
	connect(throughBox, SIGNAL(clicked()), this, SLOT(acceptParams()));
}

void PlotDialog::initHistogramPage()
{
    QHBoxLayout* hl = new QHBoxLayout();
	automaticBox = new QCheckBox(tr( "Automatic Binning" ));
    hl->addWidget(automaticBox);
    hl->addStretch();
	buttonStatistics = new QPushButton(tr( "&Show statistics" ));
    hl->addWidget(buttonStatistics);

	GroupBoxH = new QGroupBox();
    QGridLayout *gl = new QGridLayout(GroupBoxH);
    gl->addWidget(new QLabel(tr( "Bin Size" )), 0, 0);
	binSizeBox = new QLineEdit();
    gl->addWidget(binSizeBox, 0, 1);
    gl->addWidget(new QLabel(tr( "Begin" )), 1, 0);
	histogramBeginBox = new QLineEdit();
    gl->addWidget(histogramBeginBox, 1, 1);
    gl->addWidget(new QLabel(tr( "End" )), 2, 0);
	histogramEndBox = new QLineEdit();
    gl->addWidget(histogramEndBox, 2, 1);

    histogramPage = new QWidget();
	QVBoxLayout* vl = new QVBoxLayout(histogramPage);
	vl->addLayout(hl);
	vl->addWidget(GroupBoxH);
    vl->addStretch();

    privateTabWidget->insertTab( histogramPage, tr( "Histogram Data" ) );

	connect(automaticBox, SIGNAL(clicked()), this, SLOT(setAutomaticBinning()));
	connect(buttonStatistics, SIGNAL(clicked()), this, SLOT(showStatistics() ) );
}

void PlotDialog::initSpacingPage()
{
	spacingPage = new QWidget();

    QGridLayout *gl = new QGridLayout(spacingPage);
    gl->addWidget(new QLabel(tr( "Gap Between Bars (in %)" )), 0, 0);
	gapBox = new QSpinBox();
    gapBox->setRange(0, 100);
    gapBox->setSingleStep(10);
    gl->addWidget(gapBox, 0, 1);
    gl->addWidget(new QLabel(tr( "Offset (in %)" )), 1, 0);
	offsetBox = new QSpinBox();
    offsetBox->setRange(-1000, 1000);
    offsetBox->setSingleStep(50);
    gl->addWidget(offsetBox, 1, 1);
    gl->setRowStretch (2, 1);

	privateTabWidget->insertTab( spacingPage, tr( "Spacing" ));
}

void PlotDialog::initVectPage()
{
    QGroupBox *gb1 = new QGroupBox();
    QGridLayout *gl1 = new QGridLayout(gb1);
    gl1->addWidget(new QLabel(tr( "Color" )), 0, 0);
	vectColorBox = new ColorButton();
    gl1->addWidget(vectColorBox, 0, 1);
    gl1->addWidget(new QLabel(tr( "Line Width" )), 1, 0);
	vectWidthBox = new DoubleSpinBox('f');
	vectWidthBox->setLocale(((ApplicationWindow *)parent())->locale());
	vectWidthBox->setSingleStep(0.1);
    vectWidthBox->setRange(0, 100);
    gl1->addWidget(vectWidthBox, 1, 1);

	QGroupBox *gb2 = new QGroupBox(tr( "Arrowheads" ));
    QGridLayout *gl2 = new QGridLayout(gb2);
    gl2->addWidget(new QLabel(tr( "Length" )), 0, 0);
	headLengthBox = new QSpinBox();
    headLengthBox->setRange(0, 100);
    gl2->addWidget(headLengthBox, 0, 1);
    gl2->addWidget(new QLabel(tr( "Angle" )), 1, 0);
	headAngleBox = new QSpinBox();
    headAngleBox->setRange(0, 85);
    headAngleBox->setSingleStep(5);
    gl2->addWidget(headAngleBox, 1, 1);
	filledHeadBox = new QCheckBox(tr( "&Filled" ));
    gl2->addWidget(filledHeadBox, 2, 0);
    gl2->setRowStretch(3, 1);

	GroupBoxVectEnd = new QGroupBox(tr( "End Point" ));
    QGridLayout *gl3 = new QGridLayout(GroupBoxVectEnd);
    labelXEnd = new QLabel(tr( "X End" ));
    gl3->addWidget(labelXEnd, 0, 0);
	xEndBox = new QComboBox(false);
    gl3->addWidget(xEndBox, 0, 1);

	labelYEnd = new QLabel(tr( "Y End" ));
    gl3->addWidget(labelYEnd, 1, 0);
	yEndBox = new  QComboBox( false);
    gl3->addWidget(yEndBox, 1, 1);

	labelPosition = new QLabel(tr( "Position" ));
    gl3->addWidget(labelPosition, 2, 0);
	vectPosBox = new  QComboBox( false);
	vectPosBox->addItem(tr("Tail"));
	vectPosBox->addItem(tr("Middle"));
	vectPosBox->addItem(tr("Head"));
    gl3->addWidget(vectPosBox, 2, 1);
    gl3->setRowStretch(3, 1);

    vectPage = new QWidget();

    QVBoxLayout *vl1 = new QVBoxLayout();
    vl1->addWidget(gb1);
    vl1->addWidget(gb2);

	QHBoxLayout *hl = new QHBoxLayout(vectPage);
    hl->addLayout(vl1);
    hl->addWidget(GroupBoxVectEnd);

	privateTabWidget->insertTab( vectPage, tr( "Vector" ) );
}

void PlotDialog::setMultiLayer(MultiLayer *ml)
{
    if (!ml)
        return;

    d_ml = ml;
	displayPlotCoordinates(ml->applicationWindow()->d_layer_geometry_unit);
	boxResizeLayers->setChecked(!ml->scaleLayersOnResize());

	boxLinkXAxes->setChecked(d_ml->hasLinkedXLayerAxes());

	boxScaleLayers->setChecked(d_ml->scaleLayersOnPrint());
	boxPrintCrops->setChecked(d_ml->printCropmarksEnabled());

    QTreeWidgetItem *item = new QTreeWidgetItem(listBox, QStringList(ml->name()));
	item->setIcon(0, QIcon(":/folder_open.png"));
    listBox->addTopLevelItem(item);
    listBox->setCurrentItem(item);

    QList<Graph *> layers = ml->layersList();
    int i = 0;
    foreach(Graph *g, layers){
        LayerItem *layer = new LayerItem(g, item, tr("Layer") + QString::number(++i));
        item->addChild(layer);

        if (g == ml->activeLayer()){
            layer->setExpanded(true);
        	layer->setActive(true);
        	listBox->setCurrentItem(layer);
		}
    }
}

void PlotDialog::selectCurve(int index)
{
	LayerItem *layerItem = (LayerItem *)listBox->currentItem();
	if (!layerItem)
        return;
    if (layerItem->type() != LayerItem::LayerTreeItem)
        return;
	QTreeWidgetItem *item = layerItem->child(index);
	if (item){
		listBox->scrollToItem(item);
	    ((CurveTreeItem *)item)->setActive(true);
        listBox->setCurrentItem(item);
	}
}

void PlotDialog::showStatistics()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
        return;

    QTreeWidgetItem *it = listBox->currentItem();
    if (!it)
        return;
    if (it->type() != CurveTreeItem::PlotCurveTreeItem)
        return;

    QwtPlotItem *plotItem = (QwtPlotItem *)((CurveTreeItem *)it)->plotItem();
    if (!plotItem)
        return;

	QwtHistogram *h = (QwtHistogram *)plotItem;
	if (!h)
		return;

    QString tableName = app->generateUniqueName(tr("Bins"));
    Table *t = app->newTable(h->dataSize(), 4, tableName, tr("Histogram and Probabilities for") + " " + h->title().text());
    if (t)
    {
        double h_sum = 0.0;
        for (int i = 0; i < h->dataSize(); i++ )
            h_sum += h->y(i);

        double sum = 0.0;
        for (int i = 0; i < h->dataSize(); i++ )
        {
            sum += h->y(i);
            t->setCell(i, 0, h->x(i));
            t->setCell(i, 1, h->y(i));
            t->setCell(i, 2, sum);
            t->setCell(i, 3, sum/h_sum*100);
        }
        t->setHeader(QStringList() << tr("Bins") << tr("Quantity") << tr("Sum") << tr("Percent"));
        t->showMaximized();
	}

    QDateTime dt = QDateTime::currentDateTime();
	QString info = dt.toString(Qt::LocalDate)+"\t"+tr("Histogram and Probabilities for") + " " + h->title().text()+"\n";
	info += tr("Mean")+" = "+QString::number(h->mean())+"\t";
	info += tr("Standard Deviation")+" = "+QString::number(h->standardDeviation())+"\n";
	info += tr("Minimum")+" = "+QString::number(h->minimum())+"\t";
	info += tr("Maximum")+" = "+QString::number(h->maximum())+"\t";
	info += tr("Bins")+" = "+QString::number(h->dataSize())+"\n";
	info += "-------------------------------------------------------------\n";
    if (!info.isEmpty()){
        app->current_folder->appendLogInfo(info);
        app->showResults(true);
    }

	close();
}

void PlotDialog::contextMenuEvent(QContextMenuEvent *e)
{
    QTreeWidgetItem *item = listBox->currentItem();
    if (!item)
        return;
    if (item->type() != CurveTreeItem::PlotCurveTreeItem)
        return;
    QwtPlotItem *it = (QwtPlotItem *)((CurveTreeItem *)item)->plotItem();
	if (!it)
		return;

	QPoint pos = listBox->viewport()->mapFromGlobal(QCursor::pos());
	QRect rect = listBox->visualItemRect(listBox->currentItem());
	if (rect.contains(pos)){
	   QMenu contextMenu(this);
	   contextMenu.insertItem(tr("&Delete"), this, SLOT(removeSelectedCurve()));
	   if (it->rtti() == QwtPlotItem::Rtti_PlotCurve && ((PlotCurve *)it)->type() != Graph::Function)
			contextMenu.insertItem(tr("&Plot Associations..."), this, SLOT(editCurve()));
	   contextMenu.exec(QCursor::pos());
    }
    e->accept();
}

void PlotDialog::removeSelectedCurve()
{
    CurveTreeItem *item = (CurveTreeItem *)listBox->currentItem();
    if (!item)
        return;
    if (item->type() != CurveTreeItem::PlotCurveTreeItem)
        return;

    Graph *graph = item->graph();
    if (graph){
        graph->removeCurve(item->plotItemIndex());
        graph->updatePlot();

		LayerItem *layerItem = (LayerItem *)item->parent();
		QTreeWidgetItem *rootItem = layerItem->parent();

		int index = rootItem->indexOfChild (layerItem);
		rootItem->takeChild(index);
        delete layerItem;

		layerItem = new LayerItem(graph, rootItem, tr("Layer") + QString::number(d_ml->layerIndex(graph) + 1));
        rootItem->addChild(layerItem);
		if (graph->curveCount() > 0){
        	layerItem->setExpanded(true);
			CurveTreeItem *it = (CurveTreeItem *)layerItem->child(0);
			if (it){
        		listBox->setCurrentItem(it);
				setActiveCurve(it);
			}
		} else {
        	listBox->setCurrentItem(layerItem);

			clearTabWidget();
            privateTabWidget->addTab (layerPage, tr("Layer"));
			privateTabWidget->addTab (layerGeometryPage, tr("Geometry"));
			privateTabWidget->addTab (speedPage, tr("Speed"));
            privateTabWidget->showPage(layerPage);

			setActiveLayer(layerItem);
		}
    }
}

void PlotDialog::pickErrorBarsColor()
{
    CurveTreeItem *item = (CurveTreeItem *)listBox->currentItem();
    if (!item)
        return;
    if (item->type() != CurveTreeItem::PlotCurveTreeItem)
        return;

    Graph *graph = item->graph();
    if (!graph)
        return;

	graph->updateErrorBars((QwtErrorPlotCurve *)item->plotItem(), xBox->isChecked(), widthBox->value(),
			capBox->currentText().toInt(), colorBox->color(), plusBox->isChecked(), minusBox->isChecked(),
			throughBox->isChecked());
}

void PlotDialog::showAreaColor(bool show)
{
	boxAreaColor->setEnabled(show);
	boxPattern->setEnabled(show);
}

void PlotDialog::updateTabWindow(QTreeWidgetItem *currentItem, QTreeWidgetItem *previousItem)
{
	if (!currentItem)
        return;

	bool forceClearTabs = false;
	if (!previousItem){
		previousItem = currentItem;
		forceClearTabs = true;
	}

    if (previousItem->type() == CurveTreeItem::PlotCurveTreeItem)
        ((CurveTreeItem *)previousItem)->setActive(false);
    else if (previousItem->type() == LayerItem::LayerTreeItem)
        ((LayerItem *)previousItem)->setActive(false);

    boxPlotType->blockSignals(true);

    if (currentItem->type() == CurveTreeItem::PlotCurveTreeItem){
        CurveTreeItem *curveItem = (CurveTreeItem *)currentItem;
        if (previousItem->type() != CurveTreeItem::PlotCurveTreeItem ||
           ((CurveTreeItem *)previousItem)->plotItemStyle() != curveItem->plotItemStyle() ||
           ((CurveTreeItem *)previousItem)->plotItemType() != curveItem->plotItemType() ||
			forceClearTabs){
            clearTabWidget();
            int plot_type = setPlotType(curveItem);
			if (plot_type >= 0)
				insertTabs(plot_type);
            if (!curvePlotTypeBox->isVisible())
                curvePlotTypeBox->show();
        }
		setActiveCurve(curveItem);
    } else if (currentItem->type() == LayerItem::LayerTreeItem){
        if (previousItem->type() != LayerItem::LayerTreeItem){
            clearTabWidget();
            privateTabWidget->addTab (layerPage, tr("Layer"));
			privateTabWidget->addTab (layerGeometryPage, tr("Geometry"));
			privateTabWidget->addTab (speedPage, tr("Speed"));
            privateTabWidget->showPage(layerPage);
        }
        setActiveLayer((LayerItem *)currentItem);
    } else {
        clearTabWidget();
		privateTabWidget->addTab(plotGeometryPage, tr("Dimensions"));
		privateTabWidget->addTab(printPage, tr("Print"));
        privateTabWidget->addTab(fontsPage, tr("Fonts"));
		privateTabWidget->addTab(miscPage, tr("Miscellaneous"));
		privateTabWidget->showPage(plotGeometryPage);

        curvePlotTypeBox->hide();
        btnWorksheet->hide();
        btnEditCurve->hide();
    }
    boxPlotType->blockSignals(false);
}

void PlotDialog::insertTabs(int plot_type)
{
    if (plot_type == Graph::Pie){
		privateTabWidget->addTab (piePage, tr("Pattern"));
		privateTabWidget->addTab (pieGeometryPage, tr("Pie Geometry"));
		privateTabWidget->addTab (pieLabelsPage, tr("Labels"));
		privateTabWidget->showPage(piePage);
		return;
	}

    privateTabWidget->addTab (axesPage, tr("Axes"));
	if (plot_type == Graph::Line){
		boxConnect->setEnabled(true);
		privateTabWidget->addTab (linePage, tr("Line"));
		privateTabWidget->showPage(linePage);
	} else if (plot_type == Graph::Scatter){
		boxConnect->setEnabled(true);
		privateTabWidget->addTab (symbolPage, tr("Symbol"));
		privateTabWidget->showPage(symbolPage);
	} else if (plot_type == Graph::LineSymbols){
		boxConnect->setEnabled(true);
		privateTabWidget->addTab (linePage, tr("Line"));
		privateTabWidget->addTab (symbolPage, tr("Symbol"));
		privateTabWidget->showPage(symbolPage);
	} else if (plot_type == Graph::VerticalBars ||
			plot_type == Graph::HorizontalBars ||
			plot_type == Graph::Histogram){
		boxConnect->setEnabled(false);
		privateTabWidget->addTab (linePage, tr("Pattern"));
		privateTabWidget->addTab (spacingPage, tr("Spacing"));

		if (plot_type == Graph::Histogram){
			privateTabWidget->addTab (histogramPage, tr("Histogram Data"));
			privateTabWidget->showPage(histogramPage);
		} else
			privateTabWidget->showPage(linePage);
	} else if (plot_type == Graph::VectXYXY || plot_type == Graph::VectXYAM){
		boxConnect->setEnabled(true);
		//privateTabWidget->addTab (linePage, tr("Line")); //TODO: Restore this in 0.9.8 together with saving/restoring of vector curves
		privateTabWidget->addTab (vectPage, tr("Vector"));
		customVectorsPage(plot_type == Graph::VectXYAM);
		privateTabWidget->showPage(vectPage);
	} else if (plot_type == Graph::ErrorBars){
		privateTabWidget->addTab (errorsPage, tr("Error Bars"));
		privateTabWidget->showPage(errorsPage);
    } else if (plot_type == Graph::Box) {
		boxConnect->setEnabled(false);
		privateTabWidget->addTab (linePage, tr("Pattern"));
		privateTabWidget->addTab (boxPage, tr("Box/Whiskers"));
		privateTabWidget->addTab (percentilePage, tr("Percentile"));
		privateTabWidget->showPage(linePage);
		return;
	} else if (plot_type == Graph::ColorMap || plot_type == Graph::GrayScale || plot_type == Graph::Contour){
  		privateTabWidget->addTab(spectroValuesPage, tr("Values"));
  		privateTabWidget->addTab(spectrogramPage, tr("Colors"));
  		privateTabWidget->addTab(contourLinesPage, tr("Contour Lines"));
  		privateTabWidget->addTab(labelsPage, tr("Labels"));
  	    privateTabWidget->showPage(spectrogramPage);
  	    return;
  	}

  	QTreeWidgetItem *item = listBox->currentItem();
    if (!item || item->type() != CurveTreeItem::PlotCurveTreeItem)
        return;

	PlotCurve *fc = (PlotCurve *)((CurveTreeItem *)item)->plotItem();
	if (fc->type() == Graph::Function){
        privateTabWidget->addTab(functionPage, tr("&Function"));
        Graph *g = qobject_cast<Graph*>(fc->plot());
        if (g){
			functionEdit->setCurveToModify(g, g->curveIndex(fc));
			privateTabWidget->showPage(functionPage);
        }
	} else {
		DataCurve *c = (DataCurve *)((CurveTreeItem *)item)->plotItem();
		if (c && c->type() != Graph::Function && c->type() != Graph::ErrorBars){
			privateTabWidget->addTab (labelsPage, tr("Labels"));
			if (c->hasSelectedLabels())
				privateTabWidget->showPage(labelsPage);
		}
	}
}

void PlotDialog::clearTabWidget()
{
    privateTabWidget->removeTab(privateTabWidget->indexOf(labelsPage));
    privateTabWidget->removeTab(privateTabWidget->indexOf(axesPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(linePage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(symbolPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(errorsPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(histogramPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(spacingPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(vectPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(boxPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(percentilePage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(spectroValuesPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(spectrogramPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(contourLinesPage));
    privateTabWidget->removeTab(privateTabWidget->indexOf(piePage));
    privateTabWidget->removeTab(privateTabWidget->indexOf(pieGeometryPage));
    privateTabWidget->removeTab(privateTabWidget->indexOf(pieLabelsPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(layerPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(layerGeometryPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(speedPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(fontsPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(printPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(miscPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(functionPage));
	privateTabWidget->removeTab(privateTabWidget->indexOf(plotGeometryPage));
}

void PlotDialog::quit()
{
	if (acceptParams())
		close();
}

void PlotDialog::showWorksheet()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
		return;

    CurveTreeItem *item = (CurveTreeItem *)listBox->currentItem();
    if (!item)
        return;
    if (item->type() != CurveTreeItem::PlotCurveTreeItem)
        return;

	app->showCurveWorksheet(item->graph(), item->plotItemIndex());
	close();
}

int PlotDialog::setPlotType(CurveTreeItem *item)
{
	int curveType = item->plotItemStyle();
	if (curveType >= 0){
		boxPlotType->clear();

		if (curveType == Graph::ErrorBars)
			boxPlotType->addItem( tr( "Error Bars" ) );
        else if (curveType == Graph::Pie)
			boxPlotType->addItem( tr( "Pie" ) );
		else if (curveType == Graph::VerticalBars)
			boxPlotType->addItem( tr( "Vertical Bars" ) );
		else if (curveType == Graph::HorizontalBars)
			boxPlotType->addItem( tr( "Horizontal Bars" ) );
		else if (curveType == Graph::Histogram)
			boxPlotType->addItem( tr( "Histogram" ) );
		else if (curveType == Graph::VectXYXY || curveType == Graph::VectXYAM){
			boxPlotType->addItem( tr( "Vector XYXY" ) );
			boxPlotType->addItem( tr( "Vector XYAM" ) );
			if (curveType == Graph::VectXYAM)
				boxPlotType->setCurrentIndex(1);
		} else if (curveType == Graph::Box)
			boxPlotType->addItem( tr( "Box" ) );
		else if (curveType == Graph::ColorMap || curveType == Graph::GrayScale || curveType == Graph::Contour)
  	    	boxPlotType->insertItem(tr("Contour") + " / " + tr("Image"));
		else {
			boxPlotType->addItem( tr( "Line" ) );
			boxPlotType->addItem( tr( "Scatter" ) );
			boxPlotType->addItem( tr( "Line + Symbol" ) );

			QwtPlotCurve *c = (QwtPlotCurve*)item->plotItem();
			if (!c)
				return -1;

			QwtSymbol s = c->symbol();
			if (s.style() == QwtSymbol::NoSymbol){
				boxPlotType->setCurrentIndex(0);
				return Graph::Line;
			} else if (c->style() == QwtPlotCurve::NoCurve) {
				boxPlotType->setCurrentIndex(1);
				return Graph::Scatter;
			} else {
				boxPlotType->setCurrentIndex(2);
				return Graph::LineSymbols;
			}
		}
	}
	return curveType;
}

void PlotDialog::setActiveLayer(LayerItem *item)
{
    if (!item)
        return;
    item->setActive(true);

    Graph *g = item->graph();
    if (!g)
        return;

	curvePlotTypeBox->hide();
    btnWorksheet->hide();
    btnEditCurve->hide();

    boxBorderColor->blockSignals(true);
    boxBackgroundColor->blockSignals(true);
    boxCanvasColor->blockSignals(true);
    boxAntialiasing->blockSignals(true);
    boxMargin->blockSignals(true);
    boxBackgroundTransparency->blockSignals(true);
    boxCanvasTransparency->blockSignals(true);
    boxBorderWidth->blockSignals(true);

    boxMargin->setValue(g->margin());
	boxBorderWidth->setValue(g->lineWidth());
	boxBorderColor->setColor(g->frameColor());

	QColor c = g->paletteBackgroundColor();
	boxBackgroundTransparency->setValue(c.alpha());
	boxBackgroundColor->setEnabled(c.alpha());
	c.setAlpha(255);
	boxBackgroundColor->setColor(c);

	c = g->canvasBackground();
	boxCanvasTransparency->setValue(c.alpha());
	boxCanvasColor->setEnabled(c.alpha());
	c.setAlpha(255);
	boxCanvasColor->setColor(c);

	boxAntialiasing->setChecked(g->antialiasing());
	layerScaleFonts->setChecked(g->autoscaleFonts());

	boxLayerWidth->blockSignals(true);
	boxLayerHeight->blockSignals(true);

	displayCoordinates(unitBox->currentIndex(), g);

	boxLayerWidth->blockSignals(false);
	boxLayerHeight->blockSignals(false);

	aspect_ratio = (double)g->canvas()->width()/(double)g->canvas()->height();

    boxBackgroundTransparency->blockSignals(false);
    boxCanvasTransparency->blockSignals(false);
    boxBorderWidth->blockSignals(false);
    boxBorderColor->blockSignals(false);
    boxBackgroundColor->blockSignals(false);
    boxCanvasColor->blockSignals(false);
    boxAntialiasing->blockSignals(false);
    boxMargin->blockSignals(false);

	speedModeBox->blockSignals(true);
	speedModeBox->setChecked(g->getDouglasPeukerTolerance() > 0.0);
	speedModeBox->blockSignals(false);

	boxDouglasPeukerTolerance->blockSignals(true);
    boxDouglasPeukerTolerance->setValue(g->getDouglasPeukerTolerance());
    boxDouglasPeukerTolerance->blockSignals(false);

    boxMaxPoints->blockSignals(true);
    boxMaxPoints->setValue(g->speedModeMaxPoints());
    boxMaxPoints->blockSignals(false);
}

void PlotDialog::updateContourLevelsDisplay(Spectrogram *sp)
{
	QwtValueList levels = sp->contourLevels();
	levelsBox->setValue(levels.size());
	if (levels.size() >= 1)
		firstContourLineBox->setValue(levels[0]);
	if (levels.size() >= 2)
		contourLinesDistanceBox->setValue(fabs(levels[1] - levels[0]));
}

void PlotDialog::setActiveCurve(CurveTreeItem *item)
{
    if (!item)
        return;

    const QwtPlotItem *i = item->plotItem();
    if (!i)
        return;

	item->setActive(true);
	listBox->scrollToItem(item);
	btnWorksheet->show();
    btnEditCurve->show();

    //axes page
    boxXAxis->setCurrentIndex(i->xAxis()-2);
    boxYAxis->setCurrentIndex(i->yAxis());

    if (i->rtti() == QwtPlotItem::Rtti_PlotSpectrogram){
        btnEditCurve->hide();
        Spectrogram *sp = (Spectrogram *)i;

		boxSpectroMatrix->setCurrentIndex(boxSpectroMatrix->findText (sp->matrix()->objectName()));
		boxUseMatrixFormula->setChecked(sp->useMatrixFormula());
		boxUseMatrixFormula->setEnabled(!sp->matrix()->formula().isEmpty());

        imageGroupBox->setChecked(sp->testDisplayMode(QwtPlotSpectrogram::ImageMode));
        grayScaleBox->setChecked(sp->colorMapPolicy() == Spectrogram::GrayScale);
        defaultScaleBox->setChecked(sp->colorMapPolicy() == Spectrogram::Default);
        customScaleBox->setChecked(sp->colorMapPolicy() == Spectrogram::Custom);

		//colorMapEditor->setRange(sp->data().range().minValue(), sp->data().range().maxValue());
		colorMapEditor->setRange(0, sp->data().range().maxValue());
        colorMapEditor->setColorMap((const QwtLinearColorMap &)sp->colorMap());

        levelsGroupBox->setChecked(sp->testDisplayMode(QwtPlotSpectrogram::ContourMode));

        contourLinesEditor->setSpectrogram(sp);
		updateContourLevelsDisplay(sp);

        autoContourBox->setChecked(sp->useColorMapPen());
        defaultContourBox->setChecked(sp->defaultContourPen().style() != Qt::NoPen);
        customPenBtn->setChecked(sp->defaultContourPen().style() == Qt::NoPen && !sp->useColorMapPen());

        levelsColorBox->setColor(sp->defaultContourPen().color());
        contourWidthBox->setValue(sp->defaultContourPen().widthF());
        if (sp->defaultContourPen().style() != Qt::NoPen)
            boxContourStyle->setCurrentIndex(sp->defaultContourPen().style() - 1);
        else
            boxContourStyle->setCurrentIndex(0);

        axisScaleBox->setChecked(sp->hasColorScale());
        colorScaleBox->setCurrentItem((int)sp->colorScaleAxis());
        colorScaleWidthBox->setValue(sp->colorBarWidth());

        //labels page

        showAllLabelControls(false);

        labelsGroupBox->blockSignals(true);
        labelsGroupBox->setEnabled(sp->testDisplayMode(QwtPlotSpectrogram::ContourMode));
		labelsGroupBox->setChecked(sp->hasLabels() && sp->testDisplayMode(QwtPlotSpectrogram::ContourMode));
		boxLabelsColor->setColor(sp->labelsColor());
		boxLabelsAngle->setValue(sp->labelsRotation());

		boxLabelsXOffset->blockSignals(true);
		boxLabelsXOffset->setValue(qRound(sp->labelsXOffset()));
		boxLabelsXOffset->blockSignals(false);

		boxLabelsYOffset->blockSignals(true);
		boxLabelsYOffset->setValue(qRound(sp->labelsYOffset()));
		boxLabelsYOffset->blockSignals(false);
		boxLabelsWhiteOut->setChecked(sp->labelsWhiteOut());
		labelsGroupBox->blockSignals(false);

		if (sp->hasSelectedLabels())
			privateTabWidget->showPage(labelsPage);
        return;
    }

    PlotCurve *c = (PlotCurve*)i;
	btnEditCurve->setVisible(c->type() != Graph::Function);

	int curveType = item->plotItemStyle();
    if (curveType == Graph::Pie){
        QwtPieCurve *pie = (QwtPieCurve*)i;
        boxPiePattern->setPattern(pie->pattern());
        boxPieLineWidth->setValue(pie->pen().widthF());
        boxPieLineColor->setColor(pie->pen().color());
        boxPieLineStyle->setStyle(pie->pen().style());
        boxFirstColor->setCurrentIndex(pie->firstColor());

        boxPieViewAngle->blockSignals(true);
		boxPieViewAngle->setValue(pie->viewAngle());
		boxPieViewAngle->blockSignals(false);
		boxPieThickness->blockSignals(true);
		boxPieThickness->setValue(pie->thickness());
		boxPieThickness->blockSignals(false);
		boxPieStartAzimuth->blockSignals(true);
		boxPieStartAzimuth->setValue(pie->startAzimuth());
		boxPieStartAzimuth->blockSignals(false);
		boxPieConterClockwise->blockSignals(true);
		boxPieConterClockwise->setChecked(pie->counterClockwise());
		boxPieConterClockwise->blockSignals(false);
		boxRadius->blockSignals(true);
        boxRadius->setValue(pie->radius());
        boxRadius->blockSignals(false);
        boxPieOffset->blockSignals(true);
        boxPieOffset->setValue((int)pie->horizontalOffset());
        boxPieOffset->blockSignals(false);

		pieAutoLabelsBox->setChecked(pie->labelsAutoFormat());
		boxPieValues->setChecked(pie->labelsValuesFormat());
		boxPiePercentages->setChecked(pie->labelsPercentagesFormat());
		boxPieCategories->setChecked(pie->labelCategories());
		boxPieEdgeDist->blockSignals(true);
		boxPieEdgeDist->setValue(pie->labelsEdgeDistance());
		boxPieEdgeDist->blockSignals(false);
		boxPieWedge->setChecked(pie->fixedLabelsPosition());
        return;
    }

    //line page
    int style = c->style();
    if (curveType == Graph::Spline)
        style = 5;
    else if (curveType == Graph::VerticalSteps)
        style = 6;
    boxConnect->setCurrentIndex(style);

	boxLineStyle->setStyle(c->pen().style());
	boxLineColor->blockSignals(true);
    boxLineColor->setColor(c->pen().color());
    boxLineColor->blockSignals(false);
	boxLineWidth->blockSignals(true);
    boxLineWidth->setValue(c->pen().widthF());
	boxLineWidth->blockSignals(false);
    fillGroupBox->blockSignals(true);
    fillGroupBox->setChecked(c->brush().style() != Qt::NoBrush );
    fillGroupBox->blockSignals(false);

	QColor ac = c->brush().color();
	boxCurveOpacity->blockSignals(true);
	boxCurveOpacity->setValue(ac.alpha());
	curveOpacitySlider->setValue(ac.alpha());
	boxCurveOpacity->blockSignals(false);

	ac.setAlpha(255);

	boxAreaColor->blockSignals(true);
	boxAreaColor->setColor(ac);
	boxAreaColor->blockSignals(false);

	boxPattern->setPattern(c->brush().style());

    //symbol page
    const QwtSymbol s = c->symbol();
    boxSymbolSize->setValue(s.size().width()/2);
    boxSymbolStyle->setStyle(s.style());
    boxSymbolColor->blockSignals(true);
    boxSymbolColor->setColor(s.pen().color());
    boxSymbolColor->blockSignals(false);
    boxPenWidth->setValue(s.pen().widthF());
    boxFillSymbol->setChecked(s.brush() != Qt::NoBrush);
    boxFillColor->setEnabled(s.brush() != Qt::NoBrush);
    boxFillColor->blockSignals(true);
    boxFillColor->setColor(s.brush().color());
    boxFillColor->blockSignals(false);
    boxSkipSymbols->blockSignals(true);
    boxSkipSymbols->setValue(c->skipSymbolsCount());
    boxSkipSymbols->setMaximum(c->dataSize());
	boxSkipSymbols->blockSignals(false);

    if (c->type() == Graph::Function){
		functionEdit->setCurveToModify((FunctionCurve *)c);
        return;
    }

    if (curveType == Graph::VerticalBars || curveType == Graph::HorizontalBars ||
				curveType == Graph::Histogram){//spacing page
        QwtBarCurve *b = (QwtBarCurve*)i;
        if (b){
            gapBox->setValue(b->gap());
            offsetBox->setValue(b->offset());
        }
    }

    if (curveType == Graph::Histogram){//Histogram page
        QwtHistogram *h = (QwtHistogram*)i;
        if (h){
            automaticBox->setChecked(h->autoBinning());
            binSizeBox->setText(QString::number(h->binSize()));
            histogramBeginBox->setText(QString::number(h->begin()));
            histogramEndBox->setText(QString::number(h->end()));
            setAutomaticBinning();
        }
    }

    if (curveType == Graph::VectXYXY || curveType == Graph::VectXYAM){//Vector page
        VectorCurve *v = (VectorCurve*)i;
        if (v){
            vectColorBox->setColor(v->color());
            vectWidthBox->setValue(v->width());
            headLengthBox->setValue(v->headLength());
            headAngleBox->setValue(v->headAngle());
            filledHeadBox->setChecked(v->filledArrowHead());
            vectPosBox->setCurrentIndex(v->position());
            updateEndPointColumns(item->text(0));
        }
    }

    if (curveType == Graph::ErrorBars){
        QwtErrorPlotCurve *err = (QwtErrorPlotCurve*)i;
        if (err){
			widthBox->blockSignals(true);
            widthBox->setValue(err->width());
			widthBox->blockSignals(false);
            capBox->setEditText(QString::number(err->capLength()));

			colorBox->blockSignals(true);
            colorBox->setColor(err->color());
			colorBox->blockSignals(false);

			throughBox->blockSignals(true);
            throughBox->setChecked(err->throughSymbol());
			throughBox->blockSignals(false);
			plusBox->blockSignals(true);
            plusBox->setChecked(err->plusSide());
			plusBox->blockSignals(false);
			minusBox->blockSignals(true);
            minusBox->setChecked(err->minusSide());
			minusBox->blockSignals(false);
			xBox->blockSignals(true);
			xBox->setChecked(err->xErrors());
			xBox->blockSignals(false);

			boxSkipErrorBars->blockSignals(true);
			boxSkipErrorBars->setMaximum(err->dataSize());
			boxSkipErrorBars->setValue(err->skipSymbolsCount());
			boxSkipErrorBars->blockSignals(false);
        }
		return;
    }

    if (curveType == Graph::Box){
        BoxCurve *b = (BoxCurve*)i;
        if (b){
            boxMaxStyle->setStyle(b->maxStyle());
            boxMinStyle->setStyle(b->minStyle());
            boxMeanStyle->setStyle(b->meanStyle());
            box99Style->setStyle(b->p99Style());
            box1Style->setStyle(b->p1Style());

            boxPercSize->setValue(s.size().width()/2);
            boxFillSymbols->setChecked(s.brush() != Qt::NoBrush);
            boxPercFillColor->setEnabled(s.brush() != Qt::NoBrush);
            boxPercFillColor->setColor(s.brush().color());
            boxEdgeColor->setColor(s.pen().color());
            boxEdgeWidth->setValue(s.pen().widthF());

            boxRange->setCurrentIndex (b->boxRangeType()-1);
            boxType->setCurrentIndex (b->boxStyle());
            boxWidth->setValue(b->boxWidth());
            setBoxRangeType(boxRange->currentIndex());
            setBoxType(boxType->currentIndex());
            if (b->boxRangeType() == BoxCurve::SD || b->boxRangeType() == BoxCurve::SE)
                boxCnt->setValue(b->boxRange());
			else
				boxCoef->setValue((int)b->boxRange());

            boxWhiskersRange->setCurrentIndex (b->whiskersRangeType());
            setWhiskersRange(boxWhiskersRange->currentIndex());
            if (b->whiskersRangeType() == BoxCurve::SD || b->whiskersRangeType() == BoxCurve::SE)
                whiskerCnt->setValue(b->whiskersRange());
            else
                boxWhiskersCoef->setValue((int)b->whiskersRange());
        }
		return;
    }

    DataCurve *dc = (DataCurve *)i;
	if (!dc->table()){
		privateTabWidget->removeTab(privateTabWidget->indexOf(labelsPage));
		return;
	}
	showAllLabelControls();

    labelsGroupBox->blockSignals(true);
    labelsGroupBox->setChecked(dc->hasLabels());

    QStringList cols = dc->table()->columnsList();
    boxLabelsColumn->blockSignals(true);
    boxLabelsColumn->clear();
    boxLabelsColumn->addItems(cols);
    int labelsColIndex = cols.indexOf(dc->labelsColumnName());
    if (labelsColIndex >= 0)
        boxLabelsColumn->setCurrentIndex(labelsColIndex);
    boxLabelsColumn->blockSignals(false);

    boxLabelsAngle->setValue(dc->labelsRotation());
    boxLabelsColor->blockSignals(true);
    boxLabelsColor->setColor(dc->labelsColor());
    boxLabelsColor->blockSignals(false);
	boxLabelsXOffset->blockSignals(true);
    boxLabelsXOffset->setValue(dc->labelsXOffset());
	boxLabelsXOffset->blockSignals(false);

	boxLabelsYOffset->blockSignals(true);
    boxLabelsYOffset->setValue(dc->labelsYOffset());
	boxLabelsYOffset->blockSignals(false);
    boxLabelsWhiteOut->setChecked(dc->labelsWhiteOut());
    switch(dc->labelsAlignment()){
		case Qt::AlignHCenter:
			boxLabelsAlign->setCurrentIndex(0);
			break;
		case Qt::AlignLeft:
			boxLabelsAlign->setCurrentIndex(1);
			break;
		case Qt::AlignRight:
			boxLabelsAlign->setCurrentIndex(2);
			break;
	}
	labelsGroupBox->blockSignals(false);
}

void PlotDialog::updateEndPointColumns(const QString& text)
{
	QStringList cols=text.split(",", QString::SkipEmptyParts);
	QStringList aux=cols[0].split(":", QString::SkipEmptyParts);
	QString table=aux[0];
	QStringList list;
	for (int i=0; i<(int)columnNames.count(); i++)
	{
		QString s=columnNames[i];
		if (s.contains(table))
			list<<s;
	}

	xEndBox->clear();
	xEndBox->insertStringList(list);
	xEndBox->setCurrentText(table + "_" + cols[2].remove("(X)").remove("(A)"));

	yEndBox->clear();
	yEndBox->insertStringList(list);
	yEndBox->setCurrentText(table + "_" + cols[3].remove("(Y)").remove("(M)"));
}

void PlotDialog::applyCanvasSize()
{
	if (privateTabWidget->currentWidget() != layerGeometryPage)
		return;

	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	switch(sizeApplyToBox->currentIndex()){
		case 1://this window
		{
			QSize size = QSize();
			QList<Graph *> layersLst = d_ml->layersList();
			foreach(Graph *g, layersLst){
				size = layerCanvasRect(g, boxX->value(), boxY->value(), boxLayerWidth->value(),
									   boxLayerHeight->value(), (FrameWidget::Unit)unitBox->currentIndex()).size();
				g->setCanvasSize(size);
			}
			if (size.isValid())
				d_ml->setLayerCanvasSize(size.width(), size.height());
		}
		break;

		case 2://all windows
		{
			QList<MdiSubWindow *> windows = app->windowsList();
			foreach(MdiSubWindow *w, windows){
				MultiLayer *ml = qobject_cast<MultiLayer *>(w);
				if (!ml)
					continue;

				QSize size = QSize();
				QList<Graph *> layersLst = ml->layersList();
				foreach(Graph *g, layersLst){
					size = layerCanvasRect(g, boxX->value(), boxY->value(), boxLayerWidth->value(),
										   boxLayerHeight->value(), (FrameWidget::Unit)unitBox->currentIndex()).size();
					g->setCanvasSize(size);
				}
				if (size.isValid())
					ml->setLayerCanvasSize(size.width(), size.height());
			}
		}
		break;

		default:
			break;
	}
}

void PlotDialog::applyLayerFormat()
{
	if (privateTabWidget->currentWidget() != layerPage)
		return;

	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	switch(backgroundApplyToBox->currentIndex()){
		case 0://this layer
		{
			LayerItem *item = (LayerItem *)listBox->currentItem();
        	if (!item)
            	return;

        	Graph *g = item->graph();
			if (!g)
				return;

			applyFormatToLayer(g);
		}
		break;

		case 1://this window
		{
			QList<Graph *> layersLst = d_ml->layersList();
			foreach(Graph *g, layersLst)
				applyFormatToLayer(g);
		}
		break;

		case 2://all windows
		{
			QList<MdiSubWindow *> windows = app->windowsList();
			foreach(MdiSubWindow *w, windows){
				MultiLayer *ml = qobject_cast<MultiLayer *>(w);
				if (!ml)
					continue;

				QList<Graph *> layersLst = ml->layersList();
				foreach(Graph *g, layersLst)
					applyFormatToLayer(g);
			}
		}
		break;

		default:
			break;
	}
	app->modifiedProject();
}

void PlotDialog::applyFormatToLayer(Graph *g)
{
	if (!g)
		return;

	g->setFrame(boxBorderWidth->value(), boxBorderColor->color());
	g->setMargin(boxMargin->value());

	QColor c = boxBackgroundColor->color();
	c.setAlpha(boxBackgroundTransparency->value());
	g->setBackgroundColor(c);

	c = boxCanvasColor->color();
	c.setAlpha(boxCanvasTransparency->value());
	g->setCanvasBackground(c);

	g->setAntialiasing(boxAntialiasing->isChecked());
	g->setAutoscaleFonts(layerScaleFonts->isChecked());
}

bool PlotDialog::acceptParams()
{
	if (privateTabWidget->currentWidget() == plotGeometryPage){
		FrameWidget::setRect(d_ml, boxPlotX->value(), boxPlotY->value(), boxPlotWidth->value(),
		boxPlotHeight->value(), (FrameWidget::Unit)plotUnitBox->currentIndex());
		d_ml->setScaleLayersOnResize(!boxResizeLayers->isChecked());
		return true;
	} else if (privateTabWidget->currentWidget() == fontsPage){
		d_ml->setFonts(titleFont, axesFont, numbersFont, legendFont);
		return true;
    } else if (privateTabWidget->currentWidget() == printPage){
		d_ml->setScaleLayersOnPrint(boxScaleLayers->isChecked());
		d_ml->printCropmarks(boxPrintCrops->isChecked());
		return true;
	} else if (privateTabWidget->currentWidget() == miscPage){
		d_ml->linkXLayerAxes(boxLinkXAxes->isChecked());
		if (boxLinkAllXAxes->currentIndex() == 1){
			ApplicationWindow *app = (ApplicationWindow *)this->parent();
			if (app){
				QList<MdiSubWindow *> windows = app->windowsList();
				foreach(MdiSubWindow *w, windows){
					MultiLayer *ml = qobject_cast<MultiLayer *>(w);
					if (ml)
						ml->linkXLayerAxes(boxLinkXAxes->isChecked());
				}
			}
		}
		d_ml->notifyChanges();
		return true;
	} else if (privateTabWidget->currentWidget() == layerPage){
		applyLayerFormat();
		return true;
	} else if (privateTabWidget->currentWidget() == layerGeometryPage){
		LayerItem *item = (LayerItem *)listBox->currentItem();
        if (!item)
            return false;
        Graph *g = item->graph();
		if (!g)
			return false;

		g->setCanvasGeometry(layerCanvasRect(g, boxX->value(), boxY->value(), boxLayerWidth->value(),
								boxLayerHeight->value(), (FrameWidget::Unit)unitBox->currentIndex()));
		g->notifyChanges();

		applyCanvasSize();

		ApplicationWindow *app = (ApplicationWindow *)this->parent();
		if (app)
			app->d_layer_geometry_unit = unitBox->currentIndex();

		return true;
	} else if (privateTabWidget->currentWidget() == speedPage){
		LayerItem *item = (LayerItem *)listBox->currentItem();
        if (!item)
            return false;
        Graph *g = item->graph();
		if (!g)
			return false;

		double tolerance = boxDouglasPeukerTolerance->value();
		if (!speedModeBox->isChecked())
			tolerance = 0;

		g->enableDouglasPeukerSpeedMode(tolerance, boxMaxPoints->value());
		return true;
	}

    QTreeWidgetItem *it = listBox->currentItem();
    if (!it)
        return false;

    CurveTreeItem *item = (CurveTreeItem *)it;
    QwtPlotItem *plotItem = (QwtPlotItem *)item->plotItem();
    if (!plotItem)
        return false;

    Graph *graph = item->graph();
    if (!graph)
        return false;

	if (privateTabWidget->currentPage() == axesPage){
		plotItem->setAxis(boxXAxis->currentIndex() + 2, boxYAxis->currentIndex());
		graph->setAutoScale();
		return true;
	} else if (privateTabWidget->currentPage() == spectroValuesPage){
  		Spectrogram *sp = (Spectrogram *)plotItem;
  	    if (!sp || sp->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
  	    	return false;

		ApplicationWindow *app = (ApplicationWindow *)this->parent();
		Matrix *m = app->matrix(boxSpectroMatrix->currentText());
		if (!m)
			return false;
		if (!sp->setMatrix(m, boxUseMatrixFormula->isChecked()))
			boxUseMatrixFormula->setChecked(false);
		updateContourLevelsDisplay(sp);
  	} else if (privateTabWidget->currentPage() == spectrogramPage){
  		Spectrogram *sp = (Spectrogram *)plotItem;
  	    if (!sp || sp->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
  	    	return false;

  	   sp->setDisplayMode(QwtPlotSpectrogram::ImageMode, imageGroupBox->isChecked());

  	   if (grayScaleBox->isChecked()){
		   sp->setGrayScale();
	   	   colorMapEditor->setColorMap(QwtLinearColorMap(Qt::black, Qt::white));
  	   } else if (defaultScaleBox->isChecked()){
	   	   sp->setDefaultColorMap();
		   colorMapEditor->setColorMap(sp->colorMap());
	   } else
	   	   sp->setCustomColorMap(colorMapEditor->colorMap());

  	   sp->showColorScale((QwtPlot::Axis)colorScaleBox->currentItem(), axisScaleBox->isChecked());
  	   sp->setColorBarWidth(colorScaleWidthBox->value());

  	   //Update axes page
	   boxXAxis->setCurrentIndex(sp->xAxis() - 2);
	   boxYAxis->setCurrentIndex(sp->yAxis());
  	} else if (privateTabWidget->currentPage() == contourLinesPage){
  		Spectrogram *sp = (Spectrogram *)plotItem;
  	    if (!sp || sp->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
  	    	return false;

		if (defaultContourBox->isChecked()){
			QPen pen = QPen(levelsColorBox->color(), contourWidthBox->value(), boxContourStyle->style());
  	    	sp->setDefaultContourPen(pen);
		} else if (customPenBtn->isChecked())
			contourLinesEditor->updateContourPens();
		else
			sp->setColorMapPen();

		contourLinesEditor->updateContourLevels();

		sp->setDisplayMode(QwtPlotSpectrogram::ContourMode, levelsGroupBox->isChecked());

		labelsGroupBox->setEnabled(levelsGroupBox->isChecked());

		if (!levelsGroupBox->isChecked()){
			labelsGroupBox->setChecked(false);
			sp->showContourLineLabels(false);
		}
  	} else if (privateTabWidget->currentPage() == linePage){
		graph->setCurveStyle(item->plotItemIndex(), boxConnect->currentIndex());

		QColor col = boxAreaColor->color();
		col.setAlpha(boxCurveOpacity->value());
		QBrush br = QBrush(col, boxPattern->getSelectedPattern());
		if (!fillGroupBox->isChecked())
			br = QBrush();
		QPen pen = QPen(boxLineColor->color(), boxLineWidth->value(), boxLineStyle->style(), Qt::SquareCap, Qt::MiterJoin);
		pen.setCosmetic(true);
		QwtPlotCurve *curve = (QwtPlotCurve *)plotItem;
		curve->setPen(pen);
		curve->setBrush(br);

		applyLineFormat((QwtPlotCurve *)plotItem);
	} else if (privateTabWidget->currentPage() == symbolPage)
		applySymbolsFormat((QwtPlotCurve *)plotItem);
	else if (privateTabWidget->currentPage() == histogramPage){
        QwtHistogram *h = (QwtHistogram *)plotItem;
		if (!h)
			return false;

		if (validInput()){
            if (h->autoBinning() == automaticBox->isChecked() &&
                h->binSize() == binSizeBox->text().toDouble() &&
                h->begin() == histogramBeginBox->text().toDouble() &&
                h->end() == histogramEndBox->text().toDouble()) return true;

            h->setBinning(automaticBox->isChecked(), binSizeBox->text().toDouble(),
                         histogramBeginBox->text().toDouble(), histogramEndBox->text().toDouble());
            h->loadData();
			graph->updateScale();
			graph->notifyChanges();
			return true;
		}
	} else if (privateTabWidget->currentPage() == spacingPage)
		graph->setBarsGap(item->plotItemIndex(), gapBox->value(), offsetBox->value());
	else if (privateTabWidget->currentPage() == vectPage){
		ApplicationWindow *app = (ApplicationWindow *)this->parent();
		if (!app)
			return false;

		QString xEndCol = xEndBox->currentText();
		QString yEndCol = yEndBox->currentText();
		Table* w = app->table(xEndCol);
		if (!w)
			return false;

		graph->updateVectorsLayout(item->plotItemIndex(), vectColorBox->color(),
				vectWidthBox->value(), headLengthBox->value(), headAngleBox->value(),
				filledHeadBox->isChecked(), vectPosBox->currentIndex(), xEndCol, yEndCol);

		VectorCurve *v = (VectorCurve*)item->plotItem();
		QString table = v->table()->name();
		item->setText(0, table + ": " + v->plotAssociation().remove(table + "_"));
		return true;
	} else if (privateTabWidget->currentPage() == errorsPage){
		QwtErrorPlotCurve *err = (QwtErrorPlotCurve *)item->plotItem();
		if (!err)
			return false;

		applyErrorBarFormat(err);

	} else if (privateTabWidget->currentPage() == piePage){
		QwtPieCurve *pie = (QwtPieCurve*)plotItem;
		pie->setPen(QPen(boxPieLineColor->color(), boxPieLineWidth->value(), boxPieLineStyle->style()));
        pie->setBrushStyle(boxPiePattern->getSelectedPattern());
        pie->setFirstColor(boxFirstColor->currentIndex());
	} else if (privateTabWidget->currentPage() == pieGeometryPage){
		QwtPieCurve *pie = (QwtPieCurve*)plotItem;
		pie->setViewAngle(boxPieViewAngle->value());
		pie->setThickness(boxPieThickness->value());
		pie->setRadius(boxRadius->value());
        pie->setHorizontalOffset(boxPieOffset->value());
        pie->setStartAzimuth(boxPieStartAzimuth->value());
		pie->setCounterClockwise(boxPieConterClockwise->isChecked());
	} else if (privateTabWidget->currentPage() == pieLabelsPage){
		QwtPieCurve *pie = (QwtPieCurve*)plotItem;
		pie->setLabelsAutoFormat(pieAutoLabelsBox->isChecked());
        pie->setLabelValuesFormat(boxPieValues->isChecked());
        pie->setLabelPercentagesFormat(boxPiePercentages->isChecked());
		pie->setLabelCategories(boxPieCategories->isChecked());
        pie->setFixedLabelsPosition(boxPieWedge->isChecked());
        pie->setLabelsEdgeDistance(boxPieEdgeDist->value());
        graph->replot();
	} else if (privateTabWidget->currentPage() == percentilePage){
		BoxCurve *b = (BoxCurve*)plotItem;
		if (b){
			b->setMaxStyle(boxMaxStyle->selectedSymbol());
			b->setP99Style(box99Style->selectedSymbol());
			b->setMeanStyle(boxMeanStyle->selectedSymbol());
			b->setP1Style(box1Style->selectedSymbol());
			b->setMinStyle(boxMinStyle->selectedSymbol());

            int size = 2*boxPercSize->value() + 1;
            QBrush br = QBrush(boxPercFillColor->color(), Qt::SolidPattern);
            if (!boxFillSymbols->isChecked())
                br = QBrush();

			QPen pen = QPen(boxEdgeColor->color(), boxEdgeWidth->value(),
							Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
			pen.setCosmetic(true);
			b->setSymbol(QwtSymbol(QwtSymbol::NoSymbol, br, pen, QSize(size, size)));
		}
	} else if (privateTabWidget->currentPage() == boxPage){
		BoxCurve *b = (BoxCurve*)plotItem;
		if (b){
			b->setBoxWidth(boxWidth->value());
			b->setBoxStyle(boxType->currentIndex());
			if (boxCnt->isVisible())
				b->setBoxRange(boxRange->currentIndex()+1, boxCnt->value());
			else
				b->setBoxRange(boxRange->currentIndex()+1, (double)boxCoef->value());

			if (whiskerCnt->isVisible())
				b->setWhiskersRange(boxWhiskersRange->currentIndex(), whiskerCnt->value());
			else
				b->setWhiskersRange(boxWhiskersRange->currentIndex(), (double)boxWhiskersCoef->value());
		}
	} else if (privateTabWidget->currentPage() == labelsPage){
		Spectrogram *sp = (Spectrogram *)plotItem;
  	    if (sp && sp->rtti() == QwtPlotItem::Rtti_PlotSpectrogram){
  	    	sp->setLabelsRotation(boxLabelsAngle->value());
  	    	sp->showContourLineLabels(labelsGroupBox->isChecked());
			sp->setLabelsWhiteOut(boxLabelsWhiteOut->isChecked());
			sp->setLabelsOffset((double)boxLabelsXOffset->value(), (double)boxLabelsYOffset->value());
			sp->setLabelsColor(boxLabelsColor->color());
  	    } else {
			DataCurve *c = (DataCurve *)plotItem;

			QString text = item->text(0);
			QStringList t = text.split(": ", QString::SkipEmptyParts);
			QString table = t[0];
			QStringList cols = t[1].split(",", QString::SkipEmptyParts);

			if (labelsGroupBox->isChecked()){
				c->setLabelsColumnName(boxLabelsColumn->currentText());

				if (cols.count() == 3)
					cols[2] = boxLabelsColumn->currentText().remove(table + "_") + "(L)";
				else if (cols.count() == 5)//vector curves
					cols[4] = boxLabelsColumn->currentText().remove(table + "_") + "(L)";
				else
					cols << boxLabelsColumn->currentText().remove(table + "_") + "(L)";
				item->setText(0, table + ": " + cols.join(","));
			} else {
				c->setLabelsColumnName(QString());
				cols.pop_back();
				item->setText(0, table + ": " + cols.join(","));
			}

			c->setLabelsRotation(boxLabelsAngle->value());
			c->setLabelsWhiteOut(boxLabelsWhiteOut->isChecked());
			c->setLabelsOffset(boxLabelsXOffset->value(), boxLabelsYOffset->value());
			c->setLabelsColor(boxLabelsColor->color());
			c->setLabelsAlignment(labelsAlignment());
  	    }
	} else if (privateTabWidget->currentPage() == functionPage){
		functionEdit->accept();
	}

	graph->replot();
	graph->notifyChanges();
	return true;
}

void PlotDialog::setAutomaticBinning()
{
	GroupBoxH->setEnabled(!automaticBox->isChecked());
}

bool PlotDialog::validInput()
{
	QString from = histogramBeginBox->text();
	QString to = histogramEndBox->text();
	QString step = binSizeBox->text();
	QRegExp nonDigit("\\D");

	if (histogramBeginBox->text().isEmpty())
	{
		QMessageBox::critical(this, tr("QtiPlot - Input error"), tr("Please enter a valid start limit!"));
		histogramBeginBox->setFocus();
		return false;
	}

	if (histogramEndBox->text().isEmpty())
	{
		QMessageBox::critical(this, tr("QtiPlot - Input error"), tr("Please enter a valid end limit!"));
		histogramEndBox->setFocus();
		return false;
	}

	if (binSizeBox->text().isEmpty())
	{
		QMessageBox::critical(this, tr("QtiPlot - Input error"), tr("Please enter a valid bin size value!"));
		binSizeBox->setFocus();
		return false;
	}

	from = from.remove(".");
	to = to.remove(".");
	step = step.remove(".");

	int pos=from.find("-",0);
	if(pos==0)
		from=from.replace(pos,1,"");

	pos=to.find("-",0);
	if(pos==0)
		to=to.replace(pos,1,"");

	double start,end, stp;
	bool error = false;
	if (from.contains(nonDigit))
	{
		try
		{
			MyParser parser;
			parser.SetExpr((histogramBeginBox->text()).ascii());
			start=parser.Eval();
		}
		catch(mu::ParserError &e)
		{
			QMessageBox::critical(this, tr("QtiPlot - Start limit error"), QString::fromStdString(e.GetMsg()));
			histogramBeginBox->setFocus();
			error = true;
			return false;
		}
	}
	else
		start = histogramBeginBox->text().toDouble();

	if (to.contains(nonDigit))
	{
		try
		{
			MyParser parser;
			parser.SetExpr((histogramEndBox->text()).ascii());
			end=parser.Eval();
		}
		catch(mu::ParserError &e)
		{
			QMessageBox::critical(this, tr("QtiPlot - End limit error"), QString::fromStdString(e.GetMsg()));
			histogramEndBox->setFocus();
			error=true;
			return false;
		}
	}
	else
		end=histogramEndBox->text().toDouble();

	if (start>=end)
	{
		QMessageBox::critical(this, tr("QtiPlot - Input error"), tr("Please enter limits that satisfy: begin < end!"));
		histogramEndBox->setFocus();
		return false;
	}

	if (step.contains(nonDigit))
	{
		try
		{
			MyParser parser;
			parser.SetExpr((binSizeBox->text()).ascii());
			stp=parser.Eval();
		}
		catch(mu::ParserError &e)
		{
			QMessageBox::critical(this, tr("QtiPlot - Bin size input error"),QString::fromStdString(e.GetMsg()));
			binSizeBox->setFocus();
			error=true;
			return false;
		}
	}
	else
		stp=binSizeBox->text().toDouble();

	if (stp <=0)
	{
		QMessageBox::critical(this, tr("QtiPlot - Bin size input error"), tr("Please enter a positive bin size value!"));
		binSizeBox->setFocus();
		return false;
	}

	return true;
}

void PlotDialog::setBoxType(int index)
{
	boxCoeffLabel->hide();
	boxRangeLabel->hide();
	boxRange->hide();
	boxCoef->hide();
	boxCntLabel->hide();
	boxCnt->hide();

	if (index != BoxCurve::NoBox && index != BoxCurve::WindBox)
	{
		boxRange->show();
		boxRangeLabel->show();
		int id = boxRange->currentIndex() + 1;
		if (id == BoxCurve::UserDef)
		{
			boxCoef->show();
			boxCoeffLabel->show();
		}
		else if (id == BoxCurve::SD || id == BoxCurve::SE)
		{
			boxCntLabel->show();
			boxCnt->show();
		}
	}
}

void PlotDialog::setBoxRangeType(int index)
{
	boxCoeffLabel->hide();
	boxCoef->hide();
	boxCntLabel->hide();
	boxCnt->hide();

	index++;
	if (index == BoxCurve::UserDef)
	{
		boxCoeffLabel->show();
		boxCoef->show();
	}
	else if (index == BoxCurve::SD || index == BoxCurve::SE)
	{
		boxCntLabel->show();
		boxCnt->show();
	}
}

void PlotDialog::setWhiskersRange(int index)
{
	whiskerCoeffLabel->hide();
	boxWhiskersCoef->hide();
	whiskerCntLabel->hide();
	whiskerCnt->hide();

	if (index == BoxCurve::UserDef)
	{
		whiskerCoeffLabel->show();
		boxWhiskersCoef->show();
	}
	else if (index == BoxCurve::SD || index == BoxCurve::SE)
	{
		whiskerCntLabel->show();
		whiskerCnt->show();
	}
}

void PlotDialog::customVectorsPage(bool angleMag)
{
	if (angleMag)
	{
		GroupBoxVectEnd->setTitle(tr("Vector Data"));
		labelXEnd->setText(tr("Angle"));
		labelYEnd->setText(tr("Magnitude"));
		labelPosition->show();
		vectPosBox->show();
	}
	else
	{
		GroupBoxVectEnd->setTitle(tr("End Point"));
		labelXEnd->setText(tr("X End"));
		labelYEnd->setText(tr("Y End"));
		labelPosition->hide();
		vectPosBox->hide();
	}
}

void PlotDialog::showColorMapEditor(bool)
{
  	if (grayScaleBox->isChecked() || defaultScaleBox->isChecked())
  		colorMapEditor->hide();
  	else
  	{
  	 	colorMapEditor->show();
  	    colorMapEditor->setFocus();
  	}
}

void PlotDialog::showDefaultContourLinesBox(bool)
{
  	if (autoContourBox->isChecked())
  		defaultPenBox->hide();
  	else
  		defaultPenBox->show();
}

void PlotDialog::showCustomPenColumn(bool on)
{
  	contourLinesEditor->showPenColumn(on);
  	if (on)
		defaultPenBox->hide();
}

void PlotDialog::updateTreeWidgetItem(QTreeWidgetItem *item)
{
    if (item->type() != QTreeWidgetItem::Type)
        return;

    if (item->isExpanded())
		item->setIcon(0, QIcon(":/folder_open.png"));
    else
		item->setIcon(0, QIcon(":/folder_closed.png"));
}

void PlotDialog::updateBackgroundTransparency(int alpha)
{
	boxBackgroundColor->setEnabled(alpha);
	applyLayerFormat();
}

void PlotDialog::updateCanvasTransparency(int alpha)
{
    boxCanvasColor->setEnabled(alpha);
	applyLayerFormat();
}

void PlotDialog::setTitlesFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,titleFont,this);
	if ( ok )
		titleFont = font;
}

void PlotDialog::setAxesLabelsFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,axesFont,this);
	if ( ok )
		axesFont = font;
}

void PlotDialog::setAxesNumbersFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,numbersFont,this);
	if ( ok )
		numbersFont = font;
}

void PlotDialog::setLegendsFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, legendFont,this);
	if ( ok )
		legendFont = font;
}

void PlotDialog::initFonts(const QFont& titlefont, const QFont& axesfont, const QFont& numbersfont, const QFont& legendfont)
{
	axesFont = axesfont;
	titleFont = titlefont;
	numbersFont = numbersfont;
	legendFont = legendfont;
}

void PlotDialog::adjustLayerHeight(double width)
{
	if (keepRatioBox->isChecked()){
		boxLayerHeight->blockSignals(true);
		boxLayerHeight->setValue(width/aspect_ratio);
		boxLayerHeight->blockSignals(false);
	} else
		aspect_ratio = width/boxLayerHeight->value();
}

void PlotDialog::adjustLayerWidth(double height)
{
	if (keepRatioBox->isChecked()){
		boxLayerWidth->blockSignals(true);
		boxLayerWidth->setValue(height*aspect_ratio);
		boxLayerWidth->blockSignals(false);
	} else
		aspect_ratio = boxLayerWidth->value()/height;
}

void PlotDialog::adjustPlotHeight(double width)
{
	if (keepPlotRatioBox->isChecked()){
		boxPlotHeight->blockSignals(true);
		boxPlotHeight->setValue(width/plot_aspect_ratio);
		boxPlotHeight->blockSignals(false);
	} else
		plot_aspect_ratio = width/boxPlotHeight->value();
}

void PlotDialog::adjustPlotWidth(double height)
{
	if (keepPlotRatioBox->isChecked()){
		boxPlotWidth->blockSignals(true);
		boxPlotWidth->setValue(height*plot_aspect_ratio);
		boxPlotWidth->blockSignals(false);
	} else
		plot_aspect_ratio = boxPlotWidth->value()/height;
}

void PlotDialog::closeEvent(QCloseEvent* e)
{
	ApplicationWindow *app = qobject_cast<ApplicationWindow *>(this->parent());
	if (app){
		app->d_extended_plot_dialog = btnMore->isChecked ();
		app->d_keep_aspect_ration = keepRatioBox->isChecked() || keepPlotRatioBox->isChecked();
	}

	e->accept();
}

void PlotDialog::chooseLabelsFont()
{
    QTreeWidgetItem *item = listBox->currentItem();
    if (!item || item->type() != CurveTreeItem::PlotCurveTreeItem)
        return;

    const QwtPlotItem *i = ((CurveTreeItem *)item)->plotItem();
    Graph *graph = ((CurveTreeItem *)item)->graph();
    if (!i || !graph)
        return;

	QFont font = QFont();
	bool spectrogram = false;
	if (i->rtti() == QwtPlotItem::Rtti_PlotSpectrogram){
		spectrogram = true;
		font = ((Spectrogram *)i)->labelsFont();
	} else
		font = ((DataCurve *)i)->labelsFont();

    bool okF;
	QFont fnt = QFontDialog::getFont(&okF, font, this);
	if (okF && fnt != font){
		if (spectrogram)
			((Spectrogram *)i)->setLabelsFont(fnt);
		else
			((DataCurve *)i)->setLabelsFont(fnt);

        graph->replot();
        graph->notifyChanges();
	}
}

int PlotDialog::labelsAlignment()
{
	int align = -1;
	switch (boxLabelsAlign->currentIndex())
	{
		case 0:
			align = Qt::AlignHCenter;
			break;

		case 1:
			align = Qt::AlignLeft;
			break;

		case 2:
			align = Qt::AlignRight;
			break;
	}
	return align;
}

void PlotDialog::displayPlotCoordinates(int unit)
{
	if (unit == FrameWidget::Pixel || unit == FrameWidget::Point){
		boxPlotX->setFormat('f', 0);
		boxPlotY->setFormat('f', 0);
		boxPlotWidth->setFormat('f', 0);
		boxPlotHeight->setFormat('f', 0);

		boxPlotX->setSingleStep(1.0);
		boxPlotY->setSingleStep(1.0);
		boxPlotWidth->setSingleStep(1.0);
		boxPlotHeight->setSingleStep(1.0);
	} else {
		boxPlotX->setFormat('g', 6);
		boxPlotY->setFormat('g', 6);
		boxPlotWidth->setFormat('g', 6);
		boxPlotHeight->setFormat('g', 6);

		boxPlotX->setSingleStep(0.1);
		boxPlotY->setSingleStep(0.1);
		boxPlotWidth->setSingleStep(0.1);
		boxPlotHeight->setSingleStep(0.1);
	}

	plot_aspect_ratio = (double)d_ml->width()/(double)d_ml->height();

	boxPlotX->setValue(FrameWidget::xIn(d_ml, (FrameWidget::Unit)unit) + FrameWidget::xIn(d_ml, (FrameWidget::Unit)unit));
	boxPlotY->setValue(FrameWidget::yIn(d_ml, (FrameWidget::Unit)unit) + FrameWidget::yIn(d_ml, (FrameWidget::Unit)unit));
	boxPlotWidth->setValue(FrameWidget::widthIn(d_ml, (FrameWidget::Unit)unit));
	boxPlotHeight->setValue(FrameWidget::heightIn(d_ml, (FrameWidget::Unit)unit));
}

void PlotDialog::displayCoordinates(int unit, Graph *g)
{
	if (!g){
		QTreeWidgetItem *item = listBox->currentItem();
    	if (item){
        	g = ((LayerItem *)item)->graph();
			if (!g)
				return;
		}
	}

	if (unit == FrameWidget::Pixel || unit == FrameWidget::Point){
		boxX->setFormat('f', 0);
		boxY->setFormat('f', 0);
		boxLayerWidth->setFormat('f', 0);
		boxLayerHeight->setFormat('f', 0);

		boxX->setSingleStep(1.0);
		boxY->setSingleStep(1.0);
		boxLayerWidth->setSingleStep(1.0);
		boxLayerHeight->setSingleStep(1.0);
	} else {
		boxX->setFormat('g', 6);
		boxY->setFormat('g', 6);
		boxLayerWidth->setFormat('g', 6);
		boxLayerHeight->setFormat('g', 6);

		boxX->setSingleStep(0.1);
		boxY->setSingleStep(0.1);
		boxLayerWidth->setSingleStep(0.1);
		boxLayerHeight->setSingleStep(0.1);
	}

	QwtPlotCanvas *canvas = g->canvas();
	aspect_ratio = (double)canvas->width()/(double)canvas->height();

	boxX->setValue(FrameWidget::xIn(canvas, (FrameWidget::Unit)unit) + FrameWidget::xIn(g, (FrameWidget::Unit)unit));
	boxY->setValue(FrameWidget::yIn(canvas, (FrameWidget::Unit)unit) + FrameWidget::yIn(g, (FrameWidget::Unit)unit));
	boxLayerWidth->setValue(FrameWidget::widthIn(canvas, (FrameWidget::Unit)unit));
	boxLayerHeight->setValue(FrameWidget::heightIn(canvas, (FrameWidget::Unit)unit));
}

void PlotDialog::setLayerDefaultValues()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
		return;

    app->d_graph_background_color = boxBackgroundColor->color();
	app->d_graph_background_opacity = boxBackgroundTransparency->value();
	app->d_graph_canvas_color = boxCanvasColor->color();
	app->d_graph_canvas_opacity = boxCanvasTransparency->value();
	app->d_graph_border_color = boxBorderColor->color();
	app->d_graph_border_width = boxBorderWidth->value();
	app->defaultPlotMargin = boxMargin->value();
	app->antialiasing2DPlots = boxAntialiasing->isChecked();

	app->saveSettings();
}

void PlotDialog::showAllLabelControls(bool show)
{
	if (show){
		boxLabelsColumn->show();
		boxLabelsAlign->show();
		justifyLabelsLbl->show();
		labelsColumnLbl->show();
	} else {
		boxLabelsColumn->hide();
		boxLabelsAlign->hide();
		justifyLabelsLbl->hide();
		labelsColumnLbl->hide();
	}
}

void PlotDialog::setEquidistantLevels()
{
	QTreeWidgetItem *it = listBox->currentItem();
    if (!it)
        return;

	CurveTreeItem *item = (CurveTreeItem *)it;
    QwtPlotItem *plotItem = (QwtPlotItem *)item->plotItem();
    if (!plotItem)
        return;

	Spectrogram *sp = (Spectrogram *)plotItem;
	if (!sp || sp->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
		return;

	QwtValueList levels;
	double firstVal = firstContourLineBox->value();
	for (int i = 0; i < levelsBox->value(); i++)
		levels << firstVal + i*contourLinesDistanceBox->value();
	sp->setContourLevels(levels);
	sp->plot()->replot();
	contourLinesEditor->updateContents();
}

void PlotDialog::applyLineFormatToLayer(Graph *g)
{
	if (!g)
		return;

	QList<QwtPlotItem *> lst = g->curvesList();
	int i = -1;
	foreach (QwtPlotItem *it, lst){
		i++;
		if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
			continue;

		PlotCurve *c = (PlotCurve *)it;
		if (c->type() == Graph::ErrorBars)
			continue;

		QPen pen = QPen(c->pen().color(), boxLineWidth->value(),
					boxLineStyle->style(), Qt::SquareCap, Qt::MiterJoin);
		pen.setCosmetic(true);
		c->setPen(pen);

		g->setCurveStyle(i, boxConnect->currentIndex());
	}
	g->replot();
}

void PlotDialog::applyLineFormat(QwtPlotCurve *c)
{
    if (!c || privateTabWidget->currentPage() != linePage)
		return;

	Graph *layer = (Graph *)c->plot();
    ApplicationWindow *app = (ApplicationWindow *)this->parent();
	switch(lineFormatApplyToBox->currentIndex()){
		case 0://selected curve
		break;
		case 1://this layer
			applyLineFormatToLayer(layer);
		break;
		case 2://this window
		{
			QList<Graph *> layersLst = layer->multiLayer()->layersList();
			foreach(Graph *g, layersLst)
				applyLineFormatToLayer(g);
		}
		break;
		case 3://all windows
		{
			QList<MdiSubWindow *> windows = app->windowsList();
			foreach(MdiSubWindow *w, windows){
				MultiLayer *ml = qobject_cast<MultiLayer *>(w);
				if (!ml)
					continue;

				QList<Graph *> layersLst = ml->layersList();
				foreach(Graph *g, layersLst)
					applyLineFormatToLayer(g);
			}
		}
		break;
		default:
			break;
	}
	app->modifiedProject();
}

void PlotDialog::applySymbolsFormatToCurve(QwtPlotCurve *c, bool fillColor, bool penColor)
{
	if (!c)
		return;

	QwtSymbol symbol = c->symbol();
	if (symbol.style() == QwtSymbol::NoSymbol)
		return;

	int size = 2*boxSymbolSize->value() + 1;

	QBrush br = symbol.brush();
	if (fillColor)
		br = QBrush(boxFillColor->color(), Qt::SolidPattern);
	if (!boxFillSymbol->isChecked())
		br = QBrush();

	QPen pen = QPen(symbol.pen().color(), boxPenWidth->value(), Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	if (penColor)
		pen.setColor(boxSymbolColor->color());
	pen.setCosmetic(true);

	QwtSymbol s = QwtSymbol(boxSymbolStyle->selectedSymbol(), br, pen, QSize(size, size));
	c->setSymbol(s);

	((PlotCurve *)c)->setSkipSymbolsCount(boxSkipSymbols->value());
}

void PlotDialog::applySymbolsFormatToLayer(Graph *g)
{
	if (!g)
		return;

	QList<QwtPlotItem *> lst = g->curvesList();
	foreach (QwtPlotItem *it, lst){
		if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
			continue;

		QwtPlotCurve *c = (QwtPlotCurve *)it;
		if (c->symbol().style() != QwtSymbol::NoSymbol)
			applySymbolsFormatToCurve(c, false, false);
	}
	g->replot();
}

void PlotDialog::applySymbolsFormat(QwtPlotCurve *c)
{
    if (!c || privateTabWidget->currentPage() != symbolPage)
		return;

	Graph *layer = (Graph *)c->plot();
    ApplicationWindow *app = (ApplicationWindow *)this->parent();
	switch(symbolsFormatApplyToBox->currentIndex()){
		case 0://selected curve
			applySymbolsFormatToCurve(c);
		break;
		case 1://this layer
			applySymbolsFormatToLayer(layer);
		break;
		case 2://this window
		{
			QList<Graph *> layersLst = layer->multiLayer()->layersList();
			foreach(Graph *g, layersLst)
				applySymbolsFormatToLayer(g);
		}
		break;
		case 3://all windows
		{
			QList<MdiSubWindow *> windows = app->windowsList();
			foreach(MdiSubWindow *w, windows){
				MultiLayer *ml = qobject_cast<MultiLayer *>(w);
				if (!ml)
					continue;

				QList<Graph *> layersLst = ml->layersList();
				foreach(Graph *g, layersLst)
					applySymbolsFormatToLayer(g);
			}
		}
		break;
		default:
			break;
	}
	app->modifiedProject();
}

void PlotDialog::applyErrorBarFormatToCurve(QwtErrorPlotCurve *err, bool color)
{
	if (!err)
		return;

	Graph *g = (Graph *)err->plot();
	if (!g)
		return;

	QColor col = err->color();
	if (color)
		col = colorBox->color();

	g->updateErrorBars(err, xBox->isChecked(), widthBox->value(),
			capBox->currentText().toInt(), col, plusBox->isChecked(), minusBox->isChecked(),
			throughBox->isChecked());
	err->setSkipSymbolsCount(boxSkipErrorBars->value());
}

void PlotDialog::applyErrorBarFormatToLayer(Graph *g)
{
	if (!g)
		return;

	QList<QwtPlotItem *> lst = g->curvesList();
	int i = -1;
	foreach (QwtPlotItem *it, lst){
		i++;
		if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
			continue;

		PlotCurve *c = (QwtErrorPlotCurve *)it;
		if (c->type() == Graph::ErrorBars)
			applyErrorBarFormatToCurve((QwtErrorPlotCurve *)it, false);
	}
	g->replot();
}

void PlotDialog::applyErrorBarFormat(QwtErrorPlotCurve *c)
{
	if (!c || privateTabWidget->currentPage() != errorsPage)
		return;

	Graph *layer = (Graph *)c->plot();
	if (!layer)
		return;

	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	switch(errorBarsFormatApplyToBox->currentIndex()){
		case 0://selected curve
			applyErrorBarFormatToCurve(c);
		break;
		case 1://this layer
			applyErrorBarFormatToLayer(layer);
		break;
		case 2://this window
		{
			QList<Graph *> layersLst = layer->multiLayer()->layersList();
			foreach(Graph *g, layersLst)
				applyErrorBarFormatToLayer(g);
		}
		break;
		case 3://all windows
		{
			QList<MdiSubWindow *> windows = app->windowsList();
			foreach(MdiSubWindow *w, windows){
				MultiLayer *ml = qobject_cast<MultiLayer *>(w);
				if (!ml)
					continue;

				QList<Graph *> layersLst = ml->layersList();
				foreach(Graph *g, layersLst)
					applyErrorBarFormatToLayer(g);
			}
		}
		break;
		default:
			break;
	}
	app->modifiedProject();
}

QRect PlotDialog::layerCanvasRect(QWidget *widget, double x, double y, double w, double h, FrameWidget::Unit unit)
{
    if (!widget)
        return QRect(qRound(x), qRound(y), qRound(w), qRound(h));

	int dpiX = widget->logicalDpiX();
	int dpiY = widget->logicalDpiY();

    switch(unit){
		case FrameWidget::Pixel:
		default:
			return QRect(qRound(x), qRound(y), qRound(w), qRound(h));
		break;
		case FrameWidget::Inch:
			return QRect(qRound(x*dpiX), qRound(y*dpiY), qRound(w*dpiX), qRound(h*dpiY));
		break;
		case FrameWidget::Millimeter:
			return QRect(qRound(x*dpiX/25.4), qRound(y*dpiY/25.4), qRound(w*dpiX/25.4), qRound(h*dpiY/25.4));
		break;
		case FrameWidget::Centimeter:
			return QRect(qRound(x*dpiX/2.54), qRound(y*dpiY/2.54), qRound(w*dpiX/2.54), qRound(h*dpiY/2.54));
		break;
		case FrameWidget::Point:
			return QRect(qRound(x*dpiX/72.0), qRound(y*dpiY/72.0), qRound(w*dpiX/72.0), qRound(h*dpiY/72.0));
		break;
	}
	return QRect(qRound(x), qRound(y), qRound(w), qRound(h));
}

/*****************************************************************************
 *
 * Class LayerItem
 *
 *****************************************************************************/

LayerItem::LayerItem(Graph *g, QTreeWidgetItem *parent, const QString& s)
    : QTreeWidgetItem( parent, QStringList(s), LayerTreeItem ),
      d_graph(g)
{
	setIcon(0, QPixmap(":/layer_disabled.png"));
    if (g)
        insertCurvesList();
}

void LayerItem::setActive(bool on)
{
    if (on)
		setIcon(0, QPixmap(":/layer_enabled.png"));
    else
		setIcon(0, QPixmap(":/layer_disabled.png"));
}

void LayerItem::insertCurvesList()
{
	for (int i=0; i<d_graph->curveCount(); i++){
        QString plotAssociation = QString();
        QwtPlotItem *it = (QwtPlotItem *)d_graph->plotItem(i);
        if (!it)
            continue;

        if (it->rtti() == QwtPlotItem::Rtti_PlotCurve){
            PlotCurve *c = (PlotCurve *)it;
            if (c->type() != Graph::Function && ((DataCurve *)it)->table()){
                QString s = ((DataCurve *)it)->plotAssociation();
                QString table = ((DataCurve *)it)->table()->name();
                plotAssociation = table + ": " + s.remove(table + "_");
            } else
                plotAssociation = it->title().text();
        } else
            plotAssociation = it->title().text();

        addChild(new CurveTreeItem(it, this, plotAssociation));
	}
}

/*****************************************************************************
 *
 * Class CurveTreeItem
 *
 *****************************************************************************/

CurveTreeItem::CurveTreeItem(QwtPlotItem *curve, LayerItem *parent, const QString& s)
    : QTreeWidgetItem( parent, QStringList(s), PlotCurveTreeItem ),
      d_curve(curve)
{
	setIcon(0, QPixmap(":/graph_disabled.png"));
}

void CurveTreeItem::setActive(bool on)
{
    if (on)
		setIcon(0, QPixmap(":/graph_enabled.png"));
    else
		setIcon(0, QPixmap(":/graph_disabled.png"));
}

int CurveTreeItem::plotItemIndex()
{
	Graph *g = graph();
	if (!g)
    	return -1;

	QList<QwtPlotItem *> itemsList = g->curvesList();
	return itemsList.indexOf(d_curve);
}

int CurveTreeItem::plotItemStyle()
{
	if (d_curve->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
		return ((PlotCurve *)d_curve)->plotStyle();
	else
		return Graph::ColorMap;

	return -1;
}

int CurveTreeItem::plotItemType()
{
	if (d_curve->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
		return ((PlotCurve *)d_curve)->type();
	else
		return Graph::ColorMap;

	return -1;
}
