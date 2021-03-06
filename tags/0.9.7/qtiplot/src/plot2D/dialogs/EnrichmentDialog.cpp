/***************************************************************************
    File                 : EnrichmentDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A general properties dialog for the FrameWidget, using article
						  "Using a Simple Web Service with Qt" in Qt Quaterly, Issue 23, Q3 2007

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

#include <QtGui>
#include <QHttp>
#include <QIODevice>

#include "EnrichmentDialog.h"
#include "../Graph.h"
#include "../TexWidget.h"
#include "../FrameWidget.h"
#include "../ImageWidget.h"
#include "../RectangleWidget.h"
#include "../LegendWidget.h"
#include "../../ColorButton.h"
#include "../../ApplicationWindow.h"
#include "../../DoubleSpinBox.h"
#include "../../PatternBox.h"
#include "../../PenStyleBox.h"
#include "../../TextFormatButtons.h"

static const char* choose_folder_xpm[]={
    "16 16 11 1",
    "# c #000000",
    "g c #c0c0c0",
    "e c #303030",
    "a c #ffa858",
    "b c #808080",
    "d c #a0a0a4",
    "f c #585858",
    "c c #ffdca8",
    "h c #dcdcdc",
    "i c #ffffff",
    ". c None",
    "....###.........",
    "....#ab##.......",
    "....#acab####...",
    "###.#acccccca#..",
    "#ddefaaaccccca#.",
    "#bdddbaaaacccab#",
    ".eddddbbaaaacab#",
    ".#bddggdbbaaaab#",
    "..edgdggggbbaab#",
    "..#bgggghghdaab#",
    "...ebhggghicfab#",
    "....#edhhiiidab#",
    "......#egiiicfb#",
    "........#egiibb#",
    "..........#egib#",
    "............#ee#"};

EnrichmentDialog::EnrichmentDialog(WidgetType wt, Graph *g, QWidget *parent)
    : QDialog(parent), d_plot(g), d_widget(NULL), d_widget_type(wt)
{
	setSizeGripEnabled( true );
	setAttribute(Qt::WA_DeleteOnClose);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
	clearButton = NULL;
	editPage = NULL;
	imagePage = NULL;
	patternPage = NULL;
	textPage = NULL;

	if (wt == Tex){
		setWindowTitle(tr("QtiPlot") + " - " + tr("Tex Equation Editor"));

    	clearButton = buttonBox->addButton(tr("Clea&r"), QDialogButtonBox::ResetRole);
		connect(clearButton, SIGNAL(clicked()), this, SLOT(clearForm()));
	} else if (wt == MDIWindow)
        setWindowTitle(tr("QtiPlot") + " - " + tr("Window Geometry"));
	else
		setWindowTitle(tr("QtiPlot") + " - " + tr("Object Properties"));

    updateButton = buttonBox->addButton(tr("&Apply"), QDialogButtonBox::ApplyRole);
	connect(updateButton, SIGNAL(clicked()), this, SLOT(apply()));

	cancelButton = buttonBox->addButton(tr("&Close"), QDialogButtonBox::RejectRole);
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	tabWidget = new QTabWidget();
	if (wt == Text)
		initTextPage();
	else if (wt == Tex)
		initEditorPage();
	else if (wt == Image)
		initImagePage();
	else if (wt == Frame)
		initPatternPage();

    if (wt != MDIWindow)
        initFramePage();
	initGeometryPage();

    QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(tabWidget);
    layout->addWidget(buttonBox);
    setLayout(layout);

	connect(tabWidget, SIGNAL(currentChanged (QWidget *)), this, SLOT(customButtons(QWidget *)));
}

void EnrichmentDialog::initEditorPage()
{
	http = new QHttp(this);
    connect(http, SIGNAL(done(bool)), this, SLOT(updateForm(bool)));

	editPage = new QWidget();

    equationEditor = new QTextEdit;
	outputLabel = new QLabel;
    outputLabel->setFrameShape(QFrame::StyledPanel);

	QVBoxLayout *layout = new QVBoxLayout(editPage);
    layout->addWidget(equationEditor, 1);
	layout->addWidget(new QLabel(tr("Preview:")));
	layout->addWidget(outputLabel);

	tabWidget->addTab(editPage, tr( "&Text" ) );
}

void EnrichmentDialog::initTextPage()
{
	QGroupBox *gb1 = new QGroupBox();
	QGridLayout * gl1 = new QGridLayout(gb1);
	gl1->addWidget(new QLabel(tr("Color")), 0, 0);

	textColorBtn = new ColorButton();
	connect(textColorBtn, SIGNAL(colorChanged()), this, SLOT(textFormatApplyTo()));
	gl1->addWidget(textColorBtn, 0, 1);

	textFontBtn = new QPushButton(tr( "&Font" ));
	connect(textFontBtn, SIGNAL(clicked()), this, SLOT(customFont()));
	gl1->addWidget(textFontBtn, 0, 2);

    gl1->addWidget(new QLabel(tr("Background")), 1, 0);
	textBackgroundBtn = new ColorButton();
	connect(textBackgroundBtn, SIGNAL(colorChanged()), this, SLOT(textFormatApplyTo()));
	gl1->addWidget(textBackgroundBtn, 1, 1);

	gl1->addWidget(new QLabel(tr("Opacity")), 2, 0);

	boxBackgroundTransparency = new QSpinBox();
	boxBackgroundTransparency->setRange(0, 255);
    boxBackgroundTransparency->setSingleStep(5);
	boxBackgroundTransparency->setWrapping(true);
    boxBackgroundTransparency->setSpecialValueText(tr("Transparent"));
	connect(boxBackgroundTransparency, SIGNAL(valueChanged(int)),
			this, SLOT(updateTransparency(int)));
	gl1->addWidget(boxBackgroundTransparency, 2, 1);

    gl1->addWidget(new QLabel(tr("Rotate (deg.)")), 3, 0);

    boxTextAngle = new QSpinBox();
    boxTextAngle->setRange(-360, 360);
    boxTextAngle->setSingleStep(45);
    boxTextAngle->setWrapping(true);
    connect(boxTextAngle, SIGNAL(valueChanged(int)), this, SLOT(textFormatApplyTo()));
    gl1->addWidget(boxTextAngle, 3, 1);

    autoUpdateTextBox = new QCheckBox(tr("Auto-&update"));
	gl1->addWidget(autoUpdateTextBox, 1, 2);

	gl1->setColumnStretch(4, 1);

    QVBoxLayout *vl = new QVBoxLayout();
    textDefaultBtn = new QPushButton( tr( "Set As &Default" ) );
    connect(textDefaultBtn, SIGNAL(clicked()), this, SLOT(setTextDefaultValues()));
	vl->addWidget(textDefaultBtn);

    textApplyToBtn = new QPushButton(tr("Apply &to..."));
	connect(textApplyToBtn, SIGNAL(clicked()), this, SLOT(textFormatApplyTo()));
	vl->addWidget(textApplyToBtn);

	textApplyToBox = new QComboBox();
	textApplyToBox->insertItem(tr("Object"));
	textApplyToBox->insertItem(tr("Layer"));
    textApplyToBox->insertItem(tr("Window"));
    textApplyToBox->insertItem(tr("All Windows"));
	vl->addWidget(textApplyToBox);
	vl->addStretch();

    QHBoxLayout *hl = new QHBoxLayout();
	hl->addWidget(gb1);
	hl->addLayout(vl);

	textEditBox = new QTextEdit();
	textEditBox->setTextFormat(Qt::PlainText);
	textEditBox->setFont(QFont());

	formatButtons =  new TextFormatButtons(textEditBox);
	formatButtons->toggleCurveButton(true);

	setFocusPolicy(Qt::StrongFocus);
	setFocusProxy(textEditBox);

	textPage = new QWidget();

	QVBoxLayout* ml = new QVBoxLayout(textPage);
	ml->addLayout(hl);
	ml->addWidget(formatButtons);
	ml->addWidget(textEditBox, 1);

	tabWidget->addTab(textPage, tr( "&Text" ) );
}

void EnrichmentDialog::initImagePage()
{
	imagePage = new QWidget();

    QGroupBox *gb = new QGroupBox();
	QGridLayout *gl = new QGridLayout(gb);
    gl->addWidget(new QLabel( tr("File")), 0, 0);

	imagePathBox = new QLineEdit();
	gl->addWidget(imagePathBox, 0, 1);

	QPushButton *browseBtn = new QPushButton();
	connect(browseBtn, SIGNAL(clicked()), this, SLOT(chooseImageFile()));
	browseBtn->setIcon(QIcon(QPixmap(choose_folder_xpm)));
	gl->addWidget(browseBtn, 0, 2);

	boxSaveImagesInternally = new QCheckBox(tr("&Save internally"));
	connect(boxSaveImagesInternally, SIGNAL(toggled(bool)), this, SLOT(saveImagesInternally(bool)));

	gl->addWidget(boxSaveImagesInternally, 1, 1);
	gl->setColumnStretch(1, 1);
	gl->setRowStretch(2, 1);

	QVBoxLayout *layout = new QVBoxLayout(imagePage);
    layout->addWidget(gb);
	tabWidget->addTab(imagePage, tr( "&Image" ) );
}

void EnrichmentDialog::initFramePage()
{
    framePage = new QWidget();

	QGroupBox *gb = new QGroupBox();
	QGridLayout *gl = new QGridLayout(gb);
    gl->addWidget(new QLabel( tr("Shape")), 0, 0);

	frameBox = new QComboBox();
	frameBox->addItem( tr( "None" ) );
	frameBox->addItem( tr( "Rectangle" ) );
	frameBox->addItem( tr( "Shadow" ) );
	connect(frameBox, SIGNAL(activated(int)), this, SLOT(frameApplyTo()));
    gl->addWidget(frameBox, 0, 1);

    gl->addWidget(new QLabel(tr("Color")), 1, 0);
	frameColorBtn = new ColorButton();
	connect(frameColorBtn, SIGNAL(colorChanged()), this, SLOT(frameApplyTo()));
    gl->addWidget(frameColorBtn, 1, 1);

	gl->addWidget(new QLabel(tr( "Line Style" )), 2, 0);
	boxFrameLineStyle = new PenStyleBox();
	connect(boxFrameLineStyle, SIGNAL(activated(int)), this, SLOT(frameApplyTo()));
	gl->addWidget(boxFrameLineStyle, 2, 1);

	gl->setColumnStretch(1, 1);
	gl->addWidget(new QLabel(tr("Width")), 3, 0);
	boxFrameWidth = new QSpinBox();
	boxFrameWidth->setRange(1, 100);
	connect(boxFrameWidth, SIGNAL(valueChanged(int)), this, SLOT(frameApplyTo()));
	gl->addWidget(boxFrameWidth, 3, 1);
	gl->setRowStretch(4, 1);

	QVBoxLayout *vl = new QVBoxLayout();

	frameDefaultBtn = new QPushButton(tr("Set As &Default"));
	connect(frameDefaultBtn, SIGNAL(clicked()), this, SLOT(setFrameDefaultValues()));
	vl->addWidget(frameDefaultBtn);

	QLabel *l = new QLabel(tr("Apply t&o..."));
	vl->addWidget(l);

	frameApplyToBox = new QComboBox();
	frameApplyToBox->insertItem(tr("Object"));
	frameApplyToBox->insertItem(tr("Layer"));
    frameApplyToBox->insertItem(tr("Window"));
    frameApplyToBox->insertItem(tr("All Windows"));
	vl->addWidget(frameApplyToBox);
	vl->addStretch();
	l->setBuddy(frameApplyToBox);

	QHBoxLayout *hl = new QHBoxLayout(framePage);
	hl->addWidget(gb);
	hl->addLayout(vl);

	tabWidget->addTab(framePage, tr( "&Frame" ) );
}

void EnrichmentDialog::initPatternPage()
{
	patternPage = new QWidget();

	QGroupBox *gb = new QGroupBox();
	QGridLayout *gl = new QGridLayout(gb);
    gl->addWidget(new QLabel( tr("Fill Color")), 0, 0);

	backgroundColorBtn = new ColorButton();
	connect(backgroundColorBtn, SIGNAL(colorChanged()), this, SLOT(patternApplyTo()));
    gl->addWidget(backgroundColorBtn, 0, 1);

	gl->addWidget(new QLabel(tr("Opacity")), 1, 0);
	boxTransparency = new QSpinBox();
	boxTransparency->setRange(0, 255);
    boxTransparency->setSingleStep(5);
	boxTransparency->setWrapping(true);
    boxTransparency->setSpecialValueText(tr("Transparent"));
	connect(boxTransparency, SIGNAL(valueChanged(int)), this, SLOT(patternApplyTo()));
	gl->addWidget(boxTransparency, 1, 1);

	gl->addWidget(new QLabel(tr("Pattern")), 2, 0);
	patternBox = new PatternBox();
	connect(patternBox, SIGNAL(activated(int)), this, SLOT(patternApplyTo()));
	gl->addWidget(patternBox, 2, 1);

	gl->addWidget(new QLabel(tr("Pattern Color")), 3, 0);
	patternColorBtn = new ColorButton();
	connect(patternColorBtn, SIGNAL(colorChanged()), this, SLOT(patternApplyTo()));
	gl->addWidget(patternColorBtn, 3, 1);

	useFrameColorBox = new QCheckBox(tr("Use &Frame Color"));
	connect(useFrameColorBox, SIGNAL(toggled(bool)), this, SLOT(patternApplyTo()));
	connect(useFrameColorBox, SIGNAL(toggled(bool)), patternColorBtn, SLOT(setDisabled(bool)));
	gl->addWidget(useFrameColorBox, 3, 2);

	gl->setColumnStretch(1, 1);
	gl->setRowStretch(4, 1);

	QVBoxLayout *vl = new QVBoxLayout();
	rectangleDefaultBtn = new QPushButton(tr("Set As &Default"));
	connect(rectangleDefaultBtn, SIGNAL(clicked()), this, SLOT(setRectangleDefaultValues()));
	vl->addWidget(rectangleDefaultBtn);

	QLabel *l = new QLabel(tr("Apply t&o..."));
	vl->addWidget(l);

	patternApplyToBox = new QComboBox();
	patternApplyToBox->insertItem(tr("Object"));
	patternApplyToBox->insertItem(tr("Layer"));
    patternApplyToBox->insertItem(tr("Window"));
    patternApplyToBox->insertItem(tr("All Windows"));
	vl->addWidget(patternApplyToBox);
	vl->addStretch();
	l->setBuddy(patternApplyToBox);

	QHBoxLayout *hl = new QHBoxLayout(patternPage);
	hl->addWidget(gb);
	hl->addLayout(vl);

	tabWidget->addTab(patternPage, tr("Fill &Pattern"));
}

void EnrichmentDialog::initGeometryPage()
{
    geometryPage = new QWidget();

	unitBox = new QComboBox();
	unitBox->insertItem(tr("inch"));
	unitBox->insertItem(tr("mm"));
	unitBox->insertItem(tr("cm"));
	unitBox->insertItem(tr("point"));
	unitBox->insertItem(tr("pixel"));
	if (d_widget_type != MDIWindow)
        unitBox->insertItem(tr("scale"));

	QBoxLayout *bl1 = new QBoxLayout (QBoxLayout::LeftToRight);
	bl1->addWidget(new QLabel(tr( "Unit" )));
	bl1->addWidget(unitBox);

	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale locale = QLocale();
	if (app)
		locale = app->locale();

    QGroupBox *gb1 = new QGroupBox(tr("Position"));
	xBox = new DoubleSpinBox();
	xBox->setLocale(locale);
	xBox->setDecimals(6);
	xBox->setMinimumWidth(80);
	yBox = new DoubleSpinBox();
	yBox->setLocale(locale);
	yBox->setDecimals(6);

    QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(new QLabel( tr("X")), 0, 0);
    gl1->addWidget(xBox, 0, 1);
    gl1->addWidget(new QLabel(tr("Y")), 1, 0);
    gl1->addWidget(yBox, 1, 1);
	gl1->setColumnStretch(1, 10);
	gl1->setRowStretch(2, 1);
    gb1->setLayout(gl1);

    QGroupBox *gb2 = new QGroupBox(tr("Size"));
    widthBox = new DoubleSpinBox();
	widthBox->setLocale(locale);
	widthBox->setDecimals(6);
	heightBox = new DoubleSpinBox();
	heightBox->setLocale(locale);
	heightBox->setDecimals(6);

    QGridLayout *gl2 = new QGridLayout();
    gl2->addWidget(new QLabel( tr("Width")), 0, 0);
    gl2->addWidget(widthBox, 0, 1);

    gl2->addWidget(new QLabel(tr("Height")), 1, 0);
    gl2->addWidget(heightBox, 1, 1);

	keepAspectBox = new QCheckBox(tr("&Keep aspect ratio"));
	gl2->addWidget(keepAspectBox, 2, 1);

	bestSizeButton = new QPushButton(tr("&Best size"));
	bestSizeButton->hide();
	gl2->addWidget(bestSizeButton, 3, 1);

	gl2->setColumnStretch(1, 10);
	gl2->setRowStretch(4, 1);
    gb2->setLayout(gl2);

    QBoxLayout *bl2 = new QBoxLayout (QBoxLayout::LeftToRight);
	bl2->addWidget(gb1);
	bl2->addWidget(gb2);

	if (d_widget_type == Text)
		gb2->setEnabled(false);

	QVBoxLayout* vl = new QVBoxLayout(geometryPage);
    vl->addLayout(bl1);
    vl->addLayout(bl2);

	connect(unitBox, SIGNAL(activated(int)), this, SLOT(displayCoordinates(int)));
	connect(widthBox, SIGNAL(valueChanged(double)), this, SLOT(adjustHeight(double)));
	connect(heightBox, SIGNAL(valueChanged(double)), this, SLOT(adjustWidth(double)));
	connect(bestSizeButton, SIGNAL(clicked()), this, SLOT(setBestSize()));

	tabWidget->addTab(geometryPage, tr( "&Geometry" ) );
}

void EnrichmentDialog::customButtons(QWidget *w)
{
	if (d_widget_type == Tex && editPage && w == editPage && clearButton){
		clearButton->show();
		return;
	} else if (clearButton)
		clearButton->hide();

	if (w == framePage)
		updateButton->setEnabled(true);
}

void EnrichmentDialog::setWidget(QWidget *w)
{
	if (!w)
		return;

	d_widget = w;

    FrameWidget *fw = qobject_cast<FrameWidget *>(d_widget);
    if (fw){
		frameBox->blockSignals(true);
        frameBox->setCurrentIndex(fw->frameStyle());
		frameBox->blockSignals(false);
		
		frameColorBtn->blockSignals(true);
        frameColorBtn->setColor(fw->frameColor());
		frameColorBtn->blockSignals(false);
		
		boxFrameLineStyle->blockSignals(true);
		boxFrameLineStyle->setStyle(fw->framePen().style());
		boxFrameLineStyle->blockSignals(false);
		
		boxFrameWidth->blockSignals(true);
		boxFrameWidth->setValue(fw->framePen().width());
		boxFrameWidth->blockSignals(false);
    }

    unitBox->setCurrentIndex(FrameWidget::Pixel);
	displayCoordinates(FrameWidget::Pixel);

	if (d_widget_type == Text){
		LegendWidget *l = qobject_cast<LegendWidget *>(d_widget);
		if (l){
			setText(l->text());
			textFont = l->font();
			textColorBtn->blockSignals(true);
			textColorBtn->setColor(l->textColor());
			textColorBtn->blockSignals(false);

			QColor bc = l->backgroundColor();
			boxBackgroundTransparency->blockSignals(true);
			boxBackgroundTransparency->setValue(bc.alpha());
			boxBackgroundTransparency->blockSignals(false);
			
			textBackgroundBtn->blockSignals(true);
			textBackgroundBtn->setEnabled(bc.alpha());
			textBackgroundBtn->setColor(bc);
			textBackgroundBtn->blockSignals(false);

			boxTextAngle->blockSignals(true);
            boxTextAngle->setValue(l->angle());
			boxTextAngle->blockSignals(false);
			autoUpdateTextBox->setChecked(l->isAutoUpdateEnabled());
		}
	} else if (d_widget_type == Tex){
		TexWidget *tw = qobject_cast<TexWidget *>(d_widget);
		if (tw){
			equationEditor->setText(tw->formula());
			outputLabel->setPixmap(tw->pixmap());
			bestSizeButton->show();
		}
		return;
	} else if (d_widget_type == Image){
		ImageWidget *i = qobject_cast<ImageWidget *>(d_widget);
		if (i){
			imagePathBox->setText(i->fileName());
			boxSaveImagesInternally->blockSignals(true);
			boxSaveImagesInternally->setChecked(i->saveInternally());
			boxSaveImagesInternally->blockSignals(false);
		}
	} else if (d_widget_type == Frame){
		RectangleWidget *r = qobject_cast<RectangleWidget *>(d_widget);
		if (r){
			backgroundColorBtn->blockSignals(true);
			backgroundColorBtn->setColor(r->backgroundColor());
			backgroundColorBtn->blockSignals(false);
			
			boxTransparency->blockSignals(true);
			boxTransparency->setValue(r->backgroundColor().alpha());
			boxTransparency->blockSignals(false);
			
			patternBox->blockSignals(true);
			patternBox->setPattern(r->brush().style());
			patternBox->blockSignals(false);
			
			patternColorBtn->blockSignals(true);
			patternColorBtn->setColor(r->brush().color());
			patternColorBtn->blockSignals(false);
		}
	}
}

void EnrichmentDialog::clearForm()
{
    outputLabel->setPixmap(QPixmap());
    equationEditor->clear();
}

void EnrichmentDialog::apply()
{
	if (tabWidget->currentPage() == editPage)
		fetchImage();
	else if (tabWidget->currentPage() == framePage){
	    FrameWidget *fw = qobject_cast<FrameWidget *>(d_widget);
        if (fw){
			setFrameTo(fw);
			d_plot->multiLayer()->notifyChanges();
		}
	} else if (imagePage && tabWidget->currentPage() == imagePage)
		chooseImageFile(imagePathBox->text());
	else if (tabWidget->currentPage() == geometryPage)
		setCoordinates(unitBox->currentIndex());
	else if (patternPage && tabWidget->currentPage() == patternPage){
		RectangleWidget *r = qobject_cast<RectangleWidget *>(d_widget);
        if (r){
			setPatternTo(r);
			d_plot->multiLayer()->notifyChanges();
		}
	} else if (textPage && tabWidget->currentPage() == textPage){
		LegendWidget *l = qobject_cast<LegendWidget *>(d_widget);
        if (l){
            l->setText(textEditBox->text());
			setTextFormatTo(l);
			d_plot->multiLayer()->notifyChanges();
		}
	}
}

void EnrichmentDialog::fetchImage()
{
	TexWidget *tw = qobject_cast<TexWidget *>(d_widget);
	if (tw && tw->formula() == equationEditor->toPlainText() && !tw->pixmap().isNull())
		return;

	clearButton->setEnabled(false);
    updateButton->setEnabled(false);
    equationEditor->setReadOnly(true);

    QUrl url;
    url.setPath("/cgi-bin/mathtran");
    url.setQueryDelimiters('=', ';');
    url.addQueryItem("D", "3");
    url.addQueryItem("tex", QUrl::toPercentEncoding(
                     equationEditor->toPlainText()));

    http->setHost("mathtran.org");
    http->get(url.toString());

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void EnrichmentDialog::updateForm(bool error)
{
	QApplication::restoreOverrideCursor();

    if (!error) {
        QImage image;
        if (image.loadFromData(http->readAll())) {
            QPixmap pixmap = QPixmap::fromImage(image);
            outputLabel->setPixmap(pixmap);
			TexWidget *tw = qobject_cast<TexWidget *>(d_widget);
			if (tw){
				tw->setPixmap(pixmap);
				tw->setFormula(equationEditor->toPlainText());
				d_plot->multiLayer()->notifyChanges();
			}
        }
    } else {
		QMessageBox::critical((QWidget *)parent(), tr("QtiPlot") + " - " + tr("Network connection error"),
		tr("Error while trying to connect to host %1:").arg("mathtran.org") + "\n\n'" +
		http->errorString() + "'\n\n" + tr("Please verify your network connection!"));
	}

    clearButton->setEnabled(true);
    updateButton->setEnabled(true);
    equationEditor->setReadOnly(false);
}

void EnrichmentDialog::chooseImageFile(const QString& fn)
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	QString path = fn;
	if (path.isEmpty())
		path = QFileDialog::getOpenFileName(this, tr("QtiPlot - Import image from file"), app->imagesDirPath);

	if (!path.isEmpty()){
		ImageWidget *i = qobject_cast<ImageWidget *>(d_widget);
		if (i && i->load(path)){
			imagePathBox->setText(path);
			QFileInfo fi(path);
			app->imagesDirPath = fi.dirPath(true);
			app->modifiedProject();
		}
	}
}

void EnrichmentDialog::saveImagesInternally(bool save)
{
	ImageWidget *i = qobject_cast<ImageWidget *>(d_widget);
	if (i)
		i->setSaveInternally(boxSaveImagesInternally->isChecked());

	d_plot->multiLayer()->notifyChanges();

	if (save)
		return;

	QString fn = imagePathBox->text();
	if (fn.isEmpty() || !QFile::exists(fn)){
		QMessageBox::warning((ApplicationWindow *)parentWidget(), tr("QtiPlot - Warning"),
		tr("The file %1 doesn't exist. The image cannot be restored when reloading the project file!").arg(fn));
		chooseImageFile();
	}
}

void EnrichmentDialog::setCoordinates(int unit)
{
	if (!d_widget)
		return;

	if (unit == FrameWidget::Scale){//ScaleCoordinates
		double left = xBox->value();
		double top = yBox->value();
		FrameWidget *fw = qobject_cast<FrameWidget *>(d_widget);
        if (fw)
            fw->setCoordinates(left, top, left + widthBox->value(), top - heightBox->value());
	} else
		FrameWidget::setRect(d_widget, xBox->value(), yBox->value(),
        widthBox->value(), heightBox->value(), (FrameWidget::Unit)unit);

    if (d_plot)
        d_plot->multiLayer()->notifyChanges();
}

void EnrichmentDialog::displayCoordinates(int unit)
{
	if (!d_widget)
		return;

	if (unit == FrameWidget::Pixel || unit == FrameWidget::Point){
		xBox->setFormat('f', 0);
		yBox->setFormat('f', 0);
		widthBox->setFormat('f', 0);
		heightBox->setFormat('f', 0);

		xBox->setSingleStep(1.0);
		yBox->setSingleStep(1.0);
		widthBox->setSingleStep(1.0);
		heightBox->setSingleStep(1.0);
	} else {
		xBox->setFormat('g', 6);
		yBox->setFormat('g', 6);
		widthBox->setFormat('g', 6);
		heightBox->setFormat('g', 6);

		xBox->setSingleStep(0.1);
		yBox->setSingleStep(0.1);
		widthBox->setSingleStep(0.1);
		heightBox->setSingleStep(0.1);
	}

	xBox->setValue(FrameWidget::xIn(d_widget, (FrameWidget::Unit)unit));
	yBox->setValue(FrameWidget::yIn(d_widget, (FrameWidget::Unit)unit));
	widthBox->setValue(FrameWidget::widthIn(d_widget, (FrameWidget::Unit)unit));
	heightBox->setValue(FrameWidget::heightIn(d_widget, (FrameWidget::Unit)unit));

	aspect_ratio = widthBox->value()/heightBox->value();
}

void EnrichmentDialog::adjustHeight(double width)
{
	if (keepAspectBox->isChecked()){
		heightBox->blockSignals(true);
		heightBox->setValue(width/aspect_ratio);
		heightBox->blockSignals(false);
	} else
		aspect_ratio = width/heightBox->value();
}

void EnrichmentDialog::adjustWidth(double height)
{
	if (keepAspectBox->isChecked()){
		widthBox->blockSignals(true);
		widthBox->setValue(height*aspect_ratio);
		widthBox->blockSignals(false);
	} else
		aspect_ratio = widthBox->value()/height;
}

void EnrichmentDialog::setBestSize()
{
	TexWidget *tw = qobject_cast<TexWidget *>(d_widget);
	if (tw){
		tw->setBestSize();
		displayCoordinates(unitBox->currentIndex());
		d_plot->multiLayer()->notifyChanges();
	}
}

void EnrichmentDialog::frameApplyTo()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	switch(frameApplyToBox->currentIndex()){
		case 0://this layer
		{
			FrameWidget *fw = qobject_cast<FrameWidget *>(d_widget);
			if (fw)
				setFrameTo(fw);
		}
		break;
		
		case 1://this layer
		{
			QList <FrameWidget *> lst = d_plot->enrichmentsList();
			foreach(FrameWidget *fw, lst)
				setFrameTo(fw);
		}
		break;

		case 2://this window
		{
			QList<Graph *> layersLst = d_plot->multiLayer()->layersList();
			foreach(Graph *g, layersLst){
				QList <FrameWidget *> lst = g->enrichmentsList();
				foreach(FrameWidget *fw, lst)
					setFrameTo(fw);
			}
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
				foreach(Graph *g, layersLst){
					QList <FrameWidget *> lst = g->enrichmentsList();
					foreach(FrameWidget *fw, lst)
						setFrameTo(fw);
				}
			}
		}
		break;

		default:
			break;
	}
	app->modifiedProject();
}

void EnrichmentDialog::setFrameTo(FrameWidget *fw)
{
	fw->setFrameStyle(frameBox->currentIndex());	
	QPen pen = QPen(frameColorBtn->color(), boxFrameWidth->value(),
				boxFrameLineStyle->style(), Qt::SquareCap, Qt::MiterJoin);
	fw->setFramePen(pen);
	fw->repaint();
}

void EnrichmentDialog::patternApplyTo()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	switch(patternApplyToBox->currentIndex()){
		case 0://this object
		{
			RectangleWidget *r = qobject_cast<RectangleWidget *>(d_widget);
			if (r)
				setPatternTo(r);
		}
		break;
		
		case 1://this layer
		{
			QList <FrameWidget *> lst = d_plot->enrichmentsList();
			foreach(FrameWidget *fw, lst){
				RectangleWidget *r = qobject_cast<RectangleWidget *>(fw);
				if (r)
					setPatternTo(r);
			}
		}
		break;

		case 2://this window
		{
			QList<Graph *> layersLst = d_plot->multiLayer()->layersList();
			foreach(Graph *g, layersLst){
				QList <FrameWidget *> lst = g->enrichmentsList();
				foreach(FrameWidget *fw, lst){
					RectangleWidget *r = qobject_cast<RectangleWidget *>(fw);
					if (r)
						setPatternTo(r);
				}
			}
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
				foreach(Graph *g, layersLst){
					QList <FrameWidget *> lst = g->enrichmentsList();
					foreach(FrameWidget *fw, lst){
						RectangleWidget *r = qobject_cast<RectangleWidget *>(fw);
						if (r)
							setPatternTo(r);
					}
				}
			}
		}
		break;

		default:
			break;
	}
	app->modifiedProject();
}

void EnrichmentDialog::setPatternTo(RectangleWidget *r)
{
	QColor c = backgroundColorBtn->color();
	c.setAlpha(boxTransparency->value());
	r->setBackgroundColor(c);

	QColor patternColor = patternColorBtn->color();
	if (useFrameColorBox->isChecked())
		patternColor = frameColorBtn->color();
	r->setBrush(QBrush(patternColor, patternBox->getSelectedPattern()));

	r->repaint();
}

void EnrichmentDialog::setText(const QString & t)
{
	QTextCursor cursor = textEditBox->textCursor();
	// select the whole (old) text
	cursor.movePosition(QTextCursor::Start);
	cursor.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
	// replace old text
	cursor.insertText(t);
	// select the whole (new) text
	cursor.movePosition(QTextCursor::Start);
	cursor.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
	// this line makes the selection visible to the user
	// (the 2 lines above only change the selection in the
	// underlying QTextDocument)
	textEditBox->setTextCursor(cursor);
	// give focus back to text edit
	textEditBox->setFocus();
}

void EnrichmentDialog::customFont()
{
	bool okF;
	QFont fnt = QFontDialog::getFont( &okF, textFont, this);
	if (okF && fnt != textFont)
		textFont = fnt;
}

void EnrichmentDialog::updateTransparency(int alpha)
{
	textBackgroundBtn->setEnabled(alpha);
	textFormatApplyTo();
}

void EnrichmentDialog::textFormatApplyTo()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	switch(textApplyToBox->currentIndex()){
		case 0://this object
		{
			LegendWidget *l = qobject_cast<LegendWidget *>(d_widget);
			if (l)
				setTextFormatTo(l);
		}
		break;
		
		case 1://this layer
		{
			QList <FrameWidget *> lst = d_plot->enrichmentsList();
			foreach(FrameWidget *fw, lst){
				LegendWidget *l = qobject_cast<LegendWidget *>(fw);
				if (l)
					setTextFormatTo(l);
			}
		}
		break;

		case 2://this window
		{
			QList<Graph *> layersLst = d_plot->multiLayer()->layersList();
			foreach(Graph *g, layersLst){
				QList <FrameWidget *> lst = g->enrichmentsList();
				foreach(FrameWidget *fw, lst){
					LegendWidget *l = qobject_cast<LegendWidget *>(fw);
                    if (l)
                        setTextFormatTo(l);
				}
			}
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
				foreach(Graph *g, layersLst){
					QList <FrameWidget *> lst = g->enrichmentsList();
					foreach(FrameWidget *fw, lst){
						LegendWidget *l = qobject_cast<LegendWidget *>(fw);
                        if (l)
                            setTextFormatTo(l);
					}
				}
			}
		}
		break;

		default:
			break;
	}
	app->modifiedProject();
}

void EnrichmentDialog::setTextFormatTo(LegendWidget *l)
{
    QColor c = textBackgroundBtn->color();
    c.setAlpha(boxBackgroundTransparency->value());
    l->setBackgroundColor(c);
    l->setTextColor(textColorBtn->color());
    l->setFont(textFont);
    l->setAngle(boxTextAngle->value());
    l->setAutoUpdate(autoUpdateTextBox->isChecked());
    l->repaint();
}

void EnrichmentDialog::setTextDefaultValues()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
		return;

	app->legendTextColor = textColorBtn->color();
	app->plotLegendFont = textFont;

	QColor c = textBackgroundBtn->color();
	c.setAlpha(boxBackgroundTransparency->value());
	app->legendBackground = c;
	app->d_legend_default_angle = boxTextAngle->value();
	app->saveSettings();
}

void EnrichmentDialog::setFrameDefaultValues()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
		return;

	app->legendFrameStyle = frameBox->currentIndex();
	app->d_frame_widget_pen = QPen(frameColorBtn->color(), boxFrameWidth->value(), boxFrameLineStyle->style());
	app->saveSettings();
}

void EnrichmentDialog::setRectangleDefaultValues()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
		return;

	QColor c = backgroundColorBtn->color();
	c.setAlpha(boxTransparency->value());
	app->d_rect_default_background = c;

	QColor patternColor = patternColorBtn->color();
	if (useFrameColorBox->isChecked())
		patternColor = frameColorBtn->color();

	app->d_rect_default_brush = QBrush(patternColor, patternBox->getSelectedPattern());
	app->saveSettings();
}

EnrichmentDialog::~EnrichmentDialog()
{
	QApplication::restoreOverrideCursor();

	TexWidget *tw = qobject_cast<TexWidget *>(d_widget);
	if (tw && (tw->formula().isEmpty() || tw->pixmap().isNull())){
		d_plot->remove(tw);
		d_plot->setActiveTool(NULL);
		d_widget->close();
		d_widget = NULL;
	}
}
