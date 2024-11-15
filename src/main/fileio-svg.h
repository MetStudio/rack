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

#ifndef RACK_FILE_IO_SVG
#define RACK_FILE_IO_SVG

#include <string>

#include <drain/prog/CommandBank.h>
#include <drain/prog/CommandInstaller.h>

#include <drain/util/FileInfo.h>
//#include <drain/util/RegExp.h>
// #include <drain/util/TreeHTML.h>


#include "hi5/Hi5.h"

#include "resources.h"

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

	int mainHeaderHeight;

	inline
	TitleCreatorSVG() : mainHeaderHeight(50) {
	};

	int visitPostfix(TreeSVG & tree, const TreeSVG::path_t & odimPath) override;

};




/// SVG panel utils
class RackSVG { // : public drain::BasicCommand {


public:

	/// Group identifiers for IMAGE and RECT elements inside which TEXT elements will be aligned

	/// Standard group name for images to be aligned
	static
	const std::string IMAGE_FRAME; //	"imageFrame"


	// Semantics

	/// Marker class for titles of default type.
	static
	const std::string GENERAL;

	/// Marker class for vertical text alignment.
	static
	const std::string TIME;

	/// Marker class for vertical text alignment.
	static
	const std::string LOCATION;


	static
	drain::image::TreeSVG & getMain(RackContext & ctx);


	static
	drain::image::TreeSVG & getCurrentGroup(RackContext & ctx);


	static
	drain::image::TreeSVG & addImage(RackContext & ctx, const drain::image::Image & src, const drain::FilePath & filepath);

	/// Currently, uses file link (does not embed)
	static
	drain::image::TreeSVG & addImage(RackContext & ctx, const drain::image::TreeSVG & svg, const drain::FilePath & filepath);

	/// Add external image from a file path.
	static
	drain::image::TreeSVG & addImage(RackContext & ctx, const drain::FilePath & filepath, const drain::Frame2D<double> & frame = {640,400});

	/// Add rectangle
	static
	drain::image::TreeSVG & addRectangle(RackContext & ctx, const drain::Frame2D<double> & frame = {200,200});


	/// Traverse groups, collecting info, recognizing common (shared) variables and pruning them recursively.
//	static
//	void generateTitles(RackContext & ctx);

	// Re-align elements etc
	static
	void completeSVG(RackContext & ctx, const drain::FilePath & filepath);

	/// Given variable name, like "time" or "enddate", returns "TIME", and so on.
	// static
	// const std::string  & getTextClass(const std::string & key, const std::string & defaultClass = "");

protected:

	// Under construction...
	// static
	// void createTitleBox(TreeSVG & tree);

};


/// SVG panel
class CmdOutputPanel : public drain::BasicCommand {

public:

	inline
	CmdOutputPanel() : drain::BasicCommand(__FUNCTION__, "Save SVG panel of latest images. See also: --image, --outputRawImages.") {
		getParameters().link("filename", filename="");
		getParameters().link("layout", layout, "basic");
	};

	CmdOutputPanel(const CmdOutputPanel & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}


	void exec() const;


	std::string layout;
	std::string filename;

	void appendImage(TreeSVG & group, const std::string & prefix, drain::VariableMap & vmap,
			const drain::Point2D<double> & location, const drain::image::Image & image, drain::BBox & bbox) const;
};



} // rack::


#endif

