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


#include <ostream>
#include <fstream>
#include "util/Log.h"
#include "image/Sampler.h"
#include "image/File.h"
//#include "ImageMod.h"
#include "ImageModifierPack.h"


namespace drain
{

namespace image
{


void ImageChannels::initialize(Image & dst) const {

	drain::Logger mout(this->name, __FUNCTION__);

	dst.setChannelCount(imageChannelCount, alphaChannelCount);
	// Reset for the next invocation.
	imageChannelCount = 1;
	alphaChannelCount = 0;

}

void ImageEncoding::initialize(Image & dst) const { //(const std::string & params, char assignmentSymbol='=') {

	drain::Logger mout(this->name, __FUNCTION__);


	/// Step 1: change type, if requested
	type.clear();
	refMap.setValues(request);
	if (!type.empty()){
		if (dst.getType() != drain::Type::getTypeInfo(type)){
			dst.resetGeometry();
			dst.setType(type);
		}
	}

	/// Step 2: Set default values, based on its current type.
	scaling.set(dst.getScaling());
	/// Reset given values
	refMap.setValues(request);
	// Apply
	dst.setScaling(scaling);
	/*
	if ((dst.scaling.minCodeValue >= 0) && (dst.scaling.minPhysValue < 0.0)){
		/// light error
		mout.warn() << "unsigned storage type, negative physical values not supported: " << scaling.toStr() << mout.endl;
	}
	*/

	mout.debug() << scaling.toStr() << mout.endl;
	mout.debug(1) << dst << mout.endl;
}


//void ImageHistogram::traverseChannels(ImageTray<Channel> & dst) const {
void ImageHistogram::traverseChannel(Channel & dst) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.debug() << "bins: " << bins << mout.endl;

	histogram.setSize(bins);
	histogram.clearBins();
	//dst.get().fill(v[i]);
	for (Channel::iterator it = dst.begin(); it != dst.end(); ++it){
		histogram.increment(*it);
	}

	std::cout << histogram << std::endl;

	//const size_t channels = std::min(v.size(), dst.size() + dst.alpha.size());
	/*
	for (size_t i = 0; i < channels; ++i) {
		if (i < dst.size())
			dst.get(i).fill(v[i]);
		else
			dst.alpha.get(i-dst.size()).fill(v[i]);
	}
	*/
}

void ImageFill::traverseChannels(ImageTray<Channel> & dst) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.note() << value << mout.endl;

	std::vector<double> v;
	Variable(value, typeid(double)).toContainer(v);

	const size_t channels = std::min(v.size(), dst.size() + dst.alpha.size());
	for (size_t i = 0; i < channels; ++i) {
		if (i < dst.size())
			dst.get(i).fill(v[i]);
		else
			dst.alpha.get(i-dst.size()).fill(v[i]);
	}

}

void ImageFill::traverseChannel(Channel & dst) const {

	drain::Logger mout(name, __FUNCTION__);
	//mout.note() << value << mout.endl;
	dst.fill(Variable(value));
}


void ImageGeometry::initialize(Image & dst) const {

	//std::cerr << "TEST" << std::endl;

	drain::Logger mout(name, __FUNCTION__);
	// mout.note() << "file:" << value << mout.endl;
	//const size_t height = this->height > 0 ? this->height : width;
	if (height == 0)
		height = width;
	dst.setGeometry(width, height, imageChannelCount, alphaChannelCount);
	mout.debug() << dst.getGeometry() << mout.endl;
	// Reset
	imageChannelCount = 1;
	alphaChannelCount = 0;
	height = 0;
}



void ImageCoordPolicy::initialize(Image & dst) const {

	drain::Logger mout(name, __FUNCTION__);

	std::vector<int> v;
	Variable(value, typeid(int)).toContainer(v);

	/// Check values
	for (size_t i = 0; i < v.size(); ++i) {
		if ((v[i]==0) || (v[i]>4)){
			mout.warn() << "undefined coordPolicy index: " << v[i] << mout.endl;
			v[i] = 1;
		}
	}

	//CoordinatePolicy policy;
	switch (v.size()) {
	case 4:
		policy.yOverFlowPolicy  = v[3];
		policy.xOverFlowPolicy  = v[2];
		policy.yUnderFlowPolicy = v[1];
		policy.xUnderFlowPolicy = v[0];
		break;
	case 2:
		policy.yOverFlowPolicy  = v[1];
		policy.xOverFlowPolicy  = v[0];
		policy.yUnderFlowPolicy = v[1];
		policy.xUnderFlowPolicy = v[0];
		break;
	case 1:
		policy.yOverFlowPolicy  = v[0]; //.get<int>(0);
		policy.xOverFlowPolicy  = v[0];
		policy.yUnderFlowPolicy = v[0];
		policy.xUnderFlowPolicy = v[0];
		break;
	default:
		mout.error() << "Wrong number of parameters (not 1, 2 or 4): " << v.size() << " (" << value << ")" << mout.endl;
		break;
	}

	mout.debug() << policy << mout.endl;
	dst.setCoordinatePolicy(policy);
}


void ImageCoordPolicy::traverseChannel(Channel & dst) const {

	drain::Logger mout(name, __FUNCTION__);
	//mout.note() << "file:" << value << mout.endl;
	dst.setCoordinatePolicy(policy);
}



void ImagePlot::traverseChannels(ImageTray<Channel> & dst) const {
//void ImagePlot::traverseChannel(Channel & dst) const {

	drain::Logger mout(name, __FUNCTION__);

	// Note: value is of type string
	//mout.note() << getParameters() << mout.endl;

	typedef double data_t;
	std::vector<data_t> v;
	//Variable(value, typeid(data_t)).toContainer(v);
	StringTools::split(value, v, ',');
	//Variable(value, typeid(data_t)).toContainer(v);

	if (v.size() <= 2){
		mout.warn() << "coordinates ("<< value << ") given but no intensities; returning";
		return;
	}

	//mout.debug() << "Variable p=" << p.toStr() << " vector vField[" << vField.size() << ']' << mout.endl;

	const size_t nImages = dst.size();
	const size_t nAlphas = dst.alpha.size();
	const size_t nChannels = nImages + nAlphas;

	size_t n = v.size()-2;

	mout.debug() << "i=" << nImages << ", a=" << nAlphas << " n=" << n << mout.endl;

	if (n < nChannels){
		mout.info() << "image has " << nChannels << " channels, yet only " << n << " intensities supplied" << mout.endl;
	}
	else if (n > nChannels){
		mout.warn() << "image has " << nChannels << " channels, discarding " << (n-nChannels) << " trailing intensities " << mout.endl;
		n = nChannels;
	}

	CoordinateHandler2D coordHandler; //(dst.getWidth(), dst.getHeight(), dst.getCoordinatePolicy());
	coordHandler.setPolicy(dst.get().getCoordinatePolicy());
	coordHandler.setLimits(dst.getGeometry().getWidth(), dst.getGeometry().getHeight());
	int i = v[0];
	int j = v[1];

	if (!coordHandler.handle(i, j)){
		for (size_t k=0; k<n; ++k){
			//mout.warn() << k << " kaneli: " << vField[2+k] << "\n";
			Channel & channel = (k<nImages) ? dst.get(k) : dst.getAlpha(k-nImages);
			if (k>=nImages){
				//mout.warn() << k << "quality channel <=" << vField[2+k] << mout.endl;
			}
			else {
				//mout.warn() << k << "normi channel <=" << vField[2+k] << mout.endl;
			}
			// coordHandler.setPolicy(channel.getCoordinatePolicy());
			// coordHandler.setLimits(channel.getWidth(), channel.getHeight());
			/*
			i = v[0];
			j = v[1];
		    if (!coordHandler.handle(i, j)){
			  const drain::typeLimiter<data_t>::value_t & limit = channel.getLimiter<data_t>();
			  channel.putScaled(i, j, limit(v[2+k]));
		     }
			 */
			const ImageScaling & scaling = channel.getScaling();
			const drain::typeLimiter<data_t>::value_t & limit = channel.getLimiter<data_t>();
			channel.put(i, j, limit(scaling.inv(v[2+k])));
			//channel.putScaled(i, j, limit(v[2+k])); // ! scaled
			// channel.put(i, j, v[2+k]);
		}
	}


}



//void ImagePlotFile::process(ImageDst & imageDst) const {
void ImagePlotFile::traverseFrame(ImageFrame & dst) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.note() << "file: " << filename << mout.endl;

	//Drainage & r = getDrainage();
	//Image & dst = r.getDst();
	//Image & dst = imageDst.get();

	std::ifstream ifstr;
	ifstr.open(filename.c_str());
	//read(ifstr);
	std::istream &istr = ifstr; // std::cin;

	std::string line;
	double x;
	double y;
	double d = 0.0;
	const size_t n = dst.getChannelCount(); //imageDst.size();
	std::stringstream sstr;
	while ( !getline(istr,line).eof() ){

		//if ((line.length() > 0) && line.at(0))
		if (line.at(0) != '%'){
			sstr.str(line);
			sstr >> x >> y;
			for (size_t k=0; k<n; k++){
				sstr >> d;
				if (sstr.eof())
					break;
				dst.put<double>(x,y,k,d);
				//imageDst.get(k).put<double>(x,y,k,d);
			}
			std::cout << x << ',' << y << '\t' << d << '\n';
			//std::cout << i << ' ' << x << '\t' << y << '\n';
		}
		//ifstr.get
	}

	ifstr.close();

}


ImageSampler::ImageSampler() : ImageMod(__FUNCTION__, "Extract samples. See --format."){
	setReferences();
}

void ImageSampler::setReferences(){
	parameters.reference("iStep",  sampler.iStep = 10, "horz coord step");
	parameters.reference("jStep",  sampler.jStep =  0, "vert coord step");
	parameters.reference("iStart", sampler.iStart = -1, "horz coord start");
	parameters.reference("jStart", sampler.jStart = -1, "vert coord start");
	parameters.reference("iEnd",   sampler.iEnd = -1,  "horz coord end");
	parameters.reference("jEnd",   sampler.jEnd = -1,  "vert coord end");
	parameters.reference("commentChar",   sampler.commentChar = "#",  "comment character");
	parameters.reference("skipVoid", sampler.skipVoid = 0,  "skip lines with invalid/missing values");
}

void ImageSampler::process(Image & dst) const {  // consider void traverse(const Channel & src) const {

	drain::Logger mout(getName() + "(ImageSampler)", __FUNCTION__);

	drain::image::ImageReader reader(sampler.variableMap); //  reader(sampler.variableMap);

	mout.debug() << "variableMap: " << sampler.variableMap << mout.endl;

	reader.setSize(dst.getWidth(), dst.getHeight());

	std::map<std::string, ChannelView> images;

	const size_t imageChannelCount = dst.getImageChannelCount();
	for (size_t i=0; i<dst.getChannelCount(); ++i){
		std::stringstream sstr;
		if (i < imageChannelCount)
			sstr << i;
		else
			sstr << 'A' << (i-imageChannelCount);
		images[sstr.str()].setView(dst.getChannel(i));
		//mout.warn() << sstr.str() << '\t' << images[sstr.str()] << mout.endl;
		const Channel & frame = images[sstr.str()];
		mout.debug() << sstr.str() << '\t' << frame << mout.endl;
		//drain::image::File::write(frame, sstr.str()+".png");
		/*
		for (int i = 0; i < frame.getHeight(); i+=16){
			std::cout << i << '\t' << frame.get<double>(i,i) << '\n';
		}
		*/
	}

	if (!filename.empty() && (filename != "-")){
		std::ofstream ofstr(filename.c_str(), std::ios::out);
		sampler.sample(images, reader, getFormat(), ofstr); // cmdFormat.value
		ofstr.close();
	}
	else {
		sampler.sample(images, reader, getFormat(), std::cout); // cmdFormat.value
	}

}


}
}


// Drain
