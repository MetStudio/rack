/**

    Copyright 2015 - 2016  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

 */
#include "File.h"
#include "DistanceModel.h"

namespace drain {

namespace image {


void DistanceModelLinear::setRadius(dist_t horz, dist_t vert){ // , bool diag, bool knight){

	//std::cerr << getName() << ':' <<__FUNCTION__ << std::endl;

	drain::Logger mout(getImgLog(), getName(), __FUNCTION__);

	//std::cerr << getName() << ':' <<__FUNCTION__ << " 2" << std::endl;


	mout.debug(1) << "radii: " << horz << ", " << vert << mout.endl; // ", " << diag << mout.endl;
	this->width  = horz;
	this->height = vert;

	//std::cerr << getName() << ':' <<__FUNCTION__ << " 3" << std::endl;

	if (getMax() == 0.0){
		mout.warn() << "max unset " << mout.endl; // ", " << diag << mout.endl;
	}

	dist_t horzDec = getMax();
	dist_t vertDec = getMax();

	if (horz < 0.0)
		horzDec = 0.0;    // no decrement, spread to infinity
	else if (horz > 0.0)
		horzDec = (getMax()/horz  + 0.0); // "default"

	if (isnan(vert))
		vertDec = horzDec;
	else if (vert < 0.0)
		vertDec = 0.0;   // no decrement, spread to infinity
	else if (vert > 0.0)
		vertDec = (getMax()/vert + 0.0); // "default"

	setDecrement(horzDec, vertDec);  // handles diag and knight

}


void DistanceModelLinear::setDecrement(dist_t horz, dist_t vert){

	drain::Logger mout(getImgLog(), getName(), __FUNCTION__);

	if (horz < 0.0) // ? when would this occur?
		horzDecrement = 1.0;
	else
		horzDecrement = horz;

	if (vert < 0.0)
		vertDecrement = horzDecrement;
	else
		vertDecrement = vert;

	diagDecrement =       sqrt(    horzDecrement*horzDecrement +     vertDecrement*vertDecrement);
	knightDecrementHorz = sqrt(4.0*horzDecrement*horzDecrement +     vertDecrement*vertDecrement);   //diagDecrement * sqrt(5.0)/sqrt(2.0);
	knightDecrementVert = sqrt(    horzDecrement*horzDecrement + 4.0*vertDecrement*vertDecrement);

	mout.debug() << "decs: " << horzDecrement << ", " << vertDecrement;
	if (DIAG){
		mout << ", (" << diagDecrement;
		if (KNIGHT){
			mout << ", (" << knightDecrementHorz << ','  << knightDecrementVert << ") ";
		}
		mout << ") ";
	}
	mout << mout.endl;

}


void DistanceModelExponential::setRadius(dist_t horz, dist_t vert){ // , bool diag, bool knight){

	//std::cerr << getName() << ':' <<__FUNCTION__ << " 1" << std::endl;

	// NEW : nominators now 1.0 => 2.0 to match better with linear half-widths
	drain::Logger mout(getImgLog(), getName(), __FUNCTION__);

	//std::cerr << getName() << ':' <<__FUNCTION__ << " 2" << std::endl;

	this->width  = horz;
	this->height = vert;
	mout.debug(1) << "radii: " << horz << ", " << vert << mout.endl; // ", " << diag << mout.endl;

	//std::cerr << getName() << ':' <<__FUNCTION__ << " 3" << std::endl;

	dist_t horzDec = 0.0;
	dist_t vertDec = 0.0;

	// TODO: interpret handle 0 and -1 better

	if (horz < 0.0)
		horzDec = 1.0;    // no decay, spread to infinity
	else if (horz == 0.0)
		horzDec = 0.0;    // full decay
	else if (horz > 0.0)
		horzDec = pow(0.5, 2.0/horz);  // 0.5^(1/horz)

	if (isnan(vert))      // default; copy horz
		vertDec = horzDec;
	else if (vert < 0.0)  // no decay, spread to infinity
		vertDec = 1.0;
	else if (vert == 0.0)
		vertDec = 0.0;   // full decay
	else
		vertDec = pow(0.5, 2.0/vert);  // 0.5^(1/horz)

	setDecrement(horzDec, vertDec);

}


void DistanceModelExponential::setDecrement(dist_t horz, dist_t vert){

	drain::Logger mout(getImgLog(), getName(), __FUNCTION__);

	if (horz < 0.0){
		mout.error() << "'horz' less than zero." << mout.endl;
		//throw std::runtime_error();
	}
	else if (horz > 1.0) {
		mout.error() << "'horz' greater than 1.0." << mout.endl;
		// throw std::runtime_error("setDecrement: 'horz' greater than 1.0.");
	}
	else
		horzDecay = horz;  // 0.5^(1/horz)  ????


	if (isnan(vert)){
		vertDecay = horzDecay;
	}
	else if (vert < 0.0){
		mout.error() << "'vert' less than zero." << mout.endl;
	}
	else if (vert > 1.0) {
		mout.error() << "'vert' greater than 1.0." << mout.endl;
	}
	else
		vertDecay = vert;

	const double hLog = log(horzDecay);
	const double vLog = log(vertDecay);
	diagDecay =       exp(-sqrt(    hLog*hLog +     vLog*vLog));
	knightDecayHorz = exp(-sqrt(4.0*hLog*hLog +     vLog*vLog));
	knightDecayVert = exp(-sqrt(    hLog*hLog + 4.0*vLog*vLog));


	mout.debug() << "decays: " << horzDecay << ", " << vertDecay;
	if (DIAG){
		mout << ", (" << diagDecay;
		if (KNIGHT){
			mout << ", (" << knightDecayHorz << ','  << knightDecayVert << ") ";
		}
		mout << ") ";
	}
	mout << mout.endl;

}




}
}

