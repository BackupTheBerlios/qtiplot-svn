#include "widget.h"
#include <qmessagebox.h>

myWidget::myWidget(const QString& label, QWidget * parent, const char * name, WFlags f):
		QWidget (parent, name, f)
{
w_label = label;
caption_policy = Both;
askOnClose = true;
w_status = Normal;
titleBar = NULL;
}

void myWidget::updateCaption()
{
switch (caption_policy)
	{
	case Name:
		setCaption(name());
	break;

	case Label:
		if (!w_label.isEmpty())
			setCaption(w_label);
		else
			setCaption(name());
	break;

	case Both:
		if (!w_label.isEmpty())
			setCaption(QString(name()) + " - " + w_label);
		else
			setCaption(name());
	break;
	}
};

void myWidget::closeEvent( QCloseEvent *e )
{
if (askOnClose)
    {
    switch( QMessageBox::information(0,tr("QtiPlot"),
			tr("Do you want to hide or delete") + "<p><b>'" + QString(name()) + "'</b> ?",
				      tr("Delete"), tr("Hide"), tr("Cancel"), 0,2)) 
		{
		case 0:
			emit closedWindow(this);
			e->accept();
		break;

		case 1:
			e->ignore();
			emit hiddenWindow(this);
		break;

		case 2:
			e->ignore();
		break;
		} 
    }
else 
    {
	emit closedWindow(this);
    e->accept();
    }
}

QString myWidget::aspect()
{
QString s = tr("Normal");
switch (w_status)
	{
	case Hidden:
		return tr("Hidden");
	break;

	case Normal:
	break;

	case Minimized:
		return tr("Minimized");
	break;

	case Maximized:
		return tr("Maximized");
	break;
	}
return s;
};

bool myWidget::event( QEvent *e )
{
if( e->type() == QEvent::ShowMinimized ) 
	w_status = Minimized;
else if ( e->type() == QEvent::ShowMaximized ) 
	w_status = Maximized;
else if ( e->type() == QEvent::ShowNormal)
	{
	user_request = true; 
	w_status = Normal; 
	}

emit statusChanged (this);
return QWidget::event( e );
}

void myWidget::setHidden()
{
w_status = Hidden; 
emit statusChanged (this);
hide();
}

void myWidget::setNormal()
{
w_status = Normal; 
emit statusChanged (this);
}

void myWidget::showMaximized()
{
user_request = this->isVisible();
QWidget::showMaximized();
}

QString myWidget::sizeToString()
{
return QString::number(8*sizeof(this)/1024.0, 'f', 1) + " " + tr("kB");
}

void myWidget::reparent(QWidget * parent, WFlags f, const QPoint & p, bool showIt)
{
titleBar = (QWidget*) parent->child("qt_ws_titlebar","QWidget",false);
if(titleBar) titleBar->installEventFilter(this);
QWidget::reparent(parent, f, p, showIt);
}

bool myWidget::eventFilter(QObject *object, QEvent *e)
{
if (e->type()==QEvent::ContextMenu && object == titleBar)
	{
	emit showTitleBarMenu();
	((QContextMenuEvent*)e)->accept();
	return true;
	}
return QObject::eventFilter(object, e);
}
