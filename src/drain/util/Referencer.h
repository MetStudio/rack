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
#include <typeinfo>
#include <stdexcept>
#include <iostream>


#include "Convert.h"
#include "Variable.h"
#include "Sprinter.h"

#include "VariableLike.h"

#ifndef DRAIN_REFERENCE_VARIABLE_NEW
#define DRAIN_REFERENCE_VARIABLE_NEW

// // using namespace std;

namespace drain {



typedef Reference Referencer;

/// drain:: default implementation for output formatting
template <>
inline
std::ostream & Sprinter::toStream(std::ostream & ostr, const drain::Referencer & v, const SprinterLayout & layout){
	return Sprinter::toStream(ostr, (const drain::Castable &) v, layout);
};

template <>
template <class D>
inline
void Convert2<Referencer>::convert(const Referencer &src, D & dst){
	dst = (const D &)src;
	std::cout << "CONV:" << __FILE__ << src << " -> " << dst << '\n';
}


template <>
inline
void Convert2<Referencer>::convert(const char *src, Referencer & dst){
	dst = src;
	std::cout << "CONV:" << __FILE__ << src << " -> " << dst << '\n';
}



template <>
template <class S>
inline
void Convert2<Referencer>::convert(const S &src, Referencer & dst){
	dst = src;
	std::cout << "CONV:" << __FILE__ << src << " -> " << dst << '\n';
}


}  // namespace drain


#endif
