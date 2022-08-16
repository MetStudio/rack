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


#include <fstream>

#include "drain/util/Dictionary.h"
#include "drain/util/TypeUtils.h"
#include "drain/util/JSON.h"

#include "Hi5.h"


// using namespace std;

namespace hi5 {

//const drain::RegExp fileNameRegExp("^((.*/)?([^/]+))\\.(h5|hdf5)$", REG_EXTENDED | REG_ICASE);

drain::FileInfo fileInfo("h5|hdf5|hdf");


//void NodeHi5::writeText(std::ostream &ostr, const std::string & prefix) const {

void NodeHi5::writeText(std::ostream &ostr, const rack::ODIMPath & prefix) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	/// show only "big" groups explicitly?
	/// if (prefix.back().belongsTo(...))
	/// or Uncomment:
	//if (attributes.empty() && dataSet.isEmpty()){
	//if (!prefix.empty())
	ostr << prefix;
	if (noSave)
		ostr << '~';
	ostr << '\n';
	//	return;
	//}


	for (drain::VariableMap::const_iterator it = attributes.begin(); it != attributes.end(); it++){
		if (!prefix.empty())
			ostr << prefix << ':'; //'\t';
		ostr << it->first << '=';
		drain::JSONwriter::toStream(it->second, ostr);
		//ostr << ' ' << drain::Type::getTypeChar(it->second.getType());
		ostr << '\n';
	}

	//const drain::image::Image &d = n.dataSet;
	if (dataSet.getVolume() > 0){

		ostr << prefix;
		//if (noSave) ostr << '~';
		ostr << ':';
		//'\t';
		//mout.note() << dataSet.getGeometry() << mout.endl;
		if (dataSet.getGeometry().channels.getChannelCount() <= 1)
			ostr << "image=[" << dataSet.getWidth() << ',' << dataSet.getHeight() << ']';
		else
			ostr << "image=[" << dataSet.getWidth() << ',' << dataSet.getHeight() << ',' << dataSet.getChannelCount() << ']';
		// TODO:
		// ostr  << ' ' << '[' << drain::Type::getTypeChar(dataSet.getType()) << '@' << dataSet.getEncoding().getElementSize() << ']' << '\n';  // like typeInfo above
		// ostr << " # " << drain::Type::call<drain::complexName>(dataSet.getType());
		ostr << '\n';
	}

}

std::ostream &operator<<(std::ostream &ostr,const hi5::NodeHi5 &n){
	n.writeText(ostr);
	return ostr;
}


//Hi5Error Hi5Base::debug;
//drain::Log Hi5Base::hi5monitor;
//drain::Logger Hi5Base::hi5mout(Hi5Base::hi5monitor,"hi5");
drain::Log hi5monitor;
drain::Logger hi5mout(hi5monitor,"Hi5");


void Hi5Base::handleStatus(herr_t status, const std::string & message, drain::Logger &mout, int lineNo){

	if (status >= 0)
		return;

	mout.warn() << message;
	if (lineNo)
		mout << ", line=" << lineNo;
	mout << mout.endl;

}

// https://support.hdfgroup.org/HDF5/doc/RM/PredefDTypes.html
hid_t Hi5Base::getH5StandardType(const std::type_info & type){


	drain::Logger mout(__FUNCTION__, __FILE__);

	typedef drain::Dictionary2Ptr<hid_t, const std::type_info> dict_t;

	static dict_t dict;
	if (dict.empty()){

		dict.add(H5T_STD_I8BE,  typeid(char));
		dict.add(H5T_STD_U8BE,  typeid(unsigned char));
		dict.add(H5T_STD_I16BE, typeid(short int));
		dict.add(H5T_STD_U16BE, typeid(unsigned short int));
		dict.add(H5T_STD_I32BE, typeid(int));
		dict.add(H5T_STD_U32BE, typeid(unsigned int));
		dict.add(H5T_STD_I64BE, typeid(long int));
		dict.add(H5T_STD_U64BE, typeid(unsigned long int));

		dict.add(H5T_IEEE_F32BE, typeid(float));
		dict.add(H5T_IEEE_F64BE, typeid(double));

		// String
		dict.add(Hi5Base::getH5StringVariableLength(), typeid(char *));
		dict.add(Hi5Base::getH5StringVariableLength(), typeid(const char *));

		//if (type == typeid(const char *)){
		//return getH5StringVariableLength();
		//hid_t strtype = H5Tcopy(H5T_C_S1);
		//herr_t status = H5Tset_size(strtype, H5T_VARIABLE);

	}


	dict_t::const_iterator it = dict.findByValue(type);
	if (it != dict.end()){
		mout.debug() << "type_info=" << type.name() << ", size:" <<  H5Tget_size(it->first) << mout.endl;
		return it->first;
	}
	else {
		mout.warn() << "could not find HDF5 type for type_info=" << type.name() << mout.endl;
		return 0;
	}
}


hid_t Hi5Base::getH5NativeDataType(const std::type_info &type){


	if (type == typeid(bool)){  // does not work
		hi5mout.warn() << __FUNCTION__ << ": boolean type '" << type.name() << "' currently unsupported" << hi5mout.endl;
		return H5T_NATIVE_HBOOL; // experimental
	}
	else if (type == typeid(char)){
		return H5T_NATIVE_CHAR;
	}
	else if (type == typeid(unsigned char)){
		return H5T_NATIVE_UCHAR;
	}
	else if (type == typeid(short)){
		return H5T_NATIVE_SHORT;
	}
	else if (type == typeid(unsigned short)){  // and sizeof() = 16? H5T_STD_I16LE
		return H5T_NATIVE_USHORT;
	}
	else if (type == typeid(int)){
		return H5T_NATIVE_INT;
	}
	else if (type == typeid(unsigned int)){
		return H5T_NATIVE_UINT;
	}
	else if (type == typeid(long)){
		return H5T_NATIVE_LONG;
	}
	else if (type == typeid(unsigned long)){
		return H5T_NATIVE_ULONG;
	}
#ifdef  STDC99
	else if (type == typeid(long long)){
		return H5T_NATIVE_LLONG;
	}
	else if (type == typeid(unsigned long long)){
		return H5T_NATIVE_ULLONG;
	}
#endif
	else if (type == typeid(float)){
		return H5T_NATIVE_FLOAT;
	}
	else if (type == typeid(double)){
		return H5T_NATIVE_DOUBLE;
	}
	else if (type == typeid(const char *)){
		return getH5StringVariableLength();
	}
	else if (type == typeid(void)){
		hi5mout.warn() << __FUNCTION__ << "void type requested, setting string type" << hi5mout;
		return getH5StringVariableLength();
	}
	/*
	else if (type == typeid(std::string)){
		return getH5StringVariableLength();
	}
	*/
	else {
		hi5mout.error() << __FUNCTION__ << ": unsupported data type:'" << type.name() << '\'' << hi5mout.endl;
		return H5T_NATIVE_UCHAR; /// TODO
	}

}

hid_t Hi5Base::getH5StringVariableLength(){

	hid_t strtype = H5Tcopy(H5T_C_S1); // todo: delete dynamic?
	//herr_t status =
	H5Tset_size(strtype, H5T_VARIABLE);
	//if (status < 0)hi5mout.error() << "H5T_C_S1 => H5Tset_size failed " << hi5mout.endl;
	return strtype;

}


//drain::image::Image &  Hi5Base::getPalette(Hi5Tree & dst){
//Hi5Tree &
drain::image::Image & Hi5Base::getPalette(Hi5Tree & dst){

	//Hi5Tree & palette = dst["palette"];

	drain::image::Image & data = dst.data.dataSet;
	if (data.isEmpty()){
		data.setType<unsigned char>();
		data.setGeometry(3, 256);
		// TEST data
		for (int i = 0; i < 256; ++i){
			data.put(0, i, i);
			data.put(1, i, 128-abs(i-128));
			data.put(2, i, 255-i);
		}
		/*
		for (drain::image::Image::iterator it = data.begin(); it != data.end(); ++it){
			*it = rand()&0xff;
		}
		*/
		drain::VariableMap & attributes = dst.data.attributes;
		attributes["CLASS"] = "PALETTE"; // palette marker for Hi5Writer
		//attributes["IMAGE_SUBCLASS"] = "PALETTE"; // palette marker for Hi5Writer
		/*
		attributes["PAL_COLORMODEL"] = "RGB";
		// attributes["PAL_MINMAXNUMERIC"] = std::string(); // << min , max "0,255";
		attributes["PAL_TYPE"] = "STANDARD8";
		attributes["PAL_VERSION"] = "1.2";
		 */
	}

	//return palette;
	return data;

}

void Hi5Base::linkPalette(const Hi5Tree & palette, Hi5Tree & dst){
	// drain::VariableMap & attributes = dst["data"].data.attributes;
	drain::VariableMap & attributes = dst.data.attributes;
	attributes["IMAGE_SUBCLASS"] = "IMAGE_INDEXED";
	attributes["palette_link"] = (long unsigned int) & palette; //(long unsigned int);
}

// const Hi5Tree &src,
void Hi5Base::writeText(const Hi5Tree &src, const std::list<Hi5Tree::path_t> & paths, std::ostream & ostr) {

	for (std::list<Hi5Tree::path_t>::const_iterator it = paths.begin(); it != paths.end(); ++it) {
		const std::string & key = *it;
		src(key).data.writeText(ostr, key);
	}
}

void Hi5Base::readText(Hi5Tree &src, std::istream & istr) {

	std::string line;

	while ( std::getline(istr, line) ){

		if (!line.empty() && (line.at(0)!='#')){

			// const size_t j = line.find('=');
			// if (j == std::string::npos)
			readTextLine(src, line);
			// else readTextLine(src, line.substr(0, j), line.substr(j+1));

		}

	}

}



void Hi5Base::readTextLine(Hi5Tree & dst, const std::string & line){

	drain::Logger mout(__FUNCTION__, __FILE__);

	Hi5Tree::path_t path;

	// OLD -> -> _
	// std::string attrKey;
	// std::string attrValue;
	// Hi5Base::parsePath(line, path, attrKey, attrValue);

	// NEW
	std::string assignment;
	// mout.debug() << line << " => ..." << mout;

	//Hi5Base::parsePath(line, path, assignment);
	drain::StringTools::split2(line, path, assignment, ":");
	assignAttribute(dst(path), assignment);
}

//void Hi5Base::readTextLine(Hi5Tree & dst, const Hi5Tree::path_t & path, const std::string & assignment){
void Hi5Base::assignAttribute(Hi5Tree & dst, const std::string & assignment){

	drain::Logger mout(__FUNCTION__, __FILE__);
	// NEW

	std::string attrKey;
	std::string attrValue;
	const bool VALUE_GIVEN = drain::StringTools::split2(assignment, attrKey, attrValue, "=");

	/*
	mout.debug2();
	mout << "'" << assignment << "' => ";
	mout << "'" << attrKey    << "' = '";
	mout << "'" << attrValue  << "' | ";
	mout << mout.endl;
	*/

	/// Create the node always
	// NodeHi5 & n = dst(path).data;
	NodeHi5 & n = dst.data;

	if (attrKey.empty())
		return;

	if (attrKey == "image"){
		n.dataSet.setType<unsigned char>();

		drain::Variable v;
		drain::JSONreader::readValue(attrValue, v);
		switch (v.getElementCount()) {
			case 3:
				n.dataSet.setGeometry(v.get<size_t>(0), v.get<size_t>(1), v.get<size_t>(2));
				break;
			case 2:
				n.dataSet.setGeometry(v.get<size_t>(0), v.get<size_t>(1));
				break;
			case 1:
				n.dataSet.setGeometry(v.get<size_t>(0), v.get<size_t>(0));
				mout.warn() << "image height not given, assuming height=width=" << v.get<size_t>(0) << mout.endl;
				break;
			default:
				mout.warn() << "wrong number of dimensions for image data: " << v << mout.endl;
				break;
		}
		//n.dataSet.setGeometry(v.get<size_t>(0), v.get<size_t>(1));
		// What if dim < 2
	}
	else { // non-image

		drain::Variable & a = n.attributes[attrKey];
		//mout.warn() << "hey " << drain::Type::call<drain::simpleName>(a.getType()) << mout.endl;
		if (VALUE_GIVEN){
			drain::JSONreader::readValue(attrValue, a, true);
			//mout.note() << "read: " << a << ", type=" << drain::Type::call<drain::simpleName>(a.getType()) << mout.endl;
			if (attrKey == "quantity"){
				if (n.attributes.get("gain", 0.0) == 0.0){
					mout.debug() << "Consider --completeODIM to proceed" << mout.endl;
				}
			}
		}

	}

}

/*
void Hi5Base::parsePath(const std::string & line, Hi5Tree::path_t & path, std::string & assignment){

	drain::Logger mout(__FUNCTION__, __FILE__);

	mout.debug() << "line: " << line << mout.endl;

	drain::StringTools::split2(line, path, assignment, ":");

}
*/


/// Split full path string to path object and attribute key.
// consider ValueReader, TextReader instead (skipping attrType)
void Hi5Base::parsePath(const std::string & line, Hi5Tree::path_t & path, std::string & attrKey, std::string & attrValue){

	drain::Logger mout(__FUNCTION__, __FILE__);

	mout.debug() << "line: " << line << mout.endl;

	std::string assignment;
	//Hi5Base::parsePath(line, path, assignment);
	drain::StringTools::split2(line, path, assignment, ":");

	drain::StringTools::split2(assignment, attrKey, attrValue, "=");

}



void Hi5Base::deleteNoSave(Hi5Tree &src){

	drain::Logger mout(__FUNCTION__, __FILE__);

	rack::ODIMPath l;

	for (Hi5Tree::iterator it = src.begin(); it != src.end(); ++it) {
		if (! it->second.data.noSave){ // needed?
			//mout.debug2() << "delete: " <<  it->first << mout.endl;
			deleteNoSave(it->second);
		}
		else {
			l.push_back(it->first);
		}
	}

	for (rack::ODIMPath::const_iterator it = l.begin(); it != l.end(); ++it){
		Hi5Tree::path_t p;
		p << *it;
		mout.debug2() << "delete group: " <<  *it << mout.endl;
		src.erase(p);
	}


}

/*
void Hi5Base::markNoSave(Hi5Tree &src, bool noSave){

	//drain::Logger mout(__FUNCTION__, __FILE__);

	for (Hi5Tree::iterator it = src.begin(); it != src.end(); ++it) {
		it->second.data.noSave = noSave;
		markNoSave(it->second, noSave);
	}


}
*/


std::ostream & operator<<(std::ostream &ostr, const Hi5Tree & tree){

	tree.dump(ostr);
	return ostr;
}


} // ::hi5

