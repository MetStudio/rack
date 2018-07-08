/**

    Copyright 2001 - 2013  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)

    This file is part of AnoRack, a module of Rack for C++.

    AnoRack is not free software.

*/

#ifndef DOPPLER_DEALIAS2_OP_H_
#define DOPPLER_DEALIAS2_OP_H_


#include "DopplerWindowOp.h"

#include <cmath>


namespace rack {



//class DopplerDeAliasOp : public DopplerWindowOp<DopplerWindow> {  // DopplerWindow unused!
class DopplerDeAliasOp : public DopplerOp {  // DopplerWindow unused!
public:

	/// Constructor
	/**
	 *  \param widthM - width of the window, in metres.
	 *  \param heightD - azimuthal width of the window, in degrees.
	 */
	DopplerDeAliasOp(int widthM = 500, double heightD = 3.0, double nyquistVelocity=0.0) :
		DopplerOp(__FUNCTION__, "Solves Doppler speed ambiguity, outputs (u,v).") { //, widthM, heightD) {
		parameters.reference("width", this->widthM = widthM, "metres");
		parameters.reference("height", this->heightD = heightD, "degrees");
		parameters.reference("nyquist", odim.NI = nyquistVelocity, "m/s");
		parameters.reference("altitudeWeight", altitudeWeight, "Functor:a:b:c..."); // ??
		//parameters.reference("testSigns", testSigns = 3, "bits");
		parameters.reference("VVP", VVP=false, "0|1");
		dataSelector.count = 1;

		odim.type = "S";
		//odim.gain = 1.0/256.0;
		//odim.offset = -128.0;
		odim.product = "AMV";
		//odim.NI

	};

	virtual ~DopplerDeAliasOp(){};


	// Outputs u and v both, so dst dataSET needed
	virtual
	void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const ;

	//@Override
	virtual
	void processData(const Data<PolarSrc> & vradSrc, Data<PolarDst> & dstData) const {
		throw std::runtime_error(name + "::" + __FUNCTION__ + ": not implemented");
	}


	// DopplerDeAliasWindowParameters parameters;
	int widthM;
	double heightD;
	std::string altitudeWeight;  // Functor

	/// Projects wind (u,v) to beam. Unit (typically m/s) is preserved.
	// raise
	inline
	double project(double azmR, double u, double v) const {
		// double speed = sqrt(u*u + v*v);
		return u*sin(azmR) + v*cos(azmR);
	}

	inline
	double alias(double v, double vNyquist) const {
		double n=0;
		if (v >= 0.0){
			v += vNyquist;
			n = floor(v/(2.0*vNyquist));
			return v - n * (2.0*vNyquist) - vNyquist;
		}
		else {
			v -= vNyquist;
			n = floor(-v/(2.0*vNyquist));
			return v + n * (2.0*vNyquist) + vNyquist;
		}
	}

	bool VVP;

};

}


#endif /* DOPPLERDEALIASOP_H_ */
