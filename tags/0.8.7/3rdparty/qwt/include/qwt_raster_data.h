/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_RASTER_DATA_H
#define QWT_RASTER_DATA_H 1

#include <qmap.h>
#include "qwt_global.h"
#include "qwt_double_rect.h"
#include "qwt_double_interval.h"

#if QT_VERSION >= 0x040000
#include <qlist.h>
#include <QPolygonF>
template class QWT_EXPORT QMap<double, QPolygonF>;
#else
#include <qvaluelist.h>
#include "qwt_array.h"
#include "qwt_double_rect.h"
template class QWT_EXPORT QwtArray<QwtDoublePoint>;
template class QWT_EXPORT QMap<double, QwtArray<QwtDoublePoint> >;
#endif

class QwtScaleMap;

class QWT_EXPORT QwtRasterData
{
public:
#if QT_VERSION >= 0x040000
    typedef QMap<double, QPolygonF> ContourLines;
#else
    typedef QMap<double, QwtArray<QwtDoublePoint> > ContourLines;
#endif

    enum ConrecAttribute
    {
        IgnoreAllVerticesOnLevel = 1,
        IgnoreOutOfRange = 2
    };

    QwtRasterData();
    QwtRasterData(const QwtDoubleRect &);
    virtual ~QwtRasterData();

    //! Clone the data
    virtual QwtRasterData *copy() const = 0;

    virtual void setBoundingRect(const QwtDoubleRect &);
    QwtDoubleRect boundingRect() const;

    virtual QSize rasterHint(const QwtDoubleRect &) const;

    virtual void initRaster(const QwtDoubleRect &, const QSize& raster);
    virtual void discardRaster();

    //! \return the value at a raster position
    virtual double value(double x, double y) const = 0;

    //! \return the range of the values
    virtual QwtDoubleInterval range() const = 0;

#if QT_VERSION >= 0x040000
    virtual ContourLines contourLines(const QwtDoubleRect &rect,
        const QSize &raster, const QList<double> &levels, 
        int flags) const;
#else
    virtual ContourLines contourLines(const QwtDoubleRect &rect,
        const QSize &raster, const QValueList<double> &levels, 
        int flags) const;
#endif

    class Contour3DPoint;
    class ContourPlane;

private:
    QwtDoubleRect d_boundingRect;
};

#endif
