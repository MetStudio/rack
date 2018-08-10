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
#include <drain/util/Log.h>
#include <drain/util/FunctorPack.h>
#include <drain/util/Time.h>  // decayTime
#include <drain/util/Variable.h>
#include <drain/image/AccumulationMethods.h>
#include <drain/image/File.h>
#include <drain/imageops/DistanceTransformFillOp.h>
#include <drain/imageops/RecursiveRepairerOp.h>


#include "DataCoder.h"

using namespace drain;
using namespace drain::image;

namespace rack
{

double DataCoder::undetectQualityCoeff(0.75);

void DataCoder::init(){

	drain::Logger mout("DataCoder", __FUNCTION__);

	minCodeValue = dataODIM.getMin(); //drain::Type::getMin<double>(dataODIM.type); // consider embed in ODIM
	undetectValue = -std::numeric_limits<double>::max();

	detectionThreshold = undetectValue; // NEW 2018


	if (DataCoder::undetectQualityCoeff > 0.0){

		const Quantity &q = getQuantityMap().get(dataODIM.quantity);
		if (q.hasUndetectValue){
			mout.info() << "using quantity-specific zero for undetectValue: " << q.undetectValue << " (quantity="<< dataODIM.quantity << ")" <<  mout.endl;
			undetectValue = q.undetectValue;
			detectionThreshold = undetectValue+ 0.0001;

			/// This has caused (and solved) problems? In compositing, zero = -32 dBZH may undeflow.
			//if (undetectValue < dataODIM.getMin()){
			if (detectionThreshold < dataODIM.getMin()){
				//mout.debug(1) << "undetectValue(" << undetectValue << ") smaller than odim.getMin(): "  << dataODIM.getMin() << mout.endl;
				// mout.warn() << "adjusting undetectValue(" << undetectValue << ") up to minimum supported value: "  << dataODIM.getMin() << mout.endl;
				// undetectValue = dataODIM.getMin(); // This should not happen in accumulating. Say composite has min=0.0, but data_min=0.5 ?
				// NOTE: undetectValue should not be tuned according to data, but to the host resource, like composite
				mout.debug() << "tuning detectionThreshold " << detectionThreshold << ") to odim.getMin(): "  << dataODIM.getMin() << mout.endl;
				detectionThreshold = dataODIM.getMin();
			}

		}
		else { //  no undetectValue, but undetectQualityCoeff>0
			//mout.note() << "using default (storage type min) for undetectValue:" << converter.undetectValue << mout.endl;
			mout.info() << "skipping 'undetect' values (like --undetectWeight 0), consider: " ;
			//mout.warn() << "not using obsolete 'undetectValue' (" << undetectValue << "), set undetectValue value instead with:";
			mout << "--quantityConf " << dataODIM.quantity << ",zero=<value>" <<mout.endl; //  << ':' << srcData.odim.type
			//converter.undetectValue = undetectValue;
			//undetectValue = -std::numeric_limits<double>::max();
			//undetectValue = 0.0;
		}

	}

	//detectionThreshold = undetectValue+ 0.0001;

	mout.info() << " detectionThreshold: " << detectionThreshold << mout.endl;

}


/// Converts storage data containing marker codes etc to natural scale.
bool DataCoder::decode(double & value) const {

	if (value == dataODIM.nodata){
		return false; //   = "don't accumulate"
	}
	else if (value == dataODIM.undetect){
		value  = undetectValue;
		return true;
	}
	else {
		value = dataODIM.scaleForward(value);
		return true;
	}

}


/// Converts storage data containing marker codes etc to natural scale.
/**
 *   Note: handling \c undetect is decided outside, ie. if count > 0 but weight = 0. (In that case this function is probably not called.)
 */
bool DataCoder::decode(double & value, double & weight) const {

	if (value == dataODIM.nodata){
		weight = 0.0;
		return false; //   = "don't accumulate"
	}
	else if (value == dataODIM.undetect){ // huono: (weight < 0.001)
	//else if ((value > (dataODIM.undetect-0.01)) && (value < (dataODIM.undetect+0.01))){ // huono: (weight < 0.001)
		weight = undetectQualityCoeff * qualityODIM.scaleForward(weight); // may (still) be undetectValue
		value  = undetectValue;
		return true;  // TODO: this feature (true/false) could be user-modifiable? (Problem: nodata-areas in result)
	}
	else { // default
		value  = dataODIM.scaleForward(value);
		weight = qualityODIM.scaleForward(weight);
		return true;
	}

}

/// Converts natural-scale data to storage data containing marker codes etc.
void DataCoder::encode(double & value, double & weight) const {

	/*
	if (value < undetectValue){ // NEW < : <=
		value = dataODIM.undetect;
	}
	else if (weight == 0.0){
		value = dataODIM.nodata;
	}
	 */

	if (weight <= 0.0){
		//throw std::runtime_error("DataCoder::encode(double & value, double & weight) , weight <= 0.0");
		value = dataODIM.nodata;
	}
	else if (value <= detectionThreshold) { // NEW IMPORTANT (but could be < instead <= ?)
		value = dataODIM.undetect;
	}
	else {
		value = dataODIM.scaleInverse(value);
		if (value <= minCodeValue) { // NEW;  consider: embed in scaleInverse?
			value = dataODIM.undetect;
		}
	}
	// TODO LIMIT?
	// if (value < undetectValue)  NEW
	//	value = dataODIM.undetect;  // NEW

}

void DataCoder::encodeWeight(double & weight) const {
	weight = qualityODIM.scaleInverse(weight);
}





}


// Rack
