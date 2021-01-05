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
//#include <typeinfo>
#include <stdexcept>
//#include <iostream>
//#include <vector>
#include <string>

#include <map>
#include <list>

#include "Log.h"
#include "Reference.h"
#include "String.h"
#include "SmartMap.h"

#ifndef REFERENCE_MAP
#define REFERENCE_MAP

// // // using namespace std;

namespace drain {


/// A map of references to base type scalars, arrays or std::string; changing values in either are equivalent operations.
/** A class designed for objects returned by CastableIterator. Contains a pointer to external object.
 *
 *  \example ReferenceMap-example.cpp
 */
// TODO: consider clear() (shallow op)
class ReferenceMap : public SmartMap<Referencer> {//public std::map<std::string,Referencer> {

public:

	/// Default constructor
	/**
	 *  \strictness - if true, allow attempts of assigning to non-existing entries; otherways throw exception on attempt.
	 */
	// ReferenceMap(bool ordered, char separator) : SmartMap<Referencer>(ordered, separator){}; //, STRICTLY_CLOSED, keys(orderedKeyList), units(unitMap)
	ReferenceMap(char separator=',') : SmartMap<Referencer>(separator){}; //, STRICTLY_CLOSED, keys(orderedKeyList), units(unitMap)

	/// Copy constructor copies only the separators; does not copy the items. \see copy()
	inline
	ReferenceMap(const ReferenceMap & rmap): SmartMap<Referencer>(rmap.separator, rmap.arraySeparator){
	};

	/// Associates a map entry with a variable
	/**
	 *  \param key - variable name in a map
	 *  \param x   - target variable to be linked
	 */
	template <class F>
	Referencer & link(const std::string & key, F &x, const std::string & unit = std::string()){

		if (find(key) == end()) // not  already referenced
			keyList.push_back(key);

		// Create
		Referencer & r = std::map<std::string,Referencer>::operator[](key);
		r.setSeparator(arraySeparator); // applicable, if array type element
		// Link
		r.link(x);
		unitMap[key] = unit;
		return r;
	}


	/// Associates a map entry with a variable, adding key in the beginning of key list.
	/**
	 *  \param key - variable name in a map
	 *  \param x   - target variable to be linked
	 */
	template <class F>
	Referencer & referenceTop(const std::string & key, F &x, const std::string & unit = std::string()){

		if (find(key) != end()) // already referenced
			keyList.push_front(key);

		//Referencer & r = std::map<std::string,Referencer>::operator[](key).link(x);
		Referencer & r = std::map<std::string,Referencer>::operator[](key);
		r.setSeparator(arraySeparator); // applicable, if array type element
		// Link
		r.link(x);

		unitMap[key] = unit;
		return r;
	}


	/// Adopts the references of r. If replace==false, only new entries are appended.
	void append(ReferenceMap & rMap, bool replace=true){
		//std::cerr << __FILE__ << " -> " << __FUNCTION__ <<  std::endl;
		const std::list<std::string> & keys = rMap.getKeyList();
		for (std::list<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it){
			//std::cerr << " -> " << *it <<  std::endl;
			if (replace || !hasKey(*it)){
				Referencer & item = rMap[*it];
				link(*it, item, rMap.unitMap[*it]).fillArray = item.fillArray;
			}
		}
	}

	// todo: consider NULL? (but leads to problems)

	/// Removes an entry from the map.
	inline
	void delink(const std::string & key){
		std::map<std::string,Referencer>::erase(key);
		for (std::list<std::string>::iterator it = keyList.begin(); it != keyList.end(); ++it)
			if (*it == key){
				keyList.erase(it);
				break;
			}
		unitMap.erase(key);
	}

	/// Removes all the elements of the map.
	/**
	 *  Clears the map and its ordered keylist (and units).
	 */
	virtual inline
	void clear(){
		SmartMap<Referencer>::clear();
		keyList.clear();
		unitMap.clear();
	}

	/// Experimental. Copies references and values of a structure to another.
	/**
	 * 	\param m - links to the members of the source object
	 * 	\param src - the source object
	 * 	\param copyValues - copy also the member values of the source object
	 * 	\param linkExternal - if yes, link also target variables outside the source object
	 *
	 *  Also updates key list and unit map?
	 */
	template <class T>
	void copyStruct(const ReferenceMap & m, const T & src, T & dst, bool copyValues = true, bool linkExternal = false){

		Logger mout(__FUNCTION__, __FILE__);
		long s = sizeof(T); // yes signed
		mout.warn() << "experimental, obj.size=" << s << mout.endl;

		clear();

		typedef unsigned long addr_t;
		typedef          long addr_diff_t;

		const addr_t srcAddr = (addr_t)(&src);
		const addr_t dstAddr = (addr_t)(&dst);
		for (std::list<std::string>::const_iterator it = m.getKeyList().begin(); it != m.getKeyList().end(); ++it){
			const std::string & key = *it;
			const Referencer & srcRef = m[key];
			addr_t srcVarAddr = (addr_t)srcRef.getPtr();
			addr_diff_t relativeAddr = srcVarAddr - srcAddr;
			if ((relativeAddr >= 0) && (relativeAddr < s)){ // INTERNAL
				Referencer & dstMemberRef = std::map<std::string,Referencer>::operator[](key); // key order?
				dstMemberRef.link((void *)(dstAddr + relativeAddr), srcRef.getType());
				// d.setInputSeparator()
				// d.setSeparator(arraySeparator);
				if (copyValues)
					dstMemberRef = srcRef; // value
			}
			else {
				if (linkExternal){
					Referencer & dstRef = std::map<std::string,Referencer>::operator[](key); // key order?
					dstRef.link((void *)srcVarAddr, srcRef.getType());
					if (copyValues)
						dstRef = srcRef;
				}
				else
					mout.warn() << "skipping external variable: '" << key << '=' << srcRef << "' relative addr=" << relativeAddr << mout.endl;
			}
		}
		separator = m.separator;
		arraySeparator = m.arraySeparator;
		keyList = m.keyList;
		unitMap = m.unitMap;

	}


	/// Import map, adopting the element types.
	template <class T>
	inline
	ReferenceMap & operator=(const SmartMap<T> & v){
		//Logger log(__FUNCTION__, __FILE__);
		//log.error() << "in:" << v << log.endl;
		importMap(v);
		return *this;
	}

	/// Return element associated with \c key.
	virtual
	mapped_type & operator[](const std::string &key){

		Logger mout(__FUNCTION__, __FILE__);

		iterator it = this->find(key);
		if (it != this->end()) {
			return it->second;
		}
		else {
			mout.error() << "key '" << key <<"' not declared (referenced)" << mout.endl;
			//throw std::runtime_error(key + ": ReferenceMap & operator[] : key not found");
			static mapped_type empty;
			return empty;
		}

	}

	///  Calling unreferenced key throws an exception.
	virtual
	const mapped_type & operator[](const std::string &key) const {

		Logger mout(__FUNCTION__, __FILE__); //Logger mout(__FUNCTION__, __FILE__);

		const_iterator it = this->find(key);
		if (it != this->end()) {
			return it->second;
		}
		else {
			mout.error() << "key '" << key <<"' not declared (referenced)" << mout.endl;
			//throw std::runtime_error(key + ": ReferenceMap & operator[] : key not found");
			static const mapped_type empty;
			return empty;
		}
	}

	typedef std::map<std::string,std::string> unitmap_t;

	/// Returns measurement unit information of the actual map entries.
	inline
	const unitmap_t & getUnitMap() const { return unitMap; };

	/// Creating a common segment for
	/*
	template <class T>
	static
	size_t serialize(const SmartMap<T> & rmap, std::vector<char> & memory);
	*/

	/// Associates a map entry with a variable
	/**
	 *  \param key - variable name in a map
	 *  \param x   - target variable to be linked
	 */


protected:

	/// Defines the units of each element.
	// Questionable: consider SmartReference etc. with caster and unit info?
	unitmap_t unitMap;


};

/*
template <class T>
size_t ReferenceMap::serialize(const SmartMap<T> & smap, std::vector<char> & memory){

	typedef SmartMap<T> smap_t;

	size_t i;
	for (smap_t::const_iterator it = smap.begin(); it != smap.end(); ++it){

		const std::string & key = it->first;
		const Castable & value  = it->second;

		if (v.isString()){

			///  Variable-length string datatype.

		}
		else {
		}

	}

}
*/

}  // namespace drain


#endif

// Drain
