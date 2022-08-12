/*
 *   C3Mapper - create minimaps from Caesar 3 scenarios and saved games
 *   Copyright (C) 2007  Bianca van Schaik
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef c3file_h
#define c3file_h

#include "pngimage.h"
#include "grid.h"
#include "caesar3colours.h"
#include <string>
#include <iostream>

/**
* Walker - stores just the information from the walker table inside
* the saved games to draw the walkers on the minimap.
*/
typedef struct {
	unsigned short type;
	unsigned char x;
	unsigned char y;
} Walker;

/**
* Loads a Caesar 3 .sav or .map file and extracts the minimap.
*/
class C3File {
	public:
		/**
		* Constructor. Opens `filename' for parsing. `filename' should exist
		* @param filename Name of the file to open
		*/
		C3File(std::string filename);
		~C3File();
		
		/**
		* Parses the file and returns the minimap image as PNG image
		* @throws exception if the file is invalid
		*/
		PNGImage *getImage();
	
	private:
		void getBuildingColours(unsigned short building, unsigned char edge,
			unsigned char edge_right, unsigned char edge_below, int *c1, int *c2);
		void getTerrainColours(unsigned short terrain, unsigned char random, int *c1, int *c2);
		Grid<unsigned char> *readCompressedByteGrid();
		Grid<unsigned short> *readCompressedShortGrid();
		Grid<unsigned char> *readByteGrid();
		Grid<unsigned short> *readShortGrid();
		Grid<unsigned char> *getRandomData();
		Walker *getWalkers();
		void getMapsizeAndClimate(int *mapsize, int *climate);
		void skipCompressed();
		int readIntFromStream();
		unsigned short readShort();
		void getBitmapCoordinates(int x, int y, int mapsize, int *x_out, int *y_out);
		
		std::ifstream *in;
		Caesar3Colours *colours;
		static const int
			MAX_MAPSIZE = 162,
			MAX_WALKERS = 1000;
};

#endif /* c3file_h */
