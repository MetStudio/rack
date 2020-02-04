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


#include "TextReader.h"

#include "JSON.h"

namespace drain
{


void JSONreader::fromStream(std::istream & istr, Variable & v){

	drain::Logger log("JSONreader", __FUNCTION__);

	TextReader::skipChars(istr, " \t\n\r");

	std::string value;

	int c = istr.peek();
	switch (c) {
	// consider:
	// case '\'': // STRING
	case '"': // STRING
		istr.get();
		v = TextReader::scanSegment(istr, "\"");
		istr.get(); // swallow '"'
		// std::cout << "String: " << value << '\n';
		log.debug(2) << "String value '" << v << "'" << log.endl;
		break;
	case '[': // ARRAY TODO: chars/integer/float handling
		istr.get();
		value = TextReader::scanSegment(istr, "]");
		istr.get(); // swallow ']'
		/*
			if (value.find_first_of("{}") != std::string::npos){
				log.warn() << "Arrays of objects not supported (key='" << key << "')" << log.endl;
			}
		 */
		if (value.find_first_of("[]") != std::string::npos){
			log.warn() << "Arrays of arrays not supported (value='" << value << "')" << log.endl;
		}
		JSONreader::arrayFromStream(value, v);
		break;
	default: // numeric
		value = TextReader::scanSegment(istr, ",} \t\n\r");
		const std::type_info & type = Type::guessType(value);
		// v.setType(type);
		v.requestType(type);
		//log.debug(2) << "Numeric attribute '" << key << "'= " << value << ", type=" << drain::Type::getTypeChar(type) << log.endl;
		log.debug(2) << "Value " << value << ", type=" << drain::Type::getTypeChar(type) << log.endl;
		v = value;
		break;
	}
	//completed = true;

}


void JSONreader::arrayFromStream(const std::string & s, Variable & v){

	v.clear();

	std::vector<std::string> values;
	drain::StringTools::split(s, values, ',', " '\t\n\r");

	// TODO: recurse values twice such that they become converted trough friendly types (esp. true => 1)
	const std::type_info & atype = Type::guessArrayType(values);
	// Likewise, Type::getCompatibleType(typeid(bool), typeid(float));
	v.requestType(atype);
	v = values;

}



} // drain::



// Drain
