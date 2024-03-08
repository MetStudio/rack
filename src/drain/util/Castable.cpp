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
#include "Castable.h"


namespace drain {


void Castable::relink(Castable & c){

	const std::type_info &t = c.getType();

	if (t == typeid(void))
		throw std::runtime_error(std::string("Castable::") + __FUNCTION__ + "(Castable &c): c has void type");

	setPtr(c.getPtr(), t);
	elementCount = c.getElementCount();
}


void Castable::clear(){
	if (caster.typeIsSet()){
		if (caster.getType() == typeid(std::string))
			caster.put(""); //caster.put(ptr, "");
		else {
			for (size_t i = 0; i < getElementCount(); ++i) {
				caster.put(getPtr(i), 0);
			}
		}
	}
}

const char * Castable::getCharArray() const {

	if (isCharArrayString()){

		if (empty()){
			static const char * empty = "";
			return empty;
			// throw std::runtime_error("getCharArray: empty array, no even null char");
		}

		if (*getPtr(getElementCount()-1) != '\0'){
			std::cerr << getElementCount() << std::endl;
			std::cerr << "first char: " << *getPtr(0) << std::endl;
			//std::cerr << getElementCount() << std::endl;
			throw std::runtime_error("getCharArray: no terminating null char..");
		}

		return (const char *)caster.ptr;

	}
	else if (isStlString()){
		return ((const std::string *)caster.ptr)->c_str();
	}
	else {
		throw std::runtime_error("getCharArray: type neither charArray nor std::string");
		return NULL;
	}
}

// Deprecating. Use Sprinter only
std::ostream & Castable::toStream(std::ostream & ostr, char separator) const {

	const std::type_info & t = caster.getType();
	//const size_t n = getElementCount();

	if (t == typeid(std::string)){  // or any single-element?
		//ostr << "# type==std::string\n";
		caster.toOStream(ostr, caster.ptr);  // no cast needed...
		//ostr << "{std::string}";
	}
	else if (isCharArrayString()){  // char array without outputSeparator
		ostr << getCharArray();
	}
	else if (!empty()) {

		//std::cerr << __FUNCTION__ << " separator: " << this->outputSeparator << '\n';

		// Output element using given separator or default (outputSeparator)
		if (!separator)
			separator = this->outputSeparator;
		char sep = 0;
		// NEW 2019/07: keep precision through elements.
		//std::streamsize prec = ostr.precision();
		for (size_t i = 0; i < getElementCount(); ++i) {
			if (sep){
				ostr << sep;
				//ostr.precision(prec);
			}
			caster.toOStream(ostr, getPtr(i));
			sep = separator;
		}
		//std::cerr << __FUNCTION__ << " separator: " << sep << '\n';
	}
	else {
		ostr << "null"; // JSON convention
		// Output nothing!
		//ostr << '[' << "Ø" << ']';  // consider array.begin, end etc
		//ostr << "alert: getElementCount()==0, elementCount=" << elementCount;
		//ostr << " byteSize=" << getElementSize();
		//ostr << " typeChar=" << drain::Type::getTypeChar(getType()) << '=' << getType().name() << '\n';
	}

	//ostr << '{' << getElementCount() << ',' << getElementSize() << '}';
	return ostr;

}

// Deprecating. Use Sprinter only
std::string Castable::toStr() const {


	if (isStlString()){
		return *(const std::string *)caster.ptr;
	}
	else if (isCharArrayString()){
		return std::string(getCharArray());
	}
	else { // "default"
		std::stringstream sstr;
		//toStream(sstr);
		Sprinter::toStream(sstr, *this, Sprinter::plainLayout);
		return sstr.str();
	}
	//return caster.get<std::string>(ptr);
}



void Castable::typeInfo(std::ostream & ostr) const {
	ostr << '[';
	if (isString())
		ostr << '#';
	else
		ostr << drain::Type::getTypeChar(getType());
	//if (size_t n = getElementCount() != 1)
	ostr << getElementCount();
	ostr << '@' << getElementSize() << ']';
}



void Castable::info(std::ostream & ostr) const {
	Sprinter::toStream(ostr, *this, Sprinter::jsonLayout);
	// ostr << *this << " (";
	ostr << " (";
	if (isCharArrayString()){
		ostr << "char[" << (getElementCount()-1) << "+1]";
	}
	else if (isCharArrayString()){
		ostr << "std::string";
	}
	else {
		ostr << drain::Type::call<drain::simpleName>(getType());
		if (getElementCount() > 1)
			ostr << '[' << getElementCount() << ']';
	}
	ostr << ")";

}



Castable & Castable::assignCastable(const Castable &c){


	if (&c == this){
		return *this;
	}

	// If this ie. destination is a string, convert input.
	if (c.getType() == typeid(void)){
		// std::cerr << __FUNCTION__ << ": NEW: assign 'unset'\n";
		// CHECK: this may be wrong for ReferenceVar:
		reset();
	}
	else if (isString()){
		// Should call resize()?
		assignString(c.toStr());
	}
	// Same thing, if input is a string(like), because nothing should be lost when converting that to std::string.
	else if (c.isString()){
		// Should call resize()?
		assignString(c.toStr()); // when to split?
	}
	else {
		suggestType(c.getType());
		requestSize(c.getElementCount());
		const size_t n = std::min(getElementCount(), c.getElementCount());
		for (size_t i = 0; i < n; ++i) {
			c.castElement(i, caster, getPtr(i));
		}
	}
	/*
	else {
		std::cerr << __FUNCTION__ << ": could not convert:\n";
		toJSON(std::cerr);
		std::cerr << '\n';
		c.toJSON(std::cerr);
		std::cerr << '\n';
	}
	 */
	return *this;
}

void Castable::assignToCharArray(const std::string & s){
	requestSize(s.size() + 1);
	if (empty())
		throw std::runtime_error(std::string(__FUNCTION__) + ": array resize failed for: " + s);
	const size_t n = std::min(s.size(), getElementCount()-1);
	if (n < s.size()){
		std::cerr << __FUNCTION__ << ": truncating '" << s << "' to '" << s.substr(0, n) << "'\n";
	}
	for (size_t i=0; i < n; ++i){
		caster.put(getPtr(i), s[i]);
	}
	caster.put(getPtr(getElementCount()-1), '\0');
}


void Castable::assignString(const std::string &s){

	suggestType(typeid(std::string));

	if (isStlString()){
		//caster.put(ptr, s);  // does not split!
		caster.put(s);  // does not split!
	}
	else if (isCharArrayString()){
		assignToCharArray(s);  // does not split!
	}
	else {
		std::list<std::string> l;
		StringTools::split(s, l, inputSeparator);
		requestSize(l.size()); // check if true?
		if (l.size() > getElementCount()) // or throw excp?
			std::cerr << __FILE__ << ':' << __FUNCTION__ << ": provided " << s << " with "<< l.size() << " elements, assigning: "<< getElementCount() << std::endl;
		// Will suggest string type, because s is a string...
		assignContainer(l);
	}

}


/// Output implementation
template <>
std::ostream & Sprinter::toStream(std::ostream & ostr, const drain::Castable & v, const SprinterLayout & layout) {

	if (v.isString()){
		//const TypeLayout & chars = layout.stringChars;
		Sprinter::prefixToStream(ostr, layout.stringChars);
		v.toStream(ostr, layout.stringChars.separator);
		Sprinter::suffixToStream(ostr, layout.stringChars);
	}
	else if (v.getElementCount() > 1) {
		//const TypeLayout & l = layout.arrayChars;
		Sprinter::prefixToStream(ostr, layout.arrayChars);
		// Sprinter::sequenceToStream(ostr, v, layout);
		v.toStream(ostr, layout.arrayChars.separator);
		Sprinter::suffixToStream(ostr, layout.arrayChars);
	}
	else if (v.empty()) {
		ostr << "null"; // TODO: layout.nullString
	}
	else if (v.getType() == typeid(bool)) {
		ostr << ((bool)v ? "true" : "false"); // Pythonic: "True", "False"
	}
	else {
		ostr << v; //Sprinter::basicToStream(ostr, v, myChars);
	}

	return ostr;
	/*
	// Semi-kludge: imitate intended layout
	if (&layout == &Sprinter::jsonLayout){
		x.valueToJSON(ostr);
		return ostr;
	}
	else {
		return drain::Sprinter::basicToStream(ostr, x, "");
	}
	*/

}

/*
template <>
std::ostream & Sprinter::toStream(std::ostream & ostr, const drain::Variable & v, const SprinterLayout & layout) {

	if ((v.getElementCount() > 1) && !v.isString()){
		return Sprinter::sequenceToStream(ostr, v, layout);
	}
	else {
		return Sprinter::toStream(ostr, (const Castable &)v, layout); //
	}
	return ostr;
}
*/

}  // drain
