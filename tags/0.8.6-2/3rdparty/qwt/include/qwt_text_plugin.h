/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2003   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_TEXT_PLUGIN_H
#define QWT_TEXT_PLUGIN_H 1

#include "qwt_global.h"

#if QT_VERSION >= 0x040000

#include <qplugin.h>

class QwtTextEngine;

class QWT_EXPORT QwtTextPlugin
{
public:
    virtual ~QwtTextPlugin() {}

    virtual int format() const = 0;
    virtual QwtTextEngine *engine() const = 0;
};

Q_DECLARE_INTERFACE(QwtTextPlugin, "Qwt.TextPlugin/1.0");

#endif

#endif
