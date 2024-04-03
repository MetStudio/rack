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

#ifndef DRAIN_CASTABLE2
#define DRAIN_CASTABLE2 "drain::Castable"

#include <drain/UniTuple.h>
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

//#include "JSONwriter.h"
#include "Sprinter.h" // partially overlapping with JSONwriter?



namespace drain {

// Forward declaration
// class Variable;

// Forward declaration
// class Referencer;

/// An object that can be automatically casted to and from a basetype, array of basetype or std::string. Designed for objects returned by CastableIterator.
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

// Optionally, constructors of this base class could be protected.
// protected:
public:

	inline
	Castable() : fillArray(false), elementCount(0){
		setSeparator();
		caster.unsetType();
	};

	/// Copy constructor: copies the layout and the pointer to the target.
	inline
	Castable(const Castable &c) : fillArray(false), elementCount(0) {
		copyFormat(c);
		setPtr(c.caster.ptr, c.caster.getType());
	}

	// Obsolete?
	/**
	 *
	 */
	template <class F>
	Castable(F *p) : fillArray(false), elementCount(0) {
		setSeparator();
		setPtr(p, typeid(F));
		// std::cerr << "Castable(F *p) type=" << typeid(F).name() << " value=" << *p << " (" << (double)*p << ")" << std::endl;
	}

	/// Constructor for an object pointing to a variable.
	template <class F>
	Castable(F &p) : fillArray(false), elementCount(0) {
		setSeparator();
		setPtr(p);
	}

public:

	inline virtual
	~Castable(){};

	// Variable will hide std::string type.

	/// Return the storage type (base type) of the referenced variable.
	virtual inline
	const std::type_info & getType() const {
		return caster.getType();
	};

	/// Return true, if the referenced variable is external. Inherited classes - like Variable - may have internal data arrays.
	virtual inline
	bool isReference() const {
		return true;
	}


	/// Returns true, if type is C char array and outputSepator is the null char.
	inline
	bool isCharArrayString() const {
		return ((caster.getType() == typeid(char)) && (outputSeparator=='\0'));  // "close enough" guess, or  a definition?
	};

	/// Returns true, if type is std::string .
	inline
	bool isStlString() const {
		return (caster.getType() == typeid(std::string));
	};

	/// Returns true, if type is C char array or std::string .
	inline
	bool isString() const {
		return (isCharArrayString() || isStlString());
	};

	virtual inline
	bool typeIsSet() const {
		return caster.typeIsSet();
	};

	inline
	size_t getElementSize() const {
		return caster.getElementSize();
	};

	/// Returns the length of the array, the number of elements in this entry.
	/**
	 *
	 *
	 *   \see Castable::getElementSize().
	 */
	virtual inline
	size_t getElementCount() const {
		return elementCount;
	}

	/// Returns the size in bytes of the target: (elementCount * elementSize)
	virtual inline
	size_t getSize() const {
		return elementCount * caster.getElementSize();
	}


	/// Returns true, if no elements have been allocated. Does not check type. (NOTE: may change: 0 for scalars)
	/**
	 *   \see getElementSize()
	 *   \see getElementCount()
	 */
	//inline
	size_t empty() const;

	/// Set or unset filling (padding) an array if input set is smaller
	inline
	Castable & setFill(bool fill=true){
		fillArray = fill;
		return *this;
	}

	/// Clears strings, or for scalars and arrays, sets all the values to zero. Does not change type.
	// Has been protected, even private. Why? Essentially setting a value.
	void clear();

	/** Unset the type (set it to \c void ) and clear @elementCount .
	 *
	 */
	inline
	void reset(){
		caster.unsetType();
		//elementCount = 0;
		updateSize(0);
		setSeparator(',');
	}

protected:

	virtual
	void updateSize(size_t elems){
		elementCount = elems;
	}

public:


	template <class T>
	Castable &operator=(const T &x){
		//std::cout << "op= " << x << " -> " << Type::getTypeChar(getType()) << ',' << getElementCount() << std::endl;
		assign(x);
		return *this;
	}

	/** Obligatory. Many values will be read from command line or text files.
	 *
	 */
	inline
	Castable & operator=(const char *c){
		assignString(c);
		return *this;
	}


	template <class T>
	Castable & operator=(const T *x){
		throw std::runtime_error("Castable & operator=(const T *x): unsupported");
		return *this;
	}

	/// Copies the value referred to by Castable. Notice that if type is unset (void), no operation is performed.

	/** Seems to be obligatory.
	 */
	inline
	Castable &operator=(const Castable &c){
		if (&c != this){
			assignCastable(c);
		}
		return *this;
	}


	/*
	inline
	Castable &operator=(const Variable &c){
		return assignCastable(c);
	}

	inline
	Castable &operator=(const Referencer &c){
		return assignCastable(c);
	}
	*/

	// Special handler for string assignment
	inline
	void assign(const std::string &c){
		assignString(c);
	}


	inline
	void assign(const char *c){
		assignString(c);
	}

	// "NEW"
	inline
	void assign(const Castable &c){
		assignCastable(c);
	}

	/// Copies an arbitrary base type or std::string value.
	/**
	 *
	 */
	template <class T>
	void assign(const T &x){
		// std::cout << "assign " << x << " -> " << Type::getTypeChar(getType()) << ',' << getElementCount() << std::endl;
		suggestType(typeid(T));
		if (isString()){
			//std::cout << __FUNCTION__ << ':' << x << " -> " << Type::getTypeChar(getType()) << ',' << getElementCount() << '\n';
			assignToString(x);
		}
		else if (typeIsSet()){
			if (fillArray){
				for (size_t i = 0; i<getElementCount(); ++i){
					caster.put(getPtr(i), x);
				}
			}
			else {
				requestSize(1);
				caster.put(x);
			}
		}
		else {
			throw std::runtime_error(std::string(__FUNCTION__) + ": type is unset");
		}


	}



	/// Copies elements of a list.
	template<typename T>
	// Castable & operator=(std::initializer_list<T> l){
	void assign(std::initializer_list<T> l){
		assignContainer(l);
		//return *this;
	}

	/// Copies elements of a list.
	template <class T>
	inline
	// Castable &operator=(const std::list<T> & l){
	void assign(const std::list<T> & l){
		assignContainer(l);
		// return *this;
	}

	/// Copies elements of a vector.
	template <class T>
	inline
	// Castable &operator=(const std::vector<T> & v){
	void assign(const std::vector<T> & v){
		assignContainer(v);
		// return *this;
	}

	/// Copies elements of a set.
	template <class T>
	inline
	// Castable &operator=(const std::set<T> & s){
	void assign(const std::set<T> & s){
		assignContainer(s);
		//return *this;
	}





	template <class T>
	inline
	Castable &operator<<(const T &x){
		append(x);
		return *this;
	}


	/// Appends the string or appends the array by one element.
	/*
	 *  Note: if both source and target are strings, does not split it like with Variable::operator=(const char *).
	 */
	template <class T>
	inline
	//Castable &operator<<(const T &x){
	void append(const T &x){
		suggestType(typeid(T));
		if (isString()){
			appendToString(x);
		}
		else {
			appendToElementArray(x);
		}
		//return *this;
	}

	/// Appends the string or appends the array by one element.
	inline
	// Castable &operator<<
	void append(const char *s){
		//return *this << (std::string(s));
		append(std::string(s));
	}

	/// Appends elements of std::list
	template <class T>
	inline  /// Castable &operator<<(
	void append(const std::list<T> & l){
		assignContainer(l, true);
		//return *this;
	}

	/// Appends elements of a vector
	template <class T>
	inline
	// Castable &operator<<
	void append(const std::vector<T> & v){
		assignContainer(v, true);
		/// return *this;
	}

	/// Appends elements of a set
	template <class T>
	inline  // Castable &operator<<
	void append(const std::set<T> & s){
		assignContainer(s, true);
		// return *this;
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
	 *   Adding any str conversions (int, double, ...) causes the compiler to fail.
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
			return caster.get<T>(); //caster.get<T>(ptr);
	}

	// TODO: consider: operator bool() const

	// TODO: PAIR
	/*
	template <class T>
	// operator std::pair<T,T>() const {
	}
	*/

	inline
	bool operator==(const Castable &x) const{
		std::cerr << __FILE__ << __LINE__ << __FUNCTION__ << std::endl;
		throw std::runtime_error("Castable: operator== not implemented.");
		return false;
	}

	inline
	bool operator==(const char * s) const {
		if (isCharArrayString()) // 2021
			return (strcmp(getCharArray(), s) == 0);
		else
			return (caster.get<std::string>() == s);
	}

	/// Compares a value to internal data.
	template <class T>
	bool operator==(const T &x) const{
		return (caster.get<T>() == x);  //(caster.get<T>(ptr) == x);
	}


	/// Compares a value to inner data.
	/**
	 *
	 */
	template <class T>
	bool operator!=(const T &x) const {
		return (caster.get<T>() != x);
	}

	/// Compares a value to inner data.
	// strings?
	template <class T>
	bool operator<(const T &x) const {
		return (caster.get<T>() < x);
	}

	/// Compares a value with inner data.
	template <class T>
	bool operator>(const T &x) const {
		return (caster.get<T>() > x);
	}

	/// Compares a value to inner data.
	// strings?
	template <class T>
	bool operator<=(const T &x) const {
		return (caster.get<T>() <= x);
	}

	/// Compares a value with inner data.
	template <class T>
	bool operator>=(const T &x) const {
		return (caster.get<T>() >= x);
	}

	/// The character used between array elements in output stream.
	/**
	 *  \see isCharArrayString
	 */
	inline
	Castable & setSeparator(char c = ','){
		setInputSeparator(c);
		setOutputSeparator(c);
		return *this;
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

	///
	/**
	 *   May append elements dynamically, there should be a marker for separating elements
	 *   is a string.
	 */
	inline
	char getInputSeparator(){
		return inputSeparator;
	}

	///
	/**
	 *   May append elements dynamically, there should be a marker for separating elements
	 *   is a string.
	 */
	inline
	char getOutputSeparator(){
		return outputSeparator;
	}


	std::ostream & toStream(std::ostream & ostr = std::cout, char separator='\0') const;

	/*
	void toJSONold(std::ostream & ostr = std::cout, char fill = ' ', int verbosity = 0) const;
	std::ostream & valueToJSONold(std::ostream & ostr = std::cout) const;
	*/

	std::string toStr() const;

	/// Writes a string of type indentifier char and size in bytes, for example [C@8] for unsigned char and [s@16] for signed short int."
	void typeInfo(std::ostream & ostr) const;

	/// Print value, type and element count
	// Writes a bit longer type indentifier and size in bytes, for example "unsigned int(8) for char and "signed int(16) for signed short int."
	virtual
	void info(std::ostream & ostr = std::cout) const;

	// void debug(std::ostream & ostr = std::cout) const;

	/// Returns true, if the class contains an own data array.
	/**
	 *   In a drain::Variable,  \c ptr always points to data array, or to null, if the array is empty.
	 *   In a drain::FlexibleVariable, \c ptr can point to owned data array or to an external (base type) variable.
	 *
	 */
	virtual inline
	bool isVariable() const {
		return false;
	}


	/// Converts data to a STL Sequence, for example std::set, std::list or std::vector .
	/**
	 *    \tparam T - std::set, std::list or std::vector
	 *    \param container  - sequence in which elements will be assigned
	 *    \param separator  - separator char(s) overriding separator chars of the instance; applicable only with strings
	 */
	template <class T>
	void toSequence(T & container, char separator = 0) const { // FIX raise

		container.clear();

		if (isString()){
			if (!separator)
				separator = this->outputSeparator;
			StringTools::split(toStr(), container, separator," \t\n");
		}
		else {
			for (size_t i = 0; i < getElementCount(); ++i) {
				/// The most general container insert method
				container.insert(container.end(), caster.get<typename T::value_type>(getPtr(i)) );
			}
		}


	}



	template <class T>
	void toMap(T & map, char separator = 0, char equalSign = '=') const {

		map.clear();

		if (isString()){
			// This part could be in StringTools?
			typedef std::list<std::string> entryList;
			entryList entries;
			toSequence(entries, separator);

			for (entryList::const_iterator it = entries.begin(); it!=entries.end(); ++it){
				//typename T::value_type entry;
				typename T::key_type key;
				typename T::mapped_type data;
				drain::StringTools::split2(*it, key, data, std::string(1, equalSign)); // TRIM?
				map[key] = data;
			}
		}
		else {
			for (size_t i = 0; i < getElementCount(); ++i){
				//typename T::value_type entry;
				typename T::key_type key;
				typename T::mapped_type data;
				std::stringstream sstr;
				sstr << i;
				sstr >> key; // entry.first;
				std::stringstream sstr2;
				//sstr2 << caster.get<typename T::value_type>(getPtr(i)); // weirds
				sstr2 << caster.get<typename T::mapped_type>(getPtr(i)); // weirds
				sstr2 >> data;
				map[key] = data;
			}
		}


	}


	/// Returns pointer to the array of chars without validity check.
	/**
	 *  \see getCharArray
	 */
	inline
	char * getPtr(size_t i = 0) {
		return &((char *)caster.ptr)[i*caster.getElementSize()];
	}

	inline
	const char * getPtr(size_t i = 0) const {
		return &((const char *)caster.ptr)[i*caster.getElementSize()];
	}

	/// If array, assigning a scalar will fill up the current array.
	bool fillArray;

	/// Copies array layout and formatting: separators, element count, fillArray flag.
	void copyFormat(const Castable & c){
		this->elementCount    = c.elementCount;
		this->inputSeparator  = c.inputSeparator;
		this->outputSeparator = c.outputSeparator;
		this->fillArray       = c.fillArray;
	}

	/// Copy data from str Castable. Perhaps copy size and type, too.
	Castable & assignCastable(const Castable &c);


protected:

	Caster caster;

	/// Request to change in type. For Castable, simply returns true if the current type was requested.
	/**
	 *
	 */
	virtual inline
	bool suggestType(const std::type_info & t){
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
		setSeparator(','); // ?
		caster.setType(t);
		//elementCount = 1;
	}


	/// Stores the pointer and its storage type F. Assumes elementCount=1.
	template <class F>
	inline
	void setPtr(void *p){
		caster.link(p);
		elementCount = 1;
	}


	/// Stores the pointer and its storage type F.
	template <class F>
	void setPtr(F &p){
		if ((!std::is_arithmetic<F>::value) && (typeid(F)!=typeid(std::string))){
			throw std::runtime_error(std::string(__FUNCTION__) + ": unsupported type: " + typeid(F).name());
		}
		caster.link(p); // could use setPtr(void *p, const std::type_info &t) ?
		elementCount = 1;
	}

	/// Sets the data pointer and its explicit type.
	/*
	 *  Function of this kind must be available for general (8bit) memory allocators.
	 */
	inline
	void setPtr(void *p, const std::type_info &t, size_t count=1){
		//if (t == typeid(void))
		//	throw std::runtime_error(std::string(__FUNCTION__) + ": explicit void type given");
		caster.link(p, t);
		elementCount = count;
	}

	/// Stores the pair as an array of two elements.
	/**
	 *   The pointer is set to the first.
	 */
	/*
	template <class F>
	// void setPtr(std::pair<F,F> &p){
		setPtr(&p, typeid(F), 2);
		//caster.link((F*)&v);
		fillArray = true; // if a scalar is assigned, assign to both elements
		//elementCount = 2;
	}
	*/

	/// Stores the elements as an array.
	/**
	 *   The pointer is set to the first.
	 */
	template <class F, size_t N>
	void setPtr(UniTuple<F,N> & tuple){
		setPtr(tuple.begin(), typeid(F), N);
		fillArray = true; // (?)  if a scalar is assigned, assign to both elements
	}




	/// Copies the link and element count.
	void relink(Castable & c);


	/*
	inline
	void setPtr(const Castable &c){
		//caster.link0((const Caster &)c);

		caster.ptr = c.caster.ptr;
		caster.setType(c.getType());

		elementCount = c.elementCount; // TODO
		outputSeparator = c.outputSeparator; // ?
	}

	/// Note:
	inline
	void setPtr(Castable &c){
		caster.link0((Caster &)c);
		// caster.ptr = c.caster.ptr;
		// caster.setType(c.getType());
		elementCount = c.elementCount; // TODO
		outputSeparator = c.outputSeparator; // ?
	}
	*/

	/// Let Caster c convert my element #i to target *p
	// IMPORTANT!
	inline
	void castElement(size_t i, const Caster &c, void *p) const {
		c.translate(caster, getPtr(i), p);
	}


	// Destination type (current type) specific assign operations



	/// Append to std::string or char array.
	/**
	 *  Combines current data and input in stringstream, and replaces contents by sstr.str().
	 *  Hence not specialized for input that is already string-like (std::string and char array).
	 *
	 */
	template <class T>
	void appendToString(const T & x){

		std::stringstream sstr;

		// drain::Sprinter::toStream(sstr, *this, drain::Sprinter::plainLayout);
		toStream(sstr);

		if (inputSeparator && (getElementCount()>1)){
			sstr << inputSeparator;
		}
		sstr << x;

		std::string  s = sstr.str();

		suggestType(typeid(std::string));

		if (isStlString()){
			caster.put(s);
			//caster.put(ptr, s);
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

		suggestType(typeid(std::string));

		if (isStlString()){
			caster.put(x);
			//caster.put(ptr, x);
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

		suggestType(typeid(T)); // check return code?
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
		if (empty()){
			throw std::runtime_error(std::string(__FUNCTION__) + ": still empty after resize request(+1)");
			return;
		}

		// TODO: what if non-empty but fixed?

		caster.put(getPtr(getElementCount()-1), s);

	}



public:
	/* public NEW 2021 (experimental)
	 *
	 * useful in "stream" definitions:
	 * drain::Referencer(range.tuple()).setSeparator(':').setFill().assignString(ftor);
	 *
	 */


	/// Input type specific assign operations.

	void assignString(const std::string &s);

	// NEW 2024
	template <class T, size_t N>
	inline
	void assign(UniTuple<T,N> & tuple){
		assignContainer(tuple);
	}

	/// Assigns a STL Sequence, element by element.
	template <class T>
	void assignContainer(const T & v, bool append=false) {

		if (!append){
			if (isStlString())
				clear();
			else
				requestSize(0);
		}

		suggestType(typeid(typename T::value_type));
		// Note: sequence of strings, will request string type...

		if (isString()){
			std::stringstream sstr;
			if (append){
				drain::Sprinter::toStream(sstr, *this, drain::Sprinter::plainLayout);
				//toStream(sstr);  // outputsepp?
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
				typename T::const_iterator itLast = it;
				for (size_t i = 0; i<getElementCount(); ++i){
					if (it == v.end()){
						if (fillArray)
							it = itLast; //v.begin(); WHAT if empty?
						else
							return;
					}
					else {
						itLast = it;
					}
					caster.put(getPtr(i), *it);
					++it;
				}
			}
		}
		else {
			throw std::runtime_error(std::string(__FILE__) + ": type unset, cannot assign");
		}

	}


	/// Returns pointer to the array of chars, checks validity first.
	/**
	 *  \see getPtr()
	 */
	/* OLD
	inline
	const char * getCharArray() const {

		if (!isCharArrayString())
			throw std::runtime_error("getCharArray: type not char array...");

		if (empty()){
			static const char * empty = "";
			return empty;
			// throw std::runtime_error("getCharArray: empty array, no even null char");
		}

		if (*getPtr(getElementCount()-1) != '\0')
			throw std::runtime_error("getCharArray: no terminating null char");

		return getPtr();

	}
	*/

	/// Returns pointer to the array of chars, if charArray or std::string
	/**
	 *
	 *  \see getPtr() which returns pointer directly
	 *
	 *  checks validity first.
	 */
	// NEW

public:

	const char * getCharArray() const;

protected:

	/// Pointer to the data variable.
	//  void *ptr;

	/// Size of the current variable
	size_t elementCount;

	/// Element separator usein in reading a char sequence to an (numeric) array.
	char inputSeparator;

	/// Element separator usein in writing and reading character strings (streams).
	char outputSeparator;


};

/*
template <>
void Castable::setPtr(Castable &c);
*/
/*
template <class T>
T & Castable::valueToJSON(T & ostr) const {

	if ((getType() == typeid(char)) || isStlString()){
		ostr << '"';
		toStream(ostr, ','); // use JSONtree separator
		ostr << '"';
	}
	else {
		if (getElementCount() != 1){
			ostr << '[';
			toStream(ostr); // why comma not explicit? ...
			ostr << ']';
		}
		else
			toStream(ostr, ',');  // ... but here
	}
	return ostr;
}
*/

//template <>
//std::ostream & JSONwriter::toStream(const drain::Castable & v, std::ostream & ostr, unsigned short);


/// "Friend class" template implementation
template <>
std::ostream & Sprinter::toStream(std::ostream & ostr, const drain::Castable & x, const SprinterLayout & layout);


inline
std::ostream & operator<<(std::ostream &ostr, const Castable &c){
	return c.toStream(ostr);
	//return Sprinter::toStream(ostr, c, Sprinter::plainLayout);
}

/// Arithmetics: addition
template <class T>
inline
T & operator+=(T & x, const Castable &c){
	x += c.get<T>(0);
	return x;
}

/// Arithmetics: subtraction
template <class T>
inline
T & operator-=(T & x, const Castable &c){
	x -= c.get<T>(0);
	return x;
}

/// Arithmetics: addition
template <class T>
inline
T & operator*=(T & x, const Castable &c){
	x *= c.get<T>(0);
	return x;
}

/// Arithmetics: subtraction
template <class T>
inline
T & operator/=(T & x, const Castable &c){
	x /= c.get<T>(0);
	return x;
}



template <>
const std::string TypeName<Castable>::name;



}  // namespace drain


#endif

// Drain
