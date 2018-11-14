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

#ifndef DATASELECTOR_H_
#define DATASELECTOR_H_

#include <set>
#include <list>
#include <map>

#include <drain/util/BeanLike.h>

#include <drain/util/ReferenceMap.h>
#include <drain/util/RegExp.h>

#include <drain/util/Variable.h>
#include "ODIM.h"
#include "PolarODIM.h" // elangle

namespace rack {


template <class T>
class Range  {

public:

	Range() : vect(2), min(vect[0]), max(vect[1]) {
	}

	std::vector<T> vect;
	T & min;
	T & max;

	bool isInside(T x) const {
		return (min <= x) && (x <= max);
	}

};



template <class T>
std::ostream & operator<<(std::ostream & ostr, const Range<T> & r){
	ostr << r.min << ':' << r.max;
	return ostr;
}

/// Tool for selecting datasets based on paths, quantities and min/max elevations.
/**
 *    Future version will use:
 *    - data=
 *    - dataset=
 *
 *    Applies drain::RegExp in matching.
 */
class DataSelector : public drain::BeanLike {
public:


	DataSelector(const std::string & path, const std::string & quantity,
			unsigned int index=0, unsigned int count = 1000,
			double elangleMin = -90.0, double elangleMax = 90.0);

	DataSelector(const std::string & parameters = "");

	DataSelector(const DataSelector & selector);

	virtual ~DataSelector();

	/// Regular expression of accepted paths, for example ".*/data$".
	std::string path;

	// Path criteria
	Range<int> dataset;
	Range<int> data;
	// This was under consideration
	// mutable drain::RegExp pathRegExp;


	/// Regular expression of accepted PolarODIM what::quantity, for example "DBZ.?" .
	std::string quantity;
	//NEW /? std::string quantityStr;

	/// The (minimum) index of the key in the list of matching keys.
	unsigned int index;

	/// The maximum length of the list of matching keys.
	unsigned int count;

	/// The minimum and maximum elevation angle (applicable with volume scan data only).
	Range<double> elangle;
	// OLD std::vector<double> elangle;
	bool isValidPath(const ODIMPath & path) const;

	// Data criteria

	/// Check if metadata matches.
	bool isValidData(const drain::ReferenceMap & properties) const ;

	/// Restore default values.
	void reset();

	/// Select dataset/data paths using current selection criteria.
	/**
	 *   Selects paths down to \c dataset and \c data group level. Uses metadata of \c what , \c where and \c how but
	 *   does not include them in the result (container).
	 *
	 *   Note: each retrieved path starts with root element (BaseODIM::ROOT), corresponding to empty string ("").
	 *
	 *   \tparam - container class supporting push_(), e.g. std::set, std::list or std::vector.
	 *   \param src - the data structure searched for paths
	 *   \param container - container for found paths
	 *   \param dataSetsOnly - switch for collecting first level groups (\c dataset 's) only
	 *
	 */
	template <class T>
	void getPathsNEW(const HI5TREE & src, T & container, bool dataSetsOnly = false) const; // consider: bool dataSetsOnly = false ?


	/// Temporary fix: try to derive dataset and data indices from path regexp.
	/**
	 *   Variable 'path' will be probably obsolete in future.
	 */
	void updatePaths();

	/// Sets this selector similar to given selector.
	// void copy(const DataSelector & selector);

	/// Sets parameters in predefined order or sets specified parameters. (Python style calling alternatives.)
	//  *   - \c index - integer value, only changing the \c index member. ???
	/**
	 *  \param parameters - the parameters to be changed, using some of the syntaxes:
	 *   - \c path,quantity,index,count>,elangleMin,elangleMax (complete or partial list of parameter values, in this order)
	 *   - \c parameter1=value1,parameter2=value2,parameter3=value3   (specific assignments as a list)
	 *   The regular expressions should not contain comma (,).
	 *   \param specific - if true, the specific assignments are supported
	 */
	//void setParameters(const std::string &parameters, bool specific = true){ parameters.copy(p, specific); };
	// DataSelector & setParameters(const std::string &parameters, bool allowSpecific = true);

	/// Traverses down the tree and returns matching paths as a list or map (ordered by elevation).
	/**
	 *   Applicable for list\<string\> and std::map<double,std::string>.
	 */
	template <class T>
	static
	void getPaths(const HI5TREE & src, T & container, const std::string & path="", const std::string & quantity="",
			unsigned int index=0, unsigned int count = 1000,
			double elangleMin = -90.0, double elangleMax = 180.0);


	/// Traverses down the tree and returns matching paths as a list.
	/**
	 *   \param container - std::list<std::string> and std::map<double,std::string>
	 *
	 *   In the order of appearance in the hdf5 structure.
	 */
	//template <class T>
	static
	inline
	void getPaths(const HI5TREE &src, const DataSelector & selector, std::list<std::string> & l){
		getPathsT(src, selector, l);
	}

	/// Traverses down the tree and returns matching paths as a list.
	/**
	 *   \param container - std::set<std::string>
	 *
	 *   In the order of appearance in the hdf5 structure.
	 */
	static
	inline
	void getPaths(const HI5TREE &src, const DataSelector & selector, std::set<std::string> & s){
		getPathsT(src, selector, s);
	}

	/// Traverses down the tree and returns matching paths as a map with elevations as keys.
	/**
	 *   \param m
	 */
	//template <class T>
	static
	inline
	void getPathsByElevation(const HI5TREE &src, const DataSelector & selector, std::map<double,std::string> & m){
		getPathsT(src, selector, m);
	}


	static
	inline
	void getPathsByQuantity(const HI5TREE &src, const DataSelector & selector, std::map<std::string,std::string> & m){
		getPathsT(src, selector, m);
	}


	/// Returns data paths, mapped by elevation.
	//static 	void getPaths(const HI5TREE &src, std::map<double,std::string> & path);

	/// Convenience function. Often only o ne path is return in the list, or only the first path is needed.
	static
	inline
	bool getPath(const HI5TREE &src, const DataSelector & selector, std::string & path){
		std::list<std::string> l;
		getPaths(src, selector, l);
		if (l.empty()){
			path.clear();
			return false;
		}
		else {
			path = *l.begin();
			return true;
		}
	}



	// TODO ? bool getLastOrdinalPath(const HI5TREE &src, const DataSelector & selector, std::string & path, int & index){

	/// Finds the path that is numerically greatest with respect to the last numeric
	/*
	 *
	 *  \return - true, if a path was found with the given selector.
	 */
	static
	bool getLastOrdinalPath(const HI5TREE &src, const DataSelector & selector, std::string & basePath, int & index);

	/// A shortcut.
	/*
	 */
	static
	bool getLastOrdinalPath(const HI5TREE &src, const std::string & pathRegexp, std::string & path);


	/// Detect the last path accepted by regexp and increment the trailing numeral (eg. data2 => data3) and return the std::string in \c path.
	/**
	 * \param src - hdf tree to be searched
	 * \param pathRegExp - parent path(s) defined as regular expressions, for example:
	 * - \c /dataset[0-9]$
	 * - \c /dataset2/data[0-9]$
	 * - \c data[0-9]$
	 * - \c data2/quality[0-9]$
	 * \param path - std::string in which the result is stored, if found.
	 * \return - true, if a path was found
     *
     *  Notice that \c path can be initialized with a default value; if no path is found, the default value will stay intact.
	 */
	static
	inline
	bool getNextOrdinalPath(const HI5TREE &src, const std::string & pathRegExp, std::string & path){
		DataSelector s;
		s.path = pathRegExp;
		return getNextOrdinalPath(src, s, path);
	}

	/// A shortcut
	/*
	 *
	 */
	static
	bool getNextOrdinalPath(const HI5TREE &src, const DataSelector & selector, std::string & path);



	static
	inline
	const hi5::NodeHi5 & getNode(const HI5TREE & src, const DataSelector & selector){
		std::string path;
		if (getPath(src, selector, path))
			return src(path).data;
		else {
			static const hi5::NodeHi5 _empty;
			return _empty;
		}
	}

	static
	inline
	hi5::NodeHi5 & getNode(HI5TREE &src, const DataSelector & selector){
		std::string path;
		if (getPath(src, selector, path))
			return src(path).data;
		else {
			static hi5::NodeHi5 _empty;
			return _empty;
		}
	}


	static
	inline
	const drain::image::Image & getData(const HI5TREE &src, const DataSelector & selector){
		return getNode(src, selector).dataSet;
	}

	static
	inline
	drain::image::Image & getData(HI5TREE &src, const DataSelector & selector){
		return getNode(src, selector).dataSet;
	}

	/// Given a path to data, tries to find the respective "quality1" field. Currently, does not check quantity.
	/**
	 *  \param dataPath - start address, to be ascended
	 *  \param qualityPaths  - container for the result(s);  std::string or std::list<std::string>
	 *  \return - true, if at least one path has been found
	 */
	template <class T>
	static
	bool getQualityPaths(const HI5TREE &srcRoot, const std::string & datapath, T & qualityPaths);

	/// Returns the path to associated quality data.
	/**
	    \return empty string, if no quality data found.
	 */
	static
	inline
	bool getQualityPath(HI5TREE &srcGroup, std::string & path){
		std::string qualityPath;
		if (getQualityPaths(srcGroup, path, qualityPath)){
			path = qualityPath;
			return true;
		}
		else {
			path.clear();
			return false;
		}
	}


	static
	inline
	const drain::image::Image & getQualityData(const HI5TREE &srcGroup, const std::string & dataPath){
		static const drain::image::Image _empty;
		//std::string qualityPath;
		std::list<std::string> l;
		getQualityPaths(srcGroup, dataPath, l);
		//if (getQualityPath(srcGroup, dataPath, qualityPath))
		if (l.size() > 0)
			return srcGroup[*l.begin()].data.dataSet;
		else
			return _empty;
	}

	static
	inline
	drain::image::Image & getQualityData(HI5TREE &srcGroup, const std::string & dataPath){
		static drain::image::Image _empty;
		std::string qualityPath;
		if (getQualityPaths(srcGroup, dataPath, qualityPath))
			return srcGroup[qualityPath].data.dataSet;
		else
			return _empty;
	}


protected:

	//drain::ReferenceMap _parameters;

	/// Sets the default values.
	void init();

	/// Traverses down the tree and returns matching paths as a list or map (ordered by elevation).
	/**
	 *   Applicable for std::list<std::string> and std::map<double,std::string>.
	 *   \param container - std::list<std::string> and std::map<double,std::string>
	 */
	template <class T>
	static
	inline
	void getPathsT(const HI5TREE &src, const DataSelector & selector, T & container){
		getPaths(src, container, selector.path, selector.quantity,
				selector.index, selector.count,
				selector.elangle.min, selector.elangle.max); // min, max
	}



	/*
	template <class F, class T>
	inline
	void reference(const std::string & key, F & target, const T & value){
		parameters.reference(key, target, value);
	}
	*/

	/*
	template <class F>
	inline
	void reference(const std::string & key, F & target){
		parameters.reference(key, target);
	}
	*/

	/// NEW Collects paths to a list.
	template <class P>
	static
	void addPathT(std::list<P> & l, const PolarODIM & odim, const P & path){ l.push_back(path); }


	/// Collects paths to a list.
	/*
	static
	inline
	void addPathT(std::list<std::string> & l, const PolarODIM & odim, const std::string &path){ l.push_back(path); }; // discards elevation
	*/

	/// Collects paths to a set. (unused?)
	template <class P>
	static
	void addPathT(std::set<P> & l, const PolarODIM & odim, const P & path){ l.insert(path); }
	/*
	static
	inline
	void addPathT(std::set<std::string> & s, const PolarODIM & odim, const std::string &path){ s.insert(path); }; // discards elevation
	*/

	/// Collects paths by their elevation angle (elangle).
	static
	inline
	void addPathT(std::map<double,std::string> & m, const PolarODIM & odim, const std::string & path){ m[odim.elangle] = path; };

	/// Collects paths by their quantity, eg. DBZH, VRAD, and RHOHV.
	static
	inline
	void addPathT(std::map<std::string,std::string> & m, const PolarODIM & odim, const std::string & path){ m[odim.quantity] = path; };


	static
	inline
	bool addPathT(std::list<std::string> & l, const std::string &path){ l.push_back(path); return false; };  // ??


	/// Add a std::string. Search ends with the first entry encountered.
	static
	inline
	bool addPathT(std::string & str, const std::string &path){ str.assign(path); return true; };  // ??

};

/// <= Note: each path starts with root element BaseODIM::ROOT, corresponding to empty string ("").
template <class T>
void DataSelector::getPathsNEW(const HI5TREE &src, T & container, bool dataSetsOnly) const {

	drain::Logger mout(getName(), __FUNCTION__);

	PolarODIM odim;

	const drain::RegExp quantityRE(quantity);
	std::set<ODIMPathElem> stems;
	unsigned int counter = 0; // (this was needed as count would go -1 otherways below)

	// Step 1: retrieve a temporary list of all paths
	std::list<ODIMPath> l0;
	mout.debug() << "getKeys " << path << mout.endl;
	src.getKeys(l0);

	// Accept no data[n]
	const bool DATASETS = dataSetsOnly || (data.max == 0);
	if (DATASETS)
		mout.debug(1) << "datasets only" << mout.endl;

	// Step 2: select only the paths matching the criteria
	for (std::list<ODIMPath>::iterator it = l0.begin(); it != l0.end(); ++it) {

		//mout.note() << *it << mout.endl;

		if (it->size() == 1) // only the stem ("") ???
			continue;

		const ODIMPathElem & stem = *(++(it->begin()));

		mout.debug(2) << ": " << stem << "-> " << *it  << mout.endl;

		if (stem.is(BaseODIM::DATASET)){
			if (!dataset.isInside(stem.index)){
				mout.debug() << "skip dataset " << stem.index << ", not in [" <<  dataset << ']' << mout.endl;
				continue;
			}
		}

		const ODIMPathElem & leaf = it->back();

		if (leaf.is(BaseODIM::DATA)){ // what about quality?
			if (!DATASETS){
				if (!data.isInside(leaf.index)){
					mout.warn() << "data " << leaf.index << " not in [" <<  data << ']' << mout.endl;
					continue;
				}
			}
		}
		else if (!leaf.isIndexed()) { //(!leaf.is(BaseODIM::DATASET)){ //(!leaf.isIndexed()) {
			// Skip WHAT, WHERE, HOW, ARRAY
			mout.debug() << " skipping " << leaf << mout.endl;
			continue;
		}


		//mout.debug(2) << *it << mout.endl;
		const hi5::NodeHi5 & node = src(*it).data;
		if (node.noSave){
			mout.debug() << "noSave data, ok: " << *it << mout.endl;
			//continue;
		}

		const drain::image::Image & d = node.dataSet;
		//odim.clear();
		//odim.copyFrom(d);  // OK, uses true type ie. full precision, also handles img type

		if (!quantityRE.test(d.properties["what:quantity"])){
			//if (!quantityRE.test(odim.quantity)){
			mout.debug() << *it << "\n\t quantity '" << quantityRE.toStr() << "' !~ '" << d.properties["what:quantity"] << "'" << mout.endl;
			continue;
		}

		if (d.properties.hasKey("where:elangle")){
			if (!elangle.isInside(d.properties["where:elangle"])){
				mout.debug() << "outside elangle range"<< mout.endl;
				continue;
			}
		}

		// Outside index check, because mostly applied by count check as well.
		mout.debug() << "considering " << *it << mout.endl;

		// Update count
		if (stems.find(stem) == stems.end()){ // = not already in the set
			++counter;
			if (counter > count)
				return;
			stems.insert(stem);
			if (DATASETS){ // Skip adding others than first
				odim.clear();
				odim.copyFrom(d);  // OK, uses true type ie. full precision, also handles img type
				mout.debug() << "ACCEPT, counter(" << counter << "): " << *it << mout.endl;
				ODIMPath p; // consider ODIMPath(ODIMPathElement(ROOT))?
				p << ODIMPathElem(BaseODIM::ROOT) << stem;
				addPathT(container, odim, p);
			}
		}

		if (DATASETS){ // Skip adding others than first
			continue;
		}


		/// Skip non-datasets, if datasets requested, and vice versa
		if (!leaf.is(BaseODIM::DATA)){
			mout.warn() << "unexpected path: " << *it << mout.endl;
		}

		odim.clear();
		odim.copyFrom(d);  // OK, uses true type ie. full precision, also handles img type

		mout.debug() << "ACCEPT, counter(" << counter << "): " << *it << mout.endl;
		//container.push_back(*it);
		addPathT(container, odim, *it);

		//


	}

}



template <class T>
void DataSelector::getPaths(const HI5TREE &src, T & container, const std::string & path, const std::string &quantity,
		unsigned int index, unsigned int count, double elangleMin, double elangleMax) {

	drain::Logger mout("DataSelector", __FUNCTION__);

	//const drain::RegExp quantityRE(std::string("^")+quantity+std::string("$"));
	const drain::RegExp quantityRE(quantity);

	std::list<std::string> l0;

	/// Step 1: get paths that match pathRegExp TreeT
	mout.debug(10) << "getKeys " << path << mout.endl;
	src.getKeys(l0, path);
	//src.getKeys(l0, pathRegExp);

	std::set<std::string> roots;
	unsigned int counter = 0; // (this was needed as count would go -1 otherways below)

	/// Step 2: add paths that match path and quantity regexps and are with elevation limits.
	PolarODIM odim;
	for (std::list<std::string>::iterator it = l0.begin(); it != l0.end(); ++it) {

		//mout.debug(2) << *it << mout.endl;
		const hi5::NodeHi5 & node = src(*it).data;
		if (node.noSave){
			mout.debug() << "noSave data, ok: " << *it << mout.endl;
			//continue;
		}

		const drain::image::Image & d = node.dataSet;
		odim.clear();
		odim.copyFrom(d);  // OK, uses true type ie. full precision, also handles img type
		// odim.set() would be bad! Looses precision in RefMap/Castable << (std::string) << Variable

		if (!quantityRE.test(odim.quantity)){
			mout.debug(8) << *it << "\n\t quantity '" << quantityRE.toStr() << "' !~ '" << odim.quantity << "'" << mout.endl;
			// l.erase(it2);
			continue;
		}

		if ((odim.elangle < elangleMin) || (odim.elangle > elangleMax))
		{
			mout.debug(8) << "elangle inapplicable " << odim.elangle << mout.endl;
			// l.erase(it2);
			continue;
		}

		// Outside index check, because mostly applied by count check as well.
		const std::string root = it->substr(0, it->find('/', 1));  // typically dataset1/

		if (index > 0){ // "Log mode on"

			// mout.warn() << "index studying " << *it << '\t' << root << mout.endl;
			if (roots.find(root) == roots.end()){ // = new
				--index;
				if (index == 0){ // done!
					// mout.warn() << "index ACCEPTED " << *it << '\t' << root << mout.endl;
					roots.clear(); // re-use for count (below)
				}
				else {
					roots.insert(root);
					continue;
				}
			}
			else
				continue;
		}


		if (roots.find(root) == roots.end()){ // = not already in the set
			++counter;
			if (counter > count)
				return;
			roots.insert(root);
		}


		// mout.warn() << "counter(" << counter << ") ACCEPTED " << *it << '\t' << root << mout.endl;
		addPathT(container, odim, *it);

	}

}





template <class T>
bool DataSelector::getQualityPaths(const HI5TREE & srcRoot, const std::string & datapath, T & qualityPaths) {

	drain::Logger mout("DataSelector", __FUNCTION__);

	mout.debug(1) << datapath << mout.endl;

	//if (qpath.empty())
	//	return false;

	const size_t lStart = datapath[0]=='/' ? 1 : 0;

	size_t l = datapath.length();

	while (l > lStart) {

		const std::string parentPath = datapath.substr(lStart, l);

		mout.debug(12) << "checking path: " << parentPath << mout.endl;
		const HI5TREE & src = srcRoot(parentPath);

		/// Iterate children (for the srcRoot[datapath], actually its siblings)
		for (HI5TREE::const_iterator it = src.begin(); it != src.end(); it++){
			mout.debug(14) << "checking subpath: " << it->first << mout.endl;
			if (it->second["what"].data.attributes["quantity"].toStr() == "QIND" ){  // QIND
				mout.debug(5) << "found:" << it->first << mout.endl;
				//qualityPaths.push_back(parentPath + '/' + it->first + "/data");
				if (addPathT(qualityPaths, parentPath + '/' + it->first + "/data"))
					return true;
				// returns if std::string, continues if list
			}
		}

		l = datapath.rfind('/', l-1);
		if (l == std::string::npos)
			l = lStart;
	}

	return (qualityPaths.size() > 0);
}

inline
std::ostream & operator<<(std::ostream & ostr, const DataSelector &selector){
	ostr << selector.getParameters();
	return ostr;
}

} // rack::

#endif /* DATASELECTOR_H_ */

// Rack
