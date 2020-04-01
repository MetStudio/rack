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
#ifndef DRAIN_CODEBOOK
#define DRAIN_CODEBOOK

//#include "Geometry.h"
//#include "File.h"
//#include "TreeSVG.h"

//#include "util/JSONtree.h"
#include <ostream>
#include <vector>
#include <map>

#include "../util/Log.h"
#include "../util/ValueScaling.h"

namespace drain
{

template <class T>
class LookUp : public std::vector<T> {

public:

	int bitShift;
	int byteSize;

};

///
/**   \tparam double - type of lower bound of the value range associated with an entry
 *    \tparam T - entry type (LegendEntry, PaletteEntry, HistogramEntry)
 *
 */
template <class T>
//class ImageCodeMap : public std::vector<T> {
class ImageCodeMap : public std::map<double,T> {

public:

	typedef std::map<double,T> cont_t;
	//typedef typename cont_t::size_type size_t;
	typedef typename cont_t::key_type     key_t;
	typedef typename cont_t::value_type entry_t;



	/// Default constructor
	//ImageCodeMap(size_t n=0, const T & value=T()): cont_t(n, value), separator(0) {
	ImageCodeMap() : separator(0) {
	};

	/// Copy constructor //
	// ImageCodeMap(const cont_t & v): cont_t(v), separator(0) {
	ImageCodeMap(const cont_t & map): separator(0) { // COPY?
	};


	/// LOOK-UP table, consider outsourcing this

	//typedef std::vector<typename cont_t::const_iterator> lookup_t;
	typedef LookUp<typename cont_t::const_iterator> lookup_t;

	mutable
	lookup_t lookUp;

	/// TODO? createLookUp(encoding, n=0);
	//lookup_t & createLookUp(int n, const ValueScaling & scaling, int shiftBits = 0) const {
	lookup_t & createLookUp(const std::type_info & type, const ValueScaling & scaling) const { // todo N?

		drain::Logger mout(__FUNCTION__, __FILE__);

		if (type == typeid(unsigned short)){
			lookUp.byteSize = drain::Type::call<drain::sizeGetter>(type);
			lookUp.bitShift	= 6; // note => 16 - 6 = 10bits => 1024 entries
		}
		else if (type == typeid(unsigned char)){
			lookUp.byteSize = drain::Type::call<drain::sizeGetter>(type);
			lookUp.bitShift	= 0;
		}
		else {
			mout.note() << "not supported for type " << drain::Type::getTypeChar(type) << mout.endl;
			lookUp.byteSize = 0;
			lookUp.bitShift = 0;
			lookUp.clear();
			return lookUp;
		}

		const int n = (1 << (lookUp.byteSize*8 - lookUp.bitShift));
		mout.warn() << "type=" << drain::Type::getTypeChar(type) << ", creating " << n << " lookup entries" << mout.endl;

		typename cont_t::const_iterator itLower = this->begin();

		lookUp.resize(n, itLower);

		// Signed, because scaling physical values may cause underflow
		int index, indexLower=0;

		/// Main loop: traverses entries, and fills up lookUp indices between entries.
		for (typename cont_t::const_iterator it=this->begin(); it!=this->end(); ++it){

			index = static_cast<int>(scaling.inv(it->first));

			if (index < 0){
				mout.warn() << "threshold " << it->first << " mapped to negative index " << index << " (before bitShift), skipping " << mout.endl;
				continue;
			}

			index = (index >> lookUp.bitShift);

			//if (static_cast<size_t>(index) >= n){
			if (index >= n){
				mout.warn() << "threshold " << it->first << " mapped to index (" << index << ") > max (" << (n-1) << "), skipping " << mout.endl;
				continue;
			}

			if (indexLower < index){
				mout.note() << "adding index range [" << indexLower << '-' << index << "[ -> ";
				mout << "(" << itLower->first << ") => {" << itLower->second << '}';
				mout << mout.endl;
			}
			else {
				mout.warn() << "downscaling: entry skipped at [" << index << "] => " << it->first << mout.endl;
			}

			/// Fill up interval [indexLower, index[
			for (int i=indexLower; i<index; ++i){
				lookUp[i] = itLower;
			}

			indexLower = index;
			itLower    = it;

		}

		index = n;
		if (indexLower < index){
			mout.debug() << "padding [" << indexLower << '-' << index << "[ -> \t";
			mout << '[' << itLower->first << "] // " << itLower->second << mout.endl;
		}
		for (int i=indexLower; i<index; ++i){
			lookUp[i] = itLower;
		}

		return lookUp;
	}

	// inline
	typename cont_t::const_iterator retrieve(double d) const {

		typename cont_t::const_iterator it = this->begin();
		typename cont_t::const_iterator rit = it; // result

		while (it != this->end()){
			if (it->first > d)
				return rit;
			rit = it;
			++it;
		}

		return rit; // may be invalid

	}

	// inline
	typename cont_t::iterator retrieve(double d) {

		typename cont_t::iterator  it = this->begin();
		typename cont_t::iterator rit = it;

		while (it != this->end()){
			if (it->first > d)
				return rit;
			rit = it;
			++it;
		}

		return rit; // may be invalid

	}


	/*
	inline
	const entry_t & retrieve(double intensity) const {

		key_t index;

		if (scaling.isScaled()){
			index = static_cast<key_t>(scaling.inv(intensity));
		}
		else {
			index = static_cast<key_t>(intensity);
		}

		return operator [](index);

	}

	inline
	entry_t & retrieve(double intensity) {

		key_t index;

		if (scaling.isScaled()){
			index = static_cast<key_t>(scaling.inv(intensity));
		}
		else {
			index = static_cast<key_t>(intensity);
		}

		return operator [](index);

	}
	*/

	// Rename to entryEmpty?
	/*
	static inline
	bool empty(unsigned short int i){
		return (i==0);
	}
	static inline
	bool empty(short int i){
		return (i==0);
	}

	static inline
	bool empty(int i){
		return (i==0);
	}

	static inline
	bool empty(long int i){
		return (i==0);
	}

	static inline
	bool empty(unsigned int i){
		return (i==0);
	}


	template <class E>
	static
	bool empty(const E & e){
		return e.empty();
	}
	*/

	///
	/*
	 *    \param equal - typically =, :, or -
	 *    \param start - typically hyphen or leading parenthesis (, {, [
	 *    \param end   - typically hyphen or trailing parenthesis ), }, [
	 *    \param separator - typically comma or semicolon
	 */

	// ValueScaling scaling; more like property of image

	char separator;
};

/*
class ImageCodeEntry {
public:

	virtual inline
	~ImageCodeEntry(){};

	virtual
	bool empty() const = 0;
};
*/


} // drain::

#endif
