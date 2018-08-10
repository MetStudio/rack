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

   Based on the example code by Niles D. Ritter,
   http://svn.osgeo.org/metacrs/geotiff/trunk/libgeotiff/bin/makegeo.c

 */

#include <drain/util/Log.h>
#include <drain/util/ProjectionFrame.h>
#include <drain/image/AccumulatorGeo.h>

#include <drain/image/File.h>
//#include <gdal/gdal.h>

#include "main/rack.h"
#include "FileGeoTIFF.h"


namespace rack
{

int FileGeoTIFF::tileWidth(256);
int FileGeoTIFF::tileHeight(256);

}






#ifndef GEOTIFF_NO //  geotiff //RACKGEOTIFF
//#ifdef GEOTIFF_USE //  geotiff //RACKGEOTIFF

#include <proj_api.h>


#include <geotiff.h>
#include <geotiffio.h>
#include <xtiffio.h>
#include <geo_normalize.h>


#ifndef TIFFTAG_GDAL_NODATA //# ASCII tag (code 42113
#define TIFFTAG_GDAL_NODATA 42113 // 0xa481 // 42113
#endif

namespace rack
{





// // using namespace std;
using namespace drain;

//drain::Variable FileGeoTIFF::ties(typeid(double));


void SetUpTIFFDirectory(TIFF *tif, const drain::image::Image & src, int tileWidth=0, int tileHeight = 0){

	Logger mout("FileGeoTIFF", __FUNCTION__);

	const drain::VariableMap & prop = src.properties;

	const size_t width  = src.getWidth();
	const size_t height = src.getHeight();

	TIFFSetField(tif,TIFFTAG_IMAGEWIDTH,    width);
	TIFFSetField(tif,TIFFTAG_IMAGELENGTH,   height);
	TIFFSetField(tif,TIFFTAG_COMPRESSION,   COMPRESSION_NONE);
	TIFFSetField(tif,TIFFTAG_PHOTOMETRIC,   PHOTOMETRIC_MINISBLACK);
	TIFFSetField(tif,TIFFTAG_PLANARCONFIG,  PLANARCONFIG_CONTIG);

	const drain::Type t(src.getType());
	mout.debug() << " bytes=" << Type::call<drain::sizeGetter>(t) << mout.endl;
	switch ((const char)t) {
		case 'C':
			// no break
		case 'S':
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8*Type::call<drain::sizeGetter>(t));
			break;
		default:
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
			mout.warn() << "unsupported storage type=" <<  src.getType2() << ", trying 8 bit mode"<< mout.endl;
	}

	// write as tiles
	if (tileWidth > 0){

		if (tileHeight <= 0)
			tileHeight = tileWidth;

		if (TIFFSetField(tif, TIFFTAG_TILEWIDTH,  tileWidth)==0){
			mout.warn() << "invalid tileWidth=" << tileWidth << ", using 256"<< mout.endl;
			TIFFSetField(tif, TIFFTAG_TILEWIDTH, 256);
		}

		if (TIFFSetField(tif,TIFFTAG_TILELENGTH,  tileHeight)==0){
			mout.warn() << "invalid tileWidth=" << tileHeight << ", using 256"<< mout.endl;
			TIFFSetField(tif, TIFFTAG_TILELENGTH, 256);
		}

	}
	else {
		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,  20L);
	}

	// http://www.gdal.org/frmt_gtiff.html
	// Optional
	const std::string software = std::string(__RACK__) + " " + std::string(__RACK_VERSION__);
	TIFFSetField(tif,TIFFTAG_SOFTWARE, software.c_str());

	const std::string datetime = prop.get("what:date", "") + prop.get("what:time", "");
	TIFFSetField(tif, TIFFTAG_DATETIME, datetime.c_str() );

	const std::string desc = prop.get("what:object", "") + ":"+ prop.get("what:product", "") + ":" + prop.get("what:prodpar", "") + ":" + prop.get("what:quantity", "");
	TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, desc.c_str());


	// usr/include/gdal/rawdataset.h
	// Non-standard http://www.gdal.org/frmt_gtiff.html
	std::string nodata = prop["what:nodata"];
	if (!nodata.empty()){
		// http://stackoverflow.com/questions/24059421/adding-custom-tags-to-a-tiff-file
		static const TIFFFieldInfo xtiffFieldInfo[] = {
				{ TIFFTAG_GDAL_NODATA, 1, 1, TIFF_ASCII,  FIELD_CUSTOM, 0, 0, const_cast<char*>("nodata-marker") },
		};
		TIFFMergeFieldInfo(tif, xtiffFieldInfo, 1);
		mout.info() << "registering what:nodata => nodata=" << nodata << mout.endl;
		TIFFSetField(tif, TIFFTAG_GDAL_NODATA, nodata.c_str());
	}


	std::string projdef = prop["where:projdef"];
	if (projdef.empty()){
		mout.note() << "where:projdef missing, no TIFF tags written" << mout.endl;
		return;
	}

	const Rectangle<double> bboxD(prop["where:LL_lon"], prop["where:LL_lat"], prop["where:UR_lon"], prop["where:UR_lat"]);

	drain::image::GeoFrame frame;
	//frame.
	frame.setGeometry(width, height);
	frame.setProjection(projdef);
	frame.setBoundingBoxD(bboxD);
	//mout.debug() << "prjSrc: " << frame.getProjection() << mout.endl;
	//const drain::Rectangle<double> & bboxM = frame.getBoundingBoxM();
	//mout.warn() << "BBox: "  << bboxM << mout.endl;

	double tiepoints[6] = {0,0,0,0,0,0};

	// Image coords
	const int i = width/2;
	const int j = height/2;

	// Geographical coords (degrees or meters)
	double x, y;

	if (frame.isLongLat()){
		frame.pix2deg(i,j, x,y);
	}
	else { // metric
		frame.pix2m(i,j, x,y);
	}

	tiepoints[0] = static_cast<double>(i);
	tiepoints[1] = static_cast<double>(j);
	tiepoints[2] = 0;
	tiepoints[3] = x;
	tiepoints[4] = y;
	tiepoints[5] = 0;
	mout.debug() << "Tiepoint (center): " << i << ',' << j << " => " << x << ',' << y << mout.endl;

	TIFFSetField(tif,TIFFTAG_GEOTIEPOINTS, 6,tiepoints);

	double pixscale[3] = {1,1,0};
	//std::cerr << "frame: " << frame.getProjection() << '\n';
	const drain::Rectangle<double> & bbox = frame.isLongLat() ? bboxD : frame.getBoundingBoxM();
	pixscale[0] = (bbox.xUpperRight - bbox.xLowerLeft)/ static_cast<double>(frame.getFrameWidth());
	pixscale[1] = (bbox.yUpperRight - bbox.yLowerLeft)/ static_cast<double>(frame.getFrameHeight());

	/*
	if (frame.isLongLat()){
	//if (false){
		//const drain::Rectangle<double> & bboxD = frame.getBoundingBoxD();
		pixscale[0] = (bboxD.xUpperRight - bboxD.xLowerLeft)/ static_cast<double>(frame.getFrameWidth());
		pixscale[1] = (bboxD.yUpperRight - bboxD.yLowerLeft)/ static_cast<double>(frame.getFrameHeight());
	}
	else {
		const drain::Rectangle<double> & bbox = frame.getBoundingBoxM();
		pixscale[0] = (bbox.xUpperRight - bbox.xLowerLeft)/ static_cast<double>(frame.getFrameWidth());
		pixscale[1] = (bbox.yUpperRight - bbox.yLowerLeft)/ static_cast<double>(frame.getFrameHeight());
	}
	*/

	TIFFSetField(tif,TIFFTAG_GEOPIXELSCALE, 3,pixscale);


}


void SetUpGeoKeys_4326_LongLat(GTIF *gtif){
	GTIFKeySet(gtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelGeographic);
	GTIFKeySet(gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1, RasterPixelIsArea);
	// GTIFKeySet(gtif, GTCitationGeoKey, TYPE_ASCII, 0, "Just An Example");
	GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT,  1, GCSE_WGS84);
	GTIFKeySet(gtif, GeogCitationGeoKey, TYPE_ASCII, 7, "WGS 84");
	GTIFKeySet(gtif, GeogAngularUnitsGeoKey, TYPE_SHORT,  1, Angular_Degree);
	GTIFKeySet(gtif, GeogSemiMajorAxisGeoKey, TYPE_DOUBLE, 1, 6378137.0);  //6377298.556);
	GTIFKeySet(gtif, GeogInvFlatteningGeoKey, TYPE_DOUBLE, 1, 298.257223563);// 300.8017);
}


void WriteImage(TIFF *tif, const drain::image::Image & src) //, int tileWidth = 0, int tileHeight = 0)
{

	Logger mout("FileGeoTIFF", __FUNCTION__);

	const int width  = src.getWidth();
	const int height = src.getHeight();

	unsigned char *buffer = NULL;
	//unsigned short int *buffer16b = NULL;

	int tileWidth = 0;
	int tileHeight = 0;
	int bitspersample = 8;

	TIFFGetField(tif, TIFFTAG_TILEWIDTH,  &tileWidth);
	TIFFGetField(tif, TIFFTAG_TILELENGTH, &tileHeight);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);

	if (tileWidth > 0){

		if (tileHeight == 0)
			tileHeight = tileWidth;

		const int W = (width/tileWidth);
		const int H = (height/tileHeight);

		const bool UCHAR8 = (bitspersample==8); // ? 1 : 2; // 8 or 16 bit data

		Image tile;
		if (UCHAR8)
			tile.setType<unsigned char>();
		else
			tile.setType<unsigned short int>();
		tile.setGeometry(tileWidth, tileHeight);

		mout.info() << "tiled mode:"  << tile << ", bits=" << bitspersample << mout.endl;

		if ((!UCHAR8) && (width % tileWidth)){
			mout.warn() << "16bit image, width != N*tileWidth (" << tileWidth <<"), errors may occur (libgeotiff problem?)" << mout.endl;
		}

		/// current tile-widths
		int w;
		int h;
		const int wPartial = width  % tileWidth;
		const int hPartial = height % tileHeight;
		int iOffset;
		int jOffset;
		for (int l=0; l<=H; ++l){

			jOffset = l*tileHeight;
			if (l<H)
				h = tileHeight;
			else
				h = hPartial;

			for (int k=0; k<=W; ++k){

				iOffset = k*tileWidth;
				if (k<W)
					w = tileWidth;
				else // last tile is partial
					w = wPartial;


				// Copy image data to tile
				if ((w>0) && (h>0)){
					//if (!UCHAR8){tile.setGeometry(w, h);}
					mout.debug(1) << "TILE:" << k << ',' <<  l << '\t' << w << 'x' << h << mout.endl;
					for (int j=0; j<h; ++j){
						for (int i=0; i<w; ++i){
							//buffer[j*tileWidth + i] = src.get<int>(iOffset+i, jOffset+j);
							tile.put(i,j, src.get<int>(iOffset+i, jOffset+j));
						}
					}
					/*
					if ((k==W) || (l==H)){
						std::stringstream s;
						s << "tile" << l << k << ".png";
						drain::image::File::write(tile, s.str());
					}
					*/
					if(!TIFFWriteTile(tif, tile.getBuffer(), iOffset, jOffset, 0, 0)){
						TIFFError("WriteImage", "TIFFWriteTile failed \n");
					}

					//if (!UCHAR8){tile.setGeometry(tileHeight, tileHeight);}

				}
			}
		}

	}
	else {

		const drain::Type t(src.getType());
		if ((t == 'C') || (t=='S')){
			/// Address each ŕow directly
			const int rowBytes = width*src.getByteSize();
			buffer = (unsigned char *)src.getBuffer();
			for (int j=0; j<height; ++j){
				if (!TIFFWriteScanline(tif, &(buffer[j * rowBytes]), j, 0))
					TIFFError(__FUNCTION__, "failure in direct WriteScanline\n");
			}
		}
		else {
			/// Copy each ŕow to buffer
			buffer = new unsigned char[width*1]; // 8 bits
			for (int j=0; j<height; ++j){
				for (int i=0; i<width; ++i)
					buffer[i] = src.get<int>(i,j);
				if (!TIFFWriteScanline(tif, buffer, j, 0))
					TIFFError(__FUNCTION__, "failure in buffered WriteScanline\n");
			}
			delete buffer;
		}

	}


}




/** Writes drain::Image to a png image file applying G,GA, RGB or RGBA color model.
 *  Writes in 8 or 16 bits, according to template class.
 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
 */
//void FileGeoTIFF::write(const std::string &filePath, const HI5TREE & src, const std::list<std::string> & paths){
void FileGeoTIFF::write(const std::string &path, const drain::image::Image & src, int tileWidth, int tileHeight){

	Logger mout("FileGeoTIFF", __FUNCTION__);
	//mout.note() << src.properties << mout.endl;

	/// Open TIFF file for writing
	TIFF *tif = XTIFFOpen(path.c_str(), "w");
	if (tif){

		GTIF *gtif = GTIFNew(tif);
		if (gtif){

			//int tileSize = 256;

			//const drain::Type t(src.getType());
			//SetUpTIFFDirectory(tif, src.getWidth(), src.getHeight());
			SetUpTIFFDirectory(tif, src, tileWidth, tileHeight);
			WriteImage(tif, src); //, tileSize, tileSize/2);

			//mout.note() << src.properties << mout.endl;
			std::string projstr = src.properties["where:projdef"];

			if (!projstr.empty()){
				mout.info() << "where:projdef= " << projstr << mout.endl;
			}
			else
				mout.warn() << "where:projdef empty" << mout.endl;

			drain::Proj4 proj;

			//pj_is_

			proj.setProjectionDst(projstr);
			if (proj.isLongLat()){
				mout.info() << "writing 4326 longlat" << mout.endl;
				SetUpGeoKeys_4326_LongLat(gtif);
			}
			else {
				mout.info() << "writing metric projection" << mout.endl;
				//GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT,  1, GCSE_WGS84);
				//int projOK = GTIFSetFromProj4(gtif, projstr.c_str());
				if (!GTIFSetFromProj4(gtif, projstr.c_str()))
					mout.warn() << "failed in setting GeoTIFF projection, where:projdef='" << projstr << "'" << mout.endl;
			}
			/*
			// usr/include/gdal/rawdataset.h
			// Non-standard http://www.gdal.org/frmt_gtiff.html
			std::string nodata = src.properties["what:nodata"];
			if (!nodata.empty()){
				mout.toOStr() << "registering what:nodata => nodata=" << nodata << mout.endl;
				GTIFKeySet(gtif, (geokey_t)TIFFTAG_GDAL_NODATA, TYPE_ASCII, nodata.length()+1, nodata.c_str());  // yes, ascii
			}
			 */
			GTIFWriteKeys(gtif);

			GTIFFree(gtif);

		}
		else {
			mout.error() << "failed creating GeoTIFF file from TIFF object, path=" << path << mout.endl;
		}

		XTIFFClose(tif);

	}
	else {
		mout.error() << "file open error, path=" << path << mout.endl;
	}

	return ; //-1;

}

}

#endif


// Rack
