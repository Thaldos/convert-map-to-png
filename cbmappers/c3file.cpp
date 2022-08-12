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
#include "c3file.h"
#include "pkwareinputstream.h"
#include <fstream>

using namespace std;

C3File::C3File(string filename) {
	in = new ifstream();
	in->open(filename.c_str(), ios::in | ios::binary);
	if (!in->is_open()) {
		throw "Can't read file";
	}
}

C3File::~C3File() {
	delete in;
}

PNGImage *C3File::getImage() {
	if (!in->is_open()) {
		return NULL;
	}
	
	// Init grids for use
	Grid<unsigned short> *buildings = NULL, *terrain = NULL;
	Grid<unsigned char> *edges = NULL, *random = NULL;
	Walker *walkers = NULL;
	int mapsize, climate;
	bool is_scenario = false;

	// Check the first int. If it's zero, this is a scenario
	readIntFromStream();
	if (!readIntFromStream()) {
		is_scenario = true;
	}
	
	if (is_scenario) {
		in->seekg(0, ios::beg);
		// mapfiles don't have building numbers, or is it the first X*X shorts?
		buildings = readShortGrid();
		edges = readByteGrid();
		terrain = readShortGrid();
		in->seekg(26244, ios::cur); // useless zero grid
		random = readByteGrid();
		// Mapsize and climate are easy here
		in->seekg(0x335b4, ios::beg);
		mapsize = readIntFromStream();
		in->seekg(0x33ad8, ios::beg);
		climate = in->peek(); // use peek as shortcut: we only need 1 byte
	} else {
		try {
			buildings = readCompressedShortGrid();
			edges     = readCompressedByteGrid();
			skipCompressed(); // building IDs
			terrain   = readCompressedShortGrid();
			random     = getRandomData();
			walkers = getWalkers();
		} catch (PKException) {
			if (buildings) delete buildings;
			if (edges)     delete edges;
			if (terrain)   delete terrain;
			if (random)    delete random;
			if (walkers)   delete walkers;
			throw;
		}
		getMapsizeAndClimate(&mapsize, &climate);
	}
	// Lil' sanity check
	if (mapsize > MAX_MAPSIZE) {
		throw "Map size invalid!";
	}
	
	colours = new Caesar3Colours(climate);
	PNGImage *img = new PNGImage(mapsize * 2, mapsize * 2);
	int border = (MAX_MAPSIZE - mapsize) / 2;
	int max = border + mapsize;
	int c1, c2;
	int coords[2];
	unsigned char t_random, t_edge, t_edge_below, t_edge_right;
	unsigned short t_terrain, t_building;
	
	for (int y = border; y < max; y++) {
		for (int x = border; x < max; x++) {
			t_terrain  = terrain->get(x, y);
			t_building = buildings->get(x, y);
			
			if (t_terrain & 0x8 && t_building != 0xc69) { // building & is not fort ground
				t_edge = edges->get(x, y);
				t_edge_below = edges->get(x, y+1);
				t_edge_right = edges->get(x+1, y);
				getBuildingColours(t_building, t_edge, t_edge_right, t_edge_below, &c1, &c2);
			} else if (!(t_terrain & 64) && t_building >= 0x029c && t_building < 0x02b8) {
				// Terrain is an aquaduct *without* road beneath it
				c1 = colours->colour(Caesar3Colours::MAP_AQUA, 0);
				c2 = colours->colour(Caesar3Colours::MAP_AQUA, 1);
			} else {
				t_random = random->get(x, y);
				getTerrainColours(t_terrain, t_random, &c1, &c2);
			}
			
			// Set pixel colours
			getBitmapCoordinates(x-border, y-border, mapsize, &coords[0], &coords[1]);
			img->setRGB(coords[0], coords[1], c1);
			img->setRGB(coords[0]+1, coords[1], c2);
		}
	}
	delete terrain;
	delete random;
	delete buildings;
	delete edges;

	// Only do the walkers for saved games
	if (walkers) {
		// Get walkers
		int colour;
		for (int i = 0; i < MAX_WALKERS; i++) {
			if (walkers[i].type == 0x45) { // wolf
				colour = colours->colour(Caesar3Colours::MAP_SPRITES, Caesar3Colours::SPRITE_WOLF);
			} else if (walkers[i].type == 0xb || walkers[i].type == 0xc || walkers[i].type == 0xd) { // our soldiers
				colour = colours->colour(Caesar3Colours::MAP_SPRITES, Caesar3Colours::SPRITE_SOLDIER);
			} else if (walkers[i].type == 0x31) { // barbarians
				colour = colours->colour(Caesar3Colours::MAP_SPRITES, Caesar3Colours::SPRITE_BARBARIAN);
			} else if (walkers[i].type == 0x2d || walkers[i].type == 0x2f) { // enemies
				colour = colours->colour(Caesar3Colours::MAP_SPRITES, Caesar3Colours::SPRITE_ENEMY);
			} else {
				// Normal walkers don't show up
				continue;
			}
			getBitmapCoordinates(walkers[i].x, walkers[i].y, mapsize, &coords[0], &coords[1]);
			img->setRGB(coords[0], coords[1], colour);
			img->setRGB(coords[0]+1, coords[1], colour);
		}
		delete walkers;
	}
	delete colours;
	return img;
}

/**
* Figures out what the two pixel-colours for this building should be
*/
void C3File::getBuildingColours(unsigned short building, unsigned char edge,
	unsigned char edge_right, unsigned char edge_below, int *c1, int *c2) {
	
	if (building >= 0xa00 && building <= 0xb06) {
		// House
		if (edge == 64) { // 1-tile house
			*c1 = colours->colour(Caesar3Colours::MAP_HOUSE, 0);
			*c2 = colours->colour(Caesar3Colours::MAP_HOUSE, 1);
		} else {
			// left pixel
			if (edge % 8 == 0 || (edge_below & 0x3f) < edge) { // left edge OR bottom edge
				*c1 = colours->colour(Caesar3Colours::MAP_HOUSE, 2);
			} else {
				*c1 = colours->colour(Caesar3Colours::MAP_HOUSE, 0);
			}
			// right pixel
			if (edge < 8 || edge_right != (edge & 0x3f) + 1) { // top edge OR right edge
				*c2 = colours->colour(Caesar3Colours::MAP_HOUSE, 3);
			} else {
				*c2 = colours->colour(Caesar3Colours::MAP_HOUSE, 1);
			}
		}
	} else if (building == 0xb2f) { // reservoir
		// left pixel
		if (edge % 8 == 0 || edge > 15) { // left edge OR bottom edge
			*c1 = colours->colour(Caesar3Colours::MAP_AQUA, 1);
		} else {
			*c1 = colours->colour(Caesar3Colours::MAP_AQUA, 0);
		}
		// right pixel
		if (edge < 8 || edge % 8 == 2) { // top edge OR right edge
			*c2 = colours->colour(Caesar3Colours::MAP_AQUA, 1);
		} else {
			*c2 = colours->colour(Caesar3Colours::MAP_AQUA, 0);
		}
	} else { // other building
		if (edge == 64) { // 1-tile building
			*c1 = colours->colour(Caesar3Colours::MAP_BUILDING, 0);
			*c2 = colours->colour(Caesar3Colours::MAP_BUILDING, 1);
		} else {
			// left pixel
			if (edge % 8 == 0 || (edge & 0x3f) + 8 != edge_below) { // left edge OR bottom edge
				*c1 = colours->colour(Caesar3Colours::MAP_BUILDING, 0);
			} else {
				*c1 = colours->colour(Caesar3Colours::MAP_BUILDING, 2);
			}
			// right pixel
			if (edge < 8 || edge_right != (edge & 0x3f) + 1) { // top edge OR right edge
				*c2 = colours->colour(Caesar3Colours::MAP_BUILDING, 1);
			} else {
				*c2 = colours->colour(Caesar3Colours::MAP_BUILDING, 3);
			}
		}
	}
}

/**
* Figures out what the terrain colours for this tile are
*/
void C3File::getTerrainColours(unsigned short terrain,
		unsigned char random, int *c1, int *c2) {
	// 5 indicates that there is no terrain: beyond map edge
	if (terrain == 5) {
		*c1 = *c2 = colours->colour(Caesar3Colours::MAP_BACKGROUND, 0);
		return;
	}
	
	// Most terrain elements have 4 variants, depending on
	// the last 2 bits of the random number
	int num3 = random & 3;
	if (terrain & 1 || terrain & 16) { // tree/shrub
		*c1 = colours->colour(Caesar3Colours::MAP_TREE1, num3);
		*c2 = colours->colour(Caesar3Colours::MAP_TREE2, num3);
	} else if (terrain & 2 || terrain & 512) { // rock / elevation
		*c1 = colours->colour(Caesar3Colours::MAP_ROCK1, num3);
		*c2 = colours->colour(Caesar3Colours::MAP_ROCK2, num3);
	} else if (terrain & 4) { // water
		*c1 = colours->colour(Caesar3Colours::MAP_WATER1, num3);
		*c2 = colours->colour(Caesar3Colours::MAP_WATER2, num3);
	} else if (terrain & 64) { // road
		*c1 = colours->colour(Caesar3Colours::MAP_ROAD, 0);
		*c2 = colours->colour(Caesar3Colours::MAP_ROAD, 1);
	} else if (terrain & 2048) { // fertile
		*c1 = colours->colour(Caesar3Colours::MAP_FERTILE1, num3);
		*c2 = colours->colour(Caesar3Colours::MAP_FERTILE2, num3);
	} else if (terrain & 0x4000) { // wall
		*c1 = colours->colour(Caesar3Colours::MAP_WALL, 0);
		*c2 = colours->colour(Caesar3Colours::MAP_WALL, 1);
	} else { // empty land, this one has 8 variants
		*c1 = colours->colour(Caesar3Colours::MAP_EMPTY1, random & 7);
		*c2 = colours->colour(Caesar3Colours::MAP_EMPTY2, random & 7);
	}
}

/**
* Reads a compressed byte grid from the stream. Compressed chunks
* consist of a length followed by a compressed chunk of that length
*/
Grid<unsigned char> *C3File::readCompressedByteGrid() {
	int length = readIntFromStream();
	Grid<unsigned char> *g = new Grid<unsigned char>(MAX_MAPSIZE, MAX_MAPSIZE);
	
	try {
		PKWareInputStream pk(in, false, length);
		
		for (int y = 0; y < MAX_MAPSIZE; y++) {
			for (int x = 0; x < MAX_MAPSIZE; x++) {
				g->set(x, y, pk.readByte());
			}
		}
		pk.empty();
	} catch (PKException) {
		delete g;
		throw;
	}
	return g;
}

/**
* Reads a compressed short grid from the stream. Compressed chunks
* consist of a length followed by a compressed chunk of that length
*/
Grid<unsigned short> *C3File::readCompressedShortGrid() {
	int length = readIntFromStream();
	Grid<unsigned short> *g = new Grid<unsigned short>(MAX_MAPSIZE, MAX_MAPSIZE);
	
	try {
		PKWareInputStream pk(in, false, length);
		for (int y = 0; y < MAX_MAPSIZE; y++) {
			for (int x = 0; x < MAX_MAPSIZE; x++) {
				g->set(x, y, pk.readShort());
			}
		}
		pk.empty();
	} catch (PKException) {
		delete g;
		throw;
	}
	return g;
}

/**
* Reads an uncompressed byte grid from the stream.
*/
Grid<unsigned char> *C3File::readByteGrid() {
	Grid<unsigned char> *g = new Grid<unsigned char>(MAX_MAPSIZE, MAX_MAPSIZE);
	char c;
	
	for (int y = 0; y < MAX_MAPSIZE; y++) {
		for (int x = 0; x < MAX_MAPSIZE; x++) {
			in->read(&c, 1);
			g->set(x, y, (unsigned char)c);
		}
	}
	return g;
}

/**
* Reads an uncompressed short grid from the stream.
*/
Grid<unsigned short> *C3File::readShortGrid() {
	Grid<unsigned short> *g = new Grid<unsigned short>(MAX_MAPSIZE, MAX_MAPSIZE);
	
	for (int y = 0; y < MAX_MAPSIZE; y++) {
		for (int x = 0; x < MAX_MAPSIZE; x++) {
			g->set(x, y, readShort());
		}
	}
	return g;
}

/**
* Gets the random data from the saved game, which is buried after
* a few "useless" compressed chunks.
*/
Grid<unsigned char> *C3File::getRandomData() {
	// Assume the first 4 chunks of data have been read
	// The next 4 (useless) compressed parts are stored in the same way
	for (int i = 0; i < 4; i++) {
		skipCompressed();
	}
	
	// Here be the random data
	return readByteGrid();
}

/**
* Gets the walker info from the saved game, which is buried after
* a few "useless" compressed chunks.
*/
Walker * C3File::getWalkers() {
	// Assume random data has been read already
	// Which is followed by some more compressed data blocks
	for (int i = 0; i < 5; i++) {
		skipCompressed();
	}
	
	// Next come the walkers in compressed format
	Walker *walkers = new Walker[MAX_WALKERS];
	int length = readIntFromStream();
	try {
		PKWareInputStream pk(in, false, length);
		
		// Walker entries are 128 bytes
		for (int i = 0; i < MAX_WALKERS; i++) {
			pk.skip(10);
			walkers[i].type = pk.readShort();
			pk.skip(8);
			walkers[i].x = pk.readByte();
			walkers[i].y = pk.readByte();
			pk.skip(106);
		}
		pk.empty();
	} catch (PKException) {
		delete walkers;
		throw;
	}
	return walkers;
}

/**
* Gets the map size and climate from the saved game, which is
* buried deep into the game file
*/
void C3File::getMapsizeAndClimate(int *mapsize, int *climate) {
	// Assume walkers have been read already
	// 1200 bytes that might be uncompressed
	int size = readIntFromStream();
	if (size <= 0) {
		in->seekg(1200, ios::cur);
	} else {
		in->seekg(size, ios::cur);
	}
	
	// Two compressed blocks
	for (int i = 0; i < 2; i++) {
		skipCompressed();
	}
	
	// Three ints
	in->seekg(12, ios::cur);
	
	// Compressed block, followed by 70 bytes, followed by compressed
	skipCompressed();
	in->seekg(70, ios::cur);
	skipCompressed();
	
	// 208 bytes, followed by one compressed
	in->seekg(208, ios::cur);
	skipCompressed();
	
	// Start of data copied from the .map file!
	// 788 bytes to skip
	in->seekg(788, ios::cur);
	// Next int = map size
	*mapsize = readIntFromStream();
	
	// 1312 bytes to skip
	in->seekg(1312, ios::cur);
	// Next byte = climate
	char c;
	in->read(&c, 1);
	*climate = (int)c;
}

/**
* Skips a compressed data block
*/
void C3File::skipCompressed() {
	int skip = readIntFromStream();
	in->seekg(skip, ios::cur);
}

/**
* Reads an integer from the stream
*/
int C3File::readIntFromStream() {
	char data[4];
	unsigned int number = 0;
	
	in->read(data, 4);
	for (int i = 0; i < 4; i++) {
		number += ((unsigned char)data[i] << (i*8));
	}
	return number;
}

/**
* Reads a short from the stream
*/
unsigned short C3File::readShort() {
	char data[2];
	in->read(data, 2);
	return (unsigned short)(data[0] + (data[1] << 8));
}

/**
* Returns the bitmap coordinates for a given map coordinate
*/
void C3File::getBitmapCoordinates(int x, int y, int mapsize, int *x_out, int *y_out) {
	*x_out = x + mapsize - y - 1;
	*y_out = x + y;
}

int main(int argc, char **argv) {
	if (argc != 3) {
		cerr << "Usage: " << argv[0] << " [caesar 3 file] [png output file]" << endl;
		return 1;
	}
	try {
		C3File *cf = new C3File(string(argv[1]));
		PNGImage *img = cf->getImage();
		
		if (img) {
			img->write(argv[2]);
			delete img;
		}
		delete cf;
	} catch (...) {
		cerr << "Couldn't process file. Make sure it's a valid C3 file." << endl;
		return 2;
	}
	return 0;
}
