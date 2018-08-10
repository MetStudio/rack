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
#ifndef BEAMALTITUDE_H_
#define BEAMALTITUDE_H_

#include <algorithm>



//#include "RackOp.h"
#include "PolarProductOp.h"

namespace rack
{

using namespace drain::image;


/// A single-ray "product" storing the altitude at each bin.
/** The main idea is to compute one equidistant arc at a time.
   Has no parameters, but target scaling shoud be "S,100" or "C,0.1", for example.
*/
class BeamAltitudeOp : public PolarProductOp {

public:

	BeamAltitudeOp() : // const std::string & type="S", double gain=100.0, long int nbins=0, double rscale=0) :
		PolarProductOp(__FUNCTION__,"Computes the altitude at each bin")
	{

		parameters.reference("altitudeReference", this->aboveSeaLevel = true, "0=radar site|1=sea level");

		odim.product  = "ALTITUDE";
		odim.quantity = "HGHT";

		dataSelector.quantity = "";
		dataSelector.path = "dataset[0-9]+/data[0-9]+/?$";  // NOTE! A dataset for each elevation groups; should suffit for nbins and rscale. However, if a user wants to use quantity, /dataN/ should be reached.
		dataSelector.count = 1;

		this->allowedEncoding.reference("type",   odim.type = "C");
		this->allowedEncoding.reference("gain",   odim.gain = 0.1);
		this->allowedEncoding.reference("offset", odim.offset = 0.0);

		odim.nrays = 1;

		// allowedEncoding.reference("type", odim.type, "S");
		// allowedEncoding.reference("gain", odim.gain, 0.001);

	};

	virtual
	void processData(const Data<PolarSrc> & src, Data<PolarDst> &dst) const;

protected:

	virtual
	void setGeometry(const PolarODIM & srcODIM, PlainData<PolarDst> & dstData) const {
		dstData.odim.nbins = (odim.nbins>0) ? odim.nbins : srcODIM.nbins;
		dstData.odim.nrays = 1;
		dstData.odim.rscale = (static_cast<double>(srcODIM.nbins) * srcODIM.rscale + srcODIM.rstart) / static_cast<double>(dstData.odim.nbins);
		dstData.data.setGeometry(dstData.odim.nbins, dstData.odim.nrays);
	};


};





}  // ::rack

#endif /*POLARCappi_H_*/

// Rack
