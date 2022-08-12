/*
 *   PharaohMapper - create minimaps from Pharaoh scenarios and saved games
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
#ifndef pharaohfile_h
#define pharaohfile_h

#include "pngimage.h"
#include "grid.h"
#include "pharaohcolours.h"
#include <string>
#include <iostream>

typedef struct {
	unsigned short type;
	unsigned short x;
	unsigned short y;
} Walker;

typedef struct {
	unsigned short type;
	unsigned short x;
	unsigned short y;
	unsigned char size;
	unsigned char rotation;
} Building;

class PharaohFile {
	public:
		PharaohFile(std::string filename);
		~PharaohFile();
		
		PNGImage *getImage();
	
	private:
		void placeBuildings(PNGImage *img, Building *buildings,
			 Grid<unsigned int> *terrain, Grid<unsigned char> *edges,
			 int mapsize);
		void placeBuilding(PNGImage *img, Grid<unsigned char> *edges,
			int mapsize, int posX, int posY,
			int sizeX, int sizeY, int c1, int c2);
		void getBuildingColours(unsigned int building, unsigned char edge,
			unsigned char edge_right, unsigned char edge_below, int *c1, int *c2);
		void getTerrainColours(unsigned int terrain, unsigned char random, int *c1, int *c2);
		Grid<unsigned char> *readCompressedByteGrid();
		Grid<unsigned short> *readCompressedShortGrid();
		Grid<unsigned int> *readCompressedIntGrid();
		Grid<unsigned char> *readByteGrid();
		Grid<unsigned short> *readShortGrid();
		Grid<unsigned int> *readIntGrid();
		Grid<unsigned char> *getRandomData();
		Walker *getWalkers();
		Building *getBuildings();
		int getMapsize();
		void skipCompressed();
		unsigned int readInt();
		unsigned short readShort();
		void getBitmapCoordinates(int x, int y, int mapsize, int *x_out, int *y_out);
		
		std::ifstream *in;
		PharaohColours *colours;
		static const int
			MAX_MAPSIZE = 228,
			MAX_WALKERS = 2000,
			MAX_BUILDINGS = 4000;
};

#endif /* pharaohfile_h */
