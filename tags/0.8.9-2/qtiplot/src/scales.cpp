#include "scales.h"
#include "parser.h"

#include <qpainter.h>
#include <qpaintdevice.h>
#include <qdatetime.h>

#include <qwt_painter.h>
#include <qwt_text.h>		
#include <qwt_scale_map.h>

ScaleDraw::ScaleDraw(const QString& s):
	d_fmt('g'),
    d_prec(4),
	formula_string (s),
	d_majTicks(Out),
	d_minTicks(Out)
	{};

double ScaleDraw::transformValue(double value) const
	{
	if (!formula_string.isEmpty())
		{
		double lbl=0.0;
		try
			{
			myParser parser;
			if (formula_string.contains("x"))
				parser.DefineVar("x", &value);
			else if (formula_string.contains("y"))
				parser.DefineVar("y", &value);

			parser.SetExpr(formula_string.ascii());
			lbl=parser.Eval();
			}
		catch(mu::ParserError &)
			{
			return 0;
			}

		return lbl;
		}
	else
		return value;
	}

/*!
  \brief Set the number format for the major scale labels

  Format character and precision have the same meaning as for
  sprintf().
  \param f format character 'e', 'f', 'g' 
  \param prec
    - for 'e', 'f': the number of digits after the radix character (point)
    - for 'g': the maximum number of significant digits

  \sa labelFormat()
*/
void ScaleDraw::setLabelFormat(char f, int prec)
{
d_fmt = f;
d_prec = prec;
}

/*!
  \brief Return the number format for the major scale labels

  Format character and precision have the same meaning as for
  sprintf().
  \param f format character 'e', 'f' or 'g' 
  \param prec
    - for 'e', 'f': the number of digits after the radix character (point)
    - for 'g': the maximum number of significant digits

  \sa setLabelFormat()
*/
void ScaleDraw::labelFormat(char &f, int &prec) const
{
    f = d_fmt;
    prec = d_prec;
}

void ScaleDraw::drawTick(QPainter *p, double value, int len) const
{
QwtScaleDiv scDiv = scaleDiv();
QwtValueList majTicks = scDiv.ticks(QwtScaleDiv::MajorTick);
if (majTicks.contains(value) && (d_majTicks == In || d_majTicks == None))
	return;

QwtValueList medTicks = scDiv.ticks(QwtScaleDiv::MediumTick);
if (medTicks.contains(value) && (d_minTicks == In || d_minTicks == None))
	return;

QwtValueList minTicks = scDiv.ticks(QwtScaleDiv::MinorTick);
if (minTicks.contains(value) && (d_minTicks == In || d_minTicks == None))
	return;

QwtScaleDraw::drawTick(p, value, len);
}

/*****************************************************************************
 *
 * Class QwtTextScaleDraw
 *
 *****************************************************************************/

QwtTextScaleDraw::QwtTextScaleDraw(const QStringList& list):
					  labels(list)
{}
				
QwtText QwtTextScaleDraw::label(double value) const
{
int index=qRound(value);
if (floor(value) == value && index > 0 && index <= (int)labels.count())
	return QwtText(labels[index - 1]);
else
	return QwtText();
}

/*****************************************************************************
 *
 * Class TimeScaleDraw
 *
 *****************************************************************************/

TimeScaleDraw::TimeScaleDraw(const QTime& t, const QString& format):
		t_origin (t), 
		t_format (format)
		{}
			
QString TimeScaleDraw::origin() 
{
return t_origin.toString ( "hh:mm:ss.zzz" );
}
	
		
QwtText TimeScaleDraw::label(double value) const
{
QTime t = t_origin.addMSecs ( (int)value );		
return QwtText(t.toString ( t_format ));
}

/*****************************************************************************
 *
 * Class DateScaleDraw
 *
 *****************************************************************************/

DateScaleDraw::DateScaleDraw(const QDate& t, const QString& format):
			  t_origin (t), 
			  t_format (format)
{}
		
QString DateScaleDraw::origin() 
{
return t_origin.toString ( Qt::ISODate );
}
	
QwtText DateScaleDraw::label(double value) const
{
QDate t = t_origin.addDays ( (int)value );
return QwtText(t.toString ( t_format ));
}

/*****************************************************************************
 *
 * Class WeekDayScaleDraw
 *
 *****************************************************************************/

WeekDayScaleDraw:: WeekDayScaleDraw(NameFormat format):
				d_format(format)
{}
				
QwtText WeekDayScaleDraw::label(double value) const
{
int val = int(transformValue(value))%7;

if (val < 0)
	val = 7 - abs(val);
else if (val == 0)
	val = 7;

QString day;
switch(d_format)
	{
	case  ShortName:
		day = QDate::shortDayName (val);
	break;

	case  LongName:
		day = QDate::longDayName (val);
	break;

	case  Initial:
		day = (QDate::shortDayName (val)).left(1);
	break;
	}
return QwtText(day);
}

/*****************************************************************************
 *
 * Class MonthScaleDraw
 *
 *****************************************************************************/

MonthScaleDraw::MonthScaleDraw(NameFormat format):
		d_format(format)
{};
		
QwtText MonthScaleDraw::label(double value) const
{
int val = int(transformValue(value))%12;

if (val < 0)
	val = 12 - abs(val);
else if (val == 0)
	val = 12;

QString day;
switch(d_format)
	{
	case  ShortName:
		day = QDate::shortMonthName (val);
	break;

	case  LongName:
		day = QDate::longMonthName (val);
	break;

	case  Initial:
		day = (QDate::shortMonthName (val)).left(1);
	break;
	}
return QwtText(day);
}

/*****************************************************************************
 *
 * Class QwtSupersciptsScaleDraw
 *
 *****************************************************************************/

QwtSupersciptsScaleDraw::QwtSupersciptsScaleDraw(const QString& s)
{
setFormulaString(s);
}
 
QwtText QwtSupersciptsScaleDraw::label(double value) const
{
char f;
int prec;
labelFormat(f, prec);
	
double val = transformValue(value);
	
QString txt;
txt.setNum (val, 'e', prec);

QStringList list = QStringList::split ( "e", txt, FALSE );
if (list[0].toDouble() == 0.0)
	return QString("0");
	
QString s= list[1];
int l = s.length();
QChar sign = s[0];
	
s.remove (sign);
		
while (l>1 && s.startsWith ("0", false))
	{
	s.remove ( 0, 1 );
	l = s.length();
	}
		
if (sign == '-')
	s.prepend(sign);
	
if (list[0] == "1")
	return QwtText("10<sup>" + s + "</sup>");
else
	return QwtText(list[0] + "x10<sup>" + s + "</sup>");
}




