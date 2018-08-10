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

#ifndef DATACONVERSIONOP2_H_
#define DATACONVERSIONOP2_H_

#include <drain/util/LinearScaling.h>

#include "data/ODIM.h"
#include "data/Data.h"
//&#include "PolarProductOp.h"
#include "VolumeTraversalOp.h"
//#include "VolumeOpNew.h"


namespace rack {

/// Converts HDF5 data to use desired data type, scaling and encoding (ODIM gain, offset, undetect and nodata values).
/**
 *
 *  \tparam M - ODIM type
 *
 *  \see Conversion
 *
 */
template <class M>
class DataConversionOp: public ProductOp<M, M> {

public:

	typedef SrcType<M const> src_t;
	typedef DstType<M> dst_t;

	DataConversionOp(const std::string & type="C", double gain=1.0, double offset=0.0,
			double undetect=0.0, double nodata=255.0, std::string copyGroupSuffix="") :
				ProductOp<M, M>(__FUNCTION__, "Converts HDF5 data to use desired data type, scaling and encoding") {

		this->allowedEncoding.reference("what:type", this->odim.type = type);
		this->allowedEncoding.reference("what:gain", this->odim.gain = gain);
		this->allowedEncoding.reference("what:offset", this->odim.offset = offset);
		this->allowedEncoding.reference("what:undetect", this->odim.undetect = undetect);
		this->allowedEncoding.reference("what:nodata", this->odim.nodata = nodata);

	}

	virtual ~DataConversionOp(){};

	/// Ensures data to be in standard type and scaling. Makes a converted copy if needed.
	//static	const HI5TREE & getNormalizedDataOLD(const DataSet< src_t> & srcDataSet, DataSet<dst_t> & dstDataSet, const std::string & quantity){}:


	void processH5(const HI5TREE &src, HI5TREE &dst) const;

	virtual
	void processDataSet(const DataSet< src_t> & srcSweep, DataSet<dst_t> & dstProduct) const;

	/// Converts src to dst such that dst applies desired gain, offset, undetect and nodata values.
	//inline
	//void processPlainData(const PlainData< src_t> & src, PlainData<dst_t> & dst) const;

	inline
	void processImage(const PlainData< src_t> & src, drain::image::Image & dst) const {
		processImage(src.odim, src.data, this->odim, dst);
	}


	/// Converts src to dst such that dst applies desired gain, offset, undetect and nodata values.
	/**
	 *   Sometimes this is applied directly (for alpha channel ops).
	 */
	void processImage(const ODIM & odimSrc, const drain::image::ImageFrame & src, const ODIM & odimDst, drain::image::Image & dst) const;

	void traverseImageFrame(const ODIM & odimSrc, const drain::image::ImageFrame & src, const ODIM & odimDst, drain::image::ImageFrame & dst) const;


	inline
	void setGeometry(const M & srcODIM, PlainData<dst_t> & dstData) const {
		// Does not change geometry.
	}


	static
	PlainData< DstType<M> > & getNormalizedData(const DataSet< src_t> & srcDataSet, DataSet<dst_t> & dstDataSet, const std::string & quantity);


protected:

	/// Suffix for trailing path element ("/data") for storing the original.
	std::string copyGroupSuffix;

};


template <class M> //
PlainData< DstType<M> > & DataConversionOp<M>::getNormalizedData(const DataSet< src_t> & srcDataSet, DataSet<dst_t> & normDataSet,
		const std::string & quantity) { // , const PlainData< src_t> & mika

	drain::Logger mout("DataConversionOp<>", __FUNCTION__);

	const std::string quantityExt = quantity+"_norm";  // std::string("~") +

	//typename DataSet< SrcType<M const> >::const_iterator it = normDataSet.find(quantityExt);
	typename DataSet< dst_t >::iterator it = normDataSet.find(quantityExt);
	if (it != normDataSet.end()){
		mout.note() << "using cached data: " << quantityExt << mout.endl;
		return it->second;
	}
	else {

		const PlainData<src_t> &  srcData = srcDataSet.getData(quantity);
		const EncodingODIM     & odimNorm = getQuantityMap().get(quantity).get(); //[srcData.odim.type.at(0)];

		mout.info() << "converting and adding to cache: " << quantityExt << " odim: " << odimNorm << mout.endl;
		PlainData<dst_t> & dstDataNew = normDataSet.getData(quantityExt);
		dstDataNew.setNoSave();
		DataConversionOp<M> op;
		//op.odim.importMap(odimNorm);
		// mout.warn() << "odimNorm: " << odimNorm << mout.endl;
		// mout.warn() << "op.odim: " << op.odim << mout.endl;
		dstDataNew.odim.importMap(srcData.odim);
		dstDataNew.odim.importMap(odimNorm);
		dstDataNew.odim.quantity = quantity;
		//op.processData(srcData, dstDataNew);
		op.processImage(srcData.odim, srcData.data, dstDataNew.odim, dstDataNew.data);
		dstDataNew.odim.quantity = quantityExt;
		dstDataNew.updateTree2(); // @?
		mout.debug() << "obtained: " << dstDataNew << mout.endl;

		return dstDataNew;
	}

	//return normDataSet.getData(quantityExt);

}



template <class M> //// copied from VolumeOp::processVolume
void DataConversionOp<M>::processH5(const HI5TREE &src, HI5TREE &dst) const {

	drain::Logger mout(this->getName(), __FUNCTION__);

	mout.debug() << "start" << mout.endl;
	mout.debug(2) << *this << mout.endl;
	mout.debug(1) << "DataSelector: "  << this->dataSelector << mout.endl;

	/// Usually, the operator does not need groups sorted by elevation.
	mout.debug(2) << "collect the applicable paths"  << mout.endl;
	std::list<std::string> dataPaths;  // Down to ../dataN/ level, eg. /dataset5/data4
	DataSelector::getPaths(src,  this->dataSelector, dataPaths);

	mout.debug(2) << "populate the dataset map, paths=" << dataPaths.size() << mout.endl;
	std::set<ODIMPathElem> parents;

	const drain::RegExp quantityRegExp(this->dataSelector.quantity);

	for (std::list<std::string>::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){

		//mout.debug(2) << "elangles (this far> "  << elangles << mout.endl;
		//mout.debug() << *it << mout.endl;

		ODIMPath path = DataTools::getParent(*it);
		ODIMPathElem parent = path.back();

		mout.debug() << "check " << parent << '<' << *it << mout.endl;

		if (parents.find(parent) == parents.end()){
			if (parent.getType() != BaseODIM::DATASET){
				mout.note() << "non-dataset group: " << parent << mout.endl;
			}
			mout.note() << "append " <<  parent << mout.endl;
			parents.insert(parent);
			const DataSet<src_t> srcDataSet(src(path), quantityRegExp);
			DataSet<dst_t> dstDataSet(dst(path));
			processDataSet(srcDataSet, dstDataSet);
		}
		else {
			mout.note() << "already exists" << parent << mout.endl;
		}

	}
}

template <class M>
void DataConversionOp<M>::processDataSet(const DataSet<src_t> & srcSweep, DataSet<dst_t> & dstProduct) const {

	drain::Logger mout(this->name, __FUNCTION__);

	// Traverse quantities
	for (typename DataSet< src_t>::const_iterator it = srcSweep.begin(); it != srcSweep.end(); ++it){

		const std::string & quantity = it->first;

		if (quantity.empty()){
			mout.warn() << "empty quantity for data, skipping" << mout.endl;
			continue;
		}

		std::stringstream sstr; //quantity); //+"-tmp");
		//sstr << "data" << ++index << "tmp-"<< quantity;
		sstr << "data0." << quantity;

		mout.debug() << "quantity: " << quantity << mout.endl;

		const Data< src_t> & srcData = it->second;
		Data<dst_t>       & dstData = dstProduct.getData(quantity);

		mout.debug(1) << EncodingODIM(this->odim) << mout.endl;
		//mout.toOStr() << "src " << (long int) &(srcData.data) << EncodingODIM(srcData.odim) << mout.endl;
		//mout.warn() << "dst " << (long int) &(dstData.data) << EncodingODIM(dstData.odim) << mout.endl;

		const drain::Type t(this->odim.type);

		//const bool IN_PLACE = (&dstData.data == &srcData.data) && (t == srcData.data.getType());
		const bool IN_PLACE = false;
		if (IN_PLACE){

			if (ODIM::haveSimilarEncoding(srcData.odim, this->odim)){
				mout.info() << "already similar encoding, no need to convert" << mout.endl;
				continue; // to next quantity
			}

			mout.debug() << "in-place" << mout.endl;
			//processData(srcData, dstData);
			//processImage(srcData.odim, srcData.data, dstData.odim, dstData.data);
			//@ dstData.updateTree();
			mout.fatal() << "not implemented" << mout.endl;
		}
		else {
			//mout.warn() << "in-place" << mout.endl;
			mout.info() << "using tmp data (in-place computation not possible)" << mout.endl;

			if (ODIM::haveSimilarEncoding(srcData.odim, this->odim)){
				mout.info() << "already similar encoding, no need to convert" << mout.endl;
				continue; // to next quantity
			}

			const M srcODIM(srcData.odim); // copy, because src may be modified next
			dstData.odim.quantity = quantity;
			dstData.odim.NI = srcData.odim.NI; // if Cart?
			ProductBase::handleEncodingRequest(dstData.odim, this->encodingRequest);
			//processData(srcData, dstData2);
			processImage(srcODIM, srcData.data, dstData.odim, dstData.data);

		}

	}

}

/*
template <class M>
void DataConversionOp<M>::processPlainData(const PlainData< src_t> & src, PlainData<dst_t> & dst) const {


	drain::Logger mout(this->name, __FUNCTION__);

	mout.debug() << "start, " << dst << mout.endl;

	processImage(src.odim, src.data, dst.odim, dst.data);

}
*/

template <class M>
void DataConversionOp<M>::processImage(const ODIM & srcOdim, const drain::image::ImageFrame & srcImage, const ODIM & dstOdim, drain::image::Image & dstImage) const {


	drain::Logger mout(this->name, __FUNCTION__);
	mout.debug(1) << "start, type=" << this->odim.type << ", geom=" << srcImage.getGeometry() << mout.endl;

	// const drain::Type t(this->odim.type);
	const drain::Type t(dstOdim.type);

	const drain::image::Geometry g(srcImage.getGeometry());

	if (srcImage.hasOverlap(dstImage)){
		if ((t.getType() != srcImage.getType()) || (g != dstImage.getGeometry())){
			mout.debug() << "using temp image" << mout.endl;
			drain::image::Image tmp;
			tmp.setType(t);
			tmp.setGeometry(g);
			tmp.setScaling(dstOdim.gain, dstOdim.offset);
			traverseImageFrame(srcOdim, srcImage, dstOdim, tmp);
			dstImage.swap(tmp);
			return;
		}
		else {
			mout.warn() << "same type and geometry, hence only rescaling (in-place)" << mout.endl;
		}
		/*
		if (t != srcImage.getType2()){
			mout.error() << "trying to change type when dst==src" << mout.endl;
			return;
		}
		if (g != dstImage.getGeometry()){
			mout.error() << "trying to change geometry when dst==src" << mout.endl;
			return;
		}
		*/
	}
	else {
		dstImage.setType(t);
		dstImage.setGeometry(g);
		dstImage.setScaling(dstOdim.gain, dstOdim.offset);
		mout.debug(1) << "dst:" << dstImage << mout.endl;
	}

	traverseImageFrame(srcOdim, srcImage, dstOdim, dstImage);

}

template <class M>
void DataConversionOp<M>::traverseImageFrame(const ODIM & srcOdim, const drain::image::ImageFrame & srcImage,
		const ODIM & dstOdim, drain::image::ImageFrame & dstImage) const {

	drain::Logger mout(this->name, __FUNCTION__);

	mout.debug(1) << "dst:" << dstImage << mout.endl;

	dstImage.setCoordinatePolicy(srcImage.getCoordinatePolicy());

	//const double ud = std::max(odimOut.undetect, dst.getMin<double>());
	//const double nd = std::min(odimOut.nodata, dst.getMax<double>());

	//mout.debug(2) << "input name: " << src.getName() << mout.endl;

	mout.debug(1) << "src odim: " << EncodingODIM(srcOdim) << mout.endl;
	mout.debug(4) << "src props:" << srcImage.properties << mout.endl;
	//std::cerr << src.properties << std::endl;


	dstImage.properties = srcImage.properties;
	dstImage.properties.updateFromMap(dstOdim);
	//dst.odim.set(odim);
	//mout.debug(1) << "op  odim: " << EncodingODIM(odim) << mout.endl;
	mout.debug(1) << "dst odim: " << EncodingODIM(dstOdim) << mout.endl;
	mout.debug(1) << "dst props: " << dstImage.properties << mout.endl;
	//std::cerr << dst.properties << std::endl;

	//const drain::LinearScaling scaling(srcOdim.gain, srcOdim.offset, odim.gain, odim.offset);
	const drain::LinearScaling scaling(srcOdim.gain, srcOdim.offset, dstOdim.gain, dstOdim.offset);

	typedef drain::typeLimiter<double> Limiter;
	Limiter::value_t limit = drain::Type::call<Limiter>(dstOdim.type);


	mout.debug(1) << "scaling: " << scaling << mout.endl;

	Image::const_iterator s = srcImage.begin();
	Image::iterator d = dstImage.begin();

	// Long int check by wrting to pixel at (0,0)
	*d = dstOdim.nodata;
	if (static_cast<double>(*d) != dstOdim.nodata){
		mout.note() << "dstOdim.nodata=" << dstOdim.nodata << " vs. written: " << *d << mout.endl;
		mout.warn() << "dstOdim.nodata type conversion to " << dstImage.getType2() << " changed the value" << mout.endl;
	}
	//mout.debug() << "dstOdim nodata long-int check " << dstOdim.nodata << " <> " << (long int)(*d = dstOdim.nodata) << mout.endl;


	mout.debug(2) << "src:    " << srcImage << mout.endl;
	mout.debug(2) << "dst: " << dstImage << mout.endl;
	double x;
	while (s != srcImage.end()){
		x = *s;

		/// Checks 'undetect' first because 'undetect' and 'nodata' may be the same code
		if (x == srcOdim.undetect)
			*d = dstOdim.undetect;
		else if (x == srcOdim.nodata)
			*d = dstOdim.nodata;
		else {
			//  x = srcOdim.scaleForward(x);
			//  x = dst.odim.scaleInverse(x);
			// *d = dstImage.limit<double>( x );
			*d = limit( scaling.forward(x) );
			//dstImage.scaling.limit<double>( scaling.forward(x) );
		}

		++s;
		++d;
	}

	mout.debug(2) << "finished." << mout.endl;

}



}

#endif /* DATACONVERSIONOP_H_ */

// Rack
