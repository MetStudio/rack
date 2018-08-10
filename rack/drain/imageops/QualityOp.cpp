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

#include "image/FilePng.h"
#include "QualityOverrideOp.h"

namespace drain
{

namespace image
{

void QualityOp::makeCompatible(const ImageFrame & src, Image & dst) const  {

	drain::Logger mout(getImgLog(), name+"(ImageOp)", __FUNCTION__);

	mout.debug(2) << "src: " << src << mout.endl;

	if (dst.isSame(src)){
		mout.debug() << "dst == src, ok" << mout.endl;
		return;
	}

	if (!dst.typeIsSet()){
		dst.setType(src.getType());
		// dst.setScale(src.getScale()); // NEW
	}


	if (!Type::call<drain::typeIsInteger>(dst.getType())){
		//dst.scaling.setScale(src.scaling.getScale());
		dst.setScaling(src.getScaling());
		if (dst.getType() != src.getType()){
			mout.info() << "Adopting src scaling for dst (different storage types)" << mout.endl;
		}
	}


	dst.setGeometry(src.getGeometry());
	/*
	/// TODO: copy alpha, fill alpha?
	if (dst.setGeometry(src.getGeometry())){
		if (dst.getAlphaChannelCount()){
			mout.warn() << "resetting alpha channel" << mout.endl;
			dst.getAlphaChannel().fill(dst.scaling.getMax<int>());
		}
	}
	*/

	dst.setCoordinatePolicy(src.getCoordinatePolicy());

	mout.debug(3) << "dst: " << dst << mout.endl;

}


void QualityThresholdOp::traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const {

	// quality threshold
	const double t = srcAlpha.getScaling().inv(threshold);

	// Lowest quality, as a code value
	const double zero = dstAlpha.getScaling().inv(0);

	// mout.warn() << " t=" << t << mout.endl;

	Channel::const_iterator  sit = src.begin();
	Channel::const_iterator sait = srcAlpha.begin();
	Channel::iterator  dit = dst.begin();
	Channel::iterator dait = dstAlpha.begin();

	/// Only alpha is thresholded
	if (isnan(replace)){
		while (sit != src.end()){
			if (*sait < t){
				*dit = *sit;
				*dait = zero;
			}
			else { // copy
				*dit  = *sit;
				*dait = *sait; // sometimes need to skip this?
			}
			++sit; ++sait;
			++dit; ++dait;
		}
	}
	/// If alpha is thresholded, data value replaced
	else {
		const double r = src.getScaling().inv(replace);
		while (sit != src.end()){
			if (*sait < t){
				*dit  = r;
				*dait = zero;
			}
			else { // copy
				*dit  = *sit;
				*dait = *sait; // sometimes need to skip this?
			}
			++sit; ++sait;
			++dit; ++dait;
		}
	}
}

}

}

// Drain
