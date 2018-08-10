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
#include <stdexcept>

#include <drain/util/Variable.h>
//#include <drain/util/Fuzzy.h>
//#include <drain/image/AccumulationMethods.h>
//#include "RackOp.h"

//#include "data/Conversion.h"

//#include "radar/Extractor.h"
#include "PolarProductOp.h"



namespace rack {

using namespace drain::image;

const CoordinatePolicy PolarProductOp::polarCoordPolicy(CoordinatePolicy::POLAR, CoordinatePolicy::WRAP, CoordinatePolicy::LIMIT,CoordinatePolicy::WRAP);

void PolarProductOp::deriveDstGeometry(const DataSetMap<PolarSrc> & srcSweeps, PolarODIM & dstOdim) const {

	drain::Logger mout(name+"(CumulativeProductOp)", __FUNCTION__);

	const bool MAXIMIZE_AZM_RESOLUTION = (dstOdim.nrays == 0);
	const bool DERIVE_NBINS  = (dstOdim.nbins == 0); // ||(dstOdim.rscale == 0.0);
	const bool DERIVE_RSCALE = (dstOdim.rscale == 0.0); // (dstOdim.nbins == 0); //

	const bool AUTOSCALE_RANGE = (DERIVE_NBINS && DERIVE_RSCALE);

	double   rangeMax = 0;
	long int nbinsMax = 0;
	double  rscaleMin = 2000;

	mout.debug(1) << (dstOdim) << mout.endl;

	if (MAXIMIZE_AZM_RESOLUTION || DERIVE_NBINS || DERIVE_RSCALE){

		double range;

		for (DataSetMap<PolarSrc>::const_iterator it = srcSweeps.begin(); it != srcSweeps.end(); ++it){

			const DataSet<PolarSrc> & srcDataSet = it->second;
			const Data<PolarSrc>       & srcData    = srcDataSet.getFirstData();
			const PolarODIM            & srcOdim    = srcData.odim;

			if (srcData.data.isEmpty()){
				mout.warn() << "srcData empty" << mout.endl;
				continue; // warning issued later, in the main loop.
				//mout.warn() << "selected quantity=" << quantity << " not present in elangle=" << it->first << mout.endl;
			}

			mout.debug(1) << "testing: " << srcOdim << mout.endl;

			if (MAXIMIZE_AZM_RESOLUTION){
				if (srcOdim.nrays > dstOdim.nrays){
					dstOdim.nrays = srcOdim.nrays;
					mout.info() << "Updating dst nrays to: " << dstOdim.nrays << mout.endl;
				}
			}

			nbinsMax  = std::max(nbinsMax,  srcOdim.nbins);
			rscaleMin = std::min(rscaleMin, srcOdim.rscale);
			range = static_cast<double>(srcOdim.nbins) * srcOdim.rscale;
			if (range > rangeMax){
				rangeMax = range;
				if (AUTOSCALE_RANGE){
					dstOdim.nbins  = srcOdim.nbins;
					dstOdim.rscale = srcOdim.rscale;
				}
			}

		}

		if (AUTOSCALE_RANGE){
			mout.debug() << "Applied input geometry with maximum range" << mout.endl;
		}
		else if (dstOdim.nbins ==0){
			dstOdim.nbins = rangeMax/static_cast<int>(dstOdim.rscale);
			mout.debug() << "Derived nbins=" << dstOdim.nbins << mout.endl;
		}
		else if (dstOdim.rscale ==0){
			dstOdim.rscale = rangeMax/static_cast<double>(dstOdim.nbins);
			mout.debug() << "Derived rscale=" << dstOdim.rscale << mout.endl;
		}
		else {
			mout.debug() << "Adapting user-defined nbins and rscale" << mout.endl;
		}

		mout.info() << "Setting dst geometry:" << dstOdim.nbins << "bin x " << dstOdim.rscale << "m/bin (" << (dstOdim.getMaxRange()/1000.0) << "km) " << mout.endl;

	}
	else {
		//mout.warn() << "output prop" << dstOdim << mout.endl;
		mout.info() << "output properties: " << EncodingODIM(dstOdim) << mout.endl;
	}
}




}  // namespace rack

// Rack
