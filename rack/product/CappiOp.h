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
#ifndef CAPPI2_OP_H_
#define CAPPI2_OP_H_

//#include <algorithm>



//#include "RackOp.h"
#include "CumulativeProductOp.h"

namespace rack
{

using namespace drain::image;


/// Pseudo-CAPPI: the constant altitude planar position indicator product
/**
 */
class CappiOp : public CumulativeProductOp {

public:

	CappiOp(double altitude=1500.0, double weightMin = 0.01, double weightExponent=8.0, bool aboveSeaLevel=true) :
		CumulativeProductOp(__FUNCTION__, "Constant-altitude planar position indicator", "WAVG,1,8,-40")
		{

		parameters.reference("altitude", this->altitude = altitude, "metres");
		parameters.reference("weightMin", this->weightMin = weightMin, "scalar");
		parameters.reference("weightExponent", this->weightExponent = weightExponent, "scalar");
		parameters.reference("aboveSeaLevel", this->aboveSeaLevel = aboveSeaLevel, "0|1");

		odim.product = "PCAPPI";
		dataSelector.quantity = "^DBZH$";

	};

	double altitude;
	double weightExponent;
	double weightMin;

	void processData(const Data<PolarSrc> & data, RadarAccumulator<Accumulator,PolarODIM> & accumulator) const;

protected:


};





}  // ::rack

#endif /*POLARMax_H_*/

// Rack
