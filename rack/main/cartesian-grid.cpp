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



#include <drain/util/Fuzzy.h>

#include <drain/image/File.h>
#include <drain/imageops/DistanceTransformFillOp.h>
#include <drain/imageops/RecursiveRepairerOp.h>

#include "data/DataCoder.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Read.h"

#include "radar/Coordinates.h"
#include "radar/Composite.h"
//#include "radar/Extractor.h"

//#include "compositing.h"
#include "images.h"
#include "cartesian-grid.h"


namespace rack {



void CartesianGrid::exec() const {

	drain::Logger mout(name, __FUNCTION__);

	RackResources & resources = getResources();

	Composite & composite = resources.composite;

	if (!composite.isDefined()){

		mout.info() << "Defining (but not allocating) composite with input data specifications" << mout.endl;

		/// Defining composite is needed for calling  composite.pix2deg(i,j,lon,lat) further below.

		const HI5TREE & cartesian = resources.cartesianHi5;

		//const drain::VariableMap & where = cartesian["where"].data.attributes;
		//const drain::VariableMap & where = cartesian["dataset1"]["where"].data.attributes;
		// mout.warn() << where << mout.endl;
		// drain::Rectangle<double> bboxD(where["LL_lon"], where["LL_lat"], where["UR_lon"], where["UR_lat"]);
		/*
			drain::VariableMap a;
			DataSelector::getAttributes(cartesian, "dataset1", a);

		 */
		CartesianODIM odim;
		DataTools::getAttributes(cartesian, "dataset1", odim, true);
		//odim.addShortKeys();
		//odim.updateFromMap(a);
		//drain::Rectangle<double> bboxD(a["where:LL_lon"], a["where:LL_lat"], a["where:UR_lon"], a["where:UR_lat"]);
		drain::Rectangle<double> bboxD(odim.LL_lon, odim.LL_lat, odim.UR_lon, odim.UR_lat);
		composite.setBoundingBoxD(bboxD);
		composite.setGeometry(odim.xsize, odim.ysize);
		//composite.setGeometry(a["where:xsize"], a["where:ysize"]);
		//const std::string projdef = a["where:projdef"];
		//mout.warn() << "trying: " << projdef<< mout.endl;
		composite.setProjection(odim.projdef);
		//composite.setProjection(where["projdef"]);
		//mout.warn() << "passed" << mout.endl;
	}

	if ((resources.currentImage != & resources.grayImage) && (resources.currentImage != &resources.colorImage)){  // resources.grayImage.isEmpty()
		//mout.error() << "Gray or color image not created yet, use --image " << mout.endl;
		mout.info() << "Gray or color image not created yet, calling --image " << mout.endl;
		cmdImage.exec();
	}

	double width = this->width;
	if (width == 0.0)
		width = 1.0;
	//width = pow(latSpacing * lonSpacing, 0.25) / 100.0;

	const bool RGB = (resources.currentImage == &resources.colorImage);

	Image & img = RGB ? getResources().colorImage : getResources().grayImage;

	const bool ALPHA = resources.currentImage->getAlphaChannelCount();

	ImageFrame & alpha = ALPHA ? img.getAlphaChannel() : img.getChannel(0);

	const drain::Rectangle<double> & bboxD = composite.getBoundingBoxD();
	const double lonResolution = static_cast<double>(composite.getFrameWidth())  / (bboxD.xUpperRight - bboxD.xLowerLeft) / width ;
	const double latResolution = static_cast<double>(composite.getFrameHeight()) / (bboxD.yUpperRight - bboxD.yLowerLeft) / width ;

	drain::FuzzyBell2<double> peak(0.0, width);
	double lat, lon;

	double latWeight, lonWeight;
	double weight;
	/// Intensity
	double f;
	const double fMax = intensity * img.getMax<double>();
	const double fMaxAlpha = img.getMax<double>();
	for (size_t j = 0; j<img.getHeight(); ++j){
		for (size_t i = 0; i<img.getWidth(); ++i){
			composite.pix2deg(i,j,lon,lat);
			lonWeight = peak(lonResolution * (lon - lonSpacing*round(lon/lonSpacing)));
			latWeight = peak(latResolution * (lat - latSpacing*round(lat/latSpacing)));
			weight = std::max(lonWeight, latWeight);
			//if (j == 400) std::cerr << i << ' ' << lon << '\t' <<weight << '\n';
			if (RGB){
				for (int k = 0; k < 3; ++k) {
					f = img.get<double>(i,j,k);
					img.put(i,j,k, (1.0-weight)*f + weight*fMax);
				}
			}
			else {
				f = img.get<double>(i,j);
				img.put(i,j, (1.0-weight)*f + weight*fMax);
			}
			if (ALPHA){
				f = alpha.get<double>(i,j);
				alpha.put(i,j, std::max(f, weight*fMaxAlpha));
			}
			//img.put(i,j,k, (1.0-weight)*f + weight*(fMax-f));  XOR
			//img.put(i,j, (1.0-w)*f + w*255.0);
		}
	}
}



}  // namespace rack::



// Rack
