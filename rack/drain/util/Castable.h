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
#include <string.h> // Old C
#include <typeinfo>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <string>

#include "Caster.h"
#include "String.h"

#ifndef CASTABLE
#define CASTABLE

// // using namespace std;

namespace drain {


//class Variable;

/// An object that can be automatically casted to and from a base type or std::string. Designed for objects returned by CastableIterator.
/**

  Saves type info (like Caster) and also a pointer to external object.
  The object ie. the memory resource is provided by the user.
  Supports arrays.

  If input is string:
  -

  \example Castable-example.cpp

  \see ReferenceMap
  \see Variable


 */
class Castable  {

public:

	inline
	Castable() : ptr(NULL), elementCount(0) {
		setSeparator();
		caster.unsetType();
	};

	/// Copy constructor
	Castable(const Castable &c) : ptr(NULL), elementCount(0) {
		setSeparator();
		setPtr(c);
	}

	// Obsolete?
	template <class F>
	Castable(F *p) : ptr(NULL), elementCount(0) {
		setSeparator();
		setPtr(p, typeid(F));
		// std::cerr << "Castable(F *p) type=" << typeid(F).name() << " value=" << *p << " (" << (double)*p << ")" << std::endl;
	}

	/// Constructor for an object pointing to a variable.
	template <class F>
	Castable(F &p) : ptr(NULL), elementCount(0) {
		setSeparator();
		setPtr(p);
	}

	inline
	~Castable(){};



	// Variable will hide std::string type.
	virtual inline
	const std::type_info & getType() const {
		return caster.getType();
	};

	/// Returns true, if type is \c char and outputSepator is null char.
	inline
	bool isCharArrayString() const {
		return ((caster.getType() == typeid(char)) && (outputSeparator=='\0'));  // "close enough" guess
	};

	/// Returns true, if type is std::string .
	inline
	bool isStlString() const {
		return (caster.getType() == typeid(std::string));
	};

	/// Returns true, if type is std::string .
	inline
	bool isString() const {
		return (isCharArrayString() || isStlString());
	};

	virtual inline
	bool typeIsSet() const {
		return caster.typeIsSet();
	};

	inline
	size_t getByteSize() const {
		return caster.getByteSize();
	};

	/// Returns the length of the array, the number of elements in this entry.
	/**
	 *
	 *
	 *   \see Castable::getByteSize().
	 */
	virtual inline
	size_t getElementCount() const {
		return elementCount;
	}

	/// Returns true, if no elements have been allocated. Does not check type. (NOTE: may change: 0 for scalars)
	/**
	 *   \see getByteSize()
	 *   \see getElementCount()
	 */
	inline
	size_t isEmpty() const {
		return (getElementCount() == 0);
	}

	/// Clears std::strings or sets numeric values to zero. Does not change type or resize.
	// Has been protected, even private. Why? Essentially setting a value.
	void clear();


	/// Copies the value referred to by Castable. Notice that if type is unset (void), no operation is performed.
	inline
	Castable &operator=(const Castable &c){
		return assignCastable(c);
	}

	/// Copies an arbitrary base type or std::string value.
	template <class T>
	Castable &operator=(const T &x){
		// std::cout << "assign " << x << " -> " << Type::getTypeChar(getType()) << ',' << getElementCount() << '\n';
		requestType(typeid(T));
		if (isString()){
			//std::cout << __FUNCTION__ << ':' << x << " -> " << Type::getTypeChar(getType()) << ',' << getElementCount() << '\n';
			assignToString(x);
		}
		else if (typeIsSet()){
			requestSize(1);
			caster.put(ptr, x);
		}
		else {
			throw std::runtime_error(std::string(__FUNCTION__) + ": type is unset");
		}
		return *this;
	}

	inline
	Castable & operator=(const std::string &c){
		assignString(c);
		return *this;
	}

	inline
	Castable & operator=(const char *c){
		assignString(c);
		return *this;
	}


	/// Copies elements of std::list
	template <class T>
	inline
	Castable &operator=(const std::list<T> & l){
		assignContainer(l);
		return *this;
	}

	/// Copies elements of std::vector
	template <class T>
	inline
	Castable &operator=(const std::vector<T> & v){
		assignContainer(v);
		return *this;
	}

	/// Copies elements of std::vector
	template <class T>
	inline
	Castable &operator=(const std::set<T> & s){
		assignContainer(s);
		return *this;
	}


	/// Appends the string or appends the array by one element.
	/*
	 *  Note: if both source and target are strings, does not split it like with Variable::operator=(const char *).
	 */
	template <class T>
	inline
	Castable &operator<<(const T &x){
		requestType(typeid(T));
		if (isString()){
			appendToString(x);
		}
		else {
			appendToElementArray(x);
		}
		return *this;
	}

	/// Appends the string or appends the array by one element.
	inline
	Castable &operator<<(const char *s){
		return *this << (std::string(s));
	}

	/// Appends elements of std::list
	template <class T>
	inline
	Castable &operator<<(const std::list<T> & l){
		assignContainer(l, true);
		return *this;
	}

	/// Appends elements of std::vector
	template <class T>
	inline
	Castable &operator<<(const std::vector<T> & v){
		assignContainer(v, true);
		return *this;
	}

	/// Appends elements of std::vector
	template <class T>
	inline
	Castable &operator<<(const std::set<T> & s){
		assignContainer(s, true);
		return *this;
	}

	template <class T>
	inline
	T get(size_t i) const {
		if (i >= elementCount)
			throw std::runtime_error("Castable::get() index overflow");
		return caster.get<T>(getPtr(i));
	}


	/// Conversion to std::string.
	/**
	 *  http://stackoverflow.com/questions/7741531/conversion-operator-template-specialization
	 */
	inline
	operator std::string() const {
		return toStr();
	}

	/// Conversion from the internal type.
	/// Conversion operator. Works well with basic types. Due to g++ error/feature (?), does not work implicitly for std::string.
	//  Thanks to Mikael Kilpeläinen.
	/**
	 *   Due to c++ compiler error/feature, does not work implicitly for std::string. For example:
	 *   \code
	 *   Variable vField;
	 *   vField = 1.23;
	 *   std::string s = vField;  // OK
	 *   s = vField;         // FAILS, for unknown reason
	 *   \endcode
	 *   See http://stackoverflow.com/questions/7741531/conversion-operator-template-specialization .
	 *   The conversion works only if a conversion operator is defined only for std::string (operator std::string()).
	 *   Adding any other conversions (int, double, ...) causes the compiler to fail.
	 */
	template <class T>
	operator T() const {
		if (isCharArrayString()){
			T x;
			std::stringstream sstr;
			sstr << getCharArray();
			sstr >> x;
			return x;
		}
		else
			return caster.get<T>(ptr);
	}



	inline
	bool operator==(const Castable &x){
		//(this->*putCastable)(x);
		throw std::runtime_error("Castable: operator== not implemented.");
		return false;
	}

	/// Compares a value to internal data.
	template <class T>
	bool operator==(const T &x){
		return (caster.get<T>(ptr) == x);
	}

	/// Compares a value to inner data.
	/**
	 *
	 */
	template <class T>
	bool operator!=(const T &x){
		return (caster.get<T>(ptr) != x);
	}

	/// Compares a value to inner data.
	template <class T>
	bool operator<(const T &x){
		return (caster.get<T>(ptr) < x);
	}

	/// Compares a value with inner data.
	template <class T>
	bool operator>(const T &x){
		return (caster.get<T>(ptr) > x);
	}

	/// The character used between array elements in output stream.
	/**
	 *  \see isCharArrayString
	 */
	inline
	void setSeparator(char c = ','){
		setInputSeparator(c);
		setOutputSeparator(c);
	}

	///
	/**
	 *   May append elements dynamically, there should be a marker for separating elements
	 *   is a string.
	 */
	inline
	void setInputSeparator(char c = ','){
		inputSeparator = c;
	}

	///
	/**
	 *   May append elements dynamically, there should be a marker for separating elements
	 *   is a string.
	 */
	inline
	void setOutputSeparator(char c = ','){
		outputSeparator = c;
	}

	//template <class S> // std::ostream
	std::ostream & toStream(std::ostream & ostr = std::cout, char separator='\0') const;

	std::string toStr() const;

	void toJSON(std::ostream & ostr = std::cout, char fill = ' ') const;

	std::ostream & valueToJSON(std::ostream & ostr = std::cout) const;

	const void typeInfo(std::ostream & ostr) const;



	/// Converts to simple iterable STL container.
	/**
	 *    \tparam T - std::set, std::list or std::vector
	 */
	template <class T>
	void toContainer(T & container) const { // FIX raise

		container.clear();

		if (isString()){
			StringTools::split(toStr(), container, std::string(1, this->outputSeparator)," \t\n");
		}
		else {
			for (size_t i = 0; i < getElementCount(); ++i) {
				container.insert(container.end(), caster.get<typename T::value_type>(getPtr(i)) );
			}
		}


	}


	/// Returns pointer to the array of chars without validity check.
	/**
	 *  \see getCharArray
	 */
	inline
	char * getPtr(size_t i = 0) {
		return &((char *)ptr)[i*caster.getByteSize()];
	}

	inline
	const char * getPtr(size_t i = 0) const {
		return &((const char *)ptr)[i*caster.getByteSize()];
	}


protected:

	Caster caster;

	/// Request to change in type. For Castable, only returns true if the current type was requested.
	/**
	 *
	 */
	virtual inline
	bool requestType(const std::type_info & t){
		return (getType() == t);
	}

	/// Request to change the array size. For Castable (and Reference) does nothing and returns false.
	/**
	 *   Does not apply to std::string;
	 */
	virtual inline
	bool requestSize(size_t elementCount){
		return (getElementCount() == elementCount);
	}


	/// Sets the storage type. If a target value is available, use link() directly.
	template <class F>
	inline // FINAL
	void setType(){
		/// Keep this redirect, because Variable may need virtual setType()
		setType(typeid(F));
	}


	/// Sets the storage type. If a target value is available, use link() directly.
	virtual inline // virtual?  Variable may need
	void setType(const std::type_info &t){
		setSeparator(',');
		caster.setType(t);
		//elementCount = 1;
	}


	/// Stores the pointer and its storage type F. Assumes elementCount=1.
	//  Why not: setPtr(F *p)
	template <class F>
	inline
	void setPtr(void *p){
		ptr = p;
		//caster.setType(typeid(F));
		caster.setType<F>();
		elementCount = 1;
	}

	/// Stores the pointer and its storage type F.
	template <class F>
	void setPtr(F &p){
		ptr = &p;
		//caster.setType(typeid(F));
		caster.setType<F>();
		elementCount = 1;
	}

	/// Stores the pointer, storage type F, and size (element count) on the vector.
	/**
	 *   The pointer is set to the first element of the vector's array segment.
	 */
	template <class F>
	void setPtr(std::vector<F> &v){
		//setPtr(&v[0], typeid(F));
		ptr = &v[0];
		caster.setType<F>();
		elementCount = v.size();
	}

	/// Sets the data pointer. Does not change type. (Cannot, because of void *.)
	/*
	 *  Function of this kind must be available for general (8bit) memory allocators.
	 */
	inline
	void setPtr(void *p, const std::type_info &t){
		ptr = p;
		caster.setType(t);
		elementCount = 1;
	}

	///
	inline
	void setPtr(const Castable &c){
		ptr = c.ptr;
		caster.setType(c.getType());  // TODO LINK
		elementCount = c.elementCount;
		outputSeparator = c.outputSeparator;
	}

	/// Note:
	inline
	void setPtr(Castable &c){
		ptr = c.ptr;
		caster.setType(c.getType());  // TODO LINK
		elementCount = c.elementCount;
		outputSeparator = c.outputSeparator;
	}

	/// Let Caster c convert my element #i to target *p
	inline
	void castElement(size_t i, const Caster &c, void *p) const {
		c.cast(caster, getPtr(i), p);
	}


	/// Destination type (current type) specific assign operations:

	/// Copy data from other Castable. Perhaps copy size and type, too.
	Castable & assignCastable(const Castable &c);

	/// Append to std::string or char array.
	/**
	 *  Combines current data and input in stringstream, and replaces contents by sstr.str().
	 *  Hence not specialized for input that is already string-like (std::string and char array).
	 *
	 */
	template <class T>
	void appendToString(const T & x){

		std::stringstream sstr;
		toStream(sstr);
		if (inputSeparator && (getElementCount()>1)){
			sstr << inputSeparator;
		}
		sstr << x;

		std::string  s = sstr.str();

		requestType(typeid(std::string));

		if (isStlString()){
			caster.put(ptr, s);
		}
		else if (isCharArrayString()){
			assignToCharArray(s);
		}
		else {
			throw std::runtime_error(std::string(__FUNCTION__) + ": type is not a string: " + typeid(T).name());
		}
	}


	template <class T>
	void assignToString(const T & x){

		requestType(typeid(std::string));

		if (isStlString()){
			caster.put(ptr, x);
		}
		else if (isCharArrayString()){
			requestSize(0);  // works only for Variable, ok
			clear(); // works always (but if elementCount)
			appendToString(x);
		}
		else {
			throw std::runtime_error(std::string(__FUNCTION__) + ": type is std::string");
		}
	}

	void assignToCharArray(const std::string & s);

	inline
	void assignToCharArray(const char *s){
		assignToCharArray(std::string(s));
	}

	/// Input anything to char array string.
	/**
	 *  A specialized version for T=std::string is defined below.
	 */
	template <class T>
	void assignToCharArray(const T & s){
		std::stringstream sstr;
		sstr << s;
		assignToCharArray(sstr.str());
	}


	inline
	void appendToElementArray(const char *s){
		appendToElementArray(std::string(s));
	}

	template <class T>
	void appendToElementArray(const T & s){

		requestType(typeid(T)); // check return code?
		if (!typeIsSet()){
			throw std::runtime_error(std::string(__FUNCTION__) + ": type is unset");
			return;
		}

		// Check if now type==string...
		if (isString()){
			throw std::runtime_error(std::string(__FUNCTION__) + ": type is std::string (not implemented yet)");
			//return;
		}

		// Extend array.
		requestSize(getElementCount() + 1);
		if (isEmpty()){
			throw std::runtime_error(std::string(__FUNCTION__) + ": still empty after resize request(+1)");
			return;
		}

		// TODO: what if non-empty but fixed?

		caster.put(getPtr(getElementCount()-1), s);

	}




	/// Input type specific assign operations.

	void assignString(const std::string &s);

	/// Assigns an STL::Sequence, element by element.
	//  Does not clear()?
	template <class T>
	void assignContainer(const T & v, bool append=false) {

		requestType(typeid(typename T::value_type));
		// Note: sequence of strings, will request string type...

		if (isString()){
			std::stringstream sstr;
			if (append){
				toStream(sstr);  // outputsepp?
				if (inputSeparator)
					sstr << inputSeparator;
			}
			StringTools::join(v, sstr, inputSeparator); // implement!
			assignToString(sstr.str());
		}
		else if (caster.typeIsSet()){
			if (append){
				for (typename T::const_iterator it = v.begin(); it != v.end(); ++it){
					appendToElementArray(*it);
				}
			}
			else {
				requestSize(v.size());
				typename T::const_iterator it = v.begin();
				for (size_t i = 0; i<getElementCount(); ++i){
					if (it != v.end())
						caster.put(getPtr(i), *it);
					else
						return;
					++it;
				}
			}
		}
		else {
			throw std::runtime_error(std::string("__FILE__") + ": type unset, cannot assign");
		}

	}


	/// Returns pointer to the array of chars, checks validity first.
	/**
	 *  \see getPtr()
	 */
	inline
	const char * getCharArray() const {

		if (!isCharArrayString())
			throw std::runtime_error("getCharArray: type not char array...");

		if (isEmpty()){
			static const char * empty = "";
			return empty;
			// throw std::runtime_error("getCharArray: empty array, no even null char");
		}

		if (*getPtr(getElementCount()-1) != '\0')
			throw std::runtime_error("getCharArray: no terminating null char");

		return getPtr();

	}


	/// Pointer to the data variable.
	void *ptr;

	/// Size of the current variable
	size_t elementCount;


	/// Element separator usein in reading a char sequence to an (numeric) array.
	char inputSeparator;

	/// Element separator usein in writing and reading character strings (streams).
	char outputSeparator;



};



inline
std::ostream & operator<<(std::ostream &ostr, const Castable &c){
	return c.toStream(ostr);
}



}  // namespace drain


#endif

// Drain
