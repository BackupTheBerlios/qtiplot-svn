#include "scalePicker.h"

#include <qapplication.h>

#include <qwt_plot.h>
#include <qwt_scale_widget.h>
#include <qwt_text_label.h>

ScalePicker::ScalePicker(QwtPlot *plot):
    QObject(plot)
{
	movedGraph=FALSE;
	
    for ( uint i = 0; i < QwtPlot::axisCnt; i++ )
		{
        QwtScaleWidget *scale = (QwtScaleWidget *)plot->axisWidget(i);
        if ( scale )
            scale->installEventFilter(this);
		}
}

bool ScalePicker::eventFilter(QObject *object, QEvent *e)
{  
	if (!object->inherits("QwtScaleWidget"))
		return QObject::eventFilter(object, e);

	if ( e->type() == QEvent::MouseButtonDblClick )
    	{
		mouseDblClicked((const QwtScaleWidget *)object, ((QMouseEvent *)e)->pos());
        return TRUE;
    	}

	if ( e->type() == QEvent::MouseButtonPress)
    	{
		const QMouseEvent *me = (const QMouseEvent *)e;	
		if (me->button()==QEvent::LeftButton)
			{
			presspos = me->pos();
			((QwtScaleWidget *)object)->setFocus();
			emit clicked();	

			if (plot()->margin() < 2 && plot()->lineWidth() < 2)
				{
				QRect r = ((const QwtScaleWidget *)object)->rect();
				//r.addCoords(2, 2, -2, -2);
				if (!r.contains(me->pos()))
					emit highlightGraph();
				}
			return TRUE;
			}
		else if (me->button() == QEvent::RightButton)
			{
			mouseRightClicked((const QwtScaleWidget *)object, me->pos());
			return TRUE;
			}
    	}
	
	if ( e->type() == QEvent::MouseMove)
    	{	
		const QMouseEvent *me = (const QMouseEvent *)e;			

		if ((presspos - me->pos()).manhattanLength() > QApplication::startDragDistance())
			{
			movedGraph=TRUE;
			emit moveGraph(me->pos());
			}

        return TRUE;
   	 }
	
	if ( e->type() == QEvent::MouseButtonRelease)
    	{
		if (movedGraph)
			{
			emit releasedGraph();
			movedGraph=FALSE;
			}
				
        return TRUE;
    	}
		
return QObject::eventFilter(object, e);
}

void ScalePicker::mouseDblClicked(const QwtScaleWidget *scale, const QPoint &pos) 
{
QRect rect = scaleRect(scale);

int margin = 1; // pixels tolerance
rect.setRect(rect.x() - margin, rect.y() - margin, rect.width() + 2 * margin, rect.height() +  2 * margin);

if ( rect.contains(pos) ) 
	emit axisDblClicked(scale->alignment());
else
	{// Click on the title
    switch(scale->alignment())   
        {
        case QwtScaleDraw::LeftScale:
            {
			emit yAxisTitleDblClicked();
            break;
            }
        case QwtScaleDraw::RightScale:
            {
			emit rightAxisTitleDblClicked();
            break;
            }
        case QwtScaleDraw::BottomScale:
            {
			emit xAxisTitleDblClicked();
            break;
            }
        case QwtScaleDraw::TopScale:
            {
			emit topAxisTitleDblClicked();
            break;
            }
		}
	}
}

void ScalePicker::mouseRightClicked(const QwtScaleWidget *scale, const QPoint &pos) 
{
if (scaleRect(scale).contains(pos)) 
	emit axisRightClicked(scale->alignment());
else
	emit axisTitleRightClicked(scale->alignment());
}

// The rect of a scale without the title
QRect ScalePicker::scaleRect(const QwtScaleWidget *scale) const
{
QRect rect = scale->rect();
if (scale->title().text().isEmpty())
	return rect;

int dh = scale->title().textSize().height();
switch(scale->alignment())   
    {
    case QwtScaleDraw::LeftScale:
        {			
		rect.setLeft(rect.left() + dh);	
        break;
        }
    case QwtScaleDraw::RightScale:
        {
		rect.setRight(rect.right() - dh);
        break;
        }
    case QwtScaleDraw::BottomScale:
        {
		rect.setBottom(rect.bottom() - dh);
	    break;
        }
    case QwtScaleDraw::TopScale:
        {
		rect.setTop(rect.top() + dh);
        break;
        }
    }
return rect;
}

void ScalePicker::refresh()
{	
    for ( uint i = 0; i < QwtPlot::axisCnt; i++ )
    {
        QwtScaleWidget *scale = (QwtScaleWidget *)plot()->axisWidget(i);
        if ( scale )
            scale->installEventFilter(this);
    }
}

TitlePicker::TitlePicker(QwtPlot *plot):
    QObject(plot)
{
movedGraph=FALSE;

title = (QwtTextLabel *)plot->titleLabel();
title->setFocusPolicy(QWidget::StrongFocus);
if (title)
	title->installEventFilter(this);
}

bool TitlePicker::eventFilter(QObject *object, QEvent *e)
{
	if (object != (QObject *)title)
		return FALSE;
	
    if ( object->inherits("QwtTextLabel") && e->type() == QEvent::MouseButtonDblClick)
		{
		emit doubleClicked();
        return TRUE;
		}

	if ( object->inherits("QwtTextLabel") &&  e->type() == QEvent::MouseButtonPress )
		{
		const QMouseEvent *me = (const QMouseEvent *)e;	
		presspos = me->pos();
		emit clicked();

		if (me->button()==QEvent::RightButton)
			emit showTitleMenu();

		QwtPlot *plot = (QwtPlot *)title->parent();
		if (plot->margin() < 2 && plot->lineWidth() < 2)
			{
			QRect r = title->rect();
			r.addCoords(2, 2, -2, -2);
			if (!r.contains(me->pos()))
				emit highlightGraph();
			}

		return TRUE;
		}

	if ( object->inherits("QwtTextLabel") &&  e->type() == QEvent::MouseMove)
		{	
		const QMouseEvent *me = (const QMouseEvent *)e;		
		if ((presspos - me->pos()).manhattanLength() > QApplication::startDragDistance())
			{			
			movedGraph=TRUE;
			emit moveGraph(me->pos());
			}
        return TRUE;
		}
	
	if ( object->inherits("QwtTextLabel") && e->type() == QEvent::MouseButtonRelease)
		{
		const QMouseEvent *me = (const QMouseEvent *)e;
		if (me->button()== QEvent::LeftButton)
			{
			emit clicked();
			if (movedGraph)
				{
				emit releasedGraph();
				movedGraph=FALSE;
				}
        	return TRUE;
			}
		}

	if ( object->inherits("QwtTextLabel") && 
        e->type() == QEvent::KeyPress)
		{
		switch (((const QKeyEvent *)e)->key()) 
			{
			case Qt::Key_Delete: 
			emit removeTitle();	
            return TRUE;
			}
		}

    return QObject::eventFilter(object, e);
}
