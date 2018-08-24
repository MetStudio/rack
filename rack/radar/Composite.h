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
#ifndef COMPOSITE2_H_
#define COMPOSITE2_H_

#include <drain/image/AccumulationConverter.h>

#include <drain/image/AccumulatorGeo.h>
#include <drain/util/Rectangle.h>

#include "hi5/Hi5.h"
#include "data/CartesianODIM.h"
#include "data/Data.h"
//#include "data/DataSelector.h"
//#include "Coordinates.h"

#include "RadarAccumulator.h"


namespace rack
{

using namespace drain::image;





// Defines a region within a main composite.
/**
 *
 */
// OLD: An injective, cumulative radar image compositing.
/*
  Main features:


  Cumulation. The compositing starts by initiating the target image,
  see open().
  Each input radar image is projected on the target image cumulatively,
  see execute(). Finally, the cumulation must be completed by
  normalizing the result with the number/weight of
  cumulated inputs, see close().
  Hence, the target which should be seen.

  The weights are stored in the first alphaChannel (index 0) of the
  target image. If the target has a second image channel, also
  the (weighted) squared values are cumulated.

  In cumulation, ...
  \f[
  x = \sqrt[p]{\frac{\sum_i {q_i}^r x_i^p}{\sum_i {q_i}^r } }
  \f]

  The default type of target image, that is, cumulation array, is
  double. If no (float valued) weighting  is applied, target image
  could be long int as well.

\section specials Handling special codes undetect and nodata

The implementation behind a composite is based on drain::Cumulator class, which essentially consists of four 2D arrays:
accumulated data (\f$\sum w_ix_i\f$), weights (\f$\sum w_i\f$), squared sum (\f$\sum w_ix_i^2\f$) and count of samples (\f$N\f$).

For radar data ODIM standard defines two essential special codes, \c undetect and \c nodata, which above all mark samples that are
beyond the sensitivity or the range of a radar, respectively. \c nodata is also applied in marking samples where data has been rejected
due to noise and other anomalies. These codes imply adding special rules to the basically "mathematical" accumulation.

In the accumulation stage, each valid data sample with data value \f$ x_i \f$ and its weight \f$ w_i \f$ is accumulated to the arrays by calling Cumulator::add(), with the following std::exceptions:

- if  \f$ x_i = \f$ \c undetect, just increment the count \f$N\f$ (which is equal to accumulating a sample with \f$ w=0 \f$)
- if  \f$ x_i = \f$ \c nodata,    skip the sample, leaving accumulation arrays intact

In the extraction stage, a result with data value \f$ x_i \f$ and weight \f$ w_i \f$ are extracted at each data point according
to compositing principle (AVG, MAX,...) with the following special cases:

- if  \f$c_i = 0\f$ and \f$w_i = 0.0\f$, return result with \f$ x_i = \f$ \c nodata and \f$ w_i = 0.0 \f$
- if  \f$c_i > 0\f$ and \f$w_i = 0.0\f$, return result with \f$ x_i = \f$ \c undetect and \f$ w_i = 0.0 \f$





 */

/// Cartesian composite (mosaic) of data from several radars.
/*
 *
 */
class Composite : public RadarAccumulator<drain::image::AccumulatorGeo,CartesianODIM> ///
{
public:


	Composite(); /// todo add parameters?
	// TODO: what:number-of-radars
	// TODO: date, timeq

	virtual ~Composite(){};


	/// If cropping is set, calling addPolar() also crops the bounding box to intersection of radar area and original area.
	/**
	 *    Useful in tiling.
	 *
	 */
	inline
	void setCropping(bool cropping = true){ this->cropping = cropping; };

	/// Sets quantity. Warns if input has a different quantity.
	void checkQuantity(const std::string & quantity);


	/// Projects radar data in polar coordinates to Cartesian coordinates.
	/** If the composite does not already have a bounding box, it will be initialized as
	 *  azimuthal equidistant projection matched to the composite.
	 *  \param src - input data structure
	 *  \param srcQuality -
	 *  \param weight -
	 *  \param autoProj - use AEQD centered at the radar location.
	 */
	void addPolar(const PlainData<PolarSrc> & src, const PlainData<PolarSrc> & srcQuality, double weight, bool autoProj);

	/// Add data that matches the Cartesian projection and scaling of the composite.
	/** If the composite does not already have a bounding box, it will be equal to input data.
	 *
	 *  \param src - input data structure
	 *  \param weight -
	 *  \param autoProj - use AEQD centered at the radar location.
	 */
	void addCartesian(const PlainData<CartesianSrc> & src, const PlainData<CartesianSrc> & srcQuality, double weight = 1.0, int i=0, int j=0);


	/// Weight decrease (0.0...1.0), per minute, in the weighting of delayed data. Zero means no change in weighting.
	double decay;

	double getTimeDifferenceMinute(const CartesianODIM & odimIn) const;

	/// Updates internal metadata.
	//  Should to be called after addData(). Called implicitly by addPolar.
	//void updateMetadata(const ODIM & odimSrc);

	/// Updates xscale, rscale and secondary Bounding Box attributes.
	/**
	 *   Secondary BBOX means the minimum BBOX spanned by the data only.
	 */
	void updateGeoData();

	//std::map<std::string, drain::Variable> nodeMap;
	drain::VariableMap nodeMap;

	/// Range [km] for single-radar products. Typically 250 km.
	int defaultRange;

protected:

	void updateNodeMap(const std::string & node, int i, int j);


	bool cropping;



};

} // rack::

#endif /*COMPOSITE_H_*/

// Rack
