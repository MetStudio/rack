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
#ifndef DRAIN_TIFF_H_EXPERIMENTAL
#define DRAIN_TIFF_H_EXPERIMENTAL

#include "drain/util/FileInfo.h"


#include "drain/util/Dictionary.h"
#include "drain/util/Log.h"
#include "drain/util/Time.h"
#include "GeoFrame.h"
#include "Image.h"

#ifdef USE_GEOTIFF_YES

#include <xtiffio.h>
#endif

namespace drain::image
{



	/// For writing images in basic TIFF format. Reading not supported currently.
	/**
	 */
	class FileTIFF : public drain::FileHandler
	{
	public:


		static
		const drain::FileInfo fileInfo;

		static
		const drain::Dictionary2<int, std::string> & getCompressionDict();

		static drain::Frame2D<int> defaultTile;
		// https://www.awaresystems.be/imaging/tiff/tifftags/compression.html
		static int defaultCompression; // COMPRESSION_NONE = 1; COMPRESSION_LZW = 5;

#ifdef USE_GEOTIFF_YES


		FileTIFF(const std::string & path = "", const char *mode = "w") : tif(nullptr){
			if (!path.empty())
				open(path, mode);
			//tif = XTIFFOpen(path.c_str(), mode);
		}

		inline
		~FileTIFF(){
			close();
		}

		inline
		virtual
		void open(const std::string & path, const char *mode = "w"){
			tif = XTIFFOpen(path.c_str(), mode);
		}

		inline
		virtual
		void close(){
			if (isOpen()){
				drain::Logger mout(__FILE__, __FUNCTION__);
				mout.experimental("Closing TIFF...");
				XTIFFClose(tif);
				tif = nullptr;
			}
		}

		inline virtual
		bool isOpen() const {
			return (tif != nullptr);
		}


	protected:

		TIFF *tif;

		void writeImageData(const drain::image::Image & src);

		static
		drain::Dictionary2<int, std::string> compressionDict;

		// IF TIFF
		void setTileSize(int tileWidth=0, int tileHeight = 0);

		inline
		int setField(int tag, const std::string & value){
			return TIFFSetField(tif, tag, value.c_str());
		}

		template <class T>
		inline
		int setField(int tag, const std::vector<T> & value){
			return TIFFSetField(tif, tag, value.size(), &value.at(0));
		}

		template <class T>
		inline
		int setField(int tag, T value){
			return TIFFSetField(tif, tag, value);
		}

		/**
		 */
		void setTime(const drain::Time & time);

		/**
		 *
		 */
		void setUpTIFFDirectory(const drain::image::ImageConf & src); //, int tileWidth=0, int tileHeight = 0);


#endif

	};

} // drain::image



#endif
