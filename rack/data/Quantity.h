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
#ifndef ODIM_QUANTITY
#define ODIM_QUANTITY

#include <ostream>
//#include <set>

#include <stdexcept>

#include "drain/util/ReferenceMap.h"

#include "drain/util/Log.h"
#include "drain/util/Type.h"
#include "drain/util/Range.h"
#include "ODIM.h"
#include "Data.h"

namespace rack {

/// Structure for defining
/**
 *
 */
class Quantity : public std::map<char,EncodingODIM> {

public:

	/// Default storage type
	char defaultType;

	/// True, if a value corresponding a very small (unmeasurable) value has been defined.
	//bool hasUndetectValue;

	/// A physical value corresponding a very small (unmeasurable) value has been defined.
	/*
	 *  A physical value that represents all the measurement results that fall below a detection limit.
	 */
	double undetectValue;

	/// Default constructor
	Quantity(): defaultType('\0'), undetectValue(std::numeric_limits<double>::signaling_NaN()) {
		//undetectValue = std::numeric_limits<double>::signaling_NaN();
	}

	/// Declare encoding (a storage type and scaling) for this quantity.
	/*
	 *
	 *  Set defaultType, if unset.
	 */
	EncodingODIM & set(char typecode);


	/// Retrieve the scaling for a given storage type.
	inline
	const EncodingODIM & get(char typecode = '\0') const {

		if (!typecode)
			typecode = defaultType;

		const const_iterator it = find(typecode);

		if (it != end()){ // null ok
			return it->second;
		}
		else {
			//drain::Logger mout("Quantity", __FUNCTION__);
			//mout.warn() << "undefined code for this quantity, code=" << typecode << mout.endl;
			// TODO return default
			static EncodingODIM empty;
			return empty;
		}

	}

	/// Retrieve the scaling for a given storage type.
	inline
	const EncodingODIM & get(const std::string & t) const {
		if (t.length() != 1)
			//hrow (std::runtime_error(t + "<= illegal std::string in EncodingODIM::"+__FUNCTION__+" line "+__LINE__));
			throw (std::runtime_error(t+" <= illegal std::string, "+ __FUNCTION__));
		else
			return get(t.at(0));
	}

	/// True, if a value corresponding a very small (unmeasurable) value has been defined.
	inline
	bool hasUndetectValue() const {
		return !std::isnan(undetectValue);
	}

	/// Sets neutral value for purposes of interpolation.
	inline
	void setZero(double value){
		//hasUndetectValue = true;
		undetectValue = value;
	}

	/// Sets neutral value for purposes of interpolation.
	inline
	void unsetZero(){
		undetectValue = std::numeric_limits<double>::signaling_NaN();
		// hasUndetectValue = false;
		//undetectValue = std::numeric_limits<double>::min();
	}

	/// Sets absolute or typical range of this quantity.
	/**
	 *   For example, for probability the range would be [0.0, 1.0].
	 *   Setting range is useful when data are converted from some storage type to another
	 */
	inline
	void setPhysicalRange(double min, double max = std::numeric_limits<double>::max() ){
		// hasUndetectValue  = true;
		// undetectValue     = min;
		setZero(min);
		physicalRange.set(min, max);
	}

	/// Print declared encodings (storage types and scalings)
	std::ostream & toStream(std::ostream & ostr) const;

	drain::Range<double> physicalRange;

};

inline
std::ostream & operator<<(std::ostream & ostr, const Quantity & q){
	return q.toStream(ostr);
}



}  // namespace rack


#endif

// Rack
