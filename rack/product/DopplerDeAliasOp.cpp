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

#include <cmath>

#include <drain/util/FunctorBank.h>
#include <drain/util/Fuzzy.h>
#include <drain/util/Geo.h>

#include <drain/image/File.h>
#include <drain/imageops/SlidingWindowOp.h>


#include "radar/Geometry.h"
#include "radar/Doppler.h"
#include "radar/PolarSmoother.h"

#include "DopplerDeAliasWindow.h"

#include "DopplerDeAliasOp.h"





namespace rack {


void DopplerWindOp::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(name, __FUNCTION__);

	const Data<PolarSrc> & srcData = srcSweep.getFirstData(); // VRAD or VRADH

	if (srcData.data.isEmpty()){
		// Actually this should be in higher level
		mout.warn() << "data empty" << mout.endl;
		return;
	}

	//PolarODIM srcData.odim(srcData.odim);  // NI now correct
	//const PolarODIM & srcODIM = srcData.odim;  // NI now correct

	/*if (srcData.odim.NI == 0.0){
		mout.warn() << "NI (Nyquist interval) zero or not found." << mout.endl;

	}*/

	//dstProduct.odim.prodpar = getParameters().getKeys();

	PlainData<PolarDst> & dstDataU   = dstProduct.getData("AMVU");
	dstDataU.data.clear();
	PlainData<PolarDst> & dstDataV   = dstProduct.getData("AMVV");
	dstDataV.data.clear();
	PlainData<PolarDst> & dstQuality = dstProduct.getQualityData();
	dstQuality.data.clear();

	ProductBase::applyODIM(dstDataU.odim, odim, true);
	mout.debug(1) << "dstDataU.odim" << EncodingODIM(dstDataU.odim) << mout.endl;
	ProductBase::handleEncodingRequest(dstDataU.odim, encodingRequest);
	mout.debug(2) << "dstDataU.odim" << EncodingODIM(dstDataU.odim) << mout.endl;
	dstDataU.data.setType(dstDataU.odim.type);
	setGeometry(srcData.odim, dstDataU);
	mout.debug() << "dstDataU.odim" << EncodingODIM(dstDataU.odim) << mout.endl;

	ProductBase::applyODIM(dstDataV.odim, odim, true);
	ProductBase::handleEncodingRequest(dstDataV.odim, encodingRequest);
	dstDataV.data.setType(dstDataV.odim.type);
	setGeometry(srcData.odim, dstDataV);

	getQuantityMap().setQuantityDefaults(dstQuality, "QIND");
	setGeometry(srcData.odim, dstQuality);

	/*
	mout.warn() << "scr:  " << srcData << mout.endl;
	mout.warn() << "U:    " << dstDataU << mout.endl;
	mout.warn() << "V:    " << dstDataV << mout.endl;
	mout.warn() << "QIND: " << dstQuality << mout.endl;
	//mout.warn() << "VRADC" << dstDataVRAD << mout.endl;
	 */


	const DopplerDeAliasConfig conf(widthM, heightD);


	if (srcData.hasQuality()){
		DopplerDeAliasWindowWeighted window(conf, dstDataU.odim);
		mout.warn() << "WEIGHTED, not used yet" << mout.endl;
	}
	else {

	}

	DopplerDeAliasWindow window(conf, dstDataU.odim);

	window.conf.updatePixelSize(srcData.odim);
	//window.resetAtEdges = true;

	// window.signCos = +1; //(testSigns & 1) ? +1 : -1;
	// window.signSin = -1;(testSigns & 2) ? +1 : -1;

	//window.setSize(width, height);
	window.setSrcFrame(srcData.data);
	window.setDstFrame(dstDataU.data);
	window.setDstFrame2(dstDataV.data);
	window.setDstFrameWeight(dstQuality.data);

	window.functorSetup = altitudeWeight;

	// MAIN OPERATION !
	window.run();


	dstDataU.odim.prodpar = getParameters().getKeys();
	dstDataU.odim.update(srcData.odim); // date, time, etc

	dstDataU.data.properties.importMap(dstDataU.odim);
	dstDataV.data.properties.importMap(dstDataV.odim);


	//const QuantityMap & qm = getQuantityMap();

	/// Derived products and post-processing

	//drain::IdentityFunctor ident; RadarWindowConfig avgConf(ident, 2.0*widthM, 2.0*heightD);
	//RadarWindowConfig avgConf(2500, 11);


	//drain::getLog().setVerbosity(20);

	/*
	PlainData<PolarDst> & dstDataU2   = dstProduct.getData("AMVU2");
	dstDataU2.copyEncoding(dstDataU);
	dstDataU.data.setScaling(dstDataU.odim.gain, dstDataU.odim.offset);
	dstDataU2.setGeometry(dstDataU.data.getGeometry());
	//mout.warn() << "Src for PolarSmoother::filter: " << EncodingODIM(dstDataU2.odim) << mout.endl;
	//setGeometry(srcData.odim, dstDataU2);
	PolarSmoother::filter(dstDataU.odim, dstDataU.data, dstDataU2.data, 5500);
	//mout.note()  << dstDataU2 << mout.endl;



	RadarWindowConfig avgConf(5*widthM, 5*heightD);
	RadarWindowAvg<RadarWindowConfig> avgWindow(avgConf);
	avgWindow.conf.updatePixelSize(srcData.odim);
	PlainData<PolarDst> & dstDataV2   = dstProduct.getData("AMVV2");
	dstDataV2.copyEncoding(dstDataV);
	dstDataV2.setGeometry(dstDataV.data.getGeometry());
	drain::image::SlidingWindowOp<RadarWindowAvg<RadarWindowConfig> > avg2(avgConf);
	//mout.warn() << dstDataV2.data.getScaling() << mout.endl;
	//mout.warn() << dstDataV2.data.getChannel(0).getScaling() << mout.endl;
	mout.note() << "Calling (SlidingWindowOp)avg2.process" << mout.endl;
	avg2.process(dstDataV.data, dstDataV2.data);
	*/
	//mout.warn() << EncodingODIM(dstDataV2.odim) << mout.endl;

	/// If desired, run also new, dealiased VRAD field
	//nyquist
	if (nyquist != 0.0){

		dstDataU.odim.NI = nyquist; // used for dataset-level metadata

		PlainData<PolarDst> & dstDataVRAD = dstProduct.getData("VRAD"); // de-aliased ("re-aliased")

		const QuantityMap & qm = getQuantityMap();
		qm.setQuantityDefaults(dstDataVRAD, "VRAD", "S");
		//const double dstNI = abs(odim.NI);
		dstDataVRAD.odim.NI = nyquist;
		dstDataVRAD.odim.setRange(-nyquist, +nyquist);
		mout.info() << "dealiasing (u,v) to VRAD " << mout.endl;
		mout.info() << "src VRAD  " << EncodingODIM(srcData.odim) << mout.endl;
		mout.info() << "dst VRADC " << EncodingODIM(dstDataVRAD.odim) << mout.endl;
		setGeometry(srcData.odim, dstDataVRAD);
		const double srcNI2 = 2.0*srcData.odim.getNyquist(); // 2.0*srcData.odim.NI;
		const double min = dstDataVRAD.data.getMin<double>();
		const double max = dstDataVRAD.data.getMax<double>();

		mout.info() << "dst VRADC " << EncodingODIM(dstDataVRAD.odim) << ", [" << min << ',' << max << ']' << mout.endl;


		/// Azimuth in radians
		double azmR;

		/// Original value in VRAD
		double vOrig;
		drain::image::Point2D<double> unitVOrig;

		/// Resolved (u,v), from AMVU and AMVV
		double u, v;

		/// Resolved (u,v) projected back on beam
		double vReproj;
		drain::image::Point2D<double> unitVReproj;

		const bool MATCH_ALIASED  = (matchOriginal & 1);
		const bool MATCH_UNDETECT = (matchOriginal & 2);

		bool ORIG_UNDETECT;
		bool ORIG_NODATA;
		bool ORIG_UNUSABLE; // ORIG_UNDETECT && ORIG_NODATA

		size_t address;

		for (size_t j = 0; j < dstDataVRAD.data.getHeight(); ++j) {

			azmR = srcData.odim.getBeamWidth() * static_cast<double>(j) ; // window.BEAM2RAD * static_cast<double>(j);

			for (size_t i = 0; i < dstDataVRAD.data.getWidth(); ++i) {

				address = dstDataVRAD.data.address(i,j);
				u = dstDataU.data.get<double>(address);
				v = dstDataV.data.get<double>(address);

				if (MATCH_UNDETECT || MATCH_ALIASED){
					vOrig = srcData.data.get<double>(address);
					ORIG_UNDETECT = (vOrig == srcData.odim.undetect);
					ORIG_NODATA   = (vOrig == srcData.odim.nodata);

					ORIG_UNUSABLE = ORIG_UNDETECT || ORIG_NODATA;
					if (MATCH_UNDETECT && ORIG_UNUSABLE){
						if (ORIG_UNDETECT)
							dstDataVRAD.data.put(address, dstDataVRAD.odim.undetect);
						else
							dstDataVRAD.data.put(address, dstDataVRAD.odim.nodata);
						dstQuality.data.put(address, 0);
						continue;
					}

				}


				if (dstDataU.odim.isValue(u) && dstDataV.odim.isValue(v)){

					u = dstDataU.odim.scaleForward(u);
					v = dstDataV.odim.scaleForward(v);
					vReproj = project(azmR, u,v);

					if (MATCH_ALIASED && !ORIG_UNUSABLE){
						vOrig = srcData.odim.scaleForward(vOrig);
						srcData.odim.mapDopplerSpeed(vOrig,     unitVOrig.x,   unitVOrig.y);
						srcData.odim.mapDopplerSpeed(vReproj, unitVReproj.x, unitVReproj.y);
						vReproj = srcNI2*floor(vReproj/srcNI2) + vOrig;
					}

					vReproj = dstDataVRAD.odim.scaleInverse(vReproj);
					/*
					if ((i==j))
						if ((i & 15) == 0)
							std::cout << " outo: " << vReproj << '\n';
					 */
					if ((vReproj > min) && (vReproj < max)){ // continue processing
						//dstDataVRAD.data.put(address, vReproj);
						dstDataVRAD.data.put(address, vReproj);
						dstQuality.data.put(address, dstQuality.odim.scaleInverse(0.5 + (unitVReproj.x*unitVOrig.x + unitVReproj.y*unitVOrig.y)/2.0) );
					}
					else {
						dstDataVRAD.data.put(address, dstDataVRAD.odim.nodata); // rand() & 0xffff); //
						dstQuality.data.put(address, 0);
					};
				}
				else {
					dstDataVRAD.data.put(address, dstDataVRAD.odim.undetect);
				}
			}
		}
		//@ dstDataVRAD.updateTree();
	}


	/// FUTURE EXTENSION (VVPslots > 0)
	if (VVP && false){

		mout.info() << "computing VVP " << mout.endl;

		PlainData<PolarDst> & dstDataHGHT = dstProduct.getData("HGHT"); // altitude
		PlainData<PolarDst> & dstDataVVPU = dstProduct.getData("VVPU"); // wind profile
		PlainData<PolarDst> & dstDataVVPV = dstProduct.getData("VVPV"); // wind profile
		PlainData<PolarDst> & dstDataVVPQ = dstProduct.getData("VVPQ"); // wind profile quality

		const QuantityMap & qm = getQuantityMap();
		qm.setQuantityDefaults(dstDataHGHT, "HGHT", "S");
		qm.setQuantityDefaults(dstDataVVPU, "AMVU", "S");
		qm.setQuantityDefaults(dstDataVVPV, "AMVV", "S");
		qm.setQuantityDefaults(dstDataVVPQ, "QIND", "C");

		const size_t cols = srcData.data.getWidth();
		const size_t rows = srcData.data.getHeight();
		dstDataVVPU.setGeometry(1, cols); // yes transpose
		dstDataVVPV.setGeometry(1, cols);
		dstDataVVPQ.setGeometry(1, cols);
		dstDataHGHT.setGeometry(1, cols);

		double u, v, q;
		double uSum, vSum, qSum;
		unsigned int counter;
		double beta, h;
		const double eta = srcData.odim.elangle * (M_PI/180.0);
		size_t address;
		for (size_t i = 0; i < cols; ++i) {
			beta = srcData.odim.getGroundAngle(i);
			h = 0.001 * Geometry::heightFromEtaBeta(eta, beta);
			//mout.note() << "col:" << i << " h=" << h << " = ";
			dstDataHGHT.data.put(i, dstDataHGHT.odim.scaleInverse(h));
			//mout << dstDataHGHT.odim.scaleForward(dstDataHGHT.data.get<double>(i)) << mout.endl;
			uSum = vSum = qSum = 0.0;
			counter = 0;
			for (size_t j = 0; j < rows; ++j) {
				address = dstQuality.data.address(i,j);
				q = dstQuality.odim.scaleForward(dstQuality.data.get<double>(address));
				if (q > 0.01){
					u = dstDataU.data.get<double>(address);
					v = dstDataV.data.get<double>(address);
					if ((u != dstDataU.odim.undetect) && (u != dstDataU.odim.nodata) && (v != dstDataV.odim.undetect) && (v != dstDataV.odim.nodata)){
						u = dstDataU.odim.scaleForward(u);
						v = dstDataV.odim.scaleForward(v);
						uSum += u;
						vSum += v;
						qSum += q;
						++counter;
					}
				}
			}
			if (qSum > 0.01){
				dstDataVVPU.data.put(i, dstDataVVPU.odim.scaleInverse(uSum/qSum));
				dstDataVVPV.data.put(i, dstDataVVPU.odim.scaleInverse(vSum/qSum));
				dstDataVVPQ.data.put(i, dstDataVVPU.odim.scaleInverse(qSum/static_cast<double>(counter)));
			}
			else {
				dstDataVVPU.data.put(i, dstDataVVPU.odim.nodata);
				dstDataVVPV.data.put(i, dstDataVVPV.odim.nodata);
				dstDataVVPQ.data.put(i, dstDataVVPU.odim.scaleInverse(0));
			}
		}
		// mout.warn() << "computing VVP ended " << mout.endl;
		//@ dstDataVVPU.updateTree();
		//@ dstDataVVPV.updateTree();
		//@ dstDataVVPQ.updateTree();
		//@ dstDataHGHT.updateTree();
	}

	/// Run also
	/*
	if (odim.NI < 0.0){
		mout.warn() << "deviation, " << srcData << mout.endl;
		SlidingWindowOp<RadarWindowDopplerDev<PolarODIM> > vradDevOp; // op not needed
		vradDevOp.setSize(width, height);
		vradDevOp.window.setSize(width, height);
		vradDevOp.window.countThreshold = (width*height)/5;  // require 20% of valid samples
		//const double pos = 5.4321/vradSrc.odim.NI;
		//vradDevOp.window.functor.set( 0.8*pos, 1.2*pos, 255.0 );
		vradDevOp.window.functor.gain = 0.01;
		vradDevOp.window.functor.offset = 0.0;
		vradDevOp.window.odimSrc = srcData.odim;
		vradDevOp.filter(srcData.data, dstQuality.data);
	}
	 */

	//drain::image::File::write(dst,"DopplerDeAliasOp.png");
	mout.debug(3) << window.odimSrc << mout.endl;


}



void DopplerDiffPlotterOp::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(name, __FUNCTION__);

	const Data<PolarSrc > & srcData = srcSweep.getData("VRAD");

	if (srcData.data.isEmpty()){
		mout.warn() << "data empty" << mout.endl;
		return;
	}
	// setEncoding(srcData.odim, dstData.odim);
	// DopplerDeAliasWindow dw;

	w.adjustIndices(srcData.odim);
	if (w.ray2 < w.ray1){
		w.ray2 += 360;
	}
	mout.warn() << "rays: " << w.ray1 << '-' << w.ray2 << mout.endl;
	mout.warn() << "NI: " << srcData.odim.getNyquist() << mout.endl;

	// mout.warn() << "ray=" << w.ray1 << ',' << w.ray2 << ", bins=" << w.bin1 << ',' << w.bin2 << mout.endl;

	size_t count = (w.ray2-w.ray1) * (w.bin2-w.bin1);
	//mout.warn() << "size " << count << mout.endl;

	PlainData<PolarDst> & dstDataU = dstProduct.getData("AZM");
	PlainData<PolarDst> & dstDataV = dstProduct.getData("DIFF");

	dstDataU.setEncoding(typeid(double));
	dstDataU.data.setGeometry(count, 1);
	dstDataU.odim.quantity = "AZM"; // ???
	dstDataU.data.fill(dstDataU.odim.undetect);

	dstDataV.setEncoding(typeid(double));
	dstDataV.data.setGeometry(count, 1);
	dstDataV.odim.quantity = "DIFF";
	dstDataV.data.fill(dstDataV.odim.undetect);

	mout.debug() << '\t' << dstDataU.data.getGeometry() << mout.endl;
	mout.debug() << '\t' << dstDataV.data.getGeometry() << mout.endl;


	double azm, v1,v2,vDiff; // ,x,y;
	size_t index = 0;

	int j1;
	int j2;

	for (int j=w.ray1; j<w.ray2; ++j){

		azm = srcData.odim.getBeamWidth() * static_cast<double>(j);
		j1 = (j-1 + srcData.odim.nrays) % srcData.odim.nrays;
		j2 = (j+1 + srcData.odim.nrays) % srcData.odim.nrays;

		for (int i = w.bin1; i<w.bin2; ++i){

			v1 = srcData.data.get<double>(i, j1);
			v2 = srcData.data.get<double>(i, j2);

			if (srcData.odim.deriveDifference(v1, v2, vDiff)) {
				// mout.warn() << "data d: " << (double)d << mout.endl;
				dstDataU.data.put(index, azm*drain::RAD2DEG);
				dstDataV.data.put(index, vDiff/2.0); // span=2 between j1 and j2
			}
			else {
				dstDataU.data.put(index, dstDataU.odim.nodata);
				dstDataV.data.put(index, dstDataV.odim.nodata);
			}
			// mout.warn() << '\t' << index << mout.endl;
			++index;
		}
	}

}



} // ::rack
