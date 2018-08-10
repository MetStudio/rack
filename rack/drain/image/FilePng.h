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
#ifndef FILEPNG_H_
#define FILEPNG_H_


#include <string>
#include <iostream>
#include <fstream>
#include <exception>

#include <png.h>

#include "util/RegExp.h"

#include "Image.h"
//#include "File.h"
//


namespace drain
{
namespace image
{

// using namespace std;

/// For reading and writing images in PNG format
/** A lighter alternative for Magick++ which supports tens of image formats.
 *  Portable Network Graphics (PNG) format is a widely adapted, flexible image format
 *  for gray level and color images. It supports lossless compression and alpha channels.
 */
class FilePng
{
public:

	/// Syntax for recognising png files.
	static
	const drain::RegExp fileNameRegExp;


	/// Reads a png file to an image.
	/**
	 *  Converts indexed (palette) images to RGB or RGBA.
	 *  Scales data to 8 or 16 bits, according to template class.
	 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
	 *
	 *  In addition to reading to Image, supports reading to ImageFrame if type and geometry match.
	 *
	 *  \tparam T - Image or ImageFrame
	 */
	template <class T>
	static
	void read(T & image, const std::string &path, int png_transforms = 0);  //(PNG_TRANSFORM_PACKING || PNG_TRANSFORM_EXPAND));  16 >> 8?

	/// Writes image to a png file.
	/** Writes drain::Image to a png image file applying G,GA, RGB or RGBA color model.
	 *  Writes in 8 or 16 bits, according to template class.
	 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
	*/
	static void write(const ImageFrame &image, const std::string &path);

protected:

	/// Sets target type and geometry
	/**
	 *  In the case of ImageFrame (typically, an image channel), type and geometry have to match already;
	 *  otherwise an exception is thrown.
	 *
	 *  \tparam T - Image or ImageFrame
	 */
	template <class T>
	static
	void initialize(T &, const std::type_info & t, const Geometry & g);




};


template <class T>
void FilePng::read(T & image, const std::string & path, int png_transforms ) {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.info() << "path='" << path << "'" << mout.endl;


	// Try to open the file
	FILE *fp = fopen(path.c_str(), "rb");
	if (fp == NULL)
		throw std::runtime_error(std::string("FilePng: could not open file: ") + path);

	// For checking magic code (signature)
	//const unsigned int PNG_BYTES_TO_CHECK=4;
	const size_t PNG_BYTES_TO_CHECK=4;
	png_byte buf[PNG_BYTES_TO_CHECK];

	/* Read in some of the signature bytes */
	if (fread((void *)buf, size_t(1), PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK)
		throw std::runtime_error(std::string("FilePng: suspicious size of file: ") + path);

	/* Compare the first PNG_BYTES_TO_CHECK bytes of the signature.
	   Return nonzero (true) if they match */
	if (png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK) != 0)
		throw std::runtime_error(std::string("FilePng: not a png file: ")+path);



	png_structp  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr){
		throw std::runtime_error(std::string("FilePng: problem in allocating image memory for: ")+path);
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr){
	    png_destroy_read_struct(&png_ptr,(png_infopp)NULL, (png_infopp)NULL);
		throw std::runtime_error(std::string("FilePng: problem in allocating info memory for: ")+path);
	}

	/*
	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info){
	    png_destroy_read_struct(&png_ptr, &info_ptr,(png_infopp)NULL);
	    throw std::runtime_error(std::string("FilePng: problem in allocating end_info memory for: ")+path);
	}
	*/

	// This may be unstable. According to the documentation, if one uses the high-level interface png_read_png()
	// one can only configure it with png_transforms flags (PNG_TRANSFORM_*)
	png_set_palette_to_rgb(png_ptr);

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);

	/// Main action
	//if (drain::Debug > 2)
	mout.debug(2) << "reading data" << mout.endl;

	png_read_png(png_ptr, info_ptr, png_transforms, NULL);





	/// Read comments
	mout.debug(2) << "reading image comments" << mout.endl;
	int num_text = 0;
	png_textp text_ptr = NULL;
	png_get_text(png_ptr, info_ptr,&text_ptr, &num_text);
	//mout.debug(2) << '\n';
	for (int i = 0; i < num_text; ++i) {
		mout << text_ptr[i].key << '=' << text_ptr[i].text << '\n';
		image.properties[text_ptr[i].key] = (const char *)text_ptr[i].text;
	}
	mout << mout.endl;


	const unsigned int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	drain::Type t;
	switch (bit_depth) {
	case 16:
		//image.initialize<unsigned short>();
		t.setType<unsigned short>();
		break;
	case 8:
		t.setType<unsigned char>();
		break;
	default:
		fclose(fp);
		png_destroy_read_struct(&png_ptr,&info_ptr, (png_infopp)NULL);
		//png_free_data(png_ptr,info_ptr,PNG_FREE_ALL,-1);  // ???
		throw std::runtime_error(std::string("FilePng: unsupported bit depth in : ")+path);
		return;
	}

	//image.setType(t);
	mout.info() << "initialize, type " << image.getType().name() << mout.endl;

	/// Copy to drain::Image
	const unsigned int width  = png_get_image_width(png_ptr, info_ptr);
	const unsigned int height = png_get_image_height(png_ptr, info_ptr);
	const unsigned int channels = png_get_channels(png_ptr, info_ptr);


	Geometry g(image.getGeometry());

	// This test enables read into an alpha channel.
	if  ((channels!=g.getChannelCount())||(width!=g.getWidth())||(height!=g.getHeight())){
		switch (channels) {
		case 4:
			g.setGeometry(width,height,3,1);
			break;
		case 3:
			g.setGeometry(width,height,3);
			break;
		case 2:
			g.setGeometry(width,height,1,1);
			break;
		case 1:
			g.setGeometry(width,height,1);
			break;
		default:
			throw std::runtime_error(std::string("FilePng: invalid channel count in : ")+path);
		}
	}

	// Form Image, set target type and geometry. For ImageFrame, compare target type and geometry. If differences, throw exception.
	initialize(image, t, g);
	//if (image.hasAlphaChannel())
	//	image.getAlphaChannel().getScaling().setPhysicalRange(0.0, 1.0);

	mout.debug(2) << "png geometry ok, ";
	mout << "png channels =" << channels << "\n";
	mout << "png bit_depth=" << bit_depth << "\n";
	mout << mout.endl;

	// TODO: use png_get_pCal(.........)
#ifdef PNG_pCAL_SUPPORTED___DEFUNCT
	/// Read physical scaling
	if (info_ptr->pcal_X0 == info_ptr->pcal_X1){
		mout.toOStr() << "physical scale supported, but no intensity range, pcalX0=" <<  info_ptr->pcal_X0 << ", pcalX1=" <<  info_ptr->pcal_X1  << mout.endl;
		image.setDefaultLimits();
	}
	else {
		image.setLimits(info_ptr->pcal_X0, info_ptr->pcal_X1);
		mout.note() << "setting physical scale: " << image << mout.endl;
	}

#endif

	/*
	 if ((bit_depth!=8) && (bit_depth != 16)){
		fclose(fp);
		png_destroy_read_struct(&png_ptr,&info_ptr, (png_infopp)NULL);
		//png_free_data(png_ptr,info_ptr,PNG_FREE_ALL,-1);  // ???
		throw std::runtime_error(std::string("FilePng: unsupported bit depth in : ")+path);
	}
	*/

	png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);
	png_bytep p;
	int i0;
	for (unsigned int j = 0; j < height; ++j) {
		p = row_pointers[j];
		for (unsigned int i = 0; i < width; ++i) {
			for (unsigned int k = 0; k < channels; ++k) {
				i0 = channels*i + k;
				if (bit_depth == 8) {
					//image.at(i,j,k) = p[i0];
					image.put(i,j,k, p[i0]);
				}
				else {
					image.put(i,j,k, (p[i0*2]<<8) + (p[i0*2+1]<<0));
					//image.at(i,j,k) = p[i0*2] + (p[i0*2+1]<<8);
				}
			}
		}
	}

	fclose(fp);
	png_destroy_read_struct(&png_ptr,&info_ptr, (png_infopp)NULL);
	//png_free_data(png_ptr,info_ptr,PNG_FREE_ALL,-1);  // ???

	//png_destroy_read_struct(&png_ptr,(png_infopp)NULL, (png_infopp)NULL);
	//png_destroy_info_struct(png_ptr,&info_ptr);


}



}

}

#endif /*FILEPng_H_*/

// Drain
