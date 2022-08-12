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
#include "pharaohcolours.h"

PharaohColours::PharaohColours() {
	int colours[][8] = {{0x0000ff}, // background
		{0xDEB27B, 0xCEB273, 0xEFC38C, 0xCEAA73, 0xD6AA63, 0xEFB273, 0xC6AA73, 0xEFBA7B}, // empty 1
		{0xCEA263, 0xD6BA84, 0xCEAA7B, 0xEFC38C, 0xD6AA73, 0xEFBA8C, 0xDEBA84, 0xE7B28C}, // empty 2
		{0x5A9229, 0x213808, 0x005929, 0x084100}, // tree 1
		{0x4A6129, 0x8C8231, 0x314918, 0x427118}, // tree 2
		{0xCEA29C, 0xA5827B, 0xCEAA9C, 0x947973}, // rock 1
		{0x846163, 0xBD927B, 0xA5827B, 0xC69A84}, // rock 2
		{0x396163, 0x31595A, 0x31595A, 0x315963, 0x84BAFF}, // water 1 + irrigation
		{0x31595A, 0x396163, 0x396163, 0x31595A, 0x5282BD}, // water 2 + irrigation
		{0x94924A, 0x738A31, 0xAD9231, 0x848218}, // fertile 1
		{0x637931, 0xAD9221, 0x6B7131, 0xA58A42}, // fertile 2
		{0xF7D3AD, 0xF7DBAD, 0xE7C39C, 0xEFCBA5}, // sand dune 1
		{0xDEBA94, 0xF7D3A5, 0xEFCBA5, 0xF7D3A5}, // sand dune 2
		{0x5A9229, 0x42716B, 0x4A6129, 0x5A8A73}, // marsh 1 & 2
		{0xDECBBD, 0xCEC3B5, 0x000000}, // road & roadblock
		{0x0, 0x0}, // wall
		// Buildings:
		{0x6BB200, 0x7BC300}, // food
		{0xAD0000, 0xC60000}, // industry
		{0xFF8A18, 0xFFA242}, // military
		{0x6300C6, 0x6300C6}, // religion
		{0xFFDB00, 0xFFF394}, // education
		{0xFFFFFF, 0xFFFFFF}, // health
		{0x08CBB5, 0x29FBE7}, // entertainment
		{0xA569CE, 0xA569CE}, // government
		{0x0059BD, 0x0069E7}, // safety / ferry
		{0x525152, 0x636163}, // monuments
		{0x008294, 0x73BAB5, 0x088A9C, 0x429A84}, // aesthetics
		{0xF7D38C, 0xFFFBD6, 0xFFF3BD, 0xFFE3A5}, // housing
		{0x0, 0xF70000, 0x1800FF}, // sprites
	};
	for (int i = 0; i < MAP_SIZE; i++) {
		for (int j = 0; j < 8; j++) {
			cmap[i][j] = colours[i][j];
		}
	}
}

int PharaohColours::colour(int type, int number) {
	return cmap[type][number];
}

int PharaohColours::map(int building_id) {
	// NOTE: these are hard-coded values
	static int buildingmap[256] = {
	//	 0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 27, 27, 27, 27, 27, 27, // 0
		27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 22, 22, // 1
		22, 22, 22, 22, 22, 22,  0,  0,  0, 26, 26, 26,  0,  0, 21, 21, // 2
		 0, 21,  0, 20,  0, 20, 18, 24,  0, 18,  0,  0, 19, 19, 19, 19, // 3
		19, 19, 19, 19, 19, 19, 16, 16, 17, 17, 17, 17, 16, 23, 23, 23, // 4
		 0, 24,  0,  0,  0,  0, 23,  0,  0,  0,  1,  0, 26,  0, 18, 18, // 5
		 0,  0,  0, 15, 17, 17, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, // 6
		18, 17, 17, 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 7
		 0,  0,  0,  0,  0,  0,  0,  0, 24,  0, 15,  0, 19, 19, 19, 19, // 8
		19,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 9
		 0, 17, 17,  0,  0,  0,  0, 24,  0,  0,  0,  0,  0, 18,  0,  0, // a
		 0, 17, 17, 17, 21, 18, 18, 25, 23,  0,  0, 23, 23, 23,  0,  0, // b
		 0,  0, 16, 17, 16,  0,  0, 16,  0,  0, 18, 17, 17, 18, 21, 25, // c
		25, 19, 23,  0,  0, 25,  0, 25, 17, 17,  0,  0,  0, 17, 25,  0, // d
		17,  0, 22, 25, 25, 25, 25, 17, 17, 17, 25, 25, 25,  0,  0,  0, // e
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // f
	};
	
	if (building_id > 256) {
		return 0;
	}
	return buildingmap[building_id];
}
