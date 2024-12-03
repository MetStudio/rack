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

DRAIN_TYPENAME_DEF(image::NodeSVG);

namespace image {

template <>
NodeSVG::xmldoc_attrib_map_t NodeSVG::xml_node_t::xmldoc_attribs = {
		{"version",  "1.0"},
		{"encoding", "UTF-8"},
		{"standalone", "no"},
		// {"data-remark", "svg"} debugging? inkview claims
};

const drain::FileInfo NodeSVG::fileInfo("svg");

std::string NodeSVG::xlink("http://www.w3.org/1999/xlink");
std::string NodeSVG::svg("http://www.w3.org/2000/svg");

// NodeSVG::NodeSVG(){	setType(UNDEFINED);}


template <>
std::map<svg::tag_t,std::string> NodeXML<svg::tag_t>::tags = {
	{drain::image::svg::UNDEFINED,	"UNDEFINED"},
	{drain::image::svg::COMMENT, "#"},
	{drain::image::svg::CTEXT, ""},
	{drain::image::svg::SVG,   "svg"},
	{drain::image::svg::CIRCLE,  "circle"},
	{drain::image::svg::DESC,  "desc"},
	{drain::image::svg::GROUP, "g"},
	{drain::image::svg::IMAGE, "image"},
	{drain::image::svg::LINE,  "line"},
	{drain::image::svg::METADATA,  "metadata"},
	{drain::image::svg::POLYGON,  "polygon"},
	{drain::image::svg::RECT,  "rect"},
	{drain::image::svg::STYLE, "style"}, // raise?
	{drain::image::svg::TEXT,  "text"},
	{drain::image::svg::TITLE, "title"},
	{drain::image::svg::TSPAN, "tspan"},
};



void NodeSVG::updateAlignAttributes(){
	/*
	std::stringstream sstr;
	char sep=0;
	for (AlignSVG2::pos_t p: {AlignSVG2::ORIG, AlignSVG2::REF}){
		for (AlignSVG2::axis_t a: {AlignSVG2::HORZ, AlignSVG2::VERT}){
			const AlignSVG2::value_t & v = getAlign(p, a);
			if (v != AlignSVG2::UNDEFINED){
				if (sep)
					sstr << sep;
				else
					sep=' ';
				sstr << EnumDict<AlignSVG2::pos_t>::dict.getKey(p) << ':' << EnumDict<AlignSVG2::axis_t>::dict.getKey(a) << '-' << EnumDict<AlignSVG2::value_t>::dict.getKey(v);
				//std::cerr << __FUNCTION__ << ':' << EnumDict<AlignSVG2::pos_t>::dict.getKey(p) << '_' << EnumDict<AlignSVG2::axis_t>::dict.getKey(a) << '_' << EnumDict<AlignSVG2::value_t>::dict.getKey(v) << '_' << (int)v << '\n';
			}
		}
	}
	std::string s = sstr.str();
	*/
	if (align.empty()){
		this->unlink("align");
	}
	else {
		if (!this->hasKey("align")){
			this->link("align", align); // (should be safe anyway)
		}
	}

	if (anchor.empty()){
		this->unlink("alignAnchor");
	}
	else {
		this->link("alignAnchor", anchor);
	}

}

NodeSVG::NodeSVG(tag_t t){
	type = elem_t::UNDEFINED;
	setType(t);
}

//NodeSVG::NodeSVG(const NodeSVG & node) : xml_node_t(), x(0), y(0), width(0), height(0), radius(0) {
NodeSVG::NodeSVG(const NodeSVG & node) : xml_node_t(), box(0,0,0,0), radius(0) {
	copyStruct(node, node, *this, LINK); // <-- risky! may link Variable contents?
	// type = elem_t::UNDEFINED; // = force fresh setType below
	setType(node.getType());
}


void NodeSVG::setType(const tag_t & t) {

	if (type == t){
		return; // lazy
	}

	type = t;

	switch (t) {
	case elem_t::UNDEFINED:
		break;
	case elem_t::COMMENT:
		// setComment();
		break;
	case elem_t::CTEXT:
		// setText();
		// tag = "";
		break;
	case SVG:
		//tag = "svg";
		link("x", box.x = 0);
		link("y", box.y = 0);
		link("width", box.width = 0);
		link("height", box.height = 0);
		//link("width", width = "0");
		//link("height", height = "0");
		link("xmlns", NodeSVG::svg);
		link("xmlns:svg", NodeSVG::svg);
		link("xmlns:xlink", NodeSVG::xlink);
		break;
	case TITLE:
		//tag = "title";
		break;
	case GROUP:
		// tag = "g";
		break;
	case RECT:
		// tag = "rect";
		link("x", box.x = 0);
		link("y", box.y = 0);
		link("width", box.width = 0);
		link("height", box.height = 0);
		// link("width", width = "0");
		// link("height", height = "0");
		break;
	case CIRCLE:
		// tag = "circ";
		link("cx", box.x = 0);
		link("cy", box.y = 0);
		link("r", radius = 0);
		break;
	case IMAGE:
		// tag = "image";
		link("x", box.x = 0);
		link("y", box.y = 0);
		link("width", box.width = 0);
		link("height", box.height = 0);
		//link("width", width = "0");
		// link("height", height = "0");
		// if (version == 1) {
		link("xlink:href", url); // text_anchor
		// if (version > 2.x ?) {
		//link("href", text_anchor);
		break;
	case TEXT:
		// tag = "text";
		link("x", box.x = 0);
		link("y", box.y = 0);
		// link("text-anchor", text_anchor = "");
		break;
	case TSPAN:
		// tag = "tspan";
		//link("text-anchor", text_anchor = "");
		break;
	default:
		return;
	}

	// DEPRECATING: see separate STYLE and CLASS?
	// link("style", style = "");
	// link("fill", fill = "");
	// link("opacity", opacity = ""); // string, so silent if empty


}

/// Needed for handling units in strings, like "50%" or "640px".
void NodeSVG::setAttribute(const std::string & key, const std::string &value){
	(*this)[key] = value;
}

/// Needed for handling units in strings, like "50%" or "640px".
void NodeSVG::setAttribute(const std::string & key, const char *value){
	(*this)[key] = value; // -> handleString()
}


/*
std::ostream & NodeSVG::toStream(std::ostream &ostr, const TreeSVG & tree){
	NodeXML::toStream(ostr, tree);
	return ostr;
}
*/


}  // image::

}  // drain::


template <>
template <>
drain::image::TreeSVG & drain::image::TreeSVG::operator()(const drain::image::svg::tag_t & type){ // this fails in older C++ compilers ?
	this->data.setType(type);
	return *this;
}


/*
template <>
template <>
drain::image::TreeSVG & drain::image::TreeSVG::operator()(const std::string & text){
	this->data.ctext = text;
	return *this;
}
*/

