#ifndef BOXCURVE_H
#define BOXCURVE_H

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

class BoxCurve: public QwtPlotCurve
{
public:
	enum BoxStyle{NoBox, Rect, Diamond, WindBox, Notch};
	enum Range{None, SD, SE, r25_75, r10_90, r5_95, r1_99, MinMax, UserDef};
	BoxCurve(QwtPlot *parent, const char *name=0);

	void copy(const BoxCurve *b);

	virtual void draw(QPainter *painter,const QwtScaleMap &xMap, 
		const QwtScaleMap &yMap, int from, int to) const;

	void drawBox(QPainter *painter, const QwtScaleMap &xMap, 
				const QwtScaleMap &yMap, double *dat, int size) const;
	void drawSymbols(QPainter *painter, const QwtScaleMap &xMap, 
				const QwtScaleMap &yMap, double *dat, int size) const;

	virtual QwtDoubleRect boundingRect() const;

	QwtSymbol::Style minStyle(){return min_style;};
	void setMinStyle(QwtSymbol::Style s){min_style = s;};

	QwtSymbol::Style maxStyle(){return max_style;};
	void setMaxStyle(QwtSymbol::Style s){max_style = s;};

	void setMeanStyle(QwtSymbol::Style s){mean_style = s;};
	QwtSymbol::Style meanStyle(){return mean_style;};

	void setP99Style(QwtSymbol::Style s){p99_style = s;};
	QwtSymbol::Style p99Style(){return p99_style;};

	void setP1Style(QwtSymbol::Style s){p1_style = s;};
	QwtSymbol::Style p1Style(){return p1_style;};

	int boxStyle(){return b_style;};
	void setBoxStyle(int style);

	int boxWidth(){return b_width;};
	void setBoxWidth(int width){b_width=width;};

	double boxRange(){return b_coeff;};
	int boxRangeType(){return b_range;};
	void setBoxRange(int type, double coeff);

	double whiskersRange(){return w_coeff;};
	int whiskersRangeType(){return w_range;};
	void setWhiskersRange(int type, double coeff);
	
private:
	QwtSymbol::Style min_style, max_style, mean_style, p99_style, p1_style;
	double b_coeff, w_coeff;
	int b_style, b_width, b_range, w_range;
};

class QwtSingleArrayData: public QwtData
{
public:
    QwtSingleArrayData(const double x, const double *y, size_t size)
	{
		d_y.detach();
		d_y.duplicate(y, size);
		d_x = x;
	};

    virtual QwtData *copy() const{return new QwtSingleArrayData(d_x, d_y, size());};
 
    virtual size_t size() const{return d_y.size();};
    virtual double x(size_t) const{return d_x;};
    virtual double y(size_t i) const{return d_y[int(i)];};
 
private:
    QwtArray<double> d_y;
	double d_x;
};

#endif
