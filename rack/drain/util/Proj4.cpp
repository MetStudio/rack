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

#include "Log.h"
#include "Proj4.h"
#include "String.h"

namespace drain
{

Proj4::Proj4() : projSrc(NULL), projDst(NULL) {
}

Proj4::Proj4(const Proj4 &p) : projSrc(NULL), projDst(NULL) {
	//cerr << "Proj4(const Proj4 &p) - someone needs me" << endl;
	setProjectionSrc(p.getProjectionSrc());
	setProjectionDst(p.getProjectionDst());
}

Proj4::~Proj4(){
	pj_free( projSrc );
	pj_free( projDst  );
}

// Proj4::epsg_dict_t Proj4::epsgDict;

void Proj4::_setProjection(const std::string & str, projPJ & p){

	drain::Logger mout(__FILE__, __FUNCTION__);

	pj_free(p);

	// Check plain number
	const int EPSG = atoi(str.c_str()); //drain::StringTools::convert<int>(s);
	if (EPSG > 0){
		/*
		mout.debug2("Detected plain numeric code (", EPSG, "), probably ESPG then.");
		const epsg_dict_t & d = Proj4::getEpsgDict();
		epsg_dict_t::const_iterator it = d.findByKey(EPSG);
		if (it != d.end()){
			mout.info("Found predefined (hard-coded) EPSG=", EPSG);
			p = pj_init_plus(it->second.c_str());
		}
		else */
		{
			std::string init = "+init=epsg:" +  str;
			mout.debug("Trying ", init);
			p = pj_init_plus(init.c_str());
		}
	}
	else {
		p = pj_init_plus(str.c_str());
	}

	if (p == NULL){
		throw std::runtime_error(std::string("proj4 error: ") + pj_strerrno(*pj_get_errno_ref())+ " (" + str + ")");
	}
	
}

/*
const Proj4::epsg_dict_t & Proj4::getEpsgDict(){ // Note: perhaps later non-const version needed

	static epsg_dict_t epsgDict;

	if (epsgDict.empty()){

		epsgDict = {
				{4326, "+proj=longlat +datum=WGS84 +no_defs"},
				{3067, "+proj=utm +zone=35 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs"},
				{3844, "+proj=laea +lat_0=52 +lon_0=10 +x_0=4321000 +y_0=3210000 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs"},
				{3035, "+proj=sterea +lat_0=46 +lon_0=25 +k=0.99975 +x_0=500000 +y_0=500000 +ellps=krass +towgs84=33.4,-146.6,-76.3,-0.359,-0.053,0.844,-0.84 +units=m +no_defs"},
		};

	}
	return epsgDict;
}
*/

short int Proj4::pickEpsgCode(const std::string & projDef){

	drain::Logger mout(__FILE__, __FUNCTION__);

	std::set<std::string> projArgs;
	drain::StringTools::split(projDef, projArgs, ' ');

	// short epsg = 0;
	for (const std::string & arg: projArgs){
		std::string key;
		std::string value;
		StringTools::split2(arg, key, value, '=');
		if (key == "+init"){
			mout.debug("+init detected");

			std::string k;
			short epsg;
			StringTools::split2(value, k, epsg, ':');
			if (k == "epsg"){
				mout.debug("detected EPSG: ", epsg);
				//break;
				return epsg;
			}
			else {
				mout.warn("+init detected,  but without EPSG setting, arg=", arg);
			}

		}
	}

	return 0;

}

std::ostream & operator<<(std::ostream & ostr, const Proj4 &p){
	ostr << "Proj4 src: " << p.getProjectionSrc() << '\n';
	ostr << "Proj4 dst: " << p.getProjectionDst() << '\n';
	return ostr;
}


}

// Drain
