#ifndef __COLORGENERATOR_H__
#define __COLORGENERATOR_H__

#include <qstring.h>
#include "qwt3d_global.h"
#include "qwt3d_types.h"

namespace Qwt3D
{
    class Curve;

//! Abstract base class for color functors
/*!
Use your own color model by providing an implementation of operator()(double x, double y, double z).
Colors destructor has been declared \c protected, in order to use only heap based objects. Curve 
will handle the objects destruction.
See StandardColor for an example
*/
class QWT3D_EXPORT Color
{
public:
	virtual Qwt3D::RGBA operator()(double x, double y, double z) const = 0; //!< Implement your color model here
	virtual Qwt3D::RGBA operator()(Qwt3D::Triple const& t) const {return this->operator()(t.x,t.y,t.z);}
	//! Should create a color vector usable by ColorLegend. The default implementation returns his argument
	virtual Qwt3D::ColorVector& createVector(Qwt3D::ColorVector& vec) const { return vec; }
	virtual std::vector<double> colorStops() const {return std::vector<double>();}
	void destroy(){delete this;};

protected:
	virtual ~Color(){} //!< Allow heap based objects only
};

class Curve;
//! Standard color model for Curve - implements the data driven operator()(double x, double y, double z)
/*!
The class has a ColorVector representing z values, which will be used by operator()(double x, double y, double z)
*/
class QWT3D_EXPORT StandardColor : public Color
{
public:
	//! Initializes with data and set up a ColorVector with a size of 100 z values (default);
	explicit StandardColor(Qwt3D::Curve* data, unsigned size = 100);
	Qwt3D::RGBA operator()(double x, double y, double z) const; //!< Receives z-dependend color from ColorVector
	void setColorVector(Qwt3D::ColorVector const& cv);
	void reset(unsigned size = 100); //!< Resets the standard colors;
	void setAlpha(double a); //!< Sets unitary alpha value for all colors
	/** 
		\brief Creates color vector
		
		Creates a color vector used by ColorLegend. This is essentially a copy from the internal used vector.
		\return The vector created
	*/
	virtual Qwt3D::ColorVector& createVector(Qwt3D::ColorVector& vec) const {vec = colors_; return vec;}

protected:
	~StandardColor(){colors_.clear();}
	Qwt3D::ColorVector colors_;
	Qwt3D::Curve* data_;
};

} // ns

#endif
