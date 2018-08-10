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

#ifndef BioMetOP2_H_
#define BioMetOP2_H_

#include "DetectorOp.h"

//#include <drain/image/SegmentAreaOp.h>
#include <drain/util/Fuzzy.h>
//#include <drain/image/MathOpPack.h>
//#include <drain/image/File.h>


using namespace drain::image;



namespace rack {

/// Detects birds and insects.
/**
 *
 *
 */
class BiometOp: public DetectorOp {

public:

	/**
	 * \param reflMax - maximum expected reflectance of biometeors
	 * \param maxAltitude - maximum expected altitude of biometeors
	 * \param reflDev - fuzzy width of  \c reflMax
	 * \param devAltitude - fuzzy width of \c maxAltitude
	 */
	BiometOp(double reflMax=-10.0, int maxAltitude=500, double reflDev=5.0, int devAltitude=1000) :
		DetectorOp(__FUNCTION__, "Detects birds and insects.", ECHO_CLASS_INSECT){

		parameters.reference("reflMax", this->reflMax = reflMax, "dBZ");
		parameters.reference("maxAltitude", this->maxAltitude = maxAltitude, "m");
		parameters.reference("reflDev", this->reflDev = reflDev, "dBZ");
		parameters.reference("devAltitude", this->devAltitude = devAltitude, "m");
		dataSelector.quantity = "DBZH$";
		REQUIRE_STANDARD_DATA = false;
	};

	/// Threshold for reflectance Z.
	double reflMax;

	/// Fuzzy deviation of minDBZ reflectancy.
	double reflDev;

	/// Maximum height of biometeors.
	int maxAltitude;

	/// Fuzzy deviation of maximum height of biometeors.
	int devAltitude;

protected:

	virtual
	void processData(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dst) const;  // DataSetDst & dst) const;

};



}

#endif /* BIOMET_OP_H_ */

// Rack
