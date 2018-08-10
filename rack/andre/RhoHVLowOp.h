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
#ifndef RHOHV3_OP2_H_
#define RHOHV3_OP2_H_

#include "DetectorOp.h"

using namespace drain::image;

namespace rack {

///	Marks bins with low \em RhoHV value as probable anomalies.
/**
 *
 */
class RhoHVLowOp: public DetectorOp {

public:

	///	Default constructor.
	/**
	 *  \param threshold - \c RhoHV values below this will be considered this anomalies
	 *  \param thresholdWidth - steepness coefficient for the threshold
	 *  \param windowWidth  - optional: median filtering window width in metres
	 *  \param windowHeight - optional: median filtering height width in degrees
	 *  \param medianPos    - optional: median position: 0.5 = normal median ( >0.5: conservative for precip)
	 *  //(morphology: 0.5 > opening; 0.5 < closing)
	 *
	 *  This operator is \e universal , it is computed on DBZ but it applies also to other radar parameters measured (VRAD etc)
	 */
	RhoHVLowOp(double threshold=0.85, double thresholdWidth=0.1, double windowWidth=0.0, double windowHeight=0.0, double medianPos=0.95) :
		DetectorOp("RhoHVLow","Detects clutter. Based on dual-pol parameter RhoHV . Optional postprocessing: morphological closing. Universal.", 131){
		dataSelector.quantity = "RHOHV";
		REQUIRE_STANDARD_DATA = false;
		UNIVERSAL = true;
		parameters.reference("threshold", this->threshold = threshold, "0...1");
		parameters.reference("thresholdWidth", this->thresholdWidth = thresholdWidth, "0...1");
		parameters.reference("windowWidth", this->windowWidth = windowWidth, "metres");
		parameters.reference("windowHeight", this->windowHeight = windowHeight, "degrees");
		parameters.reference("medianPos", this->medianPos = medianPos, "0...1");
		//parameters.reference("area", this->area, area);
	};

	double threshold;
	double thresholdWidth;

	double windowWidth;
	double windowHeight;
	double medianPos;

protected:

	virtual
	void processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const;
	//void filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const;

};


}

#endif /* POLARTOCARTESIANOP_H_ */
// koe

// Rack
