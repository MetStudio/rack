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
 * TreeSVG.h
 *
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */

#ifndef TREESVG_H_
#define TREESVG_H_

#include "util/TreeXML.h"

namespace drain {

namespace image {

/// A node for TreeXML
/**
  \example TreeSVG-example.cpp
 */
class NodeSVG: public NodeXML {
public:

	enum type { UNDEFINED, SVG, CTEXT, GROUP, TEXT, RECT, CIRC, LINE }; // check CTEXT, maybe implement in XML

	NodeSVG(type t = UNDEFINED);

	void setType(type t);

	static
	std::ostream & toOStr(std::ostream &ostr, const drain::Tree<NodeSVG> & t);

	/// In opening SVG tag, referred to by attribute "xmlns:xlink"
	static
	std::string xlink;

	/// In opening SVG tag, referred to by attributes "xmlns" and "xmlns:svg"
	static
	std::string svg;

protected:

	// svg:
	int x;
	int y;
	int width;
	int height;
	int radius;
	std::string style;
	std::string fill;
	std::string opacity; // empty
	std::string text_anchor;

};


typedef drain::Tree<NodeSVG> TreeSVG;



inline
std::ostream & operator<<(std::ostream &ostr, const TreeSVG & t){
	  return NodeSVG::toOStr(ostr, t);
}


}  // namespace image

}  // namespace drain

#endif /* TREESVG_H_ */

// Rack
