/***************************************************************************
    File                 : importOPJ.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2007 by Ion Vasilief, Alex Kargovsky, Tilman Hoener zu Siederdissen
    Email (use @ for *)  : ion_vasilief*yahoo.fr, kargovsky*yumr.phys.msu.su, thzs*gmx.net
    Description          : Origin project import class

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
#include "importOPJ.h"
#include "../3rdparty/liborigin/OPJFile.h"

#include <QRegExp>
#include <QMessageBox>
#include "Matrix.h"
#include "MultiLayer.h"

ImportOPJ::ImportOPJ(ApplicationWindow *app, const QString& filename) :
		mw(app)
{
OPJFile opj((const char *)filename.latin1());
parse_error = opj.Parse();
importTables(opj);
importGraphs(opj);
importFunctions(opj);
}

QColor ImportOPJ::originColor(int c)
{
	QColor clr;
	switch(c)
	{
	case OPJFile::Black:
		clr=Qt::black;
		break;
	case OPJFile::Red:
		clr=Qt::red;
		break;
	case OPJFile::Green:
		clr=Qt::green;
		break;
	case OPJFile::Blue:
		clr=Qt::blue;
		break;
	case OPJFile::Cyan:
		clr=Qt::cyan;
		break;
	case OPJFile::Magenta:
		clr=Qt::magenta;
		break;
	case OPJFile::Yellow:
		clr=Qt::yellow;
		break;
	case OPJFile::DarkYellow:
		clr.setNamedColor("#808000");
		break;
	case OPJFile::Navy:
		clr.setNamedColor("#000080");
		break;
	case OPJFile::Purple:
		clr.setNamedColor("#800080");
		break;
	case OPJFile::Wine:
		clr.setNamedColor("#800000");
		break;
	case OPJFile::Olive:
		clr.setNamedColor("#008000");
		break;
	case OPJFile::DarkCyan:
		clr.setNamedColor("#008080");
		break;
	case OPJFile::Royal:
		clr.setNamedColor("#0000A0");
		break;
	case OPJFile::Orange:
		clr.setNamedColor("#FF8000");
		break;
	case OPJFile::Violet:
		clr.setNamedColor("#8000FF");
		break;
	case OPJFile::Pink:
		clr.setNamedColor("#FF0080");
		break;
	case OPJFile::White:
		clr=Qt::white;
		break;
	case OPJFile::LightGray:
		clr.setNamedColor("#C0C0C0");
		break;
	case OPJFile::Gray:
		clr.setNamedColor("#808080");
		break;
	case OPJFile::LTYellow:
		clr.setNamedColor("#FFFF80");
		break;
	case OPJFile::LTCyan:
		clr.setNamedColor("#80FFFF");
		break;
	case OPJFile::LTMagenta:
		clr.setNamedColor("#FF80FF");
		break;
	case OPJFile::DarkGray:
		clr.setNamedColor("#404040");
		break;
	case OPJFile::Custom:
		clr=Qt::white;
		break;
	default:
		clr=Qt::black;
	}
	return clr;
}

bool ImportOPJ::importTables(OPJFile opj)
{
	int visible_count=0;
	int QtiPlot_scaling_factor=10; //in Origin width is measured in characters while in QtiPlot - pixels --- need to be accurate
	for (int s=0; s<opj.numSpreads(); s++)
	{
		int nr_cols = opj.numCols(s);
		int maxrows = opj.maxRows(s);

		Table *table = (opj.spreadHidden(s)||opj.spreadLoose(s))&&opj.Version()==7.5 ? mw->newHiddenTable(opj.spreadName(s), opj.spreadLabel(s), maxrows, nr_cols)
										: mw->newTable(opj.spreadName(s), maxrows, nr_cols);
		if (!table)
			return false;

		table->setWindowLabel(opj.spreadLabel(s));
		for (int j=0; j<nr_cols; j++)
		{
			QString name(opj.colName(s,j));
			table->setColName(j, name.replace(QRegExp(".*_"),""));
			table->setCommand(j, QString(opj.colCommand(s,j)));
			table->setColComment(j, QString(opj.colComment(s,j)));
			table->changeColWidth(opj.colWidth(s,j)*QtiPlot_scaling_factor, j);

			if (QString(opj.colType(s,j)) == "X")
				table->setColPlotDesignation(j, Table::X);
			else if (QString(opj.colType(s,j)) == "Y")
				table->setColPlotDesignation(j, Table::Y);
			else if (QString(opj.colType(s,j)) == "Z")
				table->setColPlotDesignation(j, Table::Z);
			else if (QString(opj.colType(s,j)) == "DX")
				table->setColPlotDesignation(j, Table::xErr);
			else if (QString(opj.colType(s,j)) == "DY")
				table->setColPlotDesignation(j, Table::yErr);
			else
				table->setColPlotDesignation(j, Table::None);

            table->setHeaderColType();//update header

			for (int i=0; i<opj.numRows(s,j); i++)
			{
				if(strcmp(opj.colType(s,j),"LABEL")&&opj.colValueType(s,j)!=1)
				{// number
					double val = opj.Data(s,j)[i];
					if(fabs(val)>0 && fabs(val)<2.0e-300)// empty entry
						continue;

					table->setText(i, j, QString::number(val));
				}
				else// label? doesn't seem to work
					table->setText(i, j, QString(opj.SData(s,j,i)));
			}

			QString format;
			switch(opj.colValueType(s,j))
			{
			case 0: //Numeric
			case 6: //Text&Numeric
				int f;
				if(opj.colNumDisplayType(s,j)==0)
					f=0;
				else
					switch(opj.colValueTypeSpec(s,j))
					{
					case 0: //Decimal 1000
						f=1;
						break;
					case 1: //Scientific
						f=2;
						break;
					case 2: //Engeneering
					case 3: //Decimal 1,000
						f=0;
						break;
					}
				table->setColNumericFormat(f, opj.colDecPlaces(s,j), j);
				break;
			case 1: //Text
				table->setTextFormat(j);
				break;
			case 2: // Date
				switch(opj.colValueTypeSpec(s,j))
				{
				case 0:
				case 9:
				case 10:
					format="dd.MM.yyyy";
					break;
				case 2:
					format="MMM d";
					break;
				case 3:
					format="M/d";
					break;
				case 4:
					format="d";
					break;
				case 5:
				case 6:
					format="ddd";
					break;
				case 7:
					format="yyyy";
					break;
				case 8:
					format="yy";
					break;
				case 11:
				case 12:
				case 13:
				case 14:
				case 15:
					format="yyMMdd";
					break;
				case 16:
				case 17:
					format="MMM";
					break;
				case 19:
					format="M-d-yyyy";
					break;
				default:
					format="dd.MM.yyyy";
				}
				table->setDateFormat(format, j);
				break;
			case 3: // Time
				switch(opj.colValueTypeSpec(s,j))
				{
				case 0:
					format="hh:mm";
					break;
				case 1:
					format="hh";
					break;
				case 2:
					format="hh:mm:ss";
					break;
				case 3:
					format="hh:mm:ss.zzz";
					break;
				case 4:
					format="hh ap";
					break;
				case 5:
					format="hh:mm ap";
					break;
				case 6:
					format="mm:ss";
					break;
				case 7:
					format="mm:ss.zzz";
					break;
				case 8:
					format="hhmm";
					break;
				case 9:
					format="hhmmss";
					break;
				case 10:
					format="hh:mm:ss.zzz";
					break;
				}
				table->setTimeFormat(format, j);
				break;
			case 4: // Month
				switch(opj.colValueTypeSpec(s,j))
				{
				case 0:
				case 2:
					format="shortMonthName";
					break;
				case 1:
					format="longMonthName";
					break;
				}
				table->setMonthFormat(format, j);
				break;
			case 5: // Day
				switch(opj.colValueTypeSpec(s,j))
				{
				case 0:
				case 2:
					format="shortDayName";
					break;
				case 1:
					format="longDayName";
					break;
				}
				table->setDayFormat(format, j);
				break;
			}
		}


		if(!(opj.spreadHidden(s)||opj.spreadLoose(s))||opj.Version()!=7.5)
		{
			table->showNormal();

			//cascade the tables
			int dx=table->verticalHeaderWidth();
			int dy=table->parentWidget()->frameGeometry().height() - table->height();
			table->parentWidget()->move(QPoint(visible_count*dx,visible_count*dy));
			visible_count++;
		}
	}

//Import matrices
	for (int s=0; s<opj.numMatrices(); s++)
	{
		int nr_cols = opj.numMartixCols(s);
		int nr_rows = opj.numMartixRows(s);

		Matrix* matrix = mw->newMatrix(opj.matrixName(s), nr_rows, nr_cols);
		if (!matrix)
			return false;

		matrix->setWindowLabel(opj.matrixLabel(s));
		matrix->setFormula(opj.matrixFormula(s));
		matrix->setColumnsWidth(opj.matrixWidth(s)*QtiPlot_scaling_factor);
		for (int j=0; j<nr_cols; j++)
		{
			for (int i=0; i<nr_rows; i++)
			{
				double val = opj.matrixData(s,j,i);
				if(fabs(val)>0 && fabs(val)<2.0e-300)// empty entry
					continue;

				matrix->setCell(i, j, val);
			}
		}

		matrix->saveCellsToMemory();

		QChar f;
		switch(opj.matrixValueTypeSpec(s))
		{
		case 0: //Decimal 1000
			f='f';
			break;
		case 1: //Scientific
			f='e';
			break;
		case 2: //Engeneering
		case 3: //Decimal 1,000
			f='g';
			break;
		}
		matrix->setNumericFormat(f, opj.matrixSignificantDigits(s));

		matrix->showNormal();

		//cascade the matrices
		int dx=matrix->verticalHeaderWidth();
		int dy=matrix->parentWidget()->frameGeometry().height() - matrix->height();
		matrix->parentWidget()->move(QPoint(visible_count*dx,visible_count*dy));
		visible_count++;

	}

return true;
}


bool ImportOPJ::importFunctions(OPJFile opj)
{
	double pi=3.141592653589793;
	for (int s=0; s<opj.numFunctions(); s++)
	{
		QStringList formulas;
		QList<double> ranges;
		int type;
		if(opj.functionType(s)==1)//Polar
		{
			type=2;
			formulas << opj.functionFormula(s) << "x";
			ranges << pi/180*opj.functionBegin(s) << pi/180*opj.functionEnd(s);
		}
		else
		{
			type=0;
			formulas << opj.functionFormula(s);
			ranges << opj.functionBegin(s) << opj.functionEnd(s);
		}
		mw->newFunctionPlot(type, formulas, "x", ranges, opj.functionPoints(s));

	}

return true;
}

bool ImportOPJ::importGraphs(OPJFile opj)
{
	for (int g=0; g<opj.numGraphs(); g++)
	{
		MultiLayer *ml = mw->multilayerPlot(opj.graphName(g));
		if (!ml)
			return false;
		ml->setWindowLabel(opj.graphLabel(g));
		for(int l=0; l<opj.numLayers(g); l++)
		{
			Graph *graph=ml->addLayer();
			if(!graph)
				return false;
			graph->setXAxisTitle(QString::fromLocal8Bit(opj.layerXAxisTitle(g,l)));
			graph->setYAxisTitle(QString::fromLocal8Bit(opj.layerYAxisTitle(g,l)));
			if(strlen(opj.layerLegend(g,l))>0)
				graph->newLegend(QString::fromLocal8Bit(opj.layerLegend(g,l)));
			for(int c=0; c<opj.numCurves(g,l); c++)
			{
				QString data(opj.curveDataName(g,l,c));
				int style=0;
				int color=0;
				switch(opj.curveType(g,l,c))
				{
				case OPJFile::Line:
					style=Graph::Line;
					break;
				case OPJFile::Scatter:
					style=Graph::Scatter;
					break;
				case OPJFile::LineSymbol:
					style=Graph::LineSymbols;
					break;
				}
				QString tableName = data.right(data.length()-2);
				graph->insertCurve(mw->table(tableName), tableName + "_" + opj.curveXColName(g,l,c), tableName + "_" + opj.curveYColName(g,l,c), style);
				CurveLayout cl = graph->initCurveLayout(style, opj.numCurves(g,l));
				cl.sSize = ceil(opj.curveSymbolSize(g,l,c));
				cl.penWidth=opj.curveSymbolThickness(g,l,c);
				color=opj.curveSymbolColor(g,l,c);
				cl.symCol=(color==0xF7?0:color); //0xF7 -Automatic color
				switch(opj.curveSymbolType(g,l,c)&0xFF)
				{
				case 0: //NoSymbol
					cl.sType=0;
					break;
				case 1: //Rect
					cl.sType=2;
					break;
				case 2: //Ellipse
				case 20://Sphere
					cl.sType=1;
					break;
				case 3: //UTriangle
					cl.sType=6;
					break;
				case 4: //DTriangle
					cl.sType=5;
					break;
				case 5: //Diamond
					cl.sType=3;
					break;
				case 6: //Cross +
					cl.sType=9;
					break;
				case 7: //Cross x
					cl.sType=10;
					break;
				case 8: //Snow
					cl.sType=13;
					break;
				case 9: //Horizontal -
					cl.sType=11;
					break;
				case 10: //Vertical |
					cl.sType=12;
					break;
				case 15: //LTriangle
					cl.sType=7;
					break;
				case 16: //RTriangle
					cl.sType=8;
					break;
				case 17: //Hexagon
				case 19: //Pentagon
					cl.sType=15;
					break;
				case 18: //Star
					cl.sType=14;
					break;
				default:
					cl.sType=0;
				}

				switch(opj.curveSymbolType(g,l,c)>>8)
				{
				case 0:
				case 8:
				case 9:
				case 10:
				case 11:
					color=opj.curveSymbolColor(g,l,c);
					cl.fillCol=(color==0xF7?0:color); //0xF7 -Automatic color
					break;
				default:
					cl.fillCol=-1;
				}

				cl.filledArea=opj.curveIsFilledArea(g,l,c)?1:0;
				if(cl.filledArea)
				{
					switch(opj.curveFillPattern(g,l,c))
					{
					case 0:
						cl.aStyle=0;
						break;
					case 1:
					case 2:
					case 3:
						cl.aStyle=4;
						break;
					case 4:
					case 5:
					case 6:
						cl.aStyle=5;
						break;
					case 7:
					case 8:
					case 9:
						cl.aStyle=6;
						break;
					case 10:
					case 11:
					case 12:
						cl.aStyle=1;
						break;
					case 13:
					case 14:
					case 15:
						cl.aStyle=2;
						break;
					case 16:
					case 17:
					case 18:
						cl.aStyle=3;
						break;
					}
					color=opj.curveFillAreaColor(g,l,c);
					cl.aCol=(color==0xF7?0:color); //0xF7 -Automatic color
				}
				cl.lWidth = ceil(opj.curveLineWidth(g,l,c));
				color=opj.curveLineColor(g,l,c);
				cl.lCol=(color==0xF7?0:color); //0xF7 -Automatic color
				switch (opj.curveLineStyle(g,l,c))
				{
					case OPJFile::Solid:
						cl.lStyle=0;
						break;
					case OPJFile::Dash:
					case OPJFile::ShortDash:
						cl.lStyle=1;
						break;
					case OPJFile::Dot:
					case OPJFile::ShortDot:
						cl.lStyle=2;
						break;
					case OPJFile::DashDot:
					case OPJFile::ShortDashDot:
						cl.lStyle=3;
						break;
					case OPJFile::DashDotDot:
						cl.lStyle=4;
						break;
				}
				graph->updateCurveLayout(c, &cl);
				switch(opj.curveLineConnect(g,l,c))
				{
				case OPJFile::NoLine:
					graph->setCurveStyle(c, QwtPlotCurve::NoCurve);
					break;
				case OPJFile::Straight:
					graph->setCurveStyle(c, QwtPlotCurve::Lines);
					break;
				case OPJFile::BSpline:
				case OPJFile::Bezier:
				case OPJFile::Spline:
					graph->setCurveStyle(c, 5);
					break;
				case OPJFile::StepHorizontal:
				case OPJFile::StepHCenter:
					graph->setCurveStyle(c, QwtPlotCurve::Steps);
					break;
				case OPJFile::StepVertical:
				case OPJFile::StepVCenter:
					graph->setCurveStyle(c, 6);
					break;
				}

			}
			vector<double> range=opj.layerXRange(g,l);
			graph->setScale(2,range[0],range[1]);
			range=opj.layerYRange(g,l);
			graph->setScale(0,range[0],range[1]);
		}
		ml->arrangeLayers(true,true);
	}

	return true;
}
