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

#ifndef ANDRE_DETECTOR2_H_
#define ANDRE_DETECTOR2_H_


//#include <drain/image/MathOpPack.h>

//#include "product/VolumeOp.h"
#include "AndreOp.h"
#include "radar/EchoClasses.h"

namespace rack {

using namespace drain::image;


/// Base class for anomaly detectors.
/**
 *
 */
class DetectorOp : public AndreOp {

public:

	/// If true, specific detection results will be stored
	static
	bool STORE;

	DetectorOp(const std::string & name = __FUNCTION__, const std::string &description = "", unsigned short code = 0) : AndreOp(name,description), classCode(code ? code : 128+(++_count)) {

		dataSelector.path = ".*/data[0-9]+/?$";
		//dataSelector.quantity = "DBZ.*";
		dataSelector.quantity = "^DBZH$";
		//cumulateDetections = MAX;
		REQUIRE_STANDARD_DATA = true;
		UNIVERSAL = false;

	}

	virtual
	~DetectorOp(){};

	/// Returns the quantity name, by default the name of the class in upper case letters.
	virtual
	const std::string & getQuantityName() const;

	static bool SUPPORT_UNIVERSAL;

	/// Index applied in the legend of the classification results
	const unsigned short int classCode;


	/// NEW POLICY => DetectorOpNEW
	virtual
	void processDataSets(const DataSetMap<PolarSrc> & srcVolume, DataSetMap<PolarDst> & dstVolume) const;

	/// Process as sweep (data in one elevation angle)
	/**
	 *  \param srcDataSet - input data of one elevation; possibly several quantities (measurement parameters).
	 *  \param dstProb    - probability field ie. the result of the detection algorithm
	 *  \param aux        - auxialiary DatasetDst for keeping a copy of normalized data.
	 */
	virtual
	void processDataSet(const DataSet<PolarSrc> & srcDataSet, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & aux)  const;

	/// Process using single data only (no quality "involved", because it is created here...)
	/**
	 *  \param srcData - input data of one elevation; possibly several quantities (measurement parameters).
	 *  \param dstProb - output data, typically in the same dataset as srcData.
	 */
	virtual
	void processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const {
		drain::Logger mout(name, __FUNCTION__);
		mout.warn() << "not implemented" << mout.endl;
	}

	//const unsigned short int CODE;

protected:

	virtual
	void initDataDst(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, const std::string & quantity = "") const;
	//void initDataDst(PlainData<PolarDst> & dstData, const std::string & quantity, const ODIM & srcODIM);


	// This is difficult...
	//const HI5TREE & getNormalizedData(const DataSet<> & srcDataSet, DataSet<> & dstDataSet, const std::string & quantity) const;

	// Consider raise to VolumeOp ?
	void storeDebugData(int debugLevel, const ImageFrame & srcImage, const std::string & label) const;

	// Consider raise
	void storeDebugData(const ImageFrame & srcImage, PlainData<PolarDst> & dstData, const std::string & quantityLabel) const;


	/// After running a cmd, write execution details.
	virtual
	void writeHow(PlainData<PolarDst> & dstData) const;

	// Detector classCode. Obsolete...
	static
	unsigned short int _count;

	mutable std::string upperCaseName;

	/// Set to true if operator expects fixed background intensities instead of "nodata" defined by the PolarODIM. Affects getValidData().
	/**
	 *  Some AnDRe detectors require harmonized input data. If source is unsuitably scaled, a converted copy of the data is used.
	 *  If a derived operator (detector) requires standardized source data, this function creates a converted copy.
	 *  Otherwise, the original data is returned.
	 *  The copy will be stored in dst for subsequent detectors to retrieve.
	 *  \return "data" or "data"
	 */
	bool REQUIRE_STANDARD_DATA;

	/// If true, applies also to quantities other than the one used in detection. The detection and the accumulation will be stored one step upwards.
	bool UNIVERSAL;

	/// Enhances the detection result by reinforcing sectors of strong response, attenuating others. Optional utility for derived classes.
	/**
	 *  \param data - detection field to be enhanced
	 *  \param medianPos - [0,1] normalised position of median (0.5 = conventional median).
	 *  \param windowWidth - azimuthal width in pixels (bins); the calling function should scale this.
	 */
	void _enhanceDirectionally(Image & data, float medianPos, int windowWidth) const;

	void _infect(Image & data, int windowWidth, int windowHeight, double enhancement) const;


};


}  // rack::

#endif /* ANDRE_OP_H_ */

// Rack
