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
/*
 * StringTools.h
 *
 *  Created on: Jul 21, 2010
 *      Author: mpeura
 */

#ifndef DRAIN_STRING_H_
#define DRAIN_STRING_H_

#include <limits>
#include <iterator>
#include <string>
#include <sstream>
#include <iostream>
#include <list>

//#include "RegExp.h"

namespace drain {

class StringTools {

public:

	/// Turns n first characters uppercase. ASCII only.
	/**
	 */
	static
	std::string & upperCase(std::string & s, size_t n = std::numeric_limits<size_t>::max());

	/// Turns a character to uppercase. ASCII only.
	/**
	 */
	static
	char upperCase(char c);

	/// Turns n first characters lowercase. ASCII only.
	/**
	 *
	 */
	static
	std::string & lowerCase(std::string & s, size_t n = std::numeric_limits<size_t>::max());

	/// Turns a character to lowercase. ASCII only.
	/**
	 */
	static
	char lowerCase(char c);

	/// Replaces instances of 'from' to 'to' in src, storing the result in dst.
	/// In src, replaces instances of 'from' to 'to', returning the result.
	/** Safe. Uses temporary std::string.
	 *  \see RegExp::replace.
	 */
	static inline
	std::string replace(const std::string &src, const std::string &from, const std::string & to){
		std::string dst;
		StringTools::replace(src, from, to, dst);
		return dst;
	}

	/// In src, replaces instances of 'from' to 'to', storing the result in dst.
	static
	void replace(const std::string &src, const std::string &from, const std::string & to, std::string & dst);


	/// Returns a string without leading and trailing whitespace (or other undesired chars).
	static
	std::string trim(const std::string &s, const std::string &trimChars=" \t\n");

	/// Returns indices to trimmed segment of s such that it can be extracted as s.substr(pos1, pos2-pos1).
	/**
	 *  Starting from pos1, searches for the first segment not containing trimChars.
	 *
	 *  \param pos1 - index of 1st char to search and then 1st char of the segment, if found; else s.size().
	 *  \param pos2 - index of the char \b after the segment, or s.size().
	 *  \return - true if a non-empty segment found.
	 *
	 *  \see trim(const std::string &, const std::string &)
	 *
	 */
	static
	bool trim(const std::string &s, size_t & pos1, size_t & pos2, const std::string &trimChars=" \t\n");

	/// Splits a given std::string to a std Sequence.
	/**
	 *  The target sequence must implement end() and insert() operators.
	 *
	 *   Assumes that T::operator=(std::string) exists.
	 *
	 *  Given an empty std::string, returns a list containing an empty std::string.
	 */
	template <class T>
	static
	void split(const std::string & s, T & sequence, const std::string &separators, const std::string & trimChars=" \t\n");

	template <class T>
	static inline
	void split(const std::string & s, T & sequence, char separator=','){
		StringTools::split(s, sequence, std::string(1,separator));
	}

	/// Writes a STL Container (list, vector, set) to a stream, using an optional separator char (e.g. ',').
	/**
	 *  \see StringTools::split()
	 */
	template <class T>
	static
	std::ostream & join(const T & container, std::ostream & ostr, char separator = 0){
		char sep = 0;
		for (typename T::const_iterator it = container.begin(); it != container.end(); ++it){
			if (sep)
				ostr << sep;
			ostr << *it;
			sep = separator;
		}
		return ostr;
	}

	/// Writes a STL Container (list, vector, set) to a string, using an optional separator char (like ',').
	/**
	 *  \see StringTools::split()
	 */
	template <class T>
	static inline
	std::string join(T & container, char separator = 0){
		std::stringstream sstr;
		StringTools::join(container, sstr, separator);
		return sstr.str();
	}

	/// Read input stream and append it to s.
	template <unsigned int S>
	static
	void read(std::istream &istr, std::string & s){
	    char buffer[S];
	    while (istr.read(buffer, S)){
	        s.append(buffer, S);
	    }
	    s.append(buffer, istr.gcount());
	}

private:

	/// Conversion from std::string to basic types.
	/**
	 *  \param str - input string
	 *  \param tmp - "hidden" temporary value; returned reference should be read instead of this.
	 *
	 *  \return - reference to the result of the conversion.
	 */
	template <class T>
	static
	const T & convert(const std::string &str, T & tmp);


};


template <class T>
void StringTools::split(const std::string & str, T & sequence, const std::string &separators, const std::string & trimChars){

	sequence.clear();

	const bool TRIM = !trimChars.empty();
	const std::string::size_type n = str.size();

	std::string::size_type pos1 = 0; // Start of segment
	std::string::size_type pos2 = std::string::npos;  // End of segment (index of last char + 1)

	/// Initial trim (with or without further split)
	if (TRIM){
		// std::cerr << __FUNCTION__ << " initial trim: " << str << '\n';
		StringTools::trim(str, pos1, pos2, trimChars);
	}

	// Tmp
	typename T::value_type tmpValue;

	if (separators.empty()){ // = no split! :-)

		if (TRIM){
			sequence.insert(sequence.end(), StringTools::convert(str.substr(pos1, pos2-pos1), tmpValue));
		}
		else
			sequence.insert(sequence.end(), StringTools::convert(str, tmpValue));

		//sequence.push_back(tmpValue); not in set
		return;
	}
	else {

		// Index of the next separator position
		std::string::size_type pos = pos1; // "irreversible"

		do {

			pos1 = pos;
			pos  = str.find_first_of(separators, pos);
			if (pos == std::string::npos){
				pos2 = n;
				pos  = n; // last
			}
			else {
				pos2 = pos;
				++pos; // for the next round
			}

			//std::cerr << __FUNCTION__ << ":: SEG = [" << pos1 << ',' << pos2 << '['<< "'...\n";

			if (TRIM){
				// std::cerr << __FUNCTION__ << ": trimming: '" << str.substr(pos1, pos2-pos1) << "' to ...\n"; // << "' [" << pos1 << ',' << pos2 << '['
				if (StringTools::trim(str, pos1, pos2, trimChars)){
					//std::cerr << __FUNCTION__ << " element trim: => => [" << pos1 << ',' << pos2 << '[' << " = '" << str.substr(pos1, pos2-pos1) << "'\n";
					// std::cerr << __FUNCTION__ << ": ... to [" << pos1 << ',' << pos2 << '[' << " = '" << str.substr(pos1, pos2-pos1) << "'\n";
					sequence.insert(sequence.end(), StringTools::convert(str.substr(pos1, pos2-pos1), tmpValue));
				}
				else {
					sequence.insert(sequence.end(), StringTools::convert("", tmpValue));
				}
			}
			else {
				//std::cerr << __FUNCTION__ << " element seg = '" << str.substr(pos1, pos2-pos1) << "'\n";
				sequence.insert(sequence.end(), StringTools::convert(str.substr(pos1, pos2-pos1), tmpValue));
				//sequence.insert(sequence.end(), StringTools::convert(str.substr(pos1, pos2-pos1), tmpValue));
			}

			//pos = pos + 1;

		} while (pos != n);
	}
}



template <>
inline
const std::string & StringTools::convert(const std::string &str, std::string & target){
	//target.assign(str);
	return str;
}


template <class T>
inline
const T & StringTools::convert(const std::string &str, T & target){
	std::stringstream sstr(str);
	sstr >> target;
	//std::cerr << __FUNCTION__ << ": " << str << '>' << target << '\n';
	return target;
}


} // drain::




#endif /* STRING_H_ */

// Drain
