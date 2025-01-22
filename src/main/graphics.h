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

// Vector graphics (mainly SVG) – EXPERIMENTAL

#ifndef RACK_GRAPHICS
#define RACK_GRAPHICS

// #include <drain/prog/CommandInstaller.h>

#include "resources.h"

// Notice: role of graphics.cpp and fileio-svg.cpp is currently equivalent

namespace rack {



/// SVG panel utils
class RackSVG { // : public drain::BasicCommand {


public:

	// Identifier for the anchor background
	static const std::string BACKGROUND_RECT; //  = "mainRect";

	/// Some SVG style classes. Identifiers for IMAGE and RECT elements over which TEXT elements will be aligned

	static
	drain::image::TreeSVG & getStyle(RackContext & ctx);


	/// Top-level GROUP used by Rack. All the graphic elements will be created inside this element.
	/**
	 *
	 *
	 */
	static
	drain::image::TreeSVG & getMainGroup(RackContext & ctx); // , const std::string & name = "");


	static
	drain::image::TreeSVG & getCurrentAlignedGroup(RackContext & ctx);

	static
	drain::image::TreeSVG & getImagePanelGroup(RackContext & ctx, const drain::FilePath & filepath);


	static
	drain::image::TreeSVG & addImage(RackContext & ctx, const drain::image::Image & src, const drain::FilePath & filepath);

	/// Currently, uses file link (does not embed)
	static
	drain::image::TreeSVG & addImage(RackContext & ctx, const drain::image::TreeSVG & svg, const drain::FilePath & filepath);

	/// Add external image from a file path.
	static
	drain::image::TreeSVG & addImage(RackContext & ctx, const drain::FilePath & filepath, const drain::Frame2D<double> & frame = {640,400});


	/// Add TEXT elements: MAINTITLE, LOCATION, TIME, GENERAL
	static
	void addTitleBox(drain::image::TreeSVG & object, GraphicsContext::ElemClass elemClass);

	static
	void addTitles(drain::image::TreeSVG & object, const std::string & anchor, GraphicsContext::ElemClass elemClass);

	/// Add rectangle
	static
	drain::image::TreeSVG & addRectangleGroup(RackContext & ctx, const drain::Frame2D<double> & frame = {200,200});


	/// Add
	/**
	 *  \param imagePanel
	 */
	static
	drain::image::TreeSVG & addImageBorder(drain::image::TreeSVG & imagePanel); // , const drain::Frame2D<double> & frame = {200,200});


	//	static
	//	void generateTitles(RackContext & ctx);

	/// Traverse groups, collecting info, recognizing common (shared) variables and pruning them recursively.
	// Re-align elements etc
	static
	void completeSVG(RackContext & ctx); // , const drain::FilePath & filepath);

	/// Given variable name, like "time" or "enddate", returns "TIME", and so on.
	// static
	// const std::string  & getTextClass(const std::string & key, const std::string & defaultClass = "");

protected:

	// Under construction...
	// static
	// void createTitleBox(TreeSVG & tree);

};

}

namespace drain {

//

template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const rack::GraphicsContext::ElemClass &x);

/// Automatic conversion of elem classes to strings.
/**
 *
template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const image::svg::tag_t & type);
 */


//template <>
//const drain::EnumDict<RackSVG::TitleClass>::dict_t  drain::EnumDict<RackSVG::TitleClass>::dict;

/*
template <>
const std::string std::static_cast<std::string>(const RackSVG::ElemClass & e){
	return drain::EnumDict<RackSVG::ElemClass>::dict.getKey(e);
}
*/

}



namespace rack {


/**
 *
 */
class MetaDataCollectorSVG : public drain::TreeVisitor<TreeSVG> {

public:

	int visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path)  override;

	int visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path)  override;

protected:

	typedef std::map<std::string, unsigned short> variableStat_t;

};

/**
 *
 */
class MetaDataPrunerSVG : public drain::TreeVisitor<TreeSVG> {

public:

	int visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path) override;

	int visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path) override;

	// Also
	GraphicsContext::TitleFlagger titles;

protected:

	typedef std::map<std::string, unsigned short> variableStat_t;

};


/// "Collects" titles from metadata. Invoked by drain::TreeUtils::traverse()
/**
 *   In tree traversal, maintains information on metadata.
 *
 *   Invoked by, hence compatible with drain::TreeUtils::traverse()
 */
class TitleCreatorSVG : public drain::TreeVisitor<TreeSVG> {

public:


	inline
	TitleCreatorSVG(GraphicsContext::TitleFlagger::ivalue_t titles) : mainHeaderHeight(50), titles(titles) {
	};

	int visitPostfix(TreeSVG & tree, const TreeSVG::path_t & odimPath) override;


	int mainHeaderHeight;

	GraphicsContext::TitleFlagger titles;

};


} // rack::


#endif
