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
#ifndef MATHOP_H_
#define MATHOP_H_

#include <cmath>

// OLD base class
#include "SequentialImageOp.h"

// NEW base class
#include "FunctorOp.h"

namespace drain
{
namespace image
{



/// Rescales intensities: f' = scale*f + offset.
/**
    Scale determines the contrast, offset determines the brightness of the resulting image.
	\code
	drainage gray.png --rescale 0.5              -o scale-dark.png
	drainage gray.png --rescale 0.5,0.5          -o scale-dim.png
	drainage gray.png --rescale 2.0,-0.5,LIMIT=1 -o scale-contrast.png
	drainage gray.png --target S --rescale 0.5     -o scale-dark16.png

	drainage color.png --rescale 0.5,0.5 -o scale-dim-color.png
	\endcode
 */
// Inversely: f = (f'-offset)/scale = a*f+b, where a=1/scale and b=-offset/scale.
class ScalingFunctor : public UnaryFunctor {
    public: //re 

	ScalingFunctor(double scale = 1.0, double bias = 0.0) : UnaryFunctor(__FUNCTION__, "Rescales values linerarly: y = scale*x + bias", scale, bias){
		this->getParameters().reference("scale", this->scale);
		this->getParameters().reference("bias", this->bias);
	};

	//virtual
	inline
	double operator()(double s) const {
		return this->scaleFinal*s + this->biasFinal;
	};

protected:

	ScalingFunctor(const std::string & name, const std::string & description, double scale = 1.0, double bias = 0.0) : UnaryFunctor(name, description, scale, bias){};

};

/*
class ScaleOp : public FunctorOp<ScalingFunctor> {
    public: //re 

	ScaleOp(double scale = 1.0, double bias = 0, bool LIMIT=false){ // : FunctorOp<ScalingFunctor>(true) {
		this->functor.setScale(scale, bias);
		this->getParameters().reference("LIMIT", this->LIMIT=LIMIT);
	}

};
*/

/// Inverts intensities: f' = f_max - f
/**
   Inverts image by subtracting the pixel intensities from the maximum intensity (255, 65535 or 1.0).
	\code
	drainage gray.png  --negate  -o negate.png
	drainage color.png --negate  -o negate-color.png
	\endcode
 */
class NegateFunctor : public ScalingFunctor {
    public: //re 
	NegateFunctor() : ScalingFunctor(__FUNCTION__, "Inverts values.", -1.0, 1.0) {
		updateScale();
	}
};

/// Maps a single intensity value to another value.
/**
	\code
	drainage gray.png  --remap 1,0.1  -o remap.png
	drainage color.png --remap 1,0.25  -o remap-color.png
	\endcode
 */
class RemappingFunctor : public UnaryFunctor {
    public: //re 

	RemappingFunctor(double fromValue = 0.0, double toValue = 0.0) : UnaryFunctor(__FUNCTION__, "Rescales intensities linerarly") , fromValue(fromValue), toValue(toValue) {
		this->getParameters().reference("fromValue", this->fromValue = fromValue);
		this->getParameters().reference("toValue", this->toValue = toValue);
	};

	//virtual
	inline
	double operator()(double s) const {
		// this->scale*s + this->bias;
		if (s == fromValue)
			return toValue;
		else
			return s;
	};

	double fromValue;
	double toValue;
	//mutable int fromValueI;
	//mutable int toValueI;




};


/// Thresholds intensity values.
/**
	\code
	drainage gray.png  --threshold 0.5   -o threshold.png
	drainage color.png --threshold 0.25  -o threshold-color.png
	\endcode
 */
class ThresholdFunctor : public UnaryFunctor {
    public: //re 

	ThresholdFunctor(double threshold = 0.5, double replace = 0.0) : UnaryFunctor(__FUNCTION__, "Rescales intensities linerarly") , threshold(threshold), replace(replace) {
		this->getParameters().reference("threshold", this->threshold = threshold);
		this->getParameters().reference("replace", this->replace = replace);
	};

	inline
	double operator()(double s) const {
		if (s < threshold)
			return replace;
		else
			return s;
	};

	double threshold;
	double replace;

};

/// Thresholds intensity values.
/**
	\code
	drainage gray.png  --thresholdBinary 0.65 -o thresholdBinary.png
	drainage color.png --thresholdBinary 0.5  -o threshold-color.png
	\endcode
 */
class BinaryThresholdFunctor : public ThresholdFunctor {
    public: //re 

	BinaryThresholdFunctor(double threshold = 0.5, double replace = 0.0, double replaceHigh = 1.0) : ThresholdFunctor(threshold, replace),  replaceHigh(replaceHigh) {
		this->getParameters().reference("replaceHigh", this->replaceHigh = replaceHigh);
	};

	inline
	double operator()(double s) const {
		if (s < threshold)
			return replace;
		else
			return replaceHigh;
	};

	double replaceHigh;

};



/// Adds a intensity values .
/**
	\code
	drainage shapes1.png shapes2.png --add 0.5  -o add.png
	\endcode
 */
class AdditionFunctor : public BinaryFunctor {
    public: //re 
	AdditionFunctor(double scale = 1.0, double bias = 0.0) : BinaryFunctor(__FUNCTION__, "Adds values", scale, bias){
		this->getParameters().reference("scale", this->scale);
		this->getParameters().reference("bias", this->bias);
		// update();
	};

	inline
	double operator()(double s1, double s2) const {
		return this->scaleFinal*(s1 + s2) + this->biasFinal;
	};
};

/// Subtracts image from another image
/**
 Scaling applies to the result.
 The following subtracts image.png from image2.png :
 \code
  drainage shapes1.png shapes2.png --sub 0.4,0.5 -o sub.png
 \endcode
 */
class SubtractionFunctor : public BinaryFunctor {
    public: //re 
	SubtractionFunctor(double scale = 1.0, double bias = 0.0, bool LIMIT=false) : BinaryFunctor(__FUNCTION__, "Subtracts values", scale, bias){
		this->getParameters().reference("scale", this->scale);
		this->getParameters().reference("bias", this->bias);

		// update();
	};

	inline
	double operator()(double s1, double s2) const {
		return this->scaleFinal*(s1 - s2) + this->biasFinal;
	};
};



/// Multiplies two images, with optional post scaling and offset
/**

  \code
  drainage shapes1.png shapes2.png --mul 1.0       -o mul.png
  drainage shapes1.png shapes2.png --mul 0.5,0.25  -o mul2.png
 \endcode

 The order of the images counts; the destination dimensions are determined from the image last read.
 \code
  drainage image.png shapes.png --mul 1 -o mul-shapes.png
  drainage shapes.png image.png --mul 1 -o mul-image.png
 \endcode

 The coordinate handler can be applied as well.
 \code
  drainage shapes.png --coordPolicy 3 image.png --mul 1 -o mul-image-mirror.png
 \endcode

 */
class MultiplicationFunctor : public BinaryFunctor {
    public: //re 
	MultiplicationFunctor(double scale = 1.0, double bias = 0.0) : BinaryFunctor(__FUNCTION__, "Rescales intensities linerarly", scale, bias){
		this->getParameters().reference("scale", this->scale);
		this->getParameters().reference("bias", this->bias);
		// update();
	};

	inline
	double operator()(double s1, double s2) const {
		return this->scaleFinal*(s1*s2) + this->biasFinal;
	};
};

/// Divides image by another image
/**
 Scaling applies to the result.
 The following divides image2.png by image.png :
 \code
  drainage shapes1.png shapes2.png --div 0.5 -o div.png
 \endcode
 */
class DivisionFunctor : public BinaryFunctor {
    public: //re 
	DivisionFunctor(double scale = 1.0, double bias = 0.0) : BinaryFunctor(__FUNCTION__, "Rescales intensities linerarly", scale, bias){
		this->getParameters().reference("scale", this->scale);
		this->getParameters().reference("bias", this->bias);
		//update();
	};

	inline
	double operator()(double s1, double s2) const {
		if (s2 != 0.0)
			return this->scale*(s1/s2) + this->bias;
		else
			return NAN; //this->bias;
	};
};


/// Blends an image to another with given proportion.
/**
 *  See also QuadraticSmootherOp .
 *
 *  ...This operator produces
 *  <CODE>
 *  F2 = cF + (1-c)M{ cF + (1-c)M{F} }
 *     = cF + (1-c)cM{F} + (1-c)^2M^2{F}
 *  </CODE>
 *  where
 *  <CODE>F</CODE> is an image,
 *  <CODE>M{}</CODE> is a <CODE>W x W</CODE> WindowAverage operator, and
 *  <CODE>c</CODE> is a coefficient.
 *
 \exec~
  convert  color.png -rotate 180 color-rot180.png
 \~
 \code
   drainage shapes1.png shapes2.png  --mix 0.75  -o mix.png
   drainage color.png color-rot180.png --mix 0.25 -o mix-color.png
 \endcode
 */
class MixerFunctor : public BinaryFunctor {
    public: //re 

	MixerFunctor(double coeff=0.5, double scale=1.0, double bias=0.0) : BinaryFunctor(__FUNCTION__, "Rescales intensities linerarly", scale, bias), coeff(coeff), scaleFinal2(1.0){
		this->getParameters().reference("coeff", this->coeff);
		this->getParameters().reference("scale", this->scale);
		this->getParameters().reference("bias", this->bias);
		//update();
	};

	inline
	double operator()(double s1, double s2) const {
		return this->scaleFinal*s1 + this->scaleFinal2*s2 + this->biasFinal;
	};

	inline
	virtual
	void updateScale() const {
		this->scaleFinal  = this->scale*this->coeff;
		this->scaleFinal2 = this->scale*(1.0-this->coeff);
		this->biasFinal   = this->bias;
	}

	double coeff;

protected:
	mutable
	double scaleFinal2;
};




///
/**
 *
 *   Prescales src to dst scale. See also MinimumOp, MaximumOp and MultiplicationOp
 \code
 drainage shapes1.png shapes2.png --max  -o max.png
\endcode
*/
class MaximumFunctor : public BinaryFunctor {
    public: //re 
	MaximumFunctor(double scale = 1.0, double bias = 0.0) : BinaryFunctor(__FUNCTION__, "Rescales intensities linerarly", scale, bias){
		//this->getParameters().reference("scale", this->scale);
		//this->getParameters().reference("bias", this->bias);
		update();
	};

	inline
	double operator()(double s1, double s2) const {
		return this->scale * std::max(static_cast<double>(s1), static_cast<double>(s2)) + this->bias;
	};
};



/// Minimum intensity. Prescaled input.
/**
 *
 *   Prescales src to dst scale.  See also MaximumOp, MinimumOp and MultiplicationOp.
 \code
 drainage shapes1.png shapes2.png --min  -o min.png
 \endcode
*/
class MinimumFunctor : public BinaryFunctor {
    public: //re 
	MinimumFunctor(double scale = 1.0, double bias = 0.0) : BinaryFunctor(__FUNCTION__, "Rescales intensities linerarly", scale, bias){
		//this->getParameters().reference("scale", this->scale);
		//this->getParameters().reference("bias", this->bias);
		update();
	};

	inline
	double operator()(double s1, double s2) const {
		return this->scale * std::min(static_cast<double>(s1), static_cast<double>(s2)) + this->bias;
	};
};



}
}

#endif /*  MATH_OP_PACK */

// Drain
