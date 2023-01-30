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
 * ProductOp.h
 *
 *  Created on: Mar 7, 2011
 *      Author: mpeura
 */

#ifndef RACKOP_H_
#define RACKOP_H_


//#include "drain/util/DataScaling.h"
//#include "drain/util/StringMapper.h"
//#include "drain/util/Variable.h"
//#include "drain/util/Tree.h"
#include "drain/util/ReferenceMap.h"



//#include "main/rack.h"
#include "hi5/Hi5.h"
#include "data/ODIM.h"
#include "data/DataSelector.h"
#include "data/Data.h"
#include "data/Quantity.h" // NEW

#include "hi5/Hi5Write.h" // debugging


#include "ProductOp.h" // NEW

namespace rack {

using namespace drain::image;

/// Base class for radar data processors.

/// Base class for radar data processors.
/** Input and output as HDF5 data, which has been converted to internal structure, drain::Tree<NodeH5>.
 *
 *  Basically, there are two kinds of polar processing
 *  - Cumulative: the volume is traversed, each sweep contributing to a single accumulation array, out of which the product layer(s) is extracted.
 *  - Sequential: each sweep generates new layer (/dataset) in the product; typically, the lowest only is applied.
 *
 *  TODO: Raise to RackOp
 */





/**
 *   \tparam M - ODIM type corresponding to products type (polar, vertical)
 */
// TODO: generalize for Cart
template <class M>
class VolumeOp : public ProductOp<const PolarODIM, M> {

public:

	VolumeOp(const std::string & name, const std::string &description="") : ProductOp<const PolarODIM, M>(name, description){
	};

	virtual inline
	~VolumeOp(){};


	/// Traverse through given volume and create new, processed data (volume or polar product).
	/**
	 *  This default implementation converts the volume to DataSetMap<PolarSrc>, creates an instance of
	 *  DataSet<DstType<M> >
	 *  and calls computeSingleProduct().
	 */
	virtual
	void processVolume(const Hi5Tree &src, Hi5Tree &dst) const;



protected:



};




template <class M>
void VolumeOp<M>::processVolume(const Hi5Tree &src, Hi5Tree &dst) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	mout.debug() << "start" << mout.endl;
	mout.debug3() << *this << mout.endl;
	mout.debug2() << "DataSelector: "  << this->dataSelector << mout.endl;

	// Step 1: collect sweeps (/datasetN/)
	//DataSetMap<PolarSrc> sweeps;
	DataSetMap<PolarSrc> sweeps;

	/// Usually, the operator does not need groups sorted by elevation.
	mout.debug3() << "collect the applicable paths"  << mout.endl;
	ODIMPathList dataPaths;  // Down to ../dataN/ level, eg. /dataset5/data4
	//this->dataSelector.getPaths(src, dataPaths, ODIMPathElem::DATASET); // RE2


	this->dataSelector.getPaths(src, dataPaths);


	if (dataPaths.empty()){
		mout.warn() << "no dataset's selected" << mout.endl;
	}
	else {
		mout.debug3() << "populate the dataset map, paths=" << dataPaths.size() << mout.endl;
	}

	for (ODIMPath & path: dataPaths){

		if (!path.front().is(ODIMPathElem::DATASET)){
			path.pop_front();
			if (path.empty()){
				mout.warn("odd 1st path elem (..), with selector: ", this->dataSelector);
				continue;
			}
		}

		const ODIMPathElem & parent = path.front();

		if (!parent.is(ODIMPathElem::DATASET)){
			mout.warn("path does not start with /dataset.. :", path, ", with selector: ", this->dataSelector);
			continue;
		}

		// mout.debug3() << "elangles (this far> "  << elangles << mout.endl;
		const Hi5Tree & srcDataSet = src(parent);

		// const double elangle = srcDataSet[ODIMPathElem::WHERE].data.attributes["elangle"];  // PATH
		// mout.deprecating("no more testing ", parent, ", elangle=", elangle, ':', srcDataSet.data.dataSet);

		const drain::VariableMap & what = srcDataSet[ODIMPathElem::WHAT].data.attributes;
		std::string datetime = what["startdate"].toStr() + what["starttime"].toStr();

		if (sweeps.find(datetime) == sweeps.end()){
			mout.debug2("adding time=", datetime, ':', parent);
			sweeps.insert(DataSetMap<PolarSrc>::value_type(datetime, DataSet<PolarSrc>(srcDataSet, drain::RegExp(this->dataSelector.quantity))));
		}
		else {
			mout.warn("datetime =", datetime, " already added?, skipping ",  parent);
		}

		/*
		if (sweeps.find(elangle) == sweeps.end()){
			mout.debug3() << "add "  << elangle << ':'  << parent << " quantity RegExp:" << this->dataSelector.quantity << mout.endl;
			sweeps.insert(DataSetMap<PolarSrc>::value_type(elangle, DataSet<PolarSrc>(srcDataSet, drain::RegExp(this->dataSelector.quantity) )));  // Something like: sweeps[elangle] = src[parent] .
			// elangles << elangle;
			//mout.warn() << "add " <<  DataSet<PolarSrc>(src(parent), drain::RegExp(this->dataSelector.quantity) ) << mout.endl;
		}
		else {
			mout.note() << "elange ="  << elangle << " already added, skipping " << parent << mout.endl;
		}
		*/
	}

	//mout.note() << "first elange =" << sweeps.begin()->first << " DS =" << sweeps.begin()->second << mout.endl;
	//mout.note() << "first qty =" << sweeps.begin()->second.begin()->first << " D =" << sweeps.begin()->second.getFirstData() << mout.endl;


	ODIMPathElem dataSetPath(ODIMPathElem::DATASET, 1);
	//if (!DataTools::getNextDescendant(dst, ProductBase::appendResults.getType(), dataSetPath))

	if (ProductBase::appendResults.is(ODIMPathElem::DATASET)){
		if (ProductBase::appendResults.getIndex()){
			dataSetPath.index = ProductBase::appendResults.getIndex();
		}
		else {
			DataSelector::getNextChild(dst, dataSetPath);
		}
	}
	else if (ProductBase::appendResults.is(ODIMPathElem::DATA)){
		//mout.info() << "appending to next available data group in " << dataSetPath <<  mout.endl;
	}
	else if (ProductBase::appendResults.is(ODIMPathElem::ROOT)){
		if (!dst.empty() && (&src != &dst)){  // latter is ANDRE test... (kludge)
			mout.info("clearing previous result, use --append [data|dataset] to avoid");
			dst.clear();
		}
	}
	else {
		dataSetPath = ProductBase::appendResults;
		mout.warn("non-standard path location '", dataSetPath, "', consider --help append ");
	}
	//++dataSetPath.index;

	//mout.warn() << "FAILED: "  << dataSetPath << mout.endl;
	//dataSetPath.push_back(ODIMPathElem(ODIMPathElem::DATASET, 1));

	mout.debug("storing product in path: ", dataSetPath);

	//Hi5Tree & dstProduct = dst[dataSetPath];

	/*   /// WARNING: Root odim has to be modified explicitly, otherwise remains empty.
	RootData<DstType<M> > root(dst);
	drain::VariableMap & whatRoot = root.getWhat();
	whatRoot["object"]  = this->odim.object;
	whatRoot["version"] = this->odim.version;
	 */

	DataSet<DstType<M> > dstProductDataset(dst[dataSetPath]); // PATH

	// Copy metadata from the input volume (note that dst may have been cleared above)
	//dstProductDataset.getWhat();
	//RootData<DstType<M> > root(dst);
	//drain::VariableMap & rootWhat = root.getWhat();
	drain::VariableMap & rootWhat = dst[ODIMPathElem::WHAT].data.attributes; // dstProduct["what"].data.attributes;
	rootWhat = src[ODIMPathElem::WHAT].data.attributes;
	rootWhat["object"]  = this->odim.object; // ?
	rootWhat["version"] = this->odim.version;

	// drain::VariableMap & where = dstProductDataset.getWhere(); // dstProduct["what"].data.attributes;
	// drain::VariableMap & rootWhere = dst[ODIMPathElem::WHERE].data.attributes;
	drain::VariableMap & rootWhere = dst[ODIMPathElem::WHERE].data.attributes; //root.getWhere();
	//where["init0"] = {0.1, 2.2};
	rootWhere = src[ODIMPathElem::WHERE].data.attributes;
	//mout.warn() << where << mout;
	//rootWhere.importCastableMap(src[ODIMPathElem::WHERE].data.attributes);
	//where.importMap(src[ODIMPathElem::WHERE].data.attributes);
	//where.importCastableMap(src[ODIMPathElem::WHERE].data.attributes);
	//where = src[ODIMPathElem::WHERE].data.attributes;
	mout.debug2("src /where/ : ", src[ODIMPathElem::WHERE].data.attributes);
	mout.debug2("dst /where/ : ", rootWhere);

	drain::VariableMap & how = dstProductDataset.getHow(); //dstProduct["how"].data.attributes;
	how = src[ODIMPathElem::HOW].data.attributes;
	ProductBase::setODIMsoftwareVersion(how);

	// how["elangles"] = elangles;  // This service could be lower in hierarchy (but for PseudoRHI and pCappi ok here)
	// how["anglesV"]   = elangles;  // NEW 2021


	// odim.copyToRoot(dst); NO! Mainly overwrites original data. fgrep 'declare(rootAttribute' odim/*.cpp

	/// MAIN
	this->computeSingleProduct(sweeps, dstProductDataset);
	// this->processSweeps(sweeps, dstProductDataset);

	// mout.warn() << "MAIN eka: " << drain::sprinter(dstProductDataset.getFirstData().odim) << mout;

	if (!dstProductDataset.empty()){
		/// Todo: how to handle undefined
		how["angles"] = dstProductDataset.getFirstData().odim.angles;
		//how["anglesXX"] = dstProductDataset.getFirstData().odim.angles;
	}
	//mout.warn() << "MAIN toka:" << drain::sprinter(dstProductDataset.getFirstData().odim) << mout;
	//mout.warn("how how", how);
	// mout.experimental("dst2", rootWhere);
	// hi5::Writer::writeFile("test0.h5", dst);

	//}

	//hi5::Writer::writeFile("test0b.h5", dst);


}


}  // namespace rack


#endif
