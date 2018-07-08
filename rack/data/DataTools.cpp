/*

    Copyright 2012  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <drain/util/Type.h>

#include "hi5/Hi5.h"
#include "DataTools.h"

namespace rack {

using namespace hi5;



/*
const drain::Variable & DataTools::getAttribute(const HI5TREE &src, const std::string & path, const std::string & group, const std::string & attributeName){

	const HI5TREE &g = src(path)[group];

	if (g.data.attributes.hasKey(attributeName))
		return g.data.attributes[attributeName];
	else {
		if (path.empty()){
			static drain::Variable dummy;
			return dummy;
		}
		else {
			size_t i = path.rfind('/');
			//if ((i != path.npos) && (i != 0))
			if (i != path.npos)
				return getAttribute(src, path.substr(0,i), group, attributeName);
			else
				return getAttribute(src, "", group, attributeName);
			//else
			//	return dummy;
		}
	}
}
 */

// const drain::image::CoordinatePolicy & policy,
//void DataSelector::_updateAttributes(HI5TREE & src, const drain::VariableMap & attributes){
void DataTools::updateAttributes(HI5TREE & src,  const drain::VariableMap & attributes){

	//drain::Logger mout(drain::monitor, "DataTools::updateAttributes");
	drain::Logger mout(__FILE__, __FUNCTION__);


	drain::VariableMap & a = src.data.dataSet.properties;
	// Copy
	//a.clear();
	a.importMap(attributes);

	//hi5::NodeHi5 & node = src.data;
	if (src.hasChild("data")){
		const drain::image::Image & img = src["data"].data.dataSet;
		//img.setCoordinatePolicy(policy);
		if (img.typeIsSet())
			a["what:type"] = std::string(1u, drain::Type::getTypeChar(img.getType()));
	}


	const std::set<std::string> & g = EncodingODIM::attributeGroups;

	// Update attributes, traversing WHAT, WHERE, HOW.
	std::stringstream sstr; // For speed
	for (std::set<std::string>::const_iterator git = g.begin(); git != g.end(); ++git){
		if (src.hasChild(*git)){
			const drain::VariableMap  & groupAttributes = src[*git].data.attributes;
			for(drain::VariableMap::const_iterator it = groupAttributes.begin(); it != groupAttributes.end(); it++){
				sstr.str("");
				sstr << *git << ':' << it->first;
				a[sstr.str()] = it->second;
				// if (it->first == "quantity") mout.warn() << "quantity=" << it->second << mout.endl;
			}
		}
	}

	// Traverse children (recursion)

	for (HI5TREE::iterator it = src.begin(); it != src.end(); ++it){
		if (g.find(it->first) == g.end())
			updateAttributes(it->second,  a); // policy,
	}


	// std::cerr << "### updateAttributes"
}

bool DataTools::removeIfNoSave(HI5TREE & dst){
	if (dst.data.noSave){
		drain::Logger mout("DataTools", __FUNCTION__);
		mout.note() << "// about to resetting noSave struct: " << dst.data << mout.endl;
		/*
		dst.data.attributes.clear();
		dst.data.dataSet.resetGeometry();
		dst.getChildren().clear();
		 */
		return true;
	}
	else
		return false;
}

void DataTools::updateCoordinatePolicy(HI5TREE & src, const drain::image::CoordinatePolicy & policy){

	drain::image::Image & data = src.data.dataSet;
	if (!data.isEmpty()){
		data.setCoordinatePolicy(policy);
		//data.setName(path + ':' + data.properties["what:quantity"].toStr());
		data.setName(data.properties["what:quantity"].toStr());
	}

	const std::set<std::string> & g = EncodingODIM::attributeGroups;

	for (HI5TREE::iterator it = src.begin(); it != src.end(); ++it){
		const std::string & key = it->first;
		//if ((key != "what" ) && (key != "where" ) && (key != "how" ))
		if (g.find(key) == g.end())
			updateCoordinatePolicy(it->second, policy);

	}
}



bool DataTools::getNewChild(HI5TREE & tree, ODIMPathElem & child, ODIMPathElem::index_t iMax){

	if (BaseODIM::isIndexed(child.getType())){
		std::cerr << __FILE__ << ':' << __FUNCTION__ << ": index requested for unindexed path element '" << child << "'" << std::endl;
		//throw std::runtime_error("DataTools::getNewChild indexing requested for path element ");
		return false;
	}

	const ODIMPathElem::index_t iMin = std::max(1, child.index);

	for (ODIMPathElem::index_t i = iMin; i<iMax; ++i){
		child.index = i;
		if (!tree.hasChild(child)){
			return true;
		}
	}

	return false;
}


bool DataTools::getLastChild(HI5TREE & tree, ODIMPathElem & child){ //, BaseODIM::group_t g



	if (!BaseODIM::isIndexed(child.getType())){
		std::cerr << __FILE__ << ':' << __FUNCTION__ << ": index requested for unindexed path element '" << child << "'" << std::endl;
		//throw std::runtime_error("DataTools::getNewChild indexing requested for path element ");
		return false;
	}

	ODIMPathElem e;
	child.index = 0; // needed
	for (HI5TREE::const_iterator it = tree.begin(); it != tree.end(); ++it){

		e.set(it->first);

		if (e.getType() == child.getType()){
			//std::cerr << __FUNCTION__ << " consider " << e << '\n';
			child.index = std::max(child.getIndex(), e.getIndex());
			/*if (e.getIndex() > child.getIndex()){
				child.index = e.getIndex();
			}
			*/
		}
	}

	return child.getIndex() > 0;

}

}  // rack::
