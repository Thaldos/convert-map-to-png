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
#include "pharaohfile.h"
#include "pkwareinputstream.h"
#include <fstream>

using namespace std;

PharaohFile::PharaohFile(string filename) {
	in = new ifstream();
	in->open(filename.c_str(), ios::in | ios::binary);
	if (!in->is_open()) {
		throw "Can't read file";
	}
}

PharaohFile::~PharaohFile() {
	delete in;
}

PNGImage *PharaohFile::getImage() {
	if (!in->is_open()) {
		return NULL;
	}
	
	Grid<unsigned int> *building_grid = NULL, *terrain = NULL;
	Grid<unsigned char> *edges = NULL, *random = NULL;
	Walker *walkers = NULL;
	Building *buildings = NULL;
	int mapsize;
	bool is_scenario = false;
	char fourcc[5];
	in->read(fourcc, 4);
	fourcc[4] = 0;
	if (string(fourcc) == "MAPS") {
		is_scenario = true;
	}
	if (is_scenario) {
		// Read scenario info
		in->seekg(0x177c, ios::beg);
		building_grid = readIntGrid();
		edges = readByteGrid();
		terrain = readIntGrid();
		in->seekg(51984, ios::cur);
		random = readByteGrid();
		in->seekg(0x99C78, ios::beg);
		mapsize = readInt();
	} else {
		in->seekg(0x177c, ios::beg);
		building_grid = readCompressedIntGrid();
		edges = readCompressedByteGrid();
		skipCompressed(); // building IDs
		terrain = readCompressedIntGrid();
		random = getRandomData();
		walkers = getWalkers();
		buildings = getBuildings();
		mapsize = getMapsize();
	}
	
	if (mapsize > MAX_MAPSIZE) {
		throw "Invalid map size";
	}
	
	// Transform it to something useful
	colours = new PharaohColours(); // all climates have the same minimap colours
	PNGImage *img = new PNGImage(mapsize, mapsize);
	int half = MAX_MAPSIZE / 2;
	int border = (MAX_MAPSIZE - mapsize) / 2;
	int max = border + mapsize;
	int c1, c2;
	int coords[2];
	int start, end;
	unsigned char t_random;
	unsigned int t_terrain, t_building;
	
	for (int y = border; y < max; y++) {
		start = (y < half) ? (border + half - y - 1) : (border + y - half);
		end   = (y < half) ? (half + y + 1 - border) : (3*half - y - border);
		for (int x = start; x < end; x++) {
			t_terrain  = terrain->get(x, y);
			t_building = building_grid->get(x, y);
			t_random = random->get(x, y);
			getTerrainColours(t_terrain, t_random, &c1, &c2);
			if ((t_terrain & 0x48) == 0x8 && (
				(t_building >= 0x3dc6 && t_building <= 0x3ed5) ||
				(t_building >= 0x3720 && t_building <= 0x3739))) {
				// Temple complex or festival square
				c1 = colours->colour(PharaohColours::MAP_RELIGION, 0);
				c2 = colours->colour(PharaohColours::MAP_RELIGION, 1);
			}
			// Set pixel colours
			getBitmapCoordinates(x-border, y-border, mapsize, &coords[0], &coords[1]);
			img->setRGB(coords[0], coords[1], c1);
			img->setRGB(coords[0]+1, coords[1], c2);
		}
	}
	
	if (buildings) {
		placeBuildings(img, buildings, terrain, edges, mapsize);
		delete buildings;
	}
	delete terrain;
	delete edges;
	delete random;
	delete building_grid;
	
	if (walkers) {
		// Get walkers
		int colour;
		for (int i = 0; i < MAX_WALKERS; i++) {
			colour = walkers[i].type;
			if (walkers[i].type == 0xb || walkers[i].type == 0xc || walkers[i].type == 0xd) { // our soldiers
				colour = colours->colour(PharaohColours::MAP_SPRITES, PharaohColours::SPRITE_SOLDIER);
			} else if (walkers[i].type == 0x14 || walkers[i].type == 0x19 || walkers[i].type == 0x4c // trade ship / fishing boat / ferry
			|| walkers[i].type == 0x4d || walkers[i].type == 0x4e) { // transport ship / warship
				colour = colours->colour(PharaohColours::MAP_SPRITES, PharaohColours::SPRITE_SHIP);
			} else if (walkers[i].type == 0x54 || walkers[i].type == 0x68) { // pharaoh or cleo killer animal
				colour = colours->colour(PharaohColours::MAP_SPRITES, PharaohColours::SPRITE_ANIMAL);
			} else if (walkers[i].type == 0x2b || walkers[i].type == 0x2c || walkers[i].type == 0x2d || // enemy
				walkers[i].type == 0x36 || walkers[i].type == 0x37 || // egyption invaders
				walkers[i].type == 0x63) { // bedouins
				colour = colours->colour(PharaohColours::MAP_SPRITES, PharaohColours::SPRITE_ENEMY);
			} else {
				continue;
			}
			
			getBitmapCoordinates(walkers[i].x, walkers[i].y, mapsize, &coords[0], &coords[1]);
			img->setRGB(coords[0], coords[1], colour);
			img->setRGB(coords[0]+1, coords[1], colour);
		}
		delete walkers;
	}
	delete colours;
	
	//img->write("out.png");
	return img;
}

/**
* Goes through the list of buildings and places them on the image
* @param img       Image to write on
* @param buildings Building list
* @param terrain   Terrain information
* @param edges     Edge information
* @param mapsize   Total size of the image
*/
void PharaohFile::placeBuildings(PNGImage *img, Building *buildings,
		Grid<unsigned int> *terrain, Grid<unsigned char> *edges, int mapsize) {
	int cid, num;
	Building *b;
	int border = (MAX_MAPSIZE - mapsize) / 2;
	for (int i = 0; i < MAX_BUILDINGS; i++) {
		if (buildings[i].type == 0) {
			// building slot unused
			continue;
		}
		// Shorten access
		b = &buildings[i];
		
		num = 0;
		
		cid = colours->map(b->type);
		if (!cid) {
			// Unknown building
			continue;
		} else if (cid != PharaohColours::MAP_MONUMENTS
		&& (terrain->get(border + b->x, border + b->y) & 0x8) != 0x8) {
			// Building is not a building on the map
			// Most likely flooded farm
			continue;
		}
		
		// Special cases
		if (cid == PharaohColours::MAP_HOUSING && b->size > 1) {
			// house bigger than 1x1
			int coords[2];
			for (int y = 0; y < b->size; y++) {
				for (int x = 0; x < b->size; x++) {
					getBitmapCoordinates(b->x + x, b->y + y, mapsize, &coords[0], &coords[1]);
					unsigned char edge = edges->get(border+b->x+x, border+b->y+y);
					unsigned char edge_right = edges->get(border+b->x+x+1, border+b->y+y);
					unsigned char edge_below = edges->get(border+b->x+x, border+b->y+y+1);
					if (edge % 8 == 0 || (edge_below & 0x3f) < edge) {
						img->setRGB(coords[0], coords[1], colours->colour(cid, 1));
					} else {
						img->setRGB(coords[0], coords[1], colours->colour(cid, 3));
					}
					if (edge < 8 || edge_right != (edge & 0x3f) + 1) {
						img->setRGB(coords[0]+1, coords[1], colours->colour(cid, 0));
					} else {
						img->setRGB(coords[0]+1, coords[1], colours->colour(cid, 2));
					}
				}
			}
			continue; // don't run place-algorithm
		} else if (b->type == 0xd1) { // festival square -- already painted
			continue;
		} else if (b->type == 0x2a) { // medium statue: all c1
			placeBuilding(img, edges, mapsize, b->x, b->y, b->size, b->size,
					colours->colour(cid, 0), colours->colour(cid, 0));
			continue;
		} else if (b->type == 0x2b) { // large statue
			int coords[2];
			for (int y = 0; y < b->size; y++) {
				for (int x = 0; x < b->size; x++) {
					getBitmapCoordinates(b->x + x, b->y + y, mapsize, &coords[0], &coords[1]);
					unsigned char edge = edges->get(border+b->x+x, border+b->y+y);
					unsigned char edge_right = edges->get(border+b->x+x+1, border+b->y+y);
					unsigned char edge_below = edges->get(border+b->x+x, border+b->y+y+1);
					if (edge % 8 == 0 || (edge_below & 0x3f) < edge) {
						img->setRGB(coords[0], coords[1], colours->colour(cid, 2));
					} else {
						img->setRGB(coords[0], coords[1], colours->colour(cid, 0));
					}
					if (edge < 8 || edge_right != (edge & 0x3f) + 1) {
						img->setRGB(coords[0]+1, coords[1], colours->colour(cid, 3));
					} else {
						img->setRGB(coords[0]+1, coords[1], colours->colour(cid, 0));
					}
				}
			}
			continue;
		} else if (b->type == 0xe5 || b->type == 0xea || b->type == 0xeb || b->type == 0xec) { // tombs
			int coords[2];
			int c1 = colours->colour(cid, 1);
			int c2 = colours->colour(cid, 0);
			for (int y = 0; y < b->size; y++) {
				for (int x = 0; x < b->size; x++) {
					if (!(terrain->get(border + b->x + x, border + b->y + y) & 0x40000000)) {
						getBitmapCoordinates(b->x + x, b->y + y, mapsize, &coords[0], &coords[1]);
						img->setRGB(coords[0], coords[1], c1);
						img->setRGB(coords[0]+1, coords[1], c2);
					}
				}
			}
			continue;
		} else if (b->type == 0xca) { // gatehouse
			// place a second building for the other side of the road, and
			// the road itself as well
			if (b->rotation) {
				placeBuilding(img, edges, mapsize, b->x + 3, b->y, 2, 2,
					colours->colour(cid, 0), colours->colour(cid, 1));
				placeBuilding(img, edges, mapsize, b->x + 2, b->y, 1, 2,
					colours->colour(cid, 0), colours->colour(cid, 1));
			} else {
				placeBuilding(img, edges, mapsize, b->x, b->y + 3, 2, 2,
					colours->colour(cid, 0), colours->colour(cid, 1));
				placeBuilding(img, edges, mapsize, b->x, b->y + 2, 2, 1,
					colours->colour(cid, 0), colours->colour(cid, 1));
			}
		}
		
		placeBuilding(img, edges, mapsize, b->x, b->y, b->size, b->size,
			colours->colour(cid, num), colours->colour(cid, num+1));
	}
}

/**
* Places a building of the specified size on the specified image at the
* specified location, taking information from the edges into account
* @param img     Image to write on
* @param edges   Edge information
* @param mapsize Total size of the image
* @param posX    X position to place building
* @param posY    Y position to place building
* @param sizeX   Width of the building
* @param sizeY   Height of the building
* @param c1      Colour 1 -- used for building interior
* @param c2      Colour 2 -- used for top & right edge
*/
void PharaohFile::placeBuilding(PNGImage *img, Grid<unsigned char> *edges,
		int mapsize, int posX, int posY, int sizeX, int sizeY, int c1, int c2) {
	int coords[2];
	if (sizeX == 1 && sizeY == 1) {
		getBitmapCoordinates(posX, posY, mapsize, &coords[0], &coords[1]);
		img->setRGB(coords[0], coords[1], c2);
		img->setRGB(coords[0]+1, coords[1], c1);
	} else {
		int offset = (MAX_MAPSIZE - mapsize) / 2;
		for (int y = 0; y < sizeY; y++) {
			for (int x = 0; x < sizeX; x++) {
				getBitmapCoordinates(posX + x, posY + y, mapsize, &coords[0], &coords[1]);
				unsigned char edge = edges->get(offset+posX+x, offset+posY+y);
				if (edge == 64) {
					img->setRGB(coords[0], coords[1], c2);
					img->setRGB(coords[0]+1, coords[1], c1);
				} else { // is not 1-tile building
					unsigned char edge_right = edges->get(offset+posX+x+1, offset+posY+y);
					img->setRGB(coords[0], coords[1], c1);
					if (edge < 8 || edge_right != (edge & 0x3f) + 1 // top edge OR right edge
						|| (sizeX == 6 && x == 2) || (sizeY == 6 && y == 3)) { // 6x6 split
						img->setRGB(coords[0]+1, coords[1], c2);
					} else {
						img->setRGB(coords[0]+1, coords[1], c1);
					}
				}
			}
		}
	}
}

/**
* Figures out what the terrain colours for this tile are
*/
void PharaohFile::getTerrainColours(unsigned int terrain, unsigned char random, int *c1, int *c2) {
	if (terrain & 0x80000) {
		*c1 = *c2 = colours->colour(PharaohColours::MAP_BACKGROUND, 0);
		return;
	}
	
	int num3 = random & 3;
	if (terrain & 0x1) { // tree/shrub
		*c1 = colours->colour(PharaohColours::MAP_TREE1, num3);
		*c2 = colours->colour(PharaohColours::MAP_TREE2, num3);
	} else if (terrain & 0x2) { // rock
		*c1 = colours->colour(PharaohColours::MAP_ROCK1, num3);
		*c2 = colours->colour(PharaohColours::MAP_ROCK2, num3);
	} else if (terrain == 0x44) { // water + bridge = road
		*c1 = colours->colour(PharaohColours::MAP_ROAD, 0);
		*c2 = colours->colour(PharaohColours::MAP_ROAD, 1);
	} else if (terrain & 0x4) { // water
		*c1 = colours->colour(PharaohColours::MAP_WATER1, num3);
		*c2 = colours->colour(PharaohColours::MAP_WATER2, num3);
	} else if (terrain & 0x20) { // garden
		if (terrain & 0x8) { // garden + building = entertainment
			*c1 = colours->colour(PharaohColours::MAP_ENTERTAINMENT, 1);
			*c2 = colours->colour(PharaohColours::MAP_ENTERTAINMENT, 0);
		} else {
			*c1 = colours->colour(PharaohColours::MAP_AESTHETICS, 1);
			*c2 = colours->colour(PharaohColours::MAP_AESTHETICS, 0);
		}
	} else if (terrain & 0x40) { // road
		*c1 = colours->colour(PharaohColours::MAP_ROAD, 0);
		*c2 = colours->colour(PharaohColours::MAP_ROAD, 1);
	} else if (terrain & 0x100) { // irrigation
		*c1 = colours->colour(PharaohColours::MAP_WATER1, 4);
		*c2 = colours->colour(PharaohColours::MAP_WATER2, 4);
	} else if (terrain & 0x800 || terrain & 0x10000) { // meadow or floodplain
		*c1 = colours->colour(PharaohColours::MAP_FERTILE1, num3);
		*c2 = colours->colour(PharaohColours::MAP_FERTILE2, num3);
	} else if (terrain & 0x2000000) { // sand dune
		*c1 = colours->colour(PharaohColours::MAP_DUNE1, num3);
		*c2 = colours->colour(PharaohColours::MAP_DUNE2, num3);
	} else if (terrain & 0x40000) { // marshland
		*c1 = colours->colour(PharaohColours::MAP_MARSH, (random & 4) >> 2);
		*c2 = colours->colour(PharaohColours::MAP_MARSH, ((random & 4) >> 2) + 2);
	} else if (terrain & 0x804000) {// wall
		*c1 = colours->colour(PharaohColours::MAP_WALL, 0);
		*c2 = colours->colour(PharaohColours::MAP_WALL, 0);
	} else if (terrain & 0x40000000) { // tomb chamber/tunnel
		*c1 = colours->colour(PharaohColours::MAP_ROAD, 0);
		*c2 = colours->colour(PharaohColours::MAP_ROAD, 1);
	} else if (terrain & 0x10000000) { // monument plazas, etc
		*c1 = colours->colour(PharaohColours::MAP_MONUMENTS, 1);
		*c2 = colours->colour(PharaohColours::MAP_MONUMENTS, 0);
	} else { // empty land or watered land
		*c1 = colours->colour(PharaohColours::MAP_EMPTY1, random & 7);
		*c2 = colours->colour(PharaohColours::MAP_EMPTY2, random & 7);
	}
}

/**
* Reads a compressed byte grid from the stream. Compressed chunks
* consist of a length followed by a compressed chunk of that length
*/
Grid<unsigned char> *PharaohFile::readCompressedByteGrid() {
	int length = readInt();
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
Grid<unsigned short> *PharaohFile::readCompressedShortGrid() {
	int length = readInt();
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
* Reads a compressed int grid from the stream. Compressed chunks
* consist of a length followed by a compressed chunk of that length
*/
Grid<unsigned int> *PharaohFile::readCompressedIntGrid() {
	int length = readInt();
	Grid<unsigned int> *g = new Grid<unsigned int>(MAX_MAPSIZE, MAX_MAPSIZE);
	
	try {
		PKWareInputStream pk(in, false, length);
		
		for (int y = 0; y < MAX_MAPSIZE; y++) {
			for (int x = 0; x < MAX_MAPSIZE; x++) {
				g->set(x, y, pk.readInt());
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
Grid<unsigned char> *PharaohFile::readByteGrid() {
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
Grid<unsigned short> *PharaohFile::readShortGrid() {
	Grid<unsigned short> *g = new Grid<unsigned short>(MAX_MAPSIZE, MAX_MAPSIZE);
	
	for (int y = 0; y < MAX_MAPSIZE; y++) {
		for (int x = 0; x < MAX_MAPSIZE; x++) {
			g->set(x, y, readShort());
		}
	}
	return g;
}

/**
* Reads an uncompressed int grid from the stream.
*/
Grid<unsigned int> *PharaohFile::readIntGrid() {
	Grid<unsigned int> *g = new Grid<unsigned int>(MAX_MAPSIZE, MAX_MAPSIZE);
	
	for (int y = 0; y < MAX_MAPSIZE; y++) {
		for (int x = 0; x < MAX_MAPSIZE; x++) {
			g->set(x, y, readInt());
		}
	}
	return g;
}

/**
* Reads the random data grid
*/
Grid<unsigned char> *PharaohFile::getRandomData() {
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
Walker * PharaohFile::getWalkers() {
	// Assume random data has been read already
	// Which is followed by some more compressed data blocks
	for (int i = 0; i < 5; i++) {
		skipCompressed();
	}
	
	// Next come the walkers in compressed format
	Walker *walkers = new Walker[MAX_WALKERS];
	int length = readInt();
	try {
		PKWareInputStream pk(in, false, length);
		
		// Walker entry = 388 bytes
		for (int i = 0; i < MAX_WALKERS; i++) {
			pk.skip(10);
			walkers[i].type = pk.readShort();
			pk.skip(8);
			walkers[i].x = pk.readShort();
			walkers[i].y = pk.readShort();
			pk.skip(364);
		}
		pk.empty();
	} catch (PKException) {
		delete walkers;
		throw;
	}
	return walkers;
}

/**
* Gets the building info from the saved game
*/
Building * PharaohFile::getBuildings() {
	// Assume walkers have been read already
	
	// Three compressed blocks
	for (int i = 0; i < 3; i++) {
		skipCompressed();
	}
	
	// Three ints
	in->seekg(12, ios::cur);
	
	// Compressed block, followed by 72 bytes, followed by walkers
	skipCompressed();
	in->seekg(72, ios::cur);
	Building *buildings = new Building[MAX_BUILDINGS];
	int length = readInt();
	try {
		PKWareInputStream pk(in, false, length);
		
		// Building entry = 264 bytes
		for (int i = 0; i < MAX_BUILDINGS; i++) {
			pk.skip(3);
			buildings[i].size = pk.readByte();
			pk.skip(2);
			buildings[i].x = pk.readShort();
			buildings[i].y = pk.readShort();
			pk.skip(6);
			buildings[i].type = pk.readShort();
			pk.skip(152);
			buildings[i].rotation = pk.readByte();
			pk.skip(93);
		}
		pk.empty();
	} catch (PKException) {
		delete buildings;
		throw;
	}
	return buildings;
}

/**
* Returns the map size of a saved game. Assumes everything up to
* and including buildings have been read already.
*/
int PharaohFile::getMapsize() {
	// Assume buildings have been read already
	
	// 68 bytes, followed by one compressed
	in->seekg(68, ios::cur);
	skipCompressed();
	
	// Start of data copied from the .map file!
	// 704 bytes to skip
	in->seekg(704, ios::cur);
	// Next int = map size
	int mapsize = readInt();
	
	return mapsize;
}

/**
* Skips a compressed data block
*/
void PharaohFile::skipCompressed() {
	int skip = readInt();
	in->seekg(skip, ios::cur);
}

/**
* Reads an integer from the stream
*/
unsigned int PharaohFile::readInt() {
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
unsigned short PharaohFile::readShort() {
	char data[2];
	in->read(data, 2);
	return (unsigned short)(data[0] + (data[1] << 8));
}

/**
* Returns the bitmap coordinates for a given map coordinate
*/
void PharaohFile::getBitmapCoordinates(int x, int y, int mapsize, int *x_out, int *y_out) {
	*x_out = mapsize / 2 + x - y - 1;
	*y_out = 1 + x + y - mapsize / 2;
}

int main(int argc, char **argv) {
	if (argc != 3) {
		cerr << "Usage: "<<argv[0] << " [pharaoh file] [png output file]" << endl;
		return 1;
	}
	try {
		PharaohFile *pf = new PharaohFile(string(argv[1]));
		PNGImage *img = pf->getImage();
		
		if (img) {
			img->write(argv[2]);
			delete img;
		}
		delete pf;
	} catch (...) {
		cerr << "Couldn't process file. Make sure it's a valid Pharaoh file." << endl;
		return 2;
	}
	return 0;
}
