// OPJFile.h
/*
Origin 7.5 column value display
Numeric, Text&Numeric:
	Decimal:1000            = 0
	Scientific:1E3          = 1
	Engeneering:1k          = 2
	Decimal:1,000           = 3

Time:
	hh:mm                   = 0
	hh                      = 1
	hh:mm:ss                = 2
	hh:mm:ss.zz             = 3
	hh ap                   = 4
	hh:mm ap                = 5
	mm:ss                   = 6
	mm:ss.zz                = 7
	hhmm                    = 8
	hhmmss                  = 9
	hh:mm:ss.zzz            = 10

Date:
	dd.MM.yyyy              = 0
	y.                      = 1  (year abbreviation - for instance, '�.' in russian)
	MMM d                   = 2
	M/d                     = 3
	d                       = 4
	ddd                     = 5
	F                       = 6	 (first letter of day of week)
	yyyy                    = 7
	yy                      = 8
	dd.MM.yyyy hh:mm        = 9
	dd.MM.yyyy hh:mm:ss     = 10
	yyMMdd                  = 11
	yyMMdd hh:mm            = 12
	yyMMdd hh:mm:ss         = 13
	yyMMdd hhmm             = 14
	yyMMdd hhmmss           = 15
	MMM                     = 16
	J                       = 17 (first letter of month)
	Q1                      = 18 (quartal)
	M-d-yyyy (Custom1)      = 19
	hh:mm:ss.zzzz (Custom2) = 20

Month:
	MMM                     = 0
	MMMM                    = 1
	J                       = 2	 (first letter of month)

Day of Week:
	ddd                     = 0
	dddd                    = 1
	F                       = 2	 (first letter of day of week)

Fill Area Patterns
	none = 0
	///  = 1
	/ /  = 2
	 /   = 3
	\\\  = 4
	\ \  = 5
	 \   = 6
	xxx  = 7
	x x  = 8
	 x   = 9
	---  = 10
	- -  = 11
	 -   = 12
	|||  = 13
	| |  = 14
	 |   = 15
	+++  = 16
	+ +  = 17
	 +   = 18
*/
#ifndef OPJFILE_H
#define OPJFILE_H

#include <string>
#include <vector>

using namespace std;

// for string entries
struct Entry {
	short spread;
	short column;
	short row;
	char *name;
};

typedef Entry Entry;

struct spreadColumn {
	string name;
	string type;
	int value_type;//Numeric = 0, Text = 1, Date = 2, Time = 3, Month = 4, Day = 5, Text&Numeric = 6
	int value_type_specification; //see above
	int significant_digits;
	int decimal_places;
	int numeric_display_type;//Default Decimal Digits=0, Decimal Places=1, Significant Digits=2
	string command;
	string comment;
	int width;
	int index;
	vector <double> data;
	vector <string> sdata;
	spreadColumn(string _name="", int _index=0)
	:	name(_name)
	,	index(_index)
	,	command("")
	,	comment("")
	,	value_type(0)
	,	value_type_specification(0)
	,	significant_digits(6)
	,	decimal_places(6)
	,	width(8)
	,	numeric_display_type(0)
	{};
};

struct spreadSheet {
	string name;
	string label;
	int maxRows;
	bool bHidden;
	bool bLoose;
	vector <spreadColumn> column;
	spreadSheet(string _name="")
	:	name(_name)
	,	label("")
	,	bHidden(false)
	,	bLoose(true)
	{};
};

struct matrix {
	string name;
	string label;
	int nr_rows;
	int nr_cols;
	int value_type_specification;
	int significant_digits;
	int decimal_places;
	int numeric_display_type;//Default Decimal Digits=0, Decimal Places=1, Significant Digits=2
	string command;
	int width;
	int index;
	vector <double> data;
	matrix(string _name="", int _index=0)
	:	name(_name)
	,	index(_index)
	,	command("")
	,	value_type_specification(0)
	,	significant_digits(6)
	,	decimal_places(6)
	,	width(8)
	,	numeric_display_type(0)
	{};
};

struct function {
	string name;
	int type;//Normal = 0, Polar = 1
	string formula;
	double begin;
	double end;
	int points;
	int index;
	function(string _name="", int _index=0)
	:	name(_name)
	,	index(_index)
	,	type(0)
	,	formula("")
	,	begin(0.0)
	,	end(0.0)
	,	points(0)
	{};
};

struct graphCurve {
	int type;
	string dataName;
	string xColName;
	string yColName;
	int line_color;
	int line_style;
	int line_connect;
	double line_width;

	bool fillarea;
	int fillarea_type;
	int fillarea_pattern;
	int fillarea_color;
	int fillarea_pattern_color;
	int fillarea_pattern_width;

	
	int symbol_type;
	int symbol_color;
	double symbol_size;
	int symbol_thickness;
	int point_offset;
};

enum AxisPosition{ Left = 0, Bottom = 1, Right = 2, Top = 3};

struct graphLayer {
	int xPos;
	int yPos;
	string xLabel;
	string yLabel;
	string legend;
	double xMin;
	double xMax;
	double yMin;
	double yMax;
	vector<graphCurve> curve;
};

struct graph {
	string name;
	string label;
	vector<graphLayer> layer;
};

class OPJFile
{
public:
	OPJFile(const char* filename);
	int Parse();
	double Version() { return version/100.0; }		//!< get version of project file
	
	//spreadsheet properties
	int numSpreads() { return SPREADSHEET.size(); }			//!< get number of spreadsheets
	const char *spreadName(int s) { return SPREADSHEET[s].name.c_str(); }	//!< get name of spreadsheet s
	bool spreadHidden(int s) { return SPREADSHEET[s].bHidden; }	//!< is spreadsheet s hidden
	bool spreadLoose(int s) { return SPREADSHEET[s].bLoose; }	//!< is spreadsheet s loose
	const char *spreadLabel(int s) { return SPREADSHEET[s].label.c_str(); }	//!< get label of spreadsheet s
	int numCols(int s) { return SPREADSHEET[s].column.size(); }		//!< get number of columns of spreadsheet s
	int numRows(int s,int c) { return SPREADSHEET[s].column[c].value_type==1 ? SPREADSHEET[s].column[c].sdata.size() : SPREADSHEET[s].column[c].data.size(); }	//!< get number of rows of column c of spreadsheet s
	int maxRows(int s) { return SPREADSHEET[s].maxRows; }		//!< get maximum number of rows of spreadsheet s
	//spreadsheet's column properties
	const char *colName(int s, int c) { printf("N"); return SPREADSHEET[s].column[c].name.c_str(); }	//!< get name of column c of spreadsheet s
	const char *colType(int s, int c) { printf("T"); return SPREADSHEET[s].column[c].type.c_str(); }	//!< get type of column c of spreadsheet s
	const char *colCommand(int s, int c) { printf("C"); return SPREADSHEET[s].column[c].command.c_str(); }	//!< get command of column c of spreadsheet s
	const char *colComment(int s, int c) { printf("C"); return SPREADSHEET[s].column[c].comment.c_str(); }	//!< get comment of column c of spreadsheet s
	int colValueType(int s, int c) { return SPREADSHEET[s].column[c].value_type; }	//!< get value type of column c of spreadsheet s
	int colValueTypeSpec(int s, int c) { return SPREADSHEET[s].column[c].value_type_specification; }	//!< get value type specification of column c of spreadsheet s
	int colSignificantDigits(int s, int c) { return SPREADSHEET[s].column[c].significant_digits; }	//!< get significant digits of column c of spreadsheet s
	int colDecPlaces(int s, int c) { return SPREADSHEET[s].column[c].decimal_places; }	//!< get decimal places of column c of spreadsheet s
	int colNumDisplayType(int s, int c) { return SPREADSHEET[s].column[c].numeric_display_type; }	//!< get numeric display type of column c of spreadsheet s
	int colWidth(int s, int c) { return SPREADSHEET[s].column[c].width; }	//!< get width of column c of spreadsheet s
	vector <double> Data(int s, int c) { return SPREADSHEET[s].column[c].data; }	//!< get data of column c of spreadsheet s

	const char* SData(int s, int c, int r) { return SPREADSHEET[s].column[c].sdata[r].c_str();}	//!< get data strings of column c/row r of spreadsheet s

	//matrix properties
	int numMatrices() { return MATRIX.size(); }			//!< get number of matrices
	const char *matrixName(int s) { return MATRIX[s].name.c_str(); }	//!< get name of matrix s
	const char *matrixLabel(int s) { return MATRIX[s].label.c_str(); }	//!< get label of matrix s
	int numMartixCols(int s) { return MATRIX[s].nr_cols; }		//!< get number of columns of matrix s
	int numMartixRows(int s) { return MATRIX[s].nr_rows; }	//!< get number of rows of matrix s
	const char *matrixFormula(int s) { return MATRIX[s].command.c_str(); }	//!< get formula of matrix s
	int matrixValueTypeSpec(int s) { return MATRIX[s].value_type_specification; }	//!< get value type specification of matrix s
	int matrixSignificantDigits(int s) { return MATRIX[s].significant_digits; }	//!< get significant digits of matrix s
	int matrixDecPlaces(int s) { return MATRIX[s].decimal_places; }	//!< get decimal places of matrix s
	int matrixNumDisplayType(int s) { return MATRIX[s].numeric_display_type; }	//!< get numeric display type of matrix s
	int matrixWidth(int s) { return MATRIX[s].width; }	//!< get width of matrix s
	double matrixData(int s, int c, int r) { return MATRIX[s].data[r*MATRIX[s].nr_cols+c]; }	//!< get data of row r of column c of matrix s

	//function properties
	int numFunctions() { return FUNCTION.size(); }			//!< get number of functions
	const char *functionName(int s) { return MATRIX[s].name.c_str(); }	//!< get name of function s	
	int functionType(int s) { return FUNCTION[s].type; }		//!< get type of function s
	double functionBegin(int s) { return FUNCTION[s].begin; }	//!< get begin of interval of function s
	double functionEnd(int s) { return FUNCTION[s].end; }	//!< get end of interval of function s
	int functionPoints(int s) { return FUNCTION[s].points; }	//!< get number of points in interval of function s
	const char *functionFormula(int s) { return FUNCTION[s].formula.c_str(); }	//!< get formula of function s

	//graph properties
	enum Color {Black=0, Red=1, Green=2, Blue=3, Cyan=4, Magenta=5, Yellow=6, DarkYellow=7, Navy=8,
		Purple=9, Wine=10, Olive=11, DarkCyan=12, Royal=13, Orange=14, Violet=15, Pink=16, White=17,
		LightGray=18, Gray=19, LTYellow=20, LTCyan=21, LTMagenta=22, DarkGray=23, Custom=255};
	
	enum Plot {Line=200, Scatter=201, LineSymbol=202, Column=203, Area=204, HiLoClose=205, Box=206, 
		ColumnFloat=207, Vector=208, PlotDot=209, Wall3D=210, Ribbon3D=211, Bar3D=212, ColumnStack=213, 
		AreaStack=214, Bar=215, BarStack=216, FlowVector=218, Histogram=219, MatrixImage=220, Pie=225, 
		Contour=226, Unknown=230, ErrorBar=231, Text=232, XErrorBar=233, SurfaceColorMap=236, 
		SurfaceColorFill=237, SurfaceWireframe=238, SurfaceBars=239, Line3D=240, Text3D=241, Mesh3D=242, 
		XYZTriangular=245, LineSeries=246, YErrorBar=254, XYErrorBar=255, GraphScatter3D=0x8AF0, 
		GraphTrajectory3D=0x8AF1, Polar=0x00020000, SmithChart=0x00040000, FillArea=0x00800000};

	enum LineStyle {Solid=0, Dash=1, Dot=2, DashDot=3, DashDotDot=4, ShortDash=5, ShortDot=6, ShortDashDot=7};

	enum LineConnect {NoLine=0, Straight=1, TwoPointSegment=2, ThreePointSegment=3, BSpline=8, Spline=9, StepHorizontal=11, StepVertical=12, StepHCenter=13, StepVCenter=14, Bezier=15};

	int numGraphs() { return GRAPH.size(); }			//!< get number of graphs
	const char *graphName(int s) { return GRAPH[s].name.c_str(); }	//!< get name of graph s
	const char *graphLabel(int s) { return GRAPH[s].label.c_str(); }	//!< get name of graph s
	int numLayers(int s) { return GRAPH[s].layer.size(); }			//!< get number of layers of graph s
	const char *layerXAxisTitle(int s, int l) { return GRAPH[s].layer[l].xLabel.c_str(); }		//!< get label of X-axis of layer l of graph s
	const char *layerYAxisTitle(int s, int l) { return GRAPH[s].layer[l].yLabel.c_str(); }		//!< get label of Y-axis of layer l of graph s
	const char *layerLegend(int s, int l) { return GRAPH[s].layer[l].legend.c_str(); }		//!< get legend of layer l of graph s
	vector<double> layerXRange(int s, int l) { 
		vector<double> range; 
		range.push_back(GRAPH[s].layer[l].xMin);
		range.push_back(GRAPH[s].layer[l].xMax);
		return range;
	} //!< get X-range of layer l of graph s
	vector<double> layerYRange(int s, int l) { 
		vector<double> range; 
		range.push_back(GRAPH[s].layer[l].yMin);
		range.push_back(GRAPH[s].layer[l].yMax);
		return range;
	} //!< get Y-range of layer l of graph s
	int numCurves(int s, int l) { return GRAPH[s].layer[l].curve.size(); }			//!< get number of curves of layer l of graph s
	const char *curveDataName(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].dataName.c_str(); }	//!< get data source name of curve c of layer l of graph s
	const char *curveXColName(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].xColName.c_str(); }	//!< get X-column name of curve c of layer l of graph s
	const char *curveYColName(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].yColName.c_str(); }	//!< get Y-column name of curve c of layer l of graph s
	int curveType(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].type; }	//!< get type of curve c of layer l of graph s
	int curveLineStyle(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].line_style; }	//!< get line style of curve c of layer l of graph s
	int curveLineColor(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].line_color; }	//!< get line color of curve c of layer l of graph s
	int curveLineConnect(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].line_connect; }	//!< get line connect of curve c of layer l of graph s
	double curveLineWidth(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].line_width; }	//!< get line width of curve c of layer l of graph s
	
	bool curveIsFilledArea(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].fillarea; }	//!< get is filled area of curve c of layer l of graph s
	int curveFillAreaColor(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].fillarea_color; }	//!< get area fillcolor of curve c of layer l of graph s
	int curveFillPattern(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].fillarea_pattern; }	//!< get fill pattern of curve c of layer l of graph s
	int curveFillPatternColor(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].fillarea_pattern_color; }	//!< get fill pattern color of curve c of layer l of graph s
	double curveFillPatternWidth(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].fillarea_pattern_width; }	//!< get fill pattern line width of curve c of layer l of graph s

	int curveSymbolType(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].symbol_type; }	//!< get symbol type of curve c of layer l of graph s
	int curveSymbolColor(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].symbol_color; }	//!< get symbol color of curve c of layer l of graph s
	double curveSymbolSize(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].symbol_size; }	//!< get symbol size of curve c of layer l of graph s
	int curveSymbolThickness(int s, int l, int c) { return GRAPH[s].layer[l].curve[c].symbol_thickness; }	//!< get symbol thickness of curve c of layer l of graph s


private:
	bool IsBigEndian();
	void ByteSwap(unsigned char * b, int n);
	int  compareSpreadnames(char *sname);				//!< returns matching spread index
	int  compareColumnnames(int spread, char *sname);	//!< returns matching column index
	int  compareMatrixnames(char *sname);				//!< returns matching matrix index
	vector<string> findDataByIndex(int index);
	void readSpreadInfo(FILE *fopj, FILE *fdebug);
	void readMatrixInfo(FILE *fopj, FILE *fdebug);
	void readGraphInfo(FILE *fopj, FILE *fdebug);
	void skipObjectInfo(FILE *fopj, FILE *fdebug);
	void setColName(int spread);		//!< set default column name starting from spreadsheet spread
	const char* filename;			//!< project file name
	int version;				//!< project version
	int dataIndex;
	vector <spreadSheet> SPREADSHEET;
	vector <matrix> MATRIX;
	vector <function> FUNCTION;
	vector <graph> GRAPH;
};

#endif // OPJFILE_H
