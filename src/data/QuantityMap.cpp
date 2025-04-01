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

#include <drain/Log.h>
#include "Quantity.h"
#include "QuantityMap.h"

namespace rack {

void QuantityMap::assign(const std::initializer_list<std::pair<std::string, Quantity> > & inits) {
	for (const auto & entry: inits){
		(*this)[entry.first] = entry.second;
	}
	//return *this;
}

std::ostream & QuantityMap::toStream(std::ostream & ostr) const {
	for (const auto & entry: *this){
		ostr << entry.first << ' ';
		if (!entry.second.variants.empty()){
			ostr << '(' << entry.second.variants << ')' <<' ';
		}
		ostr << " – " << entry.second.name << '\n';
		ostr << entry.second; // << '\n';
	}
	return ostr;
}


void QuantityMap::initializeOLD(){


	drain::Logger mout(__FILE__, __FUNCTION__);
	/*
	// NEW:
	*this = {
			{"DBZH", {
					"Radar reflectivity",
					{"DBZ[HV]", "DBZ[HV]C", "T[HV]"},
					{-32.0, +60.0},
					'C',
					{
							{'C', 0.5, -32.0},
							{'S', 0.01, -0.01*(128*256)}
					},
					-32.0 // "virtual" zero
			}
			},
			{"DBZHDEV", {
					"Deviation of radar reflectivity",
					{"DBZH_DEV"},
					{-100.0, 100.0},
					'C',
					{
							{'C', 1.0, -128},
							{'S'}
					},
					0.0
			}
			},
			{"VRAD", {
					"Radial velocity",
					{"VRAD", "VRAD[HV]"},
					{-100.0, 100},
					'C',
					{
							{'C', 0.5, -64.0},
							{'S', {-100.0,+100.0}}
					}
			}
			},
			{"VRAD_DEV", {
					"Radial velocity, deviation",
					{},
					{-100.0, 100},
					'C',
					{
							{'C', 0.0, 64.0},
							{'S', 0, 128.0}
					}
			}
			},
			{"VRAD_DIFF", {
					"Radial velocity difference", {},
					{-100.0, 100},
					'C',
					{
							{'C', {-32, +32}},
							{'S', {-256.0,+256.0}}
					}
			}
			},
			{"RHOHV", {
					"Polarimetric cross-correlation", {},
					{0.0, 1.0},
					'S',
					{
							{'C', 0.004,  -0.004},
							{'S', 0.0001, -0.0001}
					}
			}
			},
			{"ZDR", {
					"Polarimetric difference", {},
					{0.0, 1.0},
					'S',
					{
							{'C', 0.1,  -12.8},
							{'S', 0.01, -0.01*(128.0*256.0)}
					}
			}
			},
			{"KDP", {
					"Polarimetric differential phase", {},
					{-120.0, +120.0},
					'S',
					{
							{'C'},
							{'S', 0.01, -0.01*(128.0*256.0)}
					}
			}
			},
			{"PHIDP", {
					"Polarimetric differential phase", {"PhiDP"},
					{-180.0, +180.0},
					'S',
					{
							{'C'},
							{'S'}
					}
			}
			},
			{"RATE", {
					"Precipitation rate", {},
					{-180.0, +180.0},
					'S',
					{
							{'C', 0.05},
							{'S', 0.0005}
					}
			}
			},
			{"HCLASS", {
					"Classification (Vaisala)", {"HydroCLASS"},
					'S',
					{
							{'C', 1.0},
							{'S', 1.0}
					}
			}
			},
			{"CLASS", {
					"Classification", {"CLASS_.*"},
					'S',
					{
							{'C', 1.0},
							{'S', 1.0}
					}
			}
			},
			{"QIND", {
					"Quality index", {"QIND_.*"},
					{0.0, 1.0},
					'C',
					{
							{'C', 1.0/250.0},
							{'S', 1.0/65535.0}
					}
			}
			},
			{"PROB", {
					"Probability", {"PROB_.*"},
					{0.0, 1.0},
					'C',
					{
							{'C', 1.0/250.0},
							{'S', 1.0/65535.0}
					}
			}
			},
			{"COUNT", {
					"Count", {"COUNT_.*"},
					'C',
					{
							{'C'}, {'S'}, {'I'}, {'L'}, {'f'}, {'d'}
					}
			}
			},
			{"AMV", {
					"Atmospheric motion [vector component]", {"AMV[UV]"},
					'C',
					{
							{'C', {-100,100}},
							{'c', {-127,127}},
							{'S', {-327.68, +327.68}},
							{'s', 0.01},
							{'f'},
							{'d'}
					}
			}
			},
	};
	*/

	/* keep this for debugging
	Quantity q1 = {
			"Count", {"COUNT_.*"},
			'C',
			{
					{'C'}, {'S'}, {'I'}, {'L'}, {'f'}, {'d'}
			}
	};
	*/


	//DBZHDEV.set('S').setRange(-100.0, +100.0);
	/*
	const bool FIRST_INIT = !hasQuantity("HGHT");

	if (ODIM::versionFlagger.isSet(ODIM::KILOMETRES)){
		if (!FIRST_INIT)
			mout.note("Using kilometres for HGHT and HGHTDEV (ODIM version: ", ODIM::versionFlagger, ")");
		set("HGHT", 'C').setScaling( 0.1);   //   255 => 25,5km
		set("HGHT", 'S').setScaling( 0.0004); // 65535 => 26.2 km
		set("HGHTDEV", 'C').setRange( -10.0, +10.0);   //   255 => 12.5km
		set("HGHTDEV", 'S').setRange( -20.0, +20.0); // 65535 => 13.x km
	}
	else {
		if (!FIRST_INIT)
			mout.note("Using metres for HGHT and HGHTDEV (ODIM version: ", ODIM::versionFlagger, ")");
		set("HGHT", 'C').setScaling( 100.0);  //   250 => 25500m
		set("HGHT", 'S').setScaling( 0.4);    // 65535 => 26.2 km
		set("HGHTDEV", 'C').setRange( -10000.0, +10000.0);   //   255 => 12.5km
		set("HGHTDEV", 'S').setRange( -20000.0, +20000.0); // 65535 => 13.x km
	}
	*/



	// typedef EncodingODIM ;
	if (false){
		// keep this for debugging ??
		for (auto & entry: *this){

			Quantity & q = entry.second;
			//std::cout << "// Quantity " << entry.first << " " << q.name << "\n";
			if (q.name.empty()){
				q.name = "Description...";
			}
			std::cout << "m[\"" << entry.first << "\"] = {";
			// std::cout << " {";

			std::cout << '"' << q.name << '"' << ", {";
			if (!q.physicalRange.empty())
				std::cout << q.physicalRange;
			std::cout << "}, '" << q.defaultType << "',\n";

			std::cout << "\t{";
			char sep = 0;
			for (const auto & entry2: entry.second){
				std::cout << sep; sep = ',';
				std::cout << "\n\t\t{";
				std::cout << "'" << entry2.first << "', ";
				const EncodingODIM & e2 = entry2.second;
				if (e2.explicitSettings & EncodingODIM::RANGE)
					std::cout << "{"<< e2.scaling.physRange << "}, " ;
				else
					std::cout << e2.scaling.scale << ',' << e2.scaling.offset << ", "; // << ' ';
				//std::cout << ", {"<< e2.scaling.scale << ',' << e2.scaling.offset << "}, " << e2.nodata << ", " << e2.undetect; // << ' ';
				std::cout << e2.nodata << ", " << e2.undetect; // << ' ';
				if (e2.explicitSettings & EncodingODIM::RANGE){
					std::cout << ", " << e2.scaling.scale << ',' << e2.scaling.offset; // << ' ';
				}
				else {
					if (!e2.scaling.physRange.empty())
						std::cout << ", {"<< e2.scaling.physRange << "} ";
				}
				std::cout << " }";

				if (e2.explicitSettings){
					std::cout << " /* " << drain::FlagResolver::getKeys(EncodingODIM::settingDict,e2.explicitSettings) << " */";
				}
				/*
				if (e2.explicitSettings & EncodingODIM::SCALING)
					std::cout << " / * scale * / ";
				if (e2.explicitSettings & EncodingODIM::RANGE)
					std::cout << " / * range * / ";
				*/
			}
			std::cout << "\n\t}";
			if (q.hasUndetectValue()){
				std::cout << ",\n\t" << q.undetectValue;
			}
			std::cout << "\n};\n\n";
		}

	}

}


QuantityMap::const_iterator QuantityMap::retrieve(const std::string & key) const {

		// Attempt 1: find exact match
		const_iterator it = find(key);

		// Attempt 2: find a compatible variant
		if (it == end()){
			//for (const auto & entry: *this){
			for (it=begin(); it!=end(); ++it){
				if (it->second.variants.test(key, false)){
					std::cerr << "NEW: found " << key << " <-> " << it->first << " [" << it->second << "]";
					// return entry.second;
					break;
				}
			}
			// Give up, leaving: it==end()
		}
		return it;

}


QuantityMap::iterator QuantityMap::retrieve(const std::string & key) {

	// Attempt 1: find exact match
	iterator it = find(key);

	// Attempt 2: find a compatible variant
	if (it == end()){
		//for (const auto & entry: *this){
		for (it=begin(); it!=end(); ++it){
			if (it->second.variants.test(key, false)){
				std::cerr << "NEW: found " << key << " <-> " << it->first << " [" << it->second << "]";
				// return entry.second;
				break;
			}
		}
		// Give up, leaving: it==end()
	}
	return it;

}

const Quantity & QuantityMap::get(const std::string & key) const {

		// const const_iterator it = find(key); // revised 2025
		const const_iterator it = retrieve(key); // revised 2025

		if (it != end()){ // null ok
			return it->second;
		}
		else {
			//drain::Logger mout("Quantity", __FUNCTION__);
			//mout.warn("undefined quantity=" , key );
			static const Quantity empty;
			return empty;
		}

}

Quantity & QuantityMap::get(const std::string & key) {

	// const iterator it = find(key); // revised 2025
	const iterator it = retrieve(key); // revised 2025

	// Attempt 1: find fully matching one.
	if (it != end()){ // null ok
		return it->second;
	}
	else {
		// Warning: if this is declared (modified), further instances will override and hence confuse
		static Quantity empty;
		return empty;
	}

}

/*
const Quantity & QuantityMap::findApplicable(const std::string & key) const {
	throw std::runtime_error(__FUNCTION__);
}
*/


bool QuantityMap::setQuantityDefaults(EncodingODIM & dstODIM, const std::string & quantity, const std::string & values) const {  // TODO : should it add?

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug3("for quantity=" , quantity , ", values=" , values );
	//if (quantity.empty())
	//	quantity = dstODIM.quantity;

	drain::ReferenceMap refMap;
	if (!values.empty()){
		dstODIM.grantShortKeys(refMap);
		refMap.setValues(values);   // essentially, sets dstODIM.type (str values will be reset, below)
	}

	mout.debug3("searching for quantity=" , quantity );
	//const_iterator it = find(quantity);
	const_iterator it = retrieve(quantity);
	if (it != end()){

		mout.debug2("found quantity '"  , quantity , "'" );

		/// Use user-defined type. If not supplied, use default type.
		if (dstODIM.type.empty()) {
			if (it->second.defaultType)
				dstODIM.type = it->second.defaultType;
			else {
				mout.warn("type unset, and no defaultType defined for quantity=" , quantity );
				return false;
			}
		}

		/// find type conf for this Quantity and basetype
		const char typechar = dstODIM.type.at(0);
		Quantity::const_iterator qit = it->second.find(typechar);
		if (qit != it->second.end()){
			//std::cerr << "OK q=" << quantity << ", type=" << typechar << std::endl;
			// initialize values to defaults
			dstODIM = qit->second;
			//dstODIM.updateFromMap(qit->second);
			// finally, set desired scaling values, overriding those just set...
			if (!values.empty()){
				refMap.setValues(values);
			}
			mout.debug2("updated dstODIM: "  , dstODIM );
			return true;
		}
		else {
			// error: type
			mout.info("quantity=" , quantity , " found, but no conf for typechar=" , typechar );
		}
	}
	else {
		mout.info("quantity=" , quantity , " not found" );
	}

	if (!dstODIM.type.empty()) {
		//const char typechar = dstODIM.type.at(0);
		const drain::Type t(dstODIM.type);
		mout.debug("applying universal defaults (1,0,min,max) for typechar=" , t );
		dstODIM.scaling.set(1.0, 0.0); // check! Maybe "physical" range 0.0 ...1.0 better than gain=1.0
		dstODIM.undetect = drain::Type::call<drain::typeMin, double>(t); //drain::Type::getMin<double>(typechar);
		dstODIM.nodata =   drain::Type::call<drain::typeMax, double>(t); //drain::Type::call<drain::typeMax,double>(typechar);
		// finally, set desired scaling values, overriding those just set...
		if (!values.empty()){
			refMap.setValues(values);
		}
	}

	return false;

}

QuantityMap & getQuantityMap() {

	static QuantityMap quantityMap = {
			{"DBZH", {
					"Radar reflectivity",
					{"DBZ[HV]", "DBZ[HV]C", "T[HV]"},
					{-32.0, +60.0},
					'C',
					{
							{'C', 0.5, -32.0},
							{'S', 0.01, -0.01*(128*256)}
					},
					-32.0 // "virtual" zero
			}
			},
			{"DBZHDEV", {
					"Deviation of radar reflectivity",
					{"DBZH_DEV"},
					{-100.0, 100.0},
					'C',
					{
							{'C', 1.0, -128},
							{'S'}
					},
					0.0
			}
			},
			{"VRAD", {
					"Radial velocity",
					{"VRAD", "VRAD[HV]"},
					{-100.0, 100},
					'C',
					{
							{'C', 0.5, -64.0},
							{'S', {-100.0,+100.0}}
					}
			}
			},
			{"VRAD_DEV", {
					"Radial velocity, deviation",
					{},
					{-100.0, 100},
					'C',
					{
							{'C', 0.0, 64.0},
							{'S', 0, 128.0}
					}
			}
			},
			{"VRAD_DIFF", {
					"Radial velocity difference", {},
					{-100.0, 100},
					'C',
					{
							{'C', {-32, +32}},
							{'S', {-256.0,+256.0}}
					}
			}
			},
			{"RHOHV", {
					"Polarimetric cross-correlation", {},
					{0.0, 1.0},
					'S',
					{
							{'C', 0.004,  -0.004},
							{'S', 0.0001, -0.0001}
					}
			}
			},
			{"ZDR", {
					"Polarimetric difference", {},
					{0.0, 1.0},
					'S',
					{
							{'C', 0.1,  -12.8},
							{'S', 0.01, -0.01*(128.0*256.0)}
					}
			}
			},
			{"KDP", {
					"Polarimetric differential phase", {},
					{-120.0, +120.0},
					'S',
					{
							{'C'},
							{'S', 0.01, -0.01*(128.0*256.0)}
					}
			}
			},
			{"PHIDP", {
					"Polarimetric differential phase", {"PhiDP"},
					{-180.0, +180.0},
					'S',
					{
							{'C'},
							{'S'}
					}
			}
			},
			{"RATE", {
					"Precipitation rate", {"RAINRATE"},
					'S',
					{
							{'C', 0.05},
							{'S', 0.0005}
					}
			}
			},
			{"HCLASS", {
					"Classification (Vaisala)", {"HydroCLASS"},
					'S',
					{
							{'C', 1.0},
							{'S', 1.0}
					}
			}
			},
			{"CLASS", {
					"Classification", {"CLASS_.*"},
					'S',
					{
							{'C', 1.0},
							{'S', 1.0}
					}
			}
			},
			{"QIND", {
					"Quality index", {"QIND_.*"},
					{0.0, 1.0},
					'C',
					{
							{'C', 1.0/250.0},
							{'S', 1.0/65535.0}
					}
			}
			},
			{"PROB", {
					"Probability", {"PROB_.*"},
					{0.0, 1.0},
					'C',
					{
							{'C', 1.0/250.0},
							{'S', 1.0/65535.0}
					}
			}
			},
			{"COUNT", {
					"Count", {"COUNT_.*"},
					'C',
					{
							{'C'}, {'S'}, {'I'}, {'L'}, {'f'}, {'d'}
					}
			}
			},
			{"AMV", {
					"Atmospheric motion [vector component]", {"AMV[UV]"},
					'C',
					{
							{'C', {-100,100}},
							{'c', {-127,127}},
							{'S', {-327.68, +327.68}},
							{'s', 0.01},
							{'f'},
							{'d'}
					}
			}
			},
	};

	drain::Logger mout(__FILE__, __FUNCTION__);

	const bool FIRST_INIT = !quantityMap.hasQuantity("HGHT");

	if (ODIM::versionFlagger.isSet(ODIM::KILOMETRES)){
		if (!FIRST_INIT){
			mout.note("Using kilometres for HGHT and HGHTDEV (ODIM version: ", ODIM::versionFlagger, ")");
		}
		quantityMap["HGHT"] = {
				"Height/altitude [km]", {"ALT", "ALTITUDE"},
				'S',
				{
						{'C', 0.01},    //   255 => 25,5km
						{'S', 0.0004}   // 65535 => 26.2 km
				}
		};
		quantityMap["HGHTDEV"] = {
				"Height/altitude deviation [km]", {"ALT", "ALTITUDE"},
				'S',
				{
						{'C', {-10.0, +10.0}}, //   255 => 12.5km
						{'S', {-20.0, +20.0}}  // 65535 => 13.x km
				}
		};
	}
	else {

		if (!FIRST_INIT){
			mout.note("Using metres for HGHT and HGHTDEV (ODIM version: ", ODIM::versionFlagger, ")");
		}

		quantityMap["HGHT"] = {
				"Height/altitude [m]", {"ALT", "ALTITUDE"},
				'S',
				{
						{'C', 100.0},
						{'S', 0.4}
				}
		};
		quantityMap["HGHTDEV"] = {
				"Height/altitude deviation [m]", {"HGHT_?DEV","ALTDEV", "ALTITUDE"},
				'S',
				{
						{'C', {-10000.0, +10000.0}},
						{'S', {-20000.0, +20000.0}}
				}
		};
	}

	return quantityMap;
}


}  // rack::


