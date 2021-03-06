#include "layerDialog.h"
#include "multilayer.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qmessagebox.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qfont.h>
#include <qfontdialog.h>
#include <qcombobox.h>

layerDialog::layerDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "layerDialog" );
    setSizeGripEnabled( true );

	generalDialog = new QTabWidget( this, "generalDialog" );

	layout = new QWidget( generalDialog, "layout" );

	QButtonGroup *box = new QButtonGroup(2,QGroupBox::Horizontal,QString::null,layout);
	box->setFlat (true);
	box->setLineWidth (0);
	box->setInsideMargin (0);

	QVBox *vbox = new QVBox(box);
	vbox->setMargin(5);
	vbox->setSpacing(5);

	QHBox *hbox1 = new QHBox(vbox);
	hbox1->setSpacing(5);

	new QLabel( tr( "Number of Layers" ), hbox1);
	layersBox = new QSpinBox(0,100,1,hbox1);

	fitBox=new QCheckBox(vbox);
	fitBox->setChecked(FALSE);

	QButtonGroup *groupAlign = new QButtonGroup(2,QGroupBox::Horizontal,tr("Alignement"),box);

    new QLabel( tr("Horizontal" ), groupAlign, 0,0 );
	alignHorBox = new QComboBox( FALSE, groupAlign, 0 );
	alignHorBox->insertItem( tr( "Center" ) );
	alignHorBox->insertItem( tr( "Left" ) );
	alignHorBox->insertItem( tr( "Right" ) );

    new QLabel( tr( "Vertical" ), groupAlign, 0,0 );
	alignVertBox = new QComboBox( FALSE, groupAlign, 0 );
	alignVertBox->insertItem( tr( "Center" ) );
	alignVertBox->insertItem( tr( "Top" ) );
	alignVertBox->insertItem( tr( "Bottom" ) );

	QVBox *vbox2 = new QVBox(box);
	vbox2->setSpacing(5);

	GroupBox1 = new QButtonGroup( 2,QGroupBox::Horizontal,tr("Grid"),vbox2);

    new QLabel( tr( "Columns" ), GroupBox1, "TextLabel1",0 );
	boxX = new QSpinBox(1,100,1,GroupBox1, "boxX" );

    new QLabel( tr( "Rows" ), GroupBox1, "TextLabel2",0 );
	boxY = new QSpinBox(1,100,1,GroupBox1, "boxY" );

	GroupCanvasSize = new QButtonGroup(2,QGroupBox::Horizontal,tr("&Layer Canvas Size"),vbox2);
	GroupCanvasSize->setCheckable(true);
	GroupCanvasSize->setChecked(false);

    new QLabel( tr("Width" ), GroupCanvasSize, "TextLabel1",0 );
	boxCanvasWidth = new QSpinBox(0,10000,50,GroupCanvasSize, "boxCanvasWidth" );
	boxCanvasWidth->setSuffix(tr(" pixels"));

    new QLabel( tr( "Height" ), GroupCanvasSize, "TextLabel2",0 );
	boxCanvasHeight = new QSpinBox(0,10000,50,GroupCanvasSize, "boxCanvasHeight" );
	boxCanvasHeight->setSuffix(tr(" pixels"));

	GroupBox4 = new QButtonGroup( 2,QGroupBox::Horizontal,tr("Spacing"),box);

  	new QLabel( tr( "Columns gap" ), GroupBox4, "TextLabel4",0 );
	boxColsGap = new QSpinBox(0,100,5,GroupBox4, "boxColsGap" );
	boxColsGap->setSuffix(tr(" pixels"));

    new QLabel( tr( "Rows gap" ), GroupBox4, "TextLabel5",0 );
	boxRowsGap = new QSpinBox(0,100,5,GroupBox4, "boxRowsGap" );
	boxRowsGap->setSuffix(tr(" pixels"));

	new QLabel( tr( "Left margin" ), GroupBox4, "TextLabel7",0 );
	boxLeftSpace = new QSpinBox(0,1000,5,GroupBox4, "boxLeftSpace" );
	boxLeftSpace->setSuffix(tr(" pixels"));

	new QLabel( tr( "Right margin" ), GroupBox4, "TextLabel6",0 );
	boxRightSpace = new QSpinBox(0,1000,5,GroupBox4, "boxRightSpace" );
	boxRightSpace->setSuffix(tr(" pixels"));

	new QLabel( tr( "Top margin" ), GroupBox4, "TextLabel8",0 );
	boxTopSpace = new QSpinBox(0,1000,5,GroupBox4, "boxTopSpace" );
	boxTopSpace->setSuffix(tr(" pixels"));

    new QLabel( tr( "Bottom margin" ), GroupBox4, "TextLabel9",0 );
	boxBottomSpace = new QSpinBox(0,1000,5,GroupBox4, "boxBottomSpace" );
	boxBottomSpace->setSuffix(tr(" pixels"));

	QVBoxLayout* hlayout1 = new QVBoxLayout(layout,5,5, "hlayout1");
	hlayout1->addWidget(box);

	generalDialog->insertTab(layout, tr( "Layout" ) );

	fonts = new QWidget( generalDialog, "fonts" );
	GroupBox2 = new QButtonGroup( 1,QGroupBox::Horizontal, QString::null,fonts,"GroupBox2" );

    btnTitle = new QPushButton(GroupBox2, "btnTitle" );
	btnAxisLegend = new QPushButton(GroupBox2, "btnAxisLegend" );
	btnAxisNumbers = new QPushButton(GroupBox2, "btnAxisNumbers" );
	btnLegend = new QPushButton(GroupBox2, "btnLegend" );

	QVBoxLayout* vl2 = new QVBoxLayout(fonts,5,5, "vl2");
	vl2->addWidget(GroupBox2);
 
	generalDialog->insertTab(fonts, tr( "Fonts" ) );

	QHBox *hbox2=new QHBox(this, "hbox2");
	hbox2->setSpacing(5);

	buttonApply = new QPushButton(hbox2, "buttonApply" );

	buttonOk = new QPushButton(hbox2, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );

    buttonCancel = new QPushButton(hbox2, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );

	QVBoxLayout* vl = new QVBoxLayout(this,10, 5, "vl");
	vl->addWidget(generalDialog);
    vl->addWidget(hbox2);

    languageChange();

    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonApply, SIGNAL( clicked() ), this, SLOT(update() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
	connect( btnTitle, SIGNAL( clicked() ), this, SLOT( setTitlesFont() ) );
	connect( btnAxisLegend, SIGNAL( clicked() ), this, SLOT( setAxisLegendFont() ) );
	connect( btnAxisNumbers, SIGNAL( clicked() ), this, SLOT( setAxisNumbersFont() ) );
    connect( btnLegend, SIGNAL( clicked() ), this, SLOT( setLegendsFont() ) );
	connect( fitBox, SIGNAL( toggled(bool) ), this, SLOT(enableLayoutOptions(bool) ) );
}

void layerDialog::enableLayoutOptions(bool ok)
{
GroupBox1->setEnabled(!ok);
GroupCanvasSize->setEnabled(!ok);
}

layerDialog::~layerDialog()
{
}

void layerDialog::languageChange()
{
    setCaption( tr( "QtiPlot - Arrange Layers" ) );
	buttonApply->setText( tr( "&Apply" ) );
    buttonOk->setText( tr( "&OK" ) );
	buttonCancel->setText( tr( "&Cancel" ) );
	fitBox->setText(tr("Automatic &layout"));

	btnTitle->setText(tr("Titles"));
	btnAxisLegend->setText(tr("Axis Legends"));
	btnAxisNumbers->setText(tr("Axis Numbers"));
	btnLegend->setText(tr("Legends"));
}

void layerDialog::setMultiLayer(MultiLayer *g)
{
multi_layer = g;

layersBox->setValue(g->graphsNumber());
boxX->setValue(g->getCols());
boxY->setValue(g->getRows());
boxColsGap->setValue(g->colsSpacing());
boxRowsGap->setValue(g->rowsSpacing());
boxLeftSpace->setValue(g->leftMargin());
boxRightSpace->setValue(g->rightMargin());
boxTopSpace->setValue(g->topMargin());
boxBottomSpace->setValue(g->bottomMargin());
boxCanvasWidth->setValue(g->layerCanvasSize().width());
boxCanvasHeight->setValue(g->layerCanvasSize().height());
alignHorBox->setCurrentItem(g->horizontalAlignement());
alignVertBox->setCurrentItem(g->verticalAlignement());
}

void layerDialog::update()
{
if (generalDialog->currentPage()==(QWidget *)layout )
	{
	int dn = multi_layer->graphsNumber() - layersBox->value();
	if (dn > 0 && QMessageBox::question(0, tr("QtiPlot - Delete Layers?"),
				tr("You are about to delete %1 existing layers.").arg(dn)+"\n"+
                tr("Are you sure you want to continue this operation?"),
				tr("&Continue"), tr("&Cancel"), QString::null, 0, 1 )) return; 

	int graphs = layersBox->value();
	multi_layer->setLayersNumber(graphs);

	if (!graphs)
		return;

	int cols=boxX->value();
	int rows=boxY->value();
		
	if (cols>graphs && !fitBox->isChecked())
		{
		QMessageBox::about(0,tr("QtiPlot - Columns input error"),
		tr("The number of columns you've entered is greater than the number of graphs (%1)!").arg(graphs));
		boxX->setFocus();
		return;
		}

	if (rows>graphs && !fitBox->isChecked())
		{
		QMessageBox::about(0,tr("QtiPlot - Rows input error"),
		tr("The number of rows you've entered is greater than the number of graphs (%1)!").arg(graphs));
		boxY->setFocus();
		return;
		}

	if (!fitBox->isChecked())
		{
		multi_layer->setCols(cols);
		multi_layer->setRows(rows);
		}

	if (GroupCanvasSize->isChecked())
		multi_layer->setLayerCanvasSize(boxCanvasWidth->value(), boxCanvasHeight->value());

	multi_layer->setAlignement(alignHorBox->currentItem(), alignVertBox->currentItem());

	multi_layer->setMargins(boxLeftSpace->value(), boxRightSpace->value(),
							boxTopSpace->value(), boxBottomSpace->value());
	
	multi_layer->setSpacing(boxColsGap->value(), boxRowsGap->value());
	multi_layer->arrangeLayers(fitBox->isChecked(), GroupCanvasSize->isChecked());
	
	if (!GroupCanvasSize->isChecked())
		{//show new layer canvas size
		boxCanvasWidth->setValue(multi_layer->layerCanvasSize().width());
		boxCanvasHeight->setValue(multi_layer->layerCanvasSize().height());
		}

	if (fitBox->isChecked())
		{//show new grid settings
		boxX->setValue(multi_layer->getCols());
		boxY->setValue(multi_layer->getRows());
		}
	}

if (generalDialog->currentPage()==(QWidget *)fonts)
	multi_layer->setFonts(titleFont, axesFont, numbersFont, legendFont);
}

void layerDialog::accept()
{
update();
close();
}

void layerDialog::initFonts(const QFont& titlefont, const QFont& axesfont, const QFont& numbersfont, const QFont& legendfont)
{
    axesFont = axesfont;
    titleFont = titlefont;
    numbersFont = numbersfont;
    legendFont = legendfont;
}

void layerDialog::setTitlesFont()
{
bool ok;
QFont font = QFontDialog::getFont(&ok,titleFont,this);
    if ( ok ) {
        titleFont = font;
    } else {
     return;
    }
}

void layerDialog::setAxisLegendFont()
{
bool ok;
QFont font = QFontDialog::getFont(&ok,axesFont,this);
    if ( ok ) {
        axesFont = font;
    } else {
     return;
    }
}

void layerDialog::setAxisNumbersFont()
{
bool ok;
QFont font = QFontDialog::getFont(&ok,numbersFont,this);
    if ( ok ) {
        numbersFont = font;
    } else {
     return;
    }
}

void layerDialog::setLegendsFont()
{
bool ok;
QFont font = QFontDialog::getFont(&ok, legendFont,this);
    if ( ok ) {
        legendFont = font;
    } else {
     return;
    }
}
