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
#include "MaxEchoOp.h"
// #include <drain/image/File.h>

#include <drain/util/Fuzzy.h>

//#include "data/Conversion.h"
#include "radar/Constants.h"


namespace rack
{

using namespace drain::image;

//processSweep(data, cumulator);
void MaxEchoOp::processData(const Data<PolarSrc> & sweep, RadarAccumulator<Accumulator,PolarODIM> & accumulator) const {
	// void MaxEchoOp::filter(const HI5TREE &src, const std::map<double,std::string> & srcPaths, HI5TREE &dst) const {

	drain::Logger mout(drain::getLog(), getName(), __FUNCTION__);
	mout.debug(2) << "Starting MaxEchoOp (" << name << ") " << mout.endl;
	mout.debug(3) << (const drain::image::Accumulator &) accumulator << mout.endl;

	const PlainData<PolarSrc> & srcQuality = sweep.getQualityData();
	const bool WEIGHTED = !srcQuality.data.isEmpty();

	DataCoder converter(sweep.odim, srcQuality.odim);


	mout.info() << "Using quality data: " << (WEIGHTED?"YES":"NO") << mout.endl;


	// Elevation angle
	const double eta = sweep.odim.getElangleR();

	// A fuzzy beam power model.
	//drain::FuzzyPeak<double,double> beamPower(0.0, 0.25*DEG2RAD, 1.0);
	drain::FuzzyBell<double> altitudeQuality(altitude, devAltitude, 1.0);

	/// Ground angle
	double beta;

	/// Elevation angle of a bin.
	//double etaBin;
	double altitudeBin;

	// Bin distance along the beam.
	double binDistance;

	// Source value coordinate?
	int iSweep;

	// Source y coordinate?
	int jSweep;

	/// Measurement, first encoded and then decoded
	double value;

	/// Measurement, decoded
	// double dbz;

	/// Beam weight
	double weight=1.0;

	/// Measurement weight (quality)
	double w;

	/// Direct pixel address in the accumulation arrey.
	size_t address;

	for (size_t i = 0; i < accumulator.getWidth(); ++i) {

		// Ground angle
		beta = accumulator.odim.getGroundAngle(i);

		altitudeBin = Geometry::heightFromEtaBeta(eta, beta);
		weight = altitudeQuality(altitudeBin);
		//if ()
		//weight = 0.7654321;
		//weight = weight*weight;

		binDistance = Geometry::beamFromEtaBeta(eta, beta);
		iSweep = static_cast<int>(binDistance/sweep.odim.rscale + 0.5);

		// TODO: derive iStart and iEnd instead.

		if ((binDistance >= sweep.odim.rstart) && (iSweep < sweep.odim.nbins)){

			for (size_t j = 0; j < accumulator.getHeight(); ++j) {

				jSweep = (j * sweep.odim.nrays) / accumulator.getHeight();

				value = sweep.data.get<double>(iSweep,jSweep);

				//if (i==j) std::cerr << " MaxEcho(" << value << ")\t";

				if (converter.decode(value)){
					address = accumulator.data.address(i,j);
					if (WEIGHTED){
						w = weight * srcQuality.odim.scaleForward(srcQuality.data.get<double>(iSweep,jSweep));
						accumulator.add(address, value, w);
					}
					else {
						//accumulator.add(address, value, weight);
						accumulator.add(address, value, weight);
						w = weight; // debug
					}

				}

			}

		}
		//else
			//mout.warn() << "skipping range b=" << binDistance << " i1=" << iSweep << mout.endl;

	}

	//_mout.writeImage(11, dstData, "max");
}



}





// Rack
