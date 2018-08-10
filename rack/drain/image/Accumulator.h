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
#ifndef ACCUMULATOR_H_
#define ACCUMULATOR_H_

#include <limits>
#include <math.h>

#include <stdexcept>

//#include "util/Data.h"
#include "util/Variable.h"
//#include "util/DataScaling.h"

#include "Point.h"
#include "ImageT.h"
#include "Coordinates.h"
//#include "Cumulator.h"
#include "AccumulationArray.h"
#include "AccumulationMethods.h"
#include "AccumulationConverter.h"



// TODO: image/
/** See also radar::Compositor
 * 
 */
namespace drain
{

namespace image
{




// New
/**
 *
 *   Accumulator contains five built-in accumulation methods (principles):
 *   -# overwrite
 *   -# maximum
 *   -# average
 *   -# weighted average
 *   -# maximum weight
 *   The methods can be extended user defined methods with addMethod() .
 *
 */
class Accumulator : public AccumulationArray {

public:

	Accumulator() :
		undefinedMethod(*this),
		overwriteMethod(*this),
		maximumMethod(*this),
		minimumMethod(*this),
		averageMethod(*this),
		weightedAverageMethod(*this),
		maximumWeightMethod(*this),
		methodPtr(&undefinedMethod)
	{

		// setMethod(_overwriteMethod);

		addMethod(overwriteMethod);
		addMethod(maximumMethod);
		addMethod(minimumMethod);
		addMethod(averageMethod);
		addMethod(weightedAverageMethod);
		addMethod(maximumWeightMethod);


	};

	void setMethod(const std::string & method);

	//void setMethod(const std::string & name, const Variable & parameters);
	void setMethod(const std::string & name, const std::string & params);

	/// Copies the method and its parameters.
	//  Does not copy the target (accumulator &).
	inline
	void setMethod(const AccumulationMethod & method){
		//methodPtr = & method;
		// std::cerr << "setMethod(const AccumulationMethod & method): " <<  method.name << std::endl;
		setMethod(method.name, method.getParameters().getValues());  // But this limits to those already registered?
	}

	inline
	bool isMethodSet() const {
		return (methodPtr != & undefinedMethod);
	}

	inline
	const AccumulationMethod & getMethod() const {
		return *methodPtr;
	}

	inline
	AccumulationMethod & getMethod() {
		return *methodPtr;
	}


	void addMethod(AccumulationMethod & method){
		// std::cerr << "addMethod:" << method.name << '\t' << method << '\n';
		methods.insert(std::pair<std::string, AccumulationMethod &>(method.name, method));
	}

	/// Adds decoded data that applies natural scaling.
	inline
	void add(const size_t &i, double value, double weight) {
		methodPtr->add(i, value, weight);
	}

	void add2(const size_t &i, double value, double value2, double weight) {
		methodPtr->add(i, value, weight);
	}


	/// NEW
	void addData(const Image & src, const Image & srcQuality, const AccumulationConverter & converter, double weight = 1.0, int iOffset=0, int jOffset=0);

	void addData(const Image & src, const AccumulationConverter & converter, double weight = 1.0, int iOffset=0, int jOffset=0);

	/// Extracts the accumulated quantity or secondary quantities like weight and standard deviation.
	/**
	 *
	 * \param field - data layer to be extracted
	 *  - d = data, scaled
	 *  - w = weight, scaled
	 *  - c = count
	 *  - s = standard deviation, unscaled
	 *
	 *  Perhaps implemeted later:
	 *  - S = standard deviation, scaled
	 *  - p = data, sum of squared ("power"), scaled
	 *  - D = data, cumulated  (debugging)
	 *  - W = weight, cumulated
	 *  - C = count, scaled
	 *  - S = standard deviation, unscaled
	 *
	 */
	void extractField(char field, const AccumulationConverter & converter, Image & dst) const;

	inline
	std::string toStr(){
		std::stringstream sstr;
		toStream(sstr);
		return sstr.str();
	}

	virtual
	std::ostream & toStream(std::ostream & ostr) const {
		ostr << "Accumulator ("<< width << 'x' << height << ") ";
		ostr << " ["<< getMethod() << "] ";
		/*
		for (std::map<std::string, AccumulationMethod &>::const_iterator it = methods.begin(); it != methods.end(); it++)
			ostr << it->second << ',';
		*/
		//ostr << '\n';
		return ostr;
	};

public:

	/// A Some predefined methods, that can be set with setMethod(const std::string & key).
	std::map<std::string, AccumulationMethod &> methods;

protected:


	AccumulationMethod undefinedMethod;
	OverwriteMethod overwriteMethod;
	MaximumMethod   maximumMethod;
	MinimumMethod minimumMethod;
	AverageMethod   averageMethod;
	WeightedAverageMethod weightedAverageMethod;
	MaximumWeightMethod maximumWeightMethod;

	AccumulationMethod * methodPtr;


};


std::ostream & operator<<(std::ostream &ostr, const Accumulator & cumulator);


}  // image::

}  // drain::

#endif /* Cumulator_H_ */

// Drain
