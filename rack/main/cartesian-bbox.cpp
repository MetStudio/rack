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


#include "cartesian-bbox.h"  // for cmdFormat called by


namespace rack {


void CartesianBBoxTest::exec() const {

	drain::Logger mout(name, __FUNCTION__); // = getResources().mout;

	RackResources & resources = getResources();

	if ( ! resources.composite.isDefined() ){
		mout.warn() << "Composite undefined, skipping." << mout.endl;
		return;
	}

	// There may be no data, don't use Data<PolarSrc> etc here.
	HI5TREE & p = (*resources.currentPolarHi5);

	// Derive range
	drain::VariableMap & attributes = p["where"].data.attributes;
	double lon = attributes.get("lon", 0.0);
	double lat = attributes.get("lat", 0.0);
	//mout.warn() << attributes << mout.endl;
	//
	const drain::VariableMap & a = p.hasChild("dataset1") ? p["dataset1"]["where"].data.attributes : attributes;
	//mout.warn() << (int)p.hasChild("dataset1") << mout.endl;
	//mout.warn() << a << mout.endl;

	/*
	 * There may be no data, don't use Data<PolarSrc> etc here.
	 */
	const double range = a.get("nbins",0.0) * a.get("rscale",0.0)  +  a.get("rstart",0.0);

	mout.debug() << ": lon=" << lon << ", lat=" << lat << ", range=" << range << mout.endl;

	// TODO: other than aeqd?
	RadarProj pRadarToComposite;
	pRadarToComposite.setSiteLocationDeg(lon, lat);
	pRadarToComposite.setProjectionDst(resources.composite.getProjection());

	drain::Rectangle<double> bboxM;
	//resources.composite.determineBoundingBoxM(pRadarToComposite, odimIn.rscale*odimIn.nbins + odimIn.rscale/2.0 + odimIn.rstart, bboxM);
	if (range > 0.0) {
		pRadarToComposite.determineBoundingBoxM(range, bboxM.xLowerLeft, bboxM.yLowerLeft, bboxM.xUpperRight, bboxM.yUpperRight);
		//resources.composite.determineBoundingBoxM(pRadarToComposite, range, bboxM);
	}
	else {
		mout.warn() << "could not derive range, using 250km "<< mout.endl;
		pRadarToComposite.determineBoundingBoxM(250000, bboxM.xLowerLeft, bboxM.yLowerLeft, bboxM.xUpperRight, bboxM.yUpperRight);
		//resources.composite.determineBoundingBoxM(pRadarToComposite, 250000, bboxM);
	}

	// New: also store
	drain::Rectangle<double> bboxD;
	//double x,y;
	//drain::Rectangle<int> bInt;
	int i,j;

	resources.composite.m2deg(bboxM.xLowerLeft, bboxM.yLowerLeft, bboxD.xLowerLeft, bboxD.yLowerLeft);
	attributes["LL_lon"] = bboxD.xLowerLeft;
	attributes["LL_lat"] = bboxD.yLowerLeft;

	resources.composite.m2pix(bboxM.xLowerLeft, bboxM.yLowerLeft, i, j);
	attributes["LL_i"] = i;
	attributes["LL_j"] = j;

	resources.composite.m2deg(bboxM.xUpperRight, bboxM.yUpperRight, bboxD.xUpperRight, bboxD.yUpperRight);
	attributes["UR_lon"] = bboxD.xUpperRight;
	attributes["UR_lat"] = bboxD.yUpperRight;

	resources.composite.m2pix(bboxM.xUpperRight, bboxM.yUpperRight, i, j);
	attributes["UR_i"] = i;
	attributes["UR_j"] = j;

	//dataExtentD.extend();

	//mout.warn() << "comp"  << resources.composite.getBoundingBoxM() << mout.endl;
	//mout.warn() << "radar" << bboxM << mout.endl;

	overlap = bboxM.isOverLapping(resources.composite.getBoundingBoxM());

	mout.note() << "overlap:" << static_cast<int>(overlap) << mout.endl;

	//cBBox.bbox.updateDataExtent(bD);
	//cBBox.
	resources.bbox.extend(bboxD);  // extern
	mout.info() << "combined bbox:" << resources.bbox << mout.endl;

	if (!overlap)
		resources.inputOk = false;

	if (value > 1){
		exit(overlap ? 0 : value);
	}
}


void CartesianBBoxTile::exec() const {

	Logger mout(name, __FUNCTION__);
	// TODO PROJ CHECK! => error, warn?

	Composite & composite = getResources().composite;

	/// Use composite's bbox as a starting point.
	Rectangle<double> bboxTile = composite.getBoundingBoxD();

	/// Crop to desired size. (ie. section of the two bboxes)
	bboxTile.crop(bbox);

	// Project to image coordinates.
	int i, j;
	composite.deg2pix(bboxTile.xLowerLeft, bboxTile.yLowerLeft, i, j);
	int i2, j2;
	composite.deg2pix(bboxTile.xUpperRight, bboxTile.yUpperRight, i2, j2);

	// Justify the float-valued bbox to image coordinates (integers).
	// First, decrement UR image coords by one:
	--j;
	--i2;
	composite.pix2deg(i, j, bboxTile.xLowerLeft, bboxTile.yLowerLeft);
	composite.pix2deg(i2, j2, bboxTile.xUpperRight, bboxTile.yUpperRight);
	if ((bboxTile.getWidth() <= 0.0) || (bboxTile.getHeight() <= 0.0)){
		mout.error() << "negative-valued area in a bbox: " << bboxTile << mout.endl;
	}

	// Finally, redefine the composite scope.
	composite.setGeometry(static_cast<unsigned int>(i2-i), static_cast<unsigned int>(j-j2));
	composite.setBoundingBoxD(bboxTile);

}







}  // namespace rack::



// Rack
