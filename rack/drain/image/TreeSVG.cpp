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
 * TreeSVG.cpp
 *
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */

#include "TreeSVG.h"

namespace drain {

namespace image {

std::string NodeSVG::xlink("http://www.w3.org/1999/xlink");
std::string NodeSVG::svg("http://www.w3.org/2000/svg");

// NodeSVG::NodeSVG(){	setType(UNDEFINED);}

NodeSVG::NodeSVG(type t){
	setType(t);
}

void NodeSVG::setType(type t) {
	switch (t) {
	case SVG:
		tag = "svg";
		reference("x", x = 0);
		reference("y", y = 0);
		reference("width", width = 0);
		reference("height", height = 0);
		reference("xmlns", NodeSVG::svg);
		reference("xmlns:svg", NodeSVG::svg);
		reference("xmlns:xlink", NodeSVG::xlink);
		break;
	case GROUP:
		tag = "g";
		//reference("x", x, 0);
		//reference("y", y, 0);
		break;
	case TEXT:
		tag = "text";
		reference("x", x = 0);
		reference("y", y = 0);
		reference("text-anchor", text_anchor = "");
		break;
	case RECT:
		tag = "rect";
		reference("x", x = 0);
		reference("y", y = 0);
		reference("width", width = 0);
		reference("height", height = 0);
		break;
	case CIRC:
		tag = "circ";
		reference("x", x = 0);
		reference("y", y = 0);
		reference("radius", radius = 0);
		break;
	case CTEXT:
		tag = "";
		//reference("x", x, 0);
		break;
	case UNDEFINED:
	default:
		return;
	}

	//if ((t = TEXT)||(t == TEXT)){
	reference("style", style = "");
	reference("fill", fill = "");
	reference("opacity", opacity = ""); // string, so silent if empty


}

std::ostream & NodeSVG::toOStr(std::ostream &ostr, const TreeSVG & tree){
	ostr << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>";
	ostr << '\n';
	//NodeXML::toOStr()
	//NodeXML::toOStr(ostr, tree, "svg");
	NodeXML::toOStr(ostr, tree);
	return ostr;
}



}  // namespace image

}  // namespace drain


// Rack
