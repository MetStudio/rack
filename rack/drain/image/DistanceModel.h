/*

MIT License

Copyright (c) 2017 FMI Open Development / Markus Peura, first.last@fmi.fi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
/*
Part of Rack development has been done in the BALTRAD projects part-financed
by the European Union (European Regional Development Fund and European
Neighbourhood Partnership Instrument, Baltic Sea Region Programme 2007-2013)
*/
#ifndef DISTANCEMODEL_H_
#define DISTANCEMODEL_H_



#include <math.h>

#include "util/BeanLike.h"

#include "imageops/ImageOp.h"


namespace drain
{

namespace image
{


class DistanceModel : public BeanLike {
	
public:

	typedef float dist_t;

	virtual ~DistanceModel(){};

	/// Set maximum (expected) code value. Radii, if given, will set pixel-to-pixel decrements scaled to this value.
	inline
	void setMax(dist_t maxCodeValue){ this->maxCodeValue = maxCodeValue; };

	/// Returns the maximum (expected) code value.
	inline
	dist_t getMax() const { return maxCodeValue; };

	// virtual
	//void getRadius(dist_t & horz, dist_t & vert) const = 0;

	/// Sets the geometry of the distance model.
	/**
	 * 	The values are in float, as eg. half-width radius may be sharp, under 1.0.
	 *   \param horz - horizontal distance scale, "width".
	 *   \param vert - vertical radius, "height"; if negative, set equal to horizontal
	 *
	 *   Diagonal (dx+dy) and "knight" (2dx+dy or dx+2dy) values will be adjusted as well.
	 *
	 *   Special values:
	 *   - negative value: spread to infinity (ie. identified with positive infinity)
	 *   - zero: sharp decay, value is zero in neighboring pixels
	 *   - in addition: if vert is NaN, value of horz is copied.
	 *
	 */
	virtual 
	void setRadius(dist_t horz, dist_t vert = NAN) = 0; //, bool diag=true, bool knight=true) = 0;
	
	/// Sets the distance geometry directly by modifying decrement/decay coefficients. Alternative to setRadii().
	///
	/**
	 *
	 *   \param horz - horizontal distance decrement/decay
	 *   \param vert - vertical distance decrement/decay; if negative, set equal to horizontal
	 *
	 *   Diagonal (dx+dy) and "knight" (2dx+dy or dx+2dy) values will be adjusted as well.
	 */
	virtual 
	void setDecrement(dist_t horz, dist_t vert = NAN) = 0; // , bool diag=true, bool knight=true) = 0;

	inline
	void init(){
		setTopology(topology);
		setRadius(width, height);
	}

	/// Sets the topology of the computation grid: 0=diamond, 1=diagonal, 2=extended (chess knight steps)
	virtual
	void setTopology(unsigned short topology){

		this->topology = topology;
		KNIGHT = false;
		DIAG   = false;

		switch (topology) {
			case 2:
				KNIGHT = true;
				// no break
			case 1:
				DIAG   = true;
				// no break
			case 0:
				break;
			default:
				break;
		}

	};


	/// Decreases value by horizonal decay
	//virtual
	//dist_t decrease(dist_t coeff, dist_t x) const = 0;

	/// Decreases value by horizonal decay
	virtual
	dist_t decreaseHorz(dist_t x) const = 0;

	/// Decreases value by vertical decay
	virtual 
	dist_t decreaseVert(dist_t x) const = 0;

	/// Decreases value by diagonal decay
	virtual
	dist_t decreaseDiag(dist_t x) const = 0;

	/// Decreases value by a chess knight step (2-up, 1-left) decay
	virtual 
	dist_t decreaseKnightHorz(dist_t x) const = 0;

	/// Decreases value by a chess knight step (2-up, 1-left) decay
	virtual
	dist_t decreaseKnightVert(dist_t x) const = 0;

	/// Diagnonal +1/-1 steps on/off.
	bool DIAG;

	/// Chess knight move computation on/off.
	bool KNIGHT;

	unsigned short int topology; // NEEDED, separately?

protected:

	/// Base class for digital distance models. Supports 4-, 8- and extended "chess knight" distance.
	/**
	 *  Sets the horizontal and vertical radius for the distance function.
	 *  The parameters are called "width" and "height"
	 *
	 *  By default, the geometry is octagonal, applying 8-distance.
	*/
	DistanceModel(const std::string & name, const std::string & description = "") : BeanLike(name, description), DIAG(true), KNIGHT(true) {
		parameters.reference("width",  width=10.0,  "pix");
		parameters.reference("height", height=-1.0, "pix");
		parameters.reference("topology", topology=2, "0|1|2");
		setMax(255); // warning
	};


	// Internal parameter applied upon initParams?
	dist_t width;

	// Internal parameter applied upon initParams
	dist_t height;


	/// Needed internally to get diag decrement larger than horz/vert decrements. (Not used for scaling).
	dist_t maxCodeValue;


};


class DistanceModelLinear : public DistanceModel
{

public:

	DistanceModelLinear() : DistanceModel(__FUNCTION__){
		setRadius(width, height);
	};

	void setRadius(dist_t horz, dist_t vert = NAN); // bool diag=true, bool knight=true);

	void setDecrement(dist_t horz, dist_t vert = NAN); // bool diag=true, bool knight=true);


	/*
	virtual
	dist_t decrease(dist_t coeff, dist_t x) const {
		if (x >= coeff)
			return x-coeff;
		else
			return 0;
	};
	*/

	inline
	dist_t decreaseHorz(dist_t x) const {
		if (x > horzDecrement)
			return x-horzDecrement;
		else
			return 0;
	};

	inline
	dist_t decreaseVert(dist_t x) const {
		if (x > vertDecrement)
			return x-vertDecrement;
		else
			return 0;
	};

	inline
	dist_t decreaseDiag(dist_t x) const {
		if (x > diagDecrement)
			return x-diagDecrement;
		else
			return 0;
	};

	inline
	dist_t decreaseKnightHorz(dist_t x) const {
		if (x > knightDecrementHorz)
			return x-knightDecrementHorz;
		else
			return 0;
	};

	inline
	dist_t decreaseKnightVert(dist_t x) const {
		if (x > knightDecrementVert)
			return x-knightDecrementVert;
		else
			return 0;
	};

private:
	
	//T max;
	dist_t horzDecrement;
	dist_t vertDecrement;
	dist_t diagDecrement;
	dist_t knightDecrementHorz;
	dist_t knightDecrementVert;
	
};

/**
 *  In this model, the diagonal decay is computed as
 *  \$ d_d = \log - \sqrt { \log^2 d_x + \log^2 d_x)) } ;
 *  \$
 *  because
 *  \$ d_d = \log - \sqrt { \log^2 d_x + \log^2 d_x)) } ;
 *  \$
 */
class DistanceModelExponential : public DistanceModel {

public:

	DistanceModelExponential() : DistanceModel(__FUNCTION__){
		setRadius(width, height);
	};

	void setRadius(dist_t horz,    dist_t vert = NAN);

	void setDecrement(dist_t horz, dist_t vert = NAN);

	inline
	dist_t decreaseHorz(dist_t x) const {
		return static_cast<dist_t>(horzDecay*x);
	};

	inline
	dist_t decreaseVert(dist_t x) const {
		return static_cast<dist_t>(vertDecay*x);
	};

	inline
	dist_t decreaseDiag(dist_t x) const {
		return (diagDecay*x);
	};

	inline
	dist_t decreaseKnightHorz(dist_t x) const {
		return (knightDecayHorz*x);
	};

	inline
	dist_t decreaseKnightVert(dist_t x) const {
		return (knightDecayVert*x);
	};

private:
	
	dist_t horzDecay;
    dist_t vertDecay;
    dist_t diagDecay;
    dist_t knightDecayHorz;
    dist_t knightDecayVert;

};



}
}
	
#endif /*DISTANCETRANSFORMOP_H_*/

// Drain
