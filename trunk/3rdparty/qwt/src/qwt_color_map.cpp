/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_array.h"
#include "qwt_math.h"
#include "qwt_double_interval.h"
#include "qwt_color_map.h"

#if QT_VERSION < 0x040000
#include <qvaluelist.h>
typedef QValueVector<QRgb> QwtColorTable;
#else
typedef QVector<QRgb> QwtColorTable;
#endif

class QwtLinearColorMap::ColorStops
{
public:
    ColorStops()
    {
#if QT_VERSION >= 0x040000
        _stops.reserve(256);
#endif
    }

    void insert(double pos, const QColor &color);
    QRgb rgb(QwtLinearColorMap::Mode, double pos) const;

private:

    class ColorStop
    {
    public:
        ColorStop():
            pos(0.0),
            rgb(0)
        {
        };

        ColorStop(double p, const QColor &c):
            pos(p),
            rgb(c.rgb())
        {
            r = qRed(rgb);
            g = qGreen(rgb);
            b = qBlue(rgb);
        }

        double pos;
        QRgb rgb;
        int r, g, b;
    };

    inline int findUpper(double pos) const; 
    QwtArray<ColorStop> _stops;
};

void QwtLinearColorMap::ColorStops::insert(double pos, const QColor &color)
{
    // Lookups need to be very fast, insertions are not so important.
    // Anyway, a balanced tree is what we need here. TODO ...

    if ( pos < 0.0 || pos > 1.0 )
        return;

    int index;
    if ( _stops.size() == 0 )
    {
        index = 0;
#if QT_VERSION < 0x040000
        _stops.resize(1, QGArray::SpeedOptim);
#else
        _stops.resize(1);
#endif
    }
    else
    {
        index = findUpper(pos);
        if ( index == (int)_stops.size() || 
            qwtAbs(_stops[index].pos - pos) >= 0.001 )
        {
#if QT_VERSION < 0x040000
            _stops.resize(_stops.size() + 1, QGArray::SpeedOptim);
#else
            _stops.resize(_stops.size() + 1);
#endif
            for ( int i = _stops.size() - 1; i > index; i-- )
                _stops[i] = _stops[i-1];
        }
    }

    _stops[index] = ColorStop(pos, color);
}

inline int QwtLinearColorMap::ColorStops::findUpper(double pos) const
{
    int index = 0;
    int n = _stops.size();

    const ColorStop *stops = _stops.data();
    
    while (n > 0) 
    {
        const int half = n >> 1;
        const int middle = index + half;

        if ( stops[middle].pos <= pos ) 
        {
            index = middle + 1;
            n -= half + 1;
        } 
        else 
            n = half;
    }

    return index;
}

inline QRgb QwtLinearColorMap::ColorStops::rgb(
    QwtLinearColorMap::Mode mode, double pos) const
{
    if ( pos <= 0.0 )
        return _stops[0].rgb;
    if ( pos >= 1.0 )
        return _stops[(int)(_stops.size() - 1)].rgb;

    const int index = findUpper(pos);
    if ( mode == FixedColors )
    {
        return _stops[index-1].rgb;
    }
    else
    {
        const ColorStop &s1 = _stops[index-1];
        const ColorStop &s2 = _stops[index];

        const double ratio = (pos - s1.pos) / (s2.pos - s1.pos);

        const int r = s1.r + qRound(ratio * (s2.r - s1.r));
        const int g = s1.g + qRound(ratio * (s2.g - s1.g));
        const int b = s1.b + qRound(ratio * (s2.b - s1.b));
    
        return qRgb(r, g, b);
    }
}

//! Constructor
QwtColorMap::QwtColorMap(Format format):
    d_format(format)
{
}

//! Destructor
QwtColorMap::~QwtColorMap()
{
}

/*!
   Build and return a color map of 256 colors

   The color table is needed for rendering indexed images in combination
   with using colorIndex(). 

   \param interval Range for the values
   \return A color table, that can be used for a QImage
*/
QwtColorTable QwtColorMap::colorTable(
    const QwtDoubleInterval &interval) const
{
    QwtColorTable table(256);

    if ( interval.isValid() )
    {
        const double step = interval.width() / (table.size() - 1);
        for ( int i = 0; i < (int) table.size(); i++ )
            table[i] = rgb(interval, interval.minValue() + step * i);
    }

    return table;
}

class QwtLinearColorMap::PrivateData
{
public:
    ColorStops colorStops;
    QwtLinearColorMap::Mode mode;
};

/*! 
   Constructor
   \param format Preferred format of the coor map
*/
QwtLinearColorMap::QwtLinearColorMap(QwtColorMap::Format format):
    QwtColorMap(format)
{
    d_data = new PrivateData;
    d_data->mode = ScaledColors;

    setColorInterval( Qt::blue, Qt::yellow);
}

//! Copy constructor
QwtLinearColorMap::QwtLinearColorMap(const QwtLinearColorMap &other):
    QwtColorMap(other)
{
    d_data = new PrivateData;
    *this = other;
}

/*!
   Build a color map from 2 colors

   \param color1 Color used for the minimum value of the value interval
   \param color2 Color used for the maximum value of the value interval
   \param format Preferred format of the coor map
*/
QwtLinearColorMap::QwtLinearColorMap(const QColor &color1, 
        const QColor &color2, QwtColorMap::Format format):
    QwtColorMap(format)
{
    d_data = new PrivateData;
    d_data->mode = ScaledColors;
    setColorInterval(color1, color2); 
}

//! Destructor
QwtLinearColorMap::~QwtLinearColorMap()
{
    delete d_data;
}

//! Assignment operator
QwtLinearColorMap &QwtLinearColorMap::operator=(
    const QwtLinearColorMap &other)
{
    QwtColorMap::operator=(other);
    *d_data = *other.d_data;
    return *this;
}

//! Clone the color map
QwtColorMap *QwtLinearColorMap::copy() const
{
    QwtLinearColorMap* map = new QwtLinearColorMap();
    *map = *this;

    return map;
}

void QwtLinearColorMap::setMode(Mode mode)
{
    d_data->mode = mode;
}

QwtLinearColorMap::Mode QwtLinearColorMap::mode() const
{
    return d_data->mode;
}

/*!
   Set the color range 

   \param color1 Color used for the minimum value of the value interval
   \param color2 Color used for the maximum value of the value interval

   \sa color1(), color2()
*/
void QwtLinearColorMap::setColorInterval(
    const QColor &color1, const QColor &color2)
{
    d_data->colorStops = ColorStops();
    d_data->colorStops.insert(0.0, color1);
    d_data->colorStops.insert(1.0, color2);
}

void QwtLinearColorMap::addColorStop(double value, const QColor& color)
{
    if ( value >= 0.0 && value <= 1.0 )
        d_data->colorStops.insert(value, color);
}

/*! 
  \return the first color of the color range
  \sa setColorInterval()
*/
QColor QwtLinearColorMap::color1() const
{
    return QColor(d_data->colorStops.rgb(d_data->mode, 0.0));
}

/*! 
  \return the second color of the color range
  \sa setColorInterval()
*/
QColor QwtLinearColorMap::color2() const
{
    return QColor(d_data->colorStops.rgb(d_data->mode, 1.0));
}

/*!
  Map a value of a given interval into a rgb value

  \param interval Range for all values
  \param value Value to map into a rgb value
*/
QRgb QwtLinearColorMap::rgb(const QwtDoubleInterval &interval,
    double value) const
{
#if 0
    if ( !interval.isValid() )
        return QColor().rgb();
#endif

    const double ratio = (value - interval.minValue()) / interval.width();
    return d_data->colorStops.rgb(d_data->mode, ratio);
}

/*!
  Map a value of a given interval into a color index, between 0 and 255

  \param interval Range for all values
  \param value Value to map into a color index
*/
unsigned char QwtLinearColorMap::colorIndex(
    const QwtDoubleInterval &interval, double value) const
{
    if ( !interval.isValid() || value <= interval.minValue() )
        return 0;

    if ( value >= interval.maxValue() )
        return (unsigned char)255;

    const double ratio = (value - interval.minValue()) / interval.width();
    
    unsigned char index;
    if ( d_data->mode == FixedColors )
        index = (unsigned char)(ratio * 255); // always floor
    else
        index = (unsigned char)qRound(ratio * 255);

    return index;
}
