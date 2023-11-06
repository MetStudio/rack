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

#include "Analysis.h"

namespace rack {


void RadarWindowConfig::setPixelConf(RadarWindowConfig & conf, const PolarODIM & inputODIM) const {

	drain::Logger mout(__FUNCTION__, "RadarWindowConfig");

	// pixelConf = this->conf;  PROBLEM: ftor prevents op=
	conf.widthM  = this->widthM;
	conf.heightD = this->heightD;
	conf.invertPolar   = this->invertPolar;
	conf.contributionThreshold  = this->contributionThreshold;
	conf.relativeScale = this->relativeScale;

	conf.updatePixelSize(inputODIM);


}

void RadarWindowConfig::updatePixelSize(const PolarODIM & inputODIM){ // DopplerWindOp wants public

	drain::Logger mout(__FUNCTION__, "RadarWindowConfig");

	//mout.note() << odimSrc << mout.endl;
	this->frame.width  = inputODIM.getBeamBins(this->widthM);
	this->frame.height = inputODIM.getAzimuthalBins(this->heightD);
	//mout.note() << this->width << '<' << this->widthM << mout.endl;

	if (this->frame.width <= 0){
		// mout.note() << this->frame.width  << "pix ~ " << this->widthM << "m " << mout.endl;
		//mout.note() << *this << mout.endl;
		mout.warn() << "Requested width (" << this->widthM <<  " meters) smaller than rscale ("<< inputODIM.rscale <<"), setting window width=1 " << mout.endl;
		this->frame.width = 1;
	}

	if (this->frame.height == 0){
		mout.warn() << "Requested height (" << this->heightD <<  " degrees) smaller than 360/nrays ("<< (360.0/inputODIM.area.height) <<"), setting window height=1 " << mout.endl;
		this->frame.height = 1;
	}

	//mout.note() << this->height << '<' << this->heightD << mout.endl;

}


void rack::RadarFunctorBase::apply(const Channel &src, Channel &dst, const drain::UnaryFunctor & ftor, bool LIMIT) const {

	drain::Logger mout(__FILE__, __FUNCTION__); //REPL getImgLog(), this->name+"(RadarFunctorOp)", __FUNCTION__);
	mout.debug() << "start" << mout.endl;

	// const double dstMax = dst.scaling.getMax<double>();
	//// NEW 2019/11 const double dstMax = dst.getEncoding().getTypeMax<double>();
	// drain::Type::call<drain::typeMax, double>(dst.getType());
	const drain::ValueScaling & dstScaling = dst.getScaling();

	typedef drain::typeLimiter<double> Limiter;
	Limiter::value_t limit = drain::Type::call<Limiter>(dst.getType());

	Image::const_iterator s  = src.begin();
	Image::iterator d = dst.begin();
	double s2;
	if (LIMIT){
		while (d != dst.end()){
			s2 = static_cast<double>(*s);
			if (s2 == odimSrc.nodata)
				*d = nodataValue;
			else if (s2 == odimSrc.undetect)
				*d = undetectValue;
			else
				//*d = dst.scaling.limit<double>(dstMax * this->functor(odimSrc.scaleForward(s2)));
				//*d = limit(dstMax * this->functor(odimSrc.scaleForward(s2)));
				*d = limit(dstScaling.inv(ftor(odimSrc.scaleForward(s2))));
			++s;
			++d;
		}
	}
	else {
		while (d != dst.end()){
			s2 = static_cast<double>(*s);
			if (s2 == odimSrc.nodata)
				*d = nodataValue;
			else if (s2 == odimSrc.undetect)
				*d = undetectValue;
			else
				*d = dstScaling.inv(ftor(odimSrc.scaleForward(s2)));
			//*d = dstMax * this->functor(odimSrc.scaleForward(s2));
			++s;
			++d;
		}
	}

}


} // rack::

