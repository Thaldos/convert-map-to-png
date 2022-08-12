/*
 *   ZeusMapper - create minimaps from Zeus scenarios and saved games
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
#ifndef zeusfile_h
#define zeusfile_h

#include "pngimage.h"
#include "grid.h"
#include "zeuscolours.h"
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

class ZeusFile {
	public:
		static const int MAX_MAPS = 5; // parent city + 4 colonies
		
		ZeusFile(std::string filename);
		~ZeusFile();
		
		/**
		* Returns the number of maps in this file. Call this function
		* before attempting to get any images from this map using
		* getImage();
		*/
		int getNumMaps();
		
		/**
		* Returns the next image available. Throws an exception if the
		* image can't be loaded for whatever reason.
		* NOTE: call getNumMaps() before calling this function
		*/
		PNGImage *getImage();
		
		/**
		* Returns whether this file is an adventure or not. Call
		* *after* calling getImages();
		*/
		bool isAdventure();
		
	private:
		void placeBuildings(PNGImage *img, Building *buildings,
			Grid<unsigned char> *edges, int mapsize, bool is_poseidon);
		void placeBuilding(PNGImage *img, Grid<unsigned char> *edges,
			int mapsize, int posX, int posY,
			int sizeX, int sizeY, int c1, int c2, bool reverse);
		void getTerrainColours(unsigned int terrain, unsigned char random,
			unsigned char meadow, unsigned char scrub, unsigned char t_marble,
			int *c1, int *c2);
		Grid<unsigned char> *readCompressedByteGrid();
		Grid<unsigned short> *readCompressedShortGrid();
		Grid<unsigned int> *readCompressedIntGrid();
		Grid<unsigned char> *readByteGrid();
		Grid<unsigned short> *readShortGrid();
		Grid<unsigned int> *readIntGrid();
		Walker *getWalkers();
		Building *getBuildings();
		int getMapsize();
		void skipCompressed();
		unsigned int readInt();
		unsigned short readShort();
		bool searchPattern(char pattern[], int length);
		void getBitmapCoordinates(int x, int y, int mapsize, int *x_out, int *y_out);
		
		static const int
			MAX_MAPSIZE = 228,
			MAX_WALKERS = 2000,
			MAX_BUILDINGS = 4000;
		enum { TYPE_ADVENTURE, TYPE_MAPFILE, TYPE_SAVEDGAME };
		
		int filetype;
		int numMaps;
		int retrievedMaps;
		int positions[MAX_MAPS];
		std::ifstream *in;
		ZeusColours *colours;
};

#endif /* zeusfile_h */
