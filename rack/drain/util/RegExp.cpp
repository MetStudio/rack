/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

 */
#include <stddef.h>  // size_t
//#include <regex.h> // wants mall
#include <iostream>
#include <string>
#include <sstream>
//#include <vector>
#include <stdexcept>

// g++ deer_regexp.cpp -o deer_regexp

#include "RegExp.h"

// // using namespace std;

namespace drain {

/// BUGS! Reads a regular expression std::string and stores it in a preprocessed format.
/*!
 *  BUGS! Calling exp() later does not work!
 *
 *  Based on POSIX regex functions <regex.h>, see man regex.
 *
 *  Examples:
 *  \example RegExp-example.cpp  RegExp-example.cpp
 */
//RegExp::RegExp() : result(writableResult) {	};
//RegExp::RegExp(const char *toStr,int flags)
RegExp::RegExp(const std::string &str,int flags) :  result(writableResult), flags(flags){
	// int result = regcomp(&regExpBinary,"",0);
	regcomp(&regExpBinary,"",0);
	//  throw std::runtime_error(std::string("RegExp(toStr,flags) failed with toStr=") + toStr);
	setExpression(str);
}

// Lack of this caused memory leakages.
RegExp::RegExp(const RegExp &r) :  result(writableResult), flags(r.flags){
	regcomp(&regExpBinary,"",0);  // IMPORTANT, because setExpression calls regfree.
	setExpression(r.regExpString);
}

RegExp::~RegExp(){
	// Clear memory;
	//cerr << "~RegExp()" << endl;
	regfree(&regExpBinary);
	//    regExpBinary = NULL;
	// Clear result variables
	writableResult.clear();
}

RegExp &RegExp::operator=(const RegExp &r){
	setExpression(r.regExpString);
	return *this;
}

// TODO: skip this?
//RegExp &RegExp::operator=(const char *toStr){
RegExp &RegExp::operator=(const std::string &str){
	setExpression(str);
	return *this;
}

bool RegExp::setExpression(const std::string &str){

	regfree(&regExpBinary);
	writableResult.clear();

	int result = regcomp(&regExpBinary,str.c_str(),flags);

	if (result > 0){
		std::stringstream sstr;
		sstr << "RegExp::setExpression(str) with str='" << str << "': ";
		const unsigned long int CBUF_LENGTH = 256;
		char cbuf[CBUF_LENGTH];
		regerror(result, &regExpBinary, cbuf, CBUF_LENGTH);
		sstr << cbuf << '\n';
		regExpString = "";  // ? should be saved instead?
		throw std::runtime_error(sstr.str());
		//regExpBinary = NULL;
		//return false; // needed?
	}

	regExpString = str;
	//matches.clear();
	//      this->clear();
	return true;

}




/// Attempts to match given std::string against the (compiled) regexp.
/**
 *  \par toStr - std::string to pe tested
 *  \return - true in success.
 */
bool RegExp::test(const std::string &str) const {
	return (regexec(&regExpBinary,str.c_str(),0,NULL,0) == 0);
}
/*
  bool RegExp::test(const char *toStr) const {  
      return (regexec(&regExpBinary,toStr,0,NULL,0) == 0);
  }
 */


/// Like test, but stores the matches.
/// Attempts to match given std::string against the (compiled) regexp.
//  bool RegExp::exec(const char *toStr){
/**
 *  \return REG_NOMATCH if match fails, resultcode otherwise. TODO consider returning bool, saving return code.
 */
/*
int RegExp::execute(const std::string &str, std::vector<std::string> & result) const {

	/// Allocates space for the matches.
	const size_t n = regExpBinary.re_nsub + 1;
	// cerr << "binary has subs:" << regExpBinary.re_nsub << endl;

	result.clear();
	result.resize(n);

	//cout << "resize => " << this->size() << endl;

	/// Allocates temp array for <regex.h> processing.
	//regmatch_t *pmatch = new regmatch_t[n];
	std::vector<regmatch_t> pmatch(n);

	/// The essential <regex.h> wrapper.
	/// (Notice the negation of boolean return values.)
	/// In success, indices (rm_so,rm_eo)
	/// will point to matching segments of toStr.
	/// Eflags not implemented (yet?).

	//cerr << "\nTrying " << toStr.c_str() << endl;
	int resultCode = regexec(&regExpBinary, str.c_str(), pmatch.size(), &pmatch[0], 0) ;

	//cerr << "result " << result << endl;

	if (resultCode == REG_NOMATCH){
		//cerr << "dont like " << toStr.c_str() << endl;
		result.clear();
	}
	else {
		regoff_t so;
		regoff_t eo;
		//size_t i;
		for (size_t i=0; i < n; i++){
			so = pmatch[i].rm_so; // start
			eo = pmatch[i].rm_eo; // end
			//cerr << "match" << so << "..." << eo << endl;
			if (so != -1)
				result[i].assign(str,so,eo - so);
		}
	}
	delete[] pmatch; // valgrind herjasi muodosta: delete pmatch
	return resultCode;

}
*/

void RegExp::replace(const std::string &src, const std::string & replacement, std::ostream  & ostr) const {

	//const size_t n = regExpBinary.re_nsub + 1;
	//cerr << "binary has subs:" << regExpBinary.re_nsub << endl;

	/// Allocates temp array for <regex.h> processing.
	std::vector<regmatch_t> pmatch(regExpBinary.re_nsub + 1);

	/// The essential <regex.h> wrapper.
	/// (Notice the negation of boolean return values.)
	/// In success, indices (rm_so,rm_eo)
	/// will point to matching segments of toStr.
	/// Eflags not implemented (yet?).

	//cerr << "\nTrying " << toStr.c_str() << endl;
	int resultCode = regexec(&regExpBinary, src.c_str(), pmatch.size(), &pmatch[0], 0) ;

	if (resultCode == REG_NOMATCH){
		//dst = src;
		ostr << src;
	}
	else {

		const regmatch_t &m = pmatch[0];
		if (m.rm_so != -1){
			//std::stringstream sstr;
			//ostr << src.substr(0, m.rm_so);
			replace(src.substr(0, m.rm_so), replacement, ostr);
			//sstr << src.substr(m.rm_so, m.rm_eo - m.rm_so);
			ostr << replacement;
			replace(src.substr(m.rm_eo), replacement, ostr);
			//ostr << src.substr(m.rm_eo);
			//dst = sstr.str();
		}
		else {
			// When does this happen?
			ostr << src; //dst = src;
		}

	}

	return; //resultCode;

}

std::ostream & operator<<(std::ostream &ostr, const drain::RegExp & r){

	ostr << r.toStr();
	for (std::vector<std::string>::const_iterator it = r.result.begin(); it != r.result.end(); ++it){
		ostr << '|' << *it;
	}
	return ostr;
}


} // drain


