/**

    Copyright 2006 - 2014 Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)

    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU Lesser Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

    Created on: Nov 17, 2014
    Author: mpeura
 */

#pragma once
#ifndef RACK_RESOURCES_H_
#define RACK_RESOURCES_H_

#include <data/ODIM.h>
//#include <drain/image/Coordinates.h>
//#include <drain/image/PaletteOp.h>
//#include <drain/prog/CommandPack.h>
//#include <drain/prog/Commands-ImageTools.h>
//#include <drain/util/Log.h>
//#include <drain/util/ReferenceMap.h>
#include <hi5/Hi5.h>
#include <drain/image/Coordinates.h>
#include <drain/image/Image.h>
#include <image/Sampler.h>
#include <imageops/PaletteOp.h>
#include <imageops/ImageModifierPack.h>
#include <prog/CommandPack.h>
#include <prog/CommandRegistry.h>

#include <radar/Composite.h>
#include <radar/RadarAccumulator.h>
#include <util/Rectangle.h>
#include <util/RegExp.h>
#include <util/Tree.h>
//#include "product/CumulativeProductOp.h"
//#include "product/VolumeOp.h"
//#include <map>
#include <string>


namespace rack {


/// A container for shared resources applied by RackLets.
/**
 *
 */
class RackResources {

public:

	RackResources(); // : inputOk(true), dataOk(true), currentHi5(&inputHi5), currentPolarHi5(&inputHi5), currentImage(NULL), currentGrayImage(NULL) {};

	// STATUS FLAGS
	// TODO: OK=0,WARNING=1,ERROR=2,FATAL=3
	// One at time: 8-2 = 6 bits = 64 events
	// INPUT=0
	// OUTPUT=4
	// DATA=8
	// PRODUCT=12
	// PRODUCT=16
	// All simultaneously: 8 = 2+2+2+2 bits => 4 events


	/// True, if the last input file operation has been successful. Helps in skipping operations for null data.
	bool inputOk;

	/// True, if the last retrieved data was found (and non-empty?). Helps in skipping operations for null data.
	bool dataOk;


	/// Clears dst if source command varies.
	void setSource(HI5TREE & dst, const drain::Command & cmd);


	/// The last input file read, typically a volume. May be concatenated ie. read in incrementally.
	HI5TREE inputHi5;
	// SOURCE inputHi5src;

	/// The polar product that has been generated from the volume.
	HI5TREE polarHi5;
	/// Flag for marking the origin of polarHi5
	//SOURCE  polarHi5src;

	/// A single-radar Cartesian product or a multi-radar composite (mosaic).
	HI5TREE cartesianHi5;
	/// Flag for marking the origin of cartesianHi5
	//SOURCE  cartesianHi5src;

	/// Pointer to the last HDF5 structure read or generated.
	HI5TREE *currentHi5; // = &inputHi5;

	/// Pointer to the last HDF5 structure in polar coordinates: input volume or polar product.
	/**
	 *   The data pointed to will be applied as input to a Cartesian product (single-radar or composite).
	 */
	HI5TREE *currentPolarHi5; // = &inputHi5;

	/// Standard (?) orientation of polar coords in radar data
	static
	const CoordinatePolicy polarLeft;

	/// Default coordinate policy; no wrapping or mirroring at edges.
	static
	const CoordinatePolicy limit;

	//static bool inputOk;

	//static 	void updateCoordinatePolicy(HI5TREE & src, const CoordinatePolicy & policy = CoordinatePolicy(CoordinatePolicy::LIMIT));

	drain::VariableMap & getUpdatedStatusMap();

	/// Retrieves image that matches a given selector.
	/*
	 *  \return true, if non-empty image was found
	 */
	bool setCurrentImage(const DataSelector & imageSelector);

	drain::image::Image grayImage;
	drain::image::Image colorImage;

	const drain::image::Image *currentImage; // = NULL;
	const drain::image::Image *currentGrayImage; // = NULL;  // data or grayImage
	drain::image::Palette palette;  // Todo: drainize


	// Accumulator for data in polar coordinates
	RadarAccumulator<Accumulator,PolarODIM> polarAccumulator;

	// Accumulator for data in Cartesian coordinates
	Composite composite;

	drain::Rectangle<double> bbox;
	std::string projStr;

	/// Global values accessible more or less directly through commands.

	//  FILE I/O
	int inputSelect; // see fileio.cpp and hi5::Reader::ATTRIBUTES|hi5::Reader::DATASETS

	/// Path prefix for input files.
	std::string inputPrefix;

	/// Path prefix for output files.
	std::string outputPrefix;

	std::string targetEncoding;

	std::string select;
	std::string andreSelect;


	drain::ScriptParser scriptParser;
	drain::ScriptExec   scriptExec;

	drain::image::ImageSampler sampler; // could be in ImageModPack?

};

/// Default instance
RackResources & getResources();



/// Adapter for commands designed for Rack.
/**
 *    \tparam T - class derived from Command
 */
template <class T>
class RackLetAdapter : public T {

public:


	RackLetAdapter(const std::string & key = "", char alias = 0){ // : Command("cart", name, alias) {
		add(key, alias);
	};

	RackLetAdapter(const std::string & key, char alias, const T & value) : T(value) { // : Command("cart", name, alias) {
		add(key, alias);
	};

	RackLetAdapter(char alias){
		add("", alias);
	}

	void add(const std::string & key, char alias){

		const std::string & k = key.empty() ? T::getName() : key;

		static RegExp nameCutter("^(Cmd|Cartesian|Composite|Polar)(.*)$");

		if (nameCutter.execute(k) == 0){ // matches
			//std::cerr << "adding () " << nameCutter.result[2] << std::endl;
			getRegistry().add(*this, nameCutter.result[2], alias);
		}
		else {
			getRegistry().add(*this, k, alias);
			//std::cerr << "adding    " << k << std::endl;
		}

	}
	/*
	const std::string & getName() const {
		return T::getName();
	}

	const std::string & getDescription() const {
		return T::getDescription();
	}
	*/


};




} /* namespace rack */

#endif /* RACKLET_H_ */
