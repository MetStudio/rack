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
#include "PolarWindow.h"


namespace rack
{


void GaussianStripeVertPolarWeighted::update() {

	value = 0.0;
	sumW  = 0.0;
	weightSum = 0.0;

	// Consider: {NEAR,LINEAR,FAR}
	if (location.x <= rangeNorm){  // if (RANGE < 0){
		for (int j = this->jMin; j <= this->jMax; j++) {
			locationTmp.setLocation(location.x, location.y + j);
			if (coordinateHandler.validate(locationTmp)){
				w = lookUp[j-jMin];
				sumW += w;
				w *= srcWeight.get<double>(locationTmp);
				weightSum += w;
				value += w * src.get<double>(locationTmp);
			}
			//	value += (lookUp[j-jMin] * src.get<double>(locationTmp));
		}
		//value = value/weightSum;
	}
	else if (location.x < rangeNormEnd){ //  if (RANGE == 0){  // "normalized" ie. spread j = (rangeNorm*j)/location.x
		for (int j = this->jMin; j <= this->jMax; j++) {
			locationTmp.setLocation(location.x, location.y + (rangeNorm*j)/(location.x+1));
			if (coordinateHandler.validate(locationTmp)){
				w = lookUp[j-jMin];
				sumW += w;
				w *= srcWeight.get<double>(locationTmp);
				weightSum += w;
				value += w * src.get<double>(locationTmp);
			}
		}
		//value = value/weightSum;
	}
	else {
		value = src.get<double>(location);
		weightSum = srcWeight.get<double>(location);
		return;
	}

	if (weightSum > 0.0){
		value = value/weightSum;
		weightSum = weightSum/sumW;
	}

}




}

// Rack
