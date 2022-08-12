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
#include "zeusfile.h"
#include "pkwareinputstream.h"
#include <fstream>

using namespace std;

ZeusFile::ZeusFile(string filename) {
	in = new ifstream();
	retrievedMaps = numMaps = 0;
	in->open(filename.c_str(), ios::in | ios::binary);
	if (!in->is_open()) {
		throw "Can't read file";
	}
}

ZeusFile::~ZeusFile() {
	delete in;
}

bool ZeusFile::isAdventure() {
	return filetype == TYPE_ADVENTURE;
}

int ZeusFile::getNumMaps() {
	if (!in->is_open()) {
		return 0;
	}
	
	char fourcc[5];
	in->read(fourcc, 4);
	fourcc[4] = 0;
	if (string(fourcc) == "MAPS") {
		// Mapfile .map
		filetype = TYPE_MAPFILE;
	} else if ((unsigned char)fourcc[0] == 0xa7 && fourcc[1] == 0x22 && !fourcc[2] && !fourcc[3]) {
		// Adventure .pak
		filetype = TYPE_ADVENTURE;
	} else {
		// Assume saved game .sav
		filetype = TYPE_SAVEDGAME;
	}
	
	if (filetype != TYPE_ADVENTURE) {
		// Map or saved game: 1 map
		numMaps = 1;
	} else {
		// Adventure, get positions of maps inside the file
		int i = 0;
		char pattern[] = "MAPS";
		if (searchPattern(pattern, 4)) {
			while (searchPattern(pattern, 4)) {
				positions[i] = in->tellg();
				i++;
			}
			numMaps = i;
		}
		in->clear();
		in->seekg(0, ios::beg);
	}
	return numMaps;
}

PNGImage *ZeusFile::getImage() {
	if (retrievedMaps >= numMaps) {
		throw "No maps left";
	}
	Grid<unsigned int> *terrain = NULL;
	Grid<unsigned char> *edges = NULL, *random = NULL, *fertile = NULL,
		*scrub = NULL, *marble = NULL;
	
	Walker *walkers = NULL;
	Building *buildings = NULL;
	int mapsize;
	bool is_poseidon = false;
	
	if (filetype == TYPE_SAVEDGAME) {
		retrievedMaps++;
		// Saved game
		in->seekg(0x1779, ios::beg);
		skipCompressed(); // 19184 bytes with unknown purpose
		in->seekg(8, ios::cur);
		skipCompressed(); // 12584 bytes with unknown purpose
		// Next come map information like map size, entry points, and whether
		// it's Poseidon or not
		in->seekg(188, ios::cur);
		mapsize = readInt();
		
		// Sanity check
		if (mapsize > MAX_MAPSIZE) {
			throw "Invalid map size";
		}
		
		//cout << "Map size: " << mapsize << endl;
		in->seekg(600, ios::cur);
		is_poseidon = (in->peek() == 1);
		//cout << "Poseidon? " << is_poseidon << endl;
		in->seekg(1384, ios::cur);
		skipCompressed(); // 14400 bytes with unknown purpose
		in->seekg(18609, ios::cur); // unknown purpose
		edges = readCompressedByteGrid(); // edges
		skipCompressed(); // short grid with all zeroes
		terrain = readCompressedIntGrid(); // Terrain info: 01 = trees, etc
		skipCompressed(); // byte grid with all zeroes
		skipCompressed(); // short grid with ID numbers: perhaps which walker is where or sth
		skipCompressed(); // byte grid: 00 / 20
		skipCompressed(); // byte grid: all zeroes
		
		// onwards to the interesting stuff:
		random = readByteGrid();
		walkers = getWalkers(); // includes 5 misc grids
		skipCompressed(); // not of proper length: 2000
		skipCompressed(); // not of proper length: 500000
		skipCompressed(); // 15600
		in->seekg(69, ios::cur);
		buildings = getBuildings(); // buildings tables
		in->seekg(352, ios::cur);
		skipCompressed(); // 60000
		in->seekg(17974, ios::cur);
		skipCompressed(); // 1000
		skipCompressed(); // 1000
		skipCompressed(); // 8000
		in->seekg(53783, ios::cur); // 53783
		fertile = readByteGrid();
		in->seekg(16, ios::cur);
		skipCompressed(); // 75168 bytes
		// marble thingy
		marble = readByteGrid(); // relates to marble quarries & sheep & goats
		in->seekg(32, ios::cur);
		skipCompressed(); // 36 bytes
		skipCompressed(); // int grid for "intelligent" maintenance officers
		in->seekg(39, ios::cur);
		skipCompressed(); // another mysterious byte grid (zeroes &)
		scrub = readCompressedByteGrid(); // this really IS the scrub
		/*
		in->seekg(4, ios::cur);
		bgrid = readCompressedByteGrid(); // elevation level, including edges
		bgrid = readCompressedByteGrid(); // elevation level, excluding edges
		skipCompressed(); // 1468
		*/
	} else {
		if (filetype == TYPE_ADVENTURE) {
			in->seekg(positions[retrievedMaps], ios::beg);
		}
		retrievedMaps++;
		
		// Read scenario info
		in->seekg(0x1778, ios::cur);
		skipCompressed(); // buildings grid: there are no placable buildings
		edges = readCompressedByteGrid();
		terrain = readCompressedIntGrid();
		skipCompressed(); // byte grid: 00 or 20
		readInt(); // indicating start of random block (or perhaps "uncompressed" indicator?)
		random = readByteGrid();
		skipCompressed(); // byte grid: all zeroes
		in->seekg(60, ios::cur);
		mapsize = readInt(); // Poseidon or not doesn't matter here
		
		// Sanity check
		if (mapsize > MAX_MAPSIZE) {
			delete edges;
			delete terrain;
			delete random;
			throw "Invalid map size";
		}
		
		in->seekg(1984, ios::cur);
		fertile = readCompressedByteGrid(); // meadow, 0-99
		in->seekg(18628, ios::cur);
		skipCompressed(); // not of proper length: 14400
		skipCompressed(); // not of proper length: 75168
		skipCompressed(); // byte grid: all ff's (counterpart of marble grid in sav?
		skipCompressed(); // 36 bytes
		in->seekg(144, ios::cur);
		scrub = readCompressedByteGrid();
	}
	
	// Extra sanity check though it should be ok by now
	if (mapsize > MAX_MAPSIZE) {
		throw "Invalid map size";
	}
	
	// Transform it to something useful
	colours = new ZeusColours(); // all climates have the same minimap colours
	PNGImage *img = new PNGImage(mapsize, mapsize);
	int half = MAX_MAPSIZE / 2;
	int border = (MAX_MAPSIZE - mapsize) / 2;
	int max = border + mapsize;
	int c1, c2;
	int coords[2];
	int start, end;
	unsigned char t_random, t_meadow, t_scrub, t_marble;
	unsigned int t_terrain;
	
	for (int y = border; y < max; y++) {
		start = (y < half) ? (border + half - y - 1) : (border + y - half);
		end   = (y < half) ? (half + y + 1 - border) : (3*half - y - border);
		for (int x = start; x < end; x++) {
			t_terrain  = terrain->get(x, y);
			t_random = random->get(x, y);
			t_meadow = fertile->get(x, y);
			t_scrub = scrub->get(x, y);
			t_marble = (marble) ? marble->get(x, y) : 255;
			
			getTerrainColours(t_terrain, t_random, t_meadow, t_scrub, t_marble, &c1, &c2);
			
			// Set pixel colours
			getBitmapCoordinates(x-border, y-border, mapsize, &coords[0], &coords[1]);
			img->setRGB(coords[0], coords[1], c1);
			img->setRGB(coords[0]+1, coords[1], c2);
		}
	}
	delete terrain;
	delete random;
	delete fertile;
	delete scrub;
	if (marble) delete marble;
	
	if (buildings) {
		placeBuildings(img, buildings, edges, mapsize, is_poseidon);
		delete buildings;
	}
	delete edges;
	
	if (walkers) {
		// Get walkers
		int colour;
		for (int i = 0; i < MAX_WALKERS; i++) {
			// Check for not shown walkers
			if (walkers[i].type == 0x6 || walkers[i].type == 0x27 // part immigrant, tower sentry
			|| walkers[i].type == 0x2E || walkers[i].type == 0x2B) { // wall sentry, horse in ranch
				continue;
			} else if ((walkers[i].type & 0xff) == 0x43) {
				colour = colours->colour(ZeusColours::MAP_SPRITES, ZeusColours::SPRITE_GOD);
			} else if ((walkers[i].type & 0xff) == 0x44) {
				colour = colours->colour(ZeusColours::MAP_SPRITES, ZeusColours::SPRITE_MONSTER);
			} else if ((walkers[i].type & 0xff) == 0x45) {
				colour = colours->colour(ZeusColours::MAP_SPRITES, ZeusColours::SPRITE_HERO);
			} else if ((walkers[i].type >= 0x28 && walkers[i].type <= 0x2a) // enemy soldiers
			|| walkers[i].type == 0x3f || walkers[i].type == 0x40) { // enemy transport/warship
				colour = colours->colour(ZeusColours::MAP_SPRITES, ZeusColours::SPRITE_ENEMY);
			} else {
				//colour = walkers[i].type;
				colour = colours->colour(ZeusColours::MAP_SPRITES, ZeusColours::SPRITE_HUMAN);
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
* @param edges     Edge information
* @param mapsize   Total size of the image
* @param is_poseidon Whether this is Poseidon or vanilla Zeus
*/
void ZeusFile::placeBuildings(PNGImage *img, Building *buildings,
		Grid<unsigned char> *edges, int mapsize, bool is_poseidon) {
	int cid, num;
	Building *b;
	int sizeX, sizeY;
	bool reverse;
	for (int i = 0; i < MAX_BUILDINGS; i++) {
		if (buildings[i].type == 0) {
			continue;
		}
		// Figure out building colour ID
		b = &buildings[i];
		
		sizeX = sizeY = b->size;
		num = 0;
		
		if (b->type <= 0x08) {
			cid = ZeusColours::MAP_HOUSING_COMMON;
			if (is_poseidon) num = 2;
		} else if (b->type <= 0x0E) {
			cid = ZeusColours::MAP_HOUSING_ELITE;
			if (is_poseidon) num = 2;
		} else if (b->type <= 0x19) {
			cid = ZeusColours::MAP_AESTHETICS;
		} else if (b->type <= 0x2e) {
			cid = ZeusColours::MAP_HUSBANDRY;
		} else if (b->type <= 0x38) {
			cid = ZeusColours::MAP_INDUSTRY;
		} else if (b->type <= 0x49) {
			cid = ZeusColours::MAP_DISTRIBUTION;
		} else if (b->type <= 0x4c || b->type == 0x79 || b->type == 0x7c) {
			cid = ZeusColours::MAP_HYGIENE;
		} else if (b->type <= 0x52) {
			cid = ZeusColours::MAP_CULTURE;
		} else if (b->type <= 0x63 || b->type == 0x7d) {
			cid = ZeusColours::MAP_SANCTUARY;
		} else if (b->type <= 0x73) { // pyramids
			cid = ZeusColours::MAP_AESTHETICS;
		} else if (b->type == 0x75 || b->type == 0x7a || b->type == 0x7b || b->type == 0xc7) {
			cid = ZeusColours::MAP_ADMINISTRATION;
		} else if (b->type <= 0x81) {
			cid = ZeusColours::MAP_AESTHETICS;
		} else if (b->type <= 0x88 || b->type == 0xd4) {
			cid = ZeusColours::MAP_MILITARY;
		} else if (b->type <= 0x98) {
			cid = ZeusColours::MAP_AESTHETICS;
		} else if (b->type <= 0xcf) {
			cid = ZeusColours::MAP_CULTURE;
		} else if (b->type <= 0xd5) {
			cid = ZeusColours::MAP_INDUSTRY;
		} else if (b->type <= 0xd9) {
			cid = ZeusColours::MAP_HUSBANDRY;
		} else {
			//cout << "Skipping building: " << hex << (int)b->type << dec << endl;
			continue;
		}
		// Special cases
		if (b->type == 0x3f) { // common agora
			switch (b->rotation) {
				case 0: sizeX = 1; sizeY = 6; b->x += 2; break;
				case 1: sizeX = 6; sizeY = 1; break;
				case 2: sizeX = 1; sizeY = 6; break;
				case 3: sizeX = 6; sizeY = 1; b->y += 2; break;
			}
		} else if (b->type == 0x40) { // grand agora
			switch (b->rotation) {
				case 0: sizeX = 1; sizeY = 6; b->x += 2; break;
				case 1: sizeX = 6; sizeY = 1; b->y += 2; break;
			}
		} else if (b->type == 0x83) { // gatehouse
			// place a second building for the other side of the road, and
			// the road itself as well
			if (b->rotation) {
				placeBuilding(img, edges, mapsize, b->x + 3, b->y, 2, 2,
					colours->colour(cid, 0), colours->colour(cid, 1), false);
				placeBuilding(img, edges, mapsize, b->x + 2, b->y, 1, 2,
					colours->colour(cid, 0), colours->colour(cid, 1), false);
			} else {
				placeBuilding(img, edges, mapsize, b->x, b->y + 3, 2, 2,
					colours->colour(cid, 0), colours->colour(cid, 1), false);
				placeBuilding(img, edges, mapsize, b->x, b->y + 2, 2, 1,
					colours->colour(cid, 0), colours->colour(cid, 1), false);
			}
		}
		
		if (cid == ZeusColours::MAP_AESTHETICS) {
			// Reverse c1 & c2 for 1x1 buildings
			reverse = true;
		} else {
			reverse = false;
		}
		placeBuilding(img, edges, mapsize, b->x, b->y, sizeX, sizeY,
			colours->colour(cid, num), colours->colour(cid, num+1), reverse);
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
* @param reverse Whether to reverse the two colours for 1x1 buildings
*/
void ZeusFile::placeBuilding(PNGImage *img, Grid<unsigned char> *edges,
		int mapsize, int posX, int posY,
		int sizeX, int sizeY, int c1, int c2, bool reverse) {
	int coords[2];
	if (sizeX == 1 && sizeY == 1) {
		getBitmapCoordinates(posX, posY, mapsize, &coords[0], &coords[1]);
		img->setRGB(coords[0], coords[1], reverse ? c2 : c1);
		img->setRGB(coords[0]+1, coords[1], reverse ? c1 : c2);
	} else {
		int offset = (MAX_MAPSIZE - mapsize) / 2;
		for (int y = 0; y < sizeY; y++) {
			for (int x = 0; x < sizeX; x++) {
				getBitmapCoordinates(posX + x, posY + y, mapsize, &coords[0], &coords[1]);
				unsigned char edge = edges->get(offset+posX+x, offset+posY+y);
				if (edge == 64) {
					img->setRGB(coords[0], coords[1], reverse ? c2 : c1);
					img->setRGB(coords[0]+1, coords[1], reverse ? c1 : c2);
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
void ZeusFile::getTerrainColours(unsigned int terrain, unsigned char random,
		unsigned char meadow, unsigned char scrub, unsigned char marble,
		int *c1, int *c2) {
	if (terrain & 0x80000) {
		*c1 = *c2 = colours->colour(ZeusColours::MAP_BACKGROUND, 0);
		return;
	}
	
	int num3 = random & 3;
	if (terrain & 0x1) { // tree/shrub
		*c1 = colours->colour(ZeusColours::MAP_TREE1, num3);
		*c2 = colours->colour(ZeusColours::MAP_TREE2, num3);
	} else if (terrain & 0x2) { // rock or ore-bearing rock
		if ((terrain & 0x300002) == 0x100002) { // copper ore
			*c1 = colours->colour(ZeusColours::MAP_COPPER1, num3);
			*c2 = colours->colour(ZeusColours::MAP_COPPER2, num3);
		} else if ((terrain & 0x300002) == 0x200002) { // silver ore
			*c1 = colours->colour(ZeusColours::MAP_SILVER1, num3);
			*c2 = colours->colour(ZeusColours::MAP_SILVER2, num3);
		} else { // normal (0x2) or cliff rock (0x300002) 
			// or black marble quarry (0x120000) ??
			*c1 = colours->colour(ZeusColours::MAP_ROCK1, num3);
			*c2 = colours->colour(ZeusColours::MAP_ROCK2, num3);
		}
	} else if (terrain & 0x10000000 && (!(terrain & 0x8) || terrain & 0x40)) { // sanctuary or pyramid
		*c1 = colours->colour(ZeusColours::MAP_SANCTUARY, 2);
		*c2 = colours->colour(ZeusColours::MAP_SANCTUARY, 3);
	} else if (terrain & 0x8) { // building, fill in for boulevard or avenue
		*c1 = colours->colour(ZeusColours::MAP_AESTHETICS, 1);
		*c2 = colours->colour(ZeusColours::MAP_AESTHETICS, 0);
	} else if (terrain & 0x20) { // park
		*c1 = colours->colour(ZeusColours::MAP_AESTHETICS, 4);
		*c2 = colours->colour(ZeusColours::MAP_AESTHETICS, 5);
	} else if (terrain & 0x200) { // elevation
		*c1 = colours->colour(ZeusColours::MAP_ELEVATION1, num3);
		*c2 = colours->colour(ZeusColours::MAP_ELEVATION2, num3);
	} else if (terrain & 0x40) { // road
		*c1 = colours->colour(ZeusColours::MAP_ROAD, 0);
		*c2 = colours->colour(ZeusColours::MAP_ROAD, 1);
	} else if (terrain & 0x4) { // water
		if (terrain & 0x4000000) { // deep water
			*c1 = colours->colour(ZeusColours::MAP_DEEPWATER1, num3);
			*c2 = colours->colour(ZeusColours::MAP_DEEPWATER2, num3);
		} else { // shallow water
			*c1 = colours->colour(ZeusColours::MAP_WATER1, num3);
			*c2 = colours->colour(ZeusColours::MAP_WATER2, num3);
		}
	} else if (terrain & 0x20000) { // marble quarry
		if (terrain & 0x100000) { // black marble
			if (marble == 255) {
				*c1 = colours->colour(ZeusColours::MAP_QUARRY1, 2);
				*c2 = colours->colour(ZeusColours::MAP_QUARRY2, 2);
			} else if (marble == 0x64) {
				*c1 = colours->colour(ZeusColours::MAP_QUARRY1, 3);
				*c2 = colours->colour(ZeusColours::MAP_QUARRY2, 3);
			} else { // marble == 0
				*c1 = colours->colour(ZeusColours::MAP_QUARRY1, 5);
				*c2 = colours->colour(ZeusColours::MAP_QUARRY2, 5);
			}
		} else { // normal marble
			if (marble == 255) {
				*c1 = colours->colour(ZeusColours::MAP_QUARRY1, num3 & 1);
				*c2 = colours->colour(ZeusColours::MAP_QUARRY2, num3 & 1);
			} else if (marble == 0x64) {
				*c1 = colours->colour(ZeusColours::MAP_QUARRY1, 2 + (num3 & 1));
				*c2 = colours->colour(ZeusColours::MAP_QUARRY2, 2 + (num3 & 1));
			} else {
				*c1 = colours->colour(ZeusColours::MAP_QUARRY1, 4 + (num3 & 1));
				*c2 = colours->colour(ZeusColours::MAP_QUARRY2, 4 + (num3 & 1));
			}
		}
	} else if ((terrain & 0x300000) == 0x300000) { // orichalc
		*c1 = colours->colour(ZeusColours::MAP_ORICHALC1, num3);
		*c2 = colours->colour(ZeusColours::MAP_ORICHALC2, num3);
	} else if (terrain & 0x4000) { // wall
		*c1 = colours->colour(ZeusColours::MAP_WALL, 0);
		*c2 = colours->colour(ZeusColours::MAP_WALL, 1);
	} else if (terrain & 0x800) { // meadow
		meadow >>= 5;
		*c1 = colours->colour(ZeusColours::MAP_FERTILE1, meadow);
		*c2 = colours->colour(ZeusColours::MAP_FERTILE2, meadow);
	} else if (terrain & 0x80) { // beach / beach edge / scrub
		if (terrain & 0x10000) { // beach sand
			*c1 = colours->colour(ZeusColours::MAP_BEACH1, random & 7);
			*c2 = colours->colour(ZeusColours::MAP_BEACH2, random & 7);
		} else { // beach edge OR scrub
			if (scrub <= 0x18) {
				*c1 = colours->colour(ZeusColours::MAP_BEACH_EDGE1, random & 1);
				*c2 = colours->colour(ZeusColours::MAP_BEACH_EDGE2, random & 1);
			} else if (scrub <= 0x38) {
				*c1 = colours->colour(ZeusColours::MAP_BEACH_EDGE1, 1);
				*c2 = colours->colour(ZeusColours::MAP_BEACH_EDGE2, 1);
			} else if (scrub <= 0x48) {
				*c1 = colours->colour(ZeusColours::MAP_BEACH_EDGE1, (random & 1) + 1);
				*c2 = colours->colour(ZeusColours::MAP_BEACH_EDGE2, (random & 1) + 1);
			} else if (scrub <= 0x50) {
				*c1 = colours->colour(ZeusColours::MAP_BEACH_EDGE1, (random & 1) + 2);
				*c2 = colours->colour(ZeusColours::MAP_BEACH_EDGE2, (random & 1) + 2);
			} else {
				*c1 = colours->colour(ZeusColours::MAP_BEACH_EDGE1, (random & 1) + 3);
				*c2 = colours->colour(ZeusColours::MAP_BEACH_EDGE2, (random & 1) + 3);
			}
		}
	} else if (terrain & 0x40000) { // marshland
		*c1 = colours->colour(ZeusColours::MAP_MARSH, (random & 4) >> 2);
		*c2 = colours->colour(ZeusColours::MAP_MARSH, ((random & 4) >> 2) + 2);
	} else if (terrain & 0x1000000) { // molten lava
		*c1 = colours->colour(ZeusColours::MAP_LAVA, (random % 2));
		*c2 = colours->colour(ZeusColours::MAP_LAVA, (random % 2) + 2);
	} else { // empty land
		*c1 = colours->colour(ZeusColours::MAP_EMPTY1, (random >> 1) % 4);
		*c2 = colours->colour(ZeusColours::MAP_EMPTY2, (random >> 1) % 4);
	}
}

/**
* Reads a compressed byte grid from the stream. Compressed chunks
* consist of a length followed by a compressed chunk of that length
*/
Grid<unsigned char> *ZeusFile::readCompressedByteGrid() {
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
Grid<unsigned short> *ZeusFile::readCompressedShortGrid() {
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
Grid<unsigned int> *ZeusFile::readCompressedIntGrid() {
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
Grid<unsigned char> *ZeusFile::readByteGrid() {
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
Grid<unsigned short> *ZeusFile::readShortGrid() {
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
Grid<unsigned int> *ZeusFile::readIntGrid() {
	Grid<unsigned int> *g = new Grid<unsigned int>(MAX_MAPSIZE, MAX_MAPSIZE);
	
	for (int y = 0; y < MAX_MAPSIZE; y++) {
		for (int x = 0; x < MAX_MAPSIZE; x++) {
			g->set(x, y, readInt());
		}
	}
	return g;
}

/**
* Gets the walker info from the saved game, which is buried after
* a few "useless" compressed chunks.
*/
Walker * ZeusFile::getWalkers() {
	// Assume random data has been read already
	// Which is followed by some more compressed data blocks (appeal + zeroesx3 + possibly fire/damage)
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
Building * ZeusFile::getBuildings() {
	Building *buildings = new Building[MAX_BUILDINGS];
	int length = readInt();
	try {
		PKWareInputStream pk(in, false, length);
		
		// Building entry = 280 bytes
		for (int i = 0; i < MAX_BUILDINGS; i++) {
			pk.skip(3);
			buildings[i].size = pk.readByte();
			pk.skip(2);
			buildings[i].x = pk.readShort();
			buildings[i].y = pk.readShort();
			pk.skip(6);
			buildings[i].type = pk.readShort();
			pk.skip(154);
			buildings[i].rotation = pk.readByte();
			pk.skip(107);
		}
		pk.empty();
	} catch (PKException) {
		delete buildings;
		throw;
	}
	return buildings;
}

/**
* Gets the map size from the saved game, which is
* buried deep into the game file
*/
int ZeusFile::getMapsize() {
	// Assume walkers have been read already
	// Three compressed blocks
	for (int i = 0; i < 3; i++) {
		skipCompressed();
	}
	
	// Three ints
	in->seekg(12, ios::cur);
	
	// Compressed block, followed by 72 bytes, followed by compressed
	skipCompressed();
	in->seekg(72, ios::cur);
	skipCompressed();
	
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
void ZeusFile::skipCompressed() {
	int skip = readInt();
	in->seekg(skip, ios::cur);
}

/**
* Reads an integer from the stream
*/
unsigned int ZeusFile::readInt() {
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
unsigned short ZeusFile::readShort() {
	char data[2];
	in->read(data, 2);
	return (unsigned short)(data[0] + (data[1] << 8));
}

/**
* Returns the bitmap coordinates for a given map coordinate
*/
void ZeusFile::getBitmapCoordinates(int x, int y, int mapsize, int *x_out, int *y_out) {
	*x_out = mapsize / 2 + x - y - 1;
	*y_out = 1 + x + y - mapsize / 2;
}

/**
* Searches the stream for a pattern. Returns true if the pattern has been
* found; the file pointer is set to the first byte *after* the pattern.
* Returns false if the pattern wasn't found.
* @param pattern Pattern to search for
* @param length Pattern length
*/
bool ZeusFile::searchPattern(char pattern[], int length) {
	bool found = false;
	int position = 0;
	char b;
	
	do {
		in->read(&b, 1);
		if (b == pattern[0]) {
			// Found first match, mark input stream for returning
			position = in->tellg();
			position++;
			
			// Try to match the rest
			found = true;
			for (int i = 1; i < length; i++) {
				in->read(&b, 1);
				if (b != pattern[i]) {
					// Byte doesn't match, reset fp and try again
					in->seekg(position, ios::beg);
					found = false;
					break;
				}
			}
		}
	} while (!found && in->good());
	return found;
}

int main(int argc, char **argv) {
	if (argc != 3) {
		cerr << "Usage: "<<argv[0] << " [zeus file] [png output file]" << endl;
		return 1;
	}
	try {
		ZeusFile *zf = new ZeusFile(string(argv[1]));
		int numMaps = zf->getNumMaps();
		
		if (zf->isAdventure()) {
			for (int i = 0; i < numMaps; i++) {
				PNGImage *img = zf->getImage();
				if (img) {
					if (i) {
						// Colony, add "Ci" before extension
						string filename(argv[2]);
						unsigned int pos = filename.find_last_of('.');
						char colony[4];
						sprintf(colony, "C%d", i);
						if (pos == string::npos) {
							filename.append(colony);
						} else {
							filename.insert(pos, colony);
						}
						img->write(filename.c_str());
					} else {
						// Parent city
						img->write(argv[2]);
					}
					delete img;
				}
			}
		} else {
			PNGImage *img = zf->getImage();
			
			if (img) {
				img->write(argv[2]);
				delete img;
			}
		}
		delete zf;
	} catch (...) {
		cerr << "Couldn't process file. Make sure it's a valid Zeus file." << endl;
		return 2;
	}
	return 0;
}
