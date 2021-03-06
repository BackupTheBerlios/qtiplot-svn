/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#include <qpainter.h>
#include <qpixmap.h>
#include <qevent.h>
#include <qframe.h>
#include <qcursor.h>
#include "qwt_panner.h"

class QwtPanner::PrivateData
{
public:
    PrivateData():
        isEnabled(false),
        button(Qt::LeftButton),
        buttonState(Qt::NoButton),
        abortKey(Qt::Key_Escape),
        abortKeyState(Qt::NoButton)
    {
    }
        
    bool isEnabled;
    int button;
    int buttonState;
    int abortKey;
    int abortKeyState;

    QPoint initialPos;
    QPoint pos;

    QPixmap pixmap;
    QCursor cursor;
};

/*!
  Creates an panner that is enabled for the left mouse button.

  \param parent Parent widget to be panned
*/
QwtPanner::QwtPanner(QWidget *parent):
    QWidget(parent)
{
    d_data = new PrivateData();

    if ( parent )
        d_data->cursor = parent->cursor();

#if QT_VERSION >= 0x040000
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_PaintOnScreen);
    setFocusPolicy(Qt::NoFocus);
#else
    setBackgroundMode(Qt::NoBackground);
    setFocusPolicy(QWidget::NoFocus);
#endif
    hide();

    setEnabled(true);
}

//! Destructor
QwtPanner::~QwtPanner()
{
    delete d_data;
}

/*!
   Change the mouse button
   The defaults are Qt::LeftButton and Qt::NoButton
*/
void QwtPanner::setMouseButton(int button, int buttonState)
{
    d_data->button = button;
    d_data->buttonState = buttonState;
}

//! Get the mouse button
void QwtPanner::getMouseButton(int &button, int &buttonState) const
{
    button = d_data->button;
    buttonState = d_data->buttonState;
}

/*!
   Change the abort key
   The defaults are Qt::Key_Escape and Qt::NoButton
*/
void QwtPanner::setAbortKey(int key, int state)
{
    d_data->abortKey = key;
    d_data->abortKeyState = state;
}

//! Get the abort key
void QwtPanner::getAbortKey(int &key, int &state) const
{
    key = d_data->abortKey;
    state = d_data->abortKeyState;
}

/*!
   Change the cursor, that is active while panning
   The default is the cursor of the parent widget.

   \param cursor New cursor

   \sa setCursor()
*/
void QwtPanner::setCursor(const QCursor &cursor)
{
    d_data->cursor = cursor;
}

/*!
   \return Cursor that is active while panning
   \sa setCursor()
*/
const QCursor QwtPanner::cursor() const
{
    return d_data->cursor;
}

/*! 
  \brief En/disable the panner
 
  When enabled is true an event filter is installed for
  the observed widget, otherwise the event filter is removed.

  \param enabled true or false
  \sa isEnabled(), eventFilter()
*/
void QwtPanner::setEnabled(bool on)
{
    if ( d_data->isEnabled != on )
    {
        d_data->isEnabled = on;

        QWidget *w = parentWidget();
        if ( w )
        {
            if ( d_data->isEnabled )
            {
                w->installEventFilter(this);
            }
            else
            {
                w->removeEventFilter(this);
                hide();
            }
        }
    }
}

/*!
  \return true when enabled, false otherwise
  \sa setEnabled, eventFilter()
*/
bool QwtPanner::isEnabled() const
{
    return d_data->isEnabled;
}

/*!
   \brief Paint event

   Repaint the grabbed pixmap on its current position and
   fill the empty spaces by the background of the parent widget.

   \param pe Paint event
*/
void QwtPanner::paintEvent(QPaintEvent *pe)
{
    QPixmap pm(size());

    QPainter painter(&pm);

    const QColor bg =
#if QT_VERSION < 0x040000
        parentWidget()->palette().color(
            QPalette::Normal, QColorGroup::Background);
#else
        parentWidget()->palette().color(
            QPalette::Normal, QPalette::Background);
#endif

    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(bg));
    painter.drawRect(0, 0, pm.width(), pm.height());

    int dx = d_data->pos.x() - d_data->initialPos.x();
    int dy = d_data->pos.y() - d_data->initialPos.y();

    QRect r(0, 0, d_data->pixmap.width(), d_data->pixmap.height());
    r.moveCenter(QPoint(r.center().x() + dx, r.center().y() + dy));

    painter.drawPixmap(r, d_data->pixmap);
    painter.end();

    painter.begin(this);
    painter.setClipRegion(pe->region());
    painter.drawPixmap(0, 0, pm);
}

/*! 
  \brief Event filter

  When isEnabled() the mouse events of the observed widget are filtered.

  \sa widgetMousePressEvent(), widgetMouseReleaseEvent(),
      widgetMouseMoveEvent()
*/
bool QwtPanner::eventFilter(QObject *o, QEvent *e)
{
    if ( o == NULL || o != parentWidget() )
        return false;

    switch(e->type())
    {
        case QEvent::MouseButtonPress:
        {
            widgetMousePressEvent((QMouseEvent *)e);
            break;
        }
        case QEvent::MouseMove:
        {
            widgetMouseMoveEvent((QMouseEvent *)e);
            break;
        }
        case QEvent::MouseButtonRelease:
        {
            widgetMouseReleaseEvent((QMouseEvent *)e);
            break;
        }
        case QEvent::KeyPress:
        {
            widgetKeyPressEvent((QKeyEvent *)e);
            break;
        }
        case QEvent::KeyRelease:
        {
            widgetKeyReleaseEvent((QKeyEvent *)e);
            break;
        }
        default:;
    }

    return false;
}

/*!
  Handle a mouse press event for the observed widget.

  \sa eventFilter(), widgetMouseReleaseEvent(),
      widgetMouseMoveEvent(),
*/
void QwtPanner::widgetMousePressEvent(QMouseEvent *me)
{
    if ( me->button() != d_data->button )
        return;

#if QT_VERSION < 0x040000
    if ( (me->state() & Qt::KeyButtonMask) !=
        (d_data->buttonState & Qt::KeyButtonMask) )
#else
    if ( (me->modifiers() & Qt::KeyboardModifierMask) !=
        (int)(d_data->buttonState & Qt::KeyboardModifierMask) )
#endif
    {
        return;
    }

    parentWidget()->setCursor(d_data->cursor);

    d_data->initialPos = d_data->pos = me->pos();

    QRect cr = parentWidget()->rect();
    if ( parentWidget()->inherits("QFrame") )
    {
        const QFrame* frame = (QFrame*)parentWidget();
        cr = frame->contentsRect();
    }
    setGeometry(cr);
    d_data->pixmap = QPixmap::grabWidget(parentWidget(),
        cr.x(), cr.y(), cr.width(), cr.height());
    show();
}

/*!
  Handle a mouse release event for the observed widget.

  \sa eventFilter(), widgetMousePressEvent(),
      widgetMouseMoveEvent(),
*/
void QwtPanner::widgetMouseMoveEvent(QMouseEvent *me)
{
    if ( isVisible() && rect().contains(me->pos()) )
    {
        d_data->pos = me->pos();
        update();

        emit moved(d_data->pos.x() - d_data->initialPos.x(), 
            d_data->pos.y() - d_data->initialPos.y());
    }
}

/*!
  Handle a mouse move event for the observed widget.

  \sa eventFilter(), widgetMousePressEvent(), widgetMouseReleaseEvent(),
*/
void QwtPanner::widgetMouseReleaseEvent(QMouseEvent *me)
{
    if ( isVisible() )
    {
        hide();
        parentWidget()->unsetCursor();

        d_data->pixmap = QPixmap();
        d_data->pos = me->pos();

        if ( d_data->pos != d_data->initialPos )
        {
            emit panned(d_data->pos.x() - d_data->initialPos.x(), 
                d_data->pos.y() - d_data->initialPos.y());
        }
    }
}

void QwtPanner::widgetKeyPressEvent(QKeyEvent *ke)
{
    if ( ke->key() == d_data->abortKey )
    {
        const bool matched =
#if QT_VERSION < 0x040000
            (ke->state() & Qt::KeyButtonMask) ==
                (d_data->abortKeyState & Qt::KeyButtonMask);
#else
            (ke->modifiers() & Qt::KeyboardModifierMask) ==
                (int)(d_data->abortKeyState & Qt::KeyboardModifierMask);
#endif
        if ( matched )
        {
            hide();
            parentWidget()->unsetCursor();
            d_data->pixmap = QPixmap();
        }
    }
}

void QwtPanner::widgetKeyReleaseEvent(QKeyEvent *)
{
}
