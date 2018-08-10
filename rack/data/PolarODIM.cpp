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

#include <drain/util/Log.h>

#include "PolarODIM.h"

namespace rack {



void PolarODIM::init(group_t initialize){ // ::referenceRootAttrs(){

	if (initialize & ROOT){
		reference("what:object", object = "PVOL");  // // or SCAN...
		reference("where:lon", lon = 0.0);
		reference("where:lat", lat = 0.0);
		reference("where:height", height = 0.0);
		reference("how:freeze", freeze = 10.0);
	}

	if (initialize & DATASET){
		reference("where:nbins",  nbins = 0L);
		reference("where:nrays",  nrays = 0L);
		reference("where:rscale", rscale = 0.0);
		reference("where:elangle", elangle = 0.0);
		reference("where:rstart", rstart = 0.0);
		reference("where:a1gate", a1gate = 0L);
		reference("where:startaz", startaz = 0.0);
		reference("where:stopaz",   stopaz = 0.0);
		reference("how:wavelength", wavelength = 0.0);
		reference("how:highprf", highprf = 0.0);
		reference("how:lowprf", lowprf = 0.0);
	}

	if (initialize & DATA){
	}

}






}  // namespace rack



// Rack
