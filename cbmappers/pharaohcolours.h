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
#ifndef pharaohcolours_h
#define pharaohcolours_h

class PharaohColours {
	public:
		enum {
			MAP_BACKGROUND = 0,
			MAP_EMPTY1,
			MAP_EMPTY2,
			MAP_TREE1,
			MAP_TREE2,
			MAP_ROCK1,
			MAP_ROCK2,
			MAP_WATER1,
			MAP_WATER2,
			MAP_FERTILE1,
			MAP_FERTILE2,
			MAP_DUNE1,
			MAP_DUNE2,
			MAP_MARSH,
			MAP_ROAD,
			MAP_WALL,
			MAP_FOOD, //16
			MAP_INDUSTRY, // 17
			MAP_MILITARY, // 18
			MAP_RELIGION, // 19
			MAP_EDUCATION, // 20
			MAP_HEALTH, // 21
			MAP_ENTERTAINMENT, // 22
			MAP_GOVERNMENT, // 23
			MAP_SAFETY, // 24
			MAP_MONUMENTS, // 25
			MAP_AESTHETICS, // 26
			MAP_HOUSING, // 27
			MAP_SPRITES,
			MAP_SIZE, /* MUST BE LAST */
		};
		
		const static int
			SPRITE_ANIMAL    = 0,
			SPRITE_SOLDIER   = 1,
			SPRITE_SHIP      = 1,
			SPRITE_ENEMY     = 2;
	private:
		int cmap[MAP_SIZE][8];
		
	public:
		PharaohColours();
		int colour(int type, int number);
		int map(int building_id);
};

#endif /* pharaohcolours_h */
