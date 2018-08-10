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


#ifndef RACK_IMAGES
#define RACK_IMAGES

#include <drain/image/Image.h>
#include <drain/prog/Command.h>
#include <drain/prog/CommandAdapter.h>

#include "data/DataSelector.h"


#include "resources.h"
#include "commands.h" // cmdSelect

namespace drain {

typedef BeanRefAdapter<drain::image::ImageOp> ImageOpAdapter;

}


namespace rack {

/**
 *   Applied also by CartesianGrid
 */
class CmdImage : public drain::BasicCommand {

public: //re

	mutable DataSelector imageSelector;

	CmdImage() : BasicCommand(__FUNCTION__, "Copies data to a separate image object. Encoding can be changed with --target ."), imageSelector(".*/data/?$","") {
	};

	inline
	void exec() const {

		//drain::Logger & mout = resources.mout;
		RackResources & resources = getResources();
		imageSelector.setParameters(resources.select);
		resources.select.clear();

		convertImage(*resources.currentHi5, imageSelector, resources.targetEncoding, resources.grayImage);
		resources.targetEncoding.clear();
		//convertImage(*getResources().currentHi5, imageSelector, properties, getResources().grayImage);

		resources.currentGrayImage = & resources.grayImage;
		resources.currentImage     = & resources.grayImage;
		//File::write(*resources.currentImage, "convert.png");
	};

	static
	void convertImage(const HI5TREE & src, const DataSelector & selector, const std::string & parameters,
			drain::image::Image &dst);


};
extern CommandEntry<CmdImage> cmdImage;

/**
 *   Applied also by CartesianGrid
 */
class CmdPhysical : public drain::SimpleCommand<bool> {

public:

	CmdPhysical() : drain::SimpleCommand<bool>(__FUNCTION__, "Handle intensities as physical quantities like dBZ (instead of that of storage type).",
			"value", true, "0,1") {
	};


};
extern CommandEntry<CmdPhysical> cmdPhysical;


/// Designed for Rack
class ImageOpRacklet : public drain::ImageOpAdapter {

public:

	/// Constructor that adapts an operator and its name.
	/**
	 *  \param op - image operator to be used
	 *  \param key - command name
	 */
	ImageOpRacklet(drain::image::ImageOp & imageOp, const std::string & key) : drain::ImageOpAdapter(imageOp), key(key) {
	};


	/// Constructor that adapts an operator and its name.
	/**
	 *  \param op - image operator to be used througjh reference
	 */
	ImageOpRacklet(drain::image::ImageOp & imageOp) : drain::ImageOpAdapter(imageOp), key(imageOp.getName()) {
	};


	/// Copy constructor.
	ImageOpRacklet(const ImageOpRacklet & a) : ImageOpAdapter(a.bean), key(a.key) {
		//imageOp.getParameters().updateFromMap(a.imageOp.getParameters());
	}

	virtual
	void exec() const;

	/// Name of this operator, to be recognized
	const std::string key;

	static std::string outputQuantity;

protected:

};


class ImageRackletModule : public CommandGroup {
public:

	typedef std::list<ImageOpRacklet> list_t;

	static
	list_t rackletList;

	ImageRackletModule(const std::string & section = "image", const std::string & prefix = "i");

};


} /* namespace rack */

#endif

// Rack
