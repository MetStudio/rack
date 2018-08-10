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
#ifndef ACCUMULATORGEO_H_
#define ACCUMULATORGEO_H_

//#include "Geometry.h"

#include "util/Rectangle.h"

#include "util/Proj4.h"  // for geographical projection of radar data bins

//#include "image/Image.h"
#include "Accumulator.h"
#include "GeoFrame.h"



namespace drain
{

namespace image
{



class AccumulatorGeo : public Accumulator,  public GeoFrame {

public:

	// Consider setFrameGeometry (protect this?)
	virtual
	inline
	void setGeometry(unsigned int width, unsigned int height) {
		GeoFrame::setGeometry(width, height);
		// See allocate below, calling Accumulator::setGeometry(width, height);
	}


	/// This should be called after setGeometry, unless the projection is used as a frame.
	virtual
	inline
	void allocate(){
		AccumulationArray::setGeometry(frameWidth,frameHeight);
	};

	inline
	void addUnprojected(double lon, double lat, double value, double weight){
		int i,j;
		size_t a;
		deg2pix(lon,lat, i,j);
		if ( (i>=0) && (i<static_cast<int>(width)) && (j>=0) && (j< static_cast<int>(height)) ){
			a = data.address(i, j);
			// std::cerr << "addUnprojected:" << i << ',' << j << '\t' << value << '/' << weight << '\n';
			add(a, value, weight);
		}
		/*
		else {
			std::cerr << "addUnprojected: outside, " << lat << ',' << lon << '\t' << value << '/' << weight << '\n';
		}
		*/
	};


};


std::ostream & operator<<(std::ostream &ostr, const AccumulatorGeo &cumulatorGeo);

} // ::image
} // ::drain

#endif // AccumulatorGeo

// Drain
