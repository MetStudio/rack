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
#ifndef CROP_OP_H_
#define CROP_OP_H_

#include "ImageOp.h"

namespace drain
{

namespace image
{
// help copy

/**

To copy image to another image:
\code
  drainage image.png --crop 100,100  -o crop.png
\endcode

Offset can be set in third and fourth arguments:
\code
  drainage image.png --crop 100,100,+100,+100  -o crop-offset.png
\endcode

Cropping area can be larger than the original image, and the offsets can be negative.
\code
  drainage image.png --crop 640,400,-100,-100  -o crop-larger.png
\endcode

The coordinates outside the image are applied using coordinate handler, which defines the handling separately for each direction.
By default, the \i coodinate \i policy is \c LIMIT , which identifies the external pixels with the the edge pixels.


 */	
class CropOp : public ImageOp
{

public:

	inline
	CropOp(int width=0, int height=0, int i0=0, int j0=0) : ImageOp (__FUNCTION__, "Crop image.") {
		 parameters.reference("width", this->width  = width);
		 parameters.reference("height",this->height = height);
		 parameters.reference("i", this->i0 = i0);
		 parameters.reference("j", this->j0 = j0);
		 //setParameters(p);
	};

	void filter(const Image & src, Image & dst) const;

	int i0;
	int j0;
	int width;
	int height;

};

} // image::

} // drain::

#endif /*COPYOP_H_*/

// Drain
