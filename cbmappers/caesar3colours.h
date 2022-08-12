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
#ifndef caesar3colours_h
#define caesar3colours_h

/**
* Provides the colours for the minimap images
*/
class Caesar3Colours {
	public:
		/**
		* Constants for the climate - the values are the same as
		* used inside the Caesar 3 files
		*/
		const static int
			CLIMATE_CENTRAL = 0,
			CLIMATE_NORTHERN = 1,
			CLIMATE_DESERT = 2;
		
		/**
		* Constants for the different terrain and building elements
		*/
		const static int
			MAP_BACKGROUND = 0,
			MAP_EMPTY1     = 1,
			MAP_EMPTY2     = 2,
			MAP_TREE1      = 3,
			MAP_TREE2      = 4,
			MAP_ROCK1      = 5,
			MAP_ROCK2      = 6,
			MAP_WATER1     = 7,
			MAP_WATER2     = 8,
			MAP_FERTILE1   = 9,
			MAP_FERTILE2   = 10,
			MAP_ROAD       = 11,
			MAP_WALL       = 12,
			MAP_AQUA       = 13,
			MAP_HOUSE      = 14,
			MAP_BUILDING   = 15,
			MAP_SPRITES    = 16,
			MAP_SIZE       = 17;
		
		/**
		* Constants for the different walkers / sprites
		*/
		const static int
			SPRITE_WOLF      = 0,
			SPRITE_SOLDIER   = 1,
			SPRITE_BARBARIAN = 2,
			SPRITE_ENEMY     = 3;
	private:
		int map[MAP_SIZE][8];
		
	public:
		/**
		* Constructor - creates a new colour set with the given climate
		* @param climate - one of the CLIMATE_* constants
		*/
		Caesar3Colours(int climate);
		
		/**
		* Retrieves a specified colour from this set.
		* @param type - one of the MAP_* constants
		* @param number - 0-7, or one of the SPRITE_* constants
		* @return int representing a colour
		*/
		int colour(int type, int number);
};

#endif /* caesar3colours_h */
