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
/**
Part of Rack development has been done in the BALTRAD projects part-financed
by the European Union (European Regional Development Fund and European
Neighbourhood Partnership Instrument, Baltic Sea Region Programme 2007-2013)
 */

#include "FilePnm.h"
#include "util/Time.h"

#include "util/JSONtree.h"
#include "util/ValueReader.h"

namespace drain
{

namespace image
{

/// Syntax for recognising image files
const drain::RegExp FilePnm::fileNameRegExp("^((.*/)?([^/]+))\\.(p([bgpn])m)$", REG_EXTENDED | REG_ICASE);


// , const CommentReader & commentReader
void FilePnm::read(Image & image, const std::string & path) {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.info() << "path='" << path << "'" << mout.endl;

	std::ifstream infile;
	infile.open(path.c_str(), std::ios::in);

	if (!infile){
		mout.warn() << "opening file '" << path << "' failed" << mout.endl;
		return;
	}

	//std::string magic;
	//infile >> magic;

	if (infile.get() != 'P'){
		mout.warn() << "file does not start with  'P' (magic code)" << mout.endl;
		mout.error() << "not an PNM file" << mout.endl;
		return;
	}

	FileType pt = UNDEFINED;
	int width;
	int height;
	int channels = 1;
	int maxValue;

	int c = infile.get();

	switch (c){
	case '1':
		pt = PBM_ASC;
		channels = 1;
		break;
	case '2':
		pt = PGM_ASC;
		channels = 1;
		break;
	case '3':
		pt = PPM_ASC;
		channels = 3;
		break;
	case '4':
		pt = PBM_RAW;
		channels = 1;
		break;
	case '5':
		pt = PGM_RAW;
		channels = 1;
		break;
	case '6':
		pt = PPM_RAW;
		channels = 3;
		break;
	default:
		mout.error() << "unrecognized PPM type" << mout.endl;
		return;
	}

	mout.note() << "PNM type: P" <<  (char)c << " (" << channels  << " channels)" << mout.endl;

	while ((c = infile.get()) != '\n'){
		// ?
	}

	std::string key;
	//std::string value;
	std::stringstream sstr;
	while (infile.peek() == '#'){
		infile.get(); // swallow '#'
		while ((c = infile.get()) !='\n' ){

			if (c == '='){
				key = drain::StringTools::trim(sstr.str());
				sstr.str("");
			}
			else
				sstr.put(c);

			if (infile.eof())
				mout.error() << "Premature end of file" << mout.endl;
		}
		if (!key.empty()){
			mout.debug(1) << "Comment: " << key << ": " <<  sstr.str() << mout.endl;
			//value = drain::StringTools::trim(sstr.str());
			//mout.note() << "Assign: " << key << ": " <<  value << '/' << value.length()<< mout.endl;
			//std::stringstream
			// image.properties [key] = value;
			ValueReader::scanValue(sstr.str(), image.properties[key]);
			//image.properties[key] = value;
			//sstr.str("");
			//image.properties[key].toJSON(sstr);
			//mout.note() << "Comment:" << key << ": " <<  sstr.str() << mout.endl;
		}
		else {
			mout.note() << "Comment:" <<  sstr.str() << mout.endl;
		}
		sstr.str("");
	}
	mout.note() << "Done" << mout.endl;
	infile >> width;
	infile >> height;
	if ((pt != PBM_ASC) && (pt != PBM_RAW))
		infile >> maxValue;

	mout.note() << "Size:" <<  width << ',' << height << mout.endl;

	image.initialize(typeid(unsigned char), width, height, channels);

	mout.debug() << image << mout.endl;

	readFrame(image, infile);

	infile.close();


}


/** Writes drain::Image to a png image file applying G,GA, RGB or RGBA color model.
 *  Writes in 8 or 16 bits, according to template class.
 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
 */
void FilePnm::readFrame(ImageFrame & image, std::istream & infile){ // , FileType t

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.info() << "reading image: " << image << mout.endl;

	const size_t channels = image.getChannelCount();

	if (channels == 1){
		ImageFrame::iterator  it = image.begin();
		const ImageFrame::iterator eit = image.end();
		while ((infile) && (it != eit)){
			*it = infile.get();
			++it;
		}
		if (it != eit){
			mout.warn() << "premature end of file: " << image << mout.endl;
		}
	}
	else if (channels == 3){
		ImageFrame::iterator  rit = image.getChannel(0).begin();
		ImageFrame::iterator  git = image.getChannel(1).begin();
		ImageFrame::iterator  bit = image.getChannel(2).begin();
		while (infile){
			*rit = infile.get();
			++rit;
			*git = infile.get();
			++git;
			*bit = infile.get();
			++bit;
			// TODO CHECK
		}
		/*
		if (rit != eit){
			mout.warn() << "premature end of file: " << image << mout.endl;
		}
		*/
	}
	else {
		mout.error() << "Sorry, this type of PNM format (" << channels << " channels) not implemented" << mout.endl;
	}

	if (!infile.eof()){
		mout.warn() << "spurious bytes in end of file" << mout.endl;
	}

	//infile.close();

}



/** Writes drain::Image to a png image file applying G,GA, RGB or RGBA color model.
 *  Writes in 8 or 16 bits, according to template class.
 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
 */
void FilePnm::write(const ImageFrame & image, const std::string & path){

	Logger mout(getImgLog(), "FilePnm", __FUNCTION__);

	if (image.isEmpty()){
		mout.warn() << "empty image, skipping" << mout.endl;
		return;  // -1
	}

	mout.debug() << "Path: " << path << mout.endl;


	const int width    = image.getWidth();
	const int height   = image.getHeight();
	const int channels = image.getChannelCount();
	const std::type_info & type = image.getType();
	const int maxValue = drain::Type::call<drain::typeMax, int>(type);
	const bool SINGLE_BYTE = (maxValue <= 0xff);

	FileType storage_type = UNDEFINED;

	// Debugging. Consider also using for checking file extension vs channel count.
	typedef std::vector<std::string> result_t;
	result_t result;
	if (!fileNameRegExp.execute(path, result)){
		mout.debug() << "file char: " << result[4] << mout.endl;
	}

	switch (channels) {
	case 4:
		mout.warn() << "four-channel image; writing channels 0,1,2 only" << mout.endl;
		// no break
	case 3:
		storage_type = PGM_RAW;
		break;
	case 2:
		mout.warn() << "two-channel image, writing channel 0" << mout.endl;
		// no break
	case 1:
		storage_type = PGM_RAW;
		break;
	case 0:
		mout.warn() << "zero-channel image" << mout.endl;
		//fclose(fp);
		return;
	default:
		mout.error()  << "unsupported channel count: " << channels << mout.endl;
		// throw std::runtime_error(s.toStr());
	}

	std::ofstream ofstr(path.c_str(), std::ios::out);

	if (!ofstr){
		mout.error()  << "unsupported channel count: " << channels << mout.endl;
		return;
	}

	/// FILE HEADER


	mout.debug() << "magic code: P" << storage_type << mout.endl;
	ofstr << 'P' << storage_type << '\n';

	// mout.debug() << "writing comments (metadata)" << mout.endl;
	const FlexVariableMap & vmap = image.getProperties();
	for (FlexVariableMap::const_iterator it = vmap.begin(); it != vmap.end(); ++it) {
		ofstr << '#' << ' ' << it->first << '=';
		it->second.valueToJSON(ofstr);
		ofstr << '\n';
	}

	// GEOMETRY
	ofstr << width << ' ' << height << '\n';
	if (drain::Type::call<drain::typeIsSmallInt>(type))
		ofstr << maxValue << '\n';
	else {
		mout.error()  << "unimplemented: double type image (needs scaling) " << mout.endl;
		ofstr << 255 << '\n';
	}

	int i;
	int j;
	int value;

	switch (storage_type) {
	case PGM_RAW:
		if (SINGLE_BYTE){
			mout.note()  << "PGM_RAW, 8 bits" << mout.endl;
			for (j = 0; j < height; ++j) {
				for (i = 0; i < width; ++i) {
					ofstr.put(image.get<unsigned char>(i,j));
					//ofstr.
				}
			}
		}
		else {
			mout.note()  << "PGM_RAW, 16 bits" << mout.endl;
			if (maxValue > 0xffff){
				mout.warn()  << "storage type over 16 bits (max value > 0xffff) unsupported" << mout.endl;
			}
			for (j = 0; j < height; ++j) {
				for (i = 0; i < width; ++i) {
					value = image.get<unsigned char>(i,j);
					ofstr.put(value & 0xff);
					value = (value >> 8);
					ofstr.put(value & 0xff);  // check order!
				}
			}
		}
		break;
	case PGM_ASC:
		mout.note()  << "PGM_ASC" << mout.endl;
		for (j = 0; j < height; ++j) {
			for (i = 0; i < width; ++i) {
				ofstr << image.get<int>(i,j) << ' ';
			}
			ofstr << '\n';
		}
		break;
	default:
		mout.error()  << "PBM/binary types: unimplemented code"<< mout.endl;
		break;
	}

	mout.debug(1) << "Closing file" << mout.endl;
	ofstr.close();

}




} // image::

} // drain::



// Drain
