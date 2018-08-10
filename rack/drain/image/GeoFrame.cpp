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

#include "util/Rectangle.h"
#include "AccumulatorGeo.h"



// using namespace std;


namespace drain
{

namespace image
{

/*
const double GeoFrame::DEG2RAD(M_PI/180.0);
const double GeoFrame::RAD2DEG(180.0/M_PI);
const int    GeoFrame::EARTH_RADIUSi(6371000);
const double GeoFrame::EARTH_RADIUS(6371000.0);
*/

/*
static const double DEG2RAD = M_PI/180.0;
static const double RAD2DEG = 180.0/M_PI;
static const int   EARTH_RADIUSi = 6371000;
static const double EARTH_RADIUS = 6371000.0;
*/

GeoFrame::GeoFrame(unsigned int frameWidth,unsigned int frameHeight) :
				xScale(1), yScale(1) //, debug(false)
{

	projR2M.setProjectionSrc("+proj=latlong +ellps=WGS84 +datum=WGS84");
	//projR2M.setProjectionDst("+proj=utm +zone=35 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs"); /// todo neutralize
	setGeometry(frameWidth,frameHeight);
	setBoundingBoxD(0,0,0,0);

}




void GeoFrame::setGeometry(unsigned int w,unsigned int h) {

	frameWidth = w;
	frameHeight = h;
	updateScaling();

}




void GeoFrame::setBoundingBoxR(double lonLL,double latLL,double lonUR,double latUR) {
	extentR.xLowerLeft  = lonLL;
	extentR.yLowerLeft  = latLL;
	extentR.xUpperRight = lonUR;
	extentR.yUpperRight = latUR;

	extentD.xLowerLeft  = lonLL*RAD2DEG;
	extentD.yLowerLeft  = latLL*RAD2DEG;
	extentD.xUpperRight = lonUR*RAD2DEG;
	extentD.yUpperRight = latUR*RAD2DEG;

	updateScaling();

}

void GeoFrame::setBoundingBoxM(double xLL,double yLL,double xUR,double yUR) {
	//double lonLL = 0.0, latLL, lonUR, latUR;
	Rectangle<double> bboxDst(0,0,0,0);
	if (projR2M.isSet()){
		projR2M.projectInv(xLL, yLL, bboxDst.xLowerLeft,  bboxDst.yLowerLeft);
		projR2M.projectInv(xUR, yUR, bboxDst.xUpperRight, bboxDst.yUpperRight);
	}
	//setBoundingBoxR(lonLL,latLL,lonUR,latUR);
	setBoundingBoxR(bboxDst); //.xLowerLeft,  bboxDst.yLowerLeft, bboxDst.xUpperRight, bboxDst.yUpperRight);
}

void GeoFrame::getCenterPixel(drain::Rectangle<double> & p) const {
	const int im = getFrameWidth()/2;
	const int jm = getFrameHeight()/2;
	pix2deg(im-1, jm-1, p.xLowerLeft,  p.yLowerLeft);
	pix2deg(im+1, jm+1, p.xUpperRight, p.yUpperRight);
}

/// Return vertical resolution of a pixel in meters (if metric) or degrees (if unprojected, "latlon").
double GeoFrame::getXScale() const {
	if (projR2M.isLongLat()){ // approximate!
		drain::Rectangle<double> p;
		getCenterPixel(p);
		return (p.xUpperRight-p.xLowerLeft )/2.0 * DEG2RAD * EARTH_RADIUS * cos(DEG2RAD*(p.yLowerLeft+p.yUpperRight)/2.0);
		//yScale = (pixelDeg.yLowerLeft -pixelDeg.yUpperRight)/2.0 * DEG2RAD * EARTH_RADIUS;
	}
	else {
		return xScale;
	}
}

/// Return vertical resolution of a pixel in meters (if metric) or degrees (if unprojected, "latlon").
double GeoFrame::getYScale() const {
	if (projR2M.isLongLat()){ // approximate!
		drain::Rectangle<double> p;
		getCenterPixel(p);
		return (p.yLowerLeft -p.yUpperRight)/2.0 * DEG2RAD * EARTH_RADIUS;
	}
	else {
		return yScale;
	}
}



void GeoFrame::cropWithM(double xLL, double yLL, double xUR, double yUR) {

	Logger mout("GeoFrame", __FUNCTION__);

	Rectangle<int> frame(0, 0, getFrameWidth(), getFrameHeight());

	Rectangle<int> cropper;
	m2pix(xLL,yLL, cropper.xLowerLeft,  cropper.yUpperRight); // j swapped  "upside down"
	m2pix(xUR,yUR, cropper.xUpperRight, cropper.yLowerLeft);  // j swapped  "upside down"
	// mout.warn() << " frame:" << frame <<  " crop0:" << cropper;

	cropper.xLowerLeft--;
	cropper.yLowerLeft--;
	cropper.xUpperRight++;
	cropper.yUpperRight++;
	frame.crop(cropper);
	// mout.warn()  << " crop:" << cropper << " => frame:" << frame << mout.endl;

	pix2LLm(frame.xLowerLeft,  frame.yUpperRight,  xLL, yLL); // j swapped  "upside down"
	pix2LLm(frame.xUpperRight, frame.yLowerLeft,   xUR, yUR); // j swapped  "upside down"

	/// Reset area and rescale
	setBoundingBoxM(xLL, yLL, xUR, yUR);
	setGeometry(frame.xUpperRight-frame.xLowerLeft, frame.yUpperRight-frame.yLowerLeft);  // +1 +1

}



void GeoFrame::setProjection(const std::string &s){
	projR2M.setProjectionDst(s);
	updateScaling();
}


/// Notice: changed! For LatLon, consider approx? See composite
void GeoFrame::updateScaling()
{
	if (projR2M.isSet()){

		// set metric bbox
		projR2M.projectFwd(extentR.xLowerLeft,  extentR.yLowerLeft,  extentM.xLowerLeft,  extentM.yLowerLeft);
		projR2M.projectFwd(extentR.xUpperRight, extentR.yUpperRight, extentM.xUpperRight, extentM.yUpperRight);

		// set xScale and yScale
		/*
		if (projR2M.isLongLat()){ // approximate!
			const int im = getFrameWidth()/2;
			const int jm = getFrameHeight()/2;
			drain::Rectangle<double> pixelDeg;
			pix2deg(im-1, jm-1, pixelDeg.xLowerLeft,  pixelDeg.yLowerLeft);
			pix2deg(im+1, jm+1, pixelDeg.xUpperRight, pixelDeg.yUpperRight);
			xScale = (pixelDeg.xUpperRight-pixelDeg.xLowerLeft )/2.0 * DEG2RAD * EARTH_RADIUS * cos(DEG2RAD*(pixelDeg.yLowerLeft+pixelDeg.yUpperRight)/2.0);
			yScale = (pixelDeg.yLowerLeft -pixelDeg.yUpperRight)/2.0 * DEG2RAD * EARTH_RADIUS;
		}
		else {
		}*/
		xScale = (extentM.xUpperRight - extentM.xLowerLeft) / getFrameWidth();
		yScale = (extentM.yUpperRight - extentM.yLowerLeft) / getFrameHeight();
	}
}



void GeoFrame::updateDataExtent(const drain::Rectangle<double> & inputExtentD)
{

	if (dataExtentD.getArea() == 0){
		dataExtentD  = inputExtentD;
		dataOverlapD = inputExtentD;
	}
	else {
		// std::cerr << "Extending:" << dataExtentD << std::endl;
		dataExtentD.extend(inputExtentD);
		dataOverlapD.contract(inputExtentD);
		// std::cerr << "    =>    " << dataExtentD << std::endl;
	}
}

std::ostream & GeoFrame::toOStr(std::ostream & ostr) const {

	ostr << "frame " << this->getFrameWidth() << 'x' << this->getFrameHeight() << "\n";
	//ostr << (const drain::image::AccumulationArray &)acc << '\n';
	//ostr << "   resolution, metres: " << acc.getScaleX() << 'x' << acc.getScaleY() << "\n";
	ostr << "   coord system: " << getCoordinateSystem() << '\n';
	ostr << "   proj:  " << getProjection() << '\n';

	ostr << "   scope, metres:  [" << getBoundingBoxM() << "]\n";
	ostr << "   scope, radians: [" << getBoundingBoxR() << "]\n";
	ostr << "   scope, degrees: [" << getBoundingBoxD() << "]\n";

	return ostr;

}


} // namespace image
} // namespace drain



// Drain
