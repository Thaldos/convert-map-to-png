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
#ifndef zeuscolours_h
#define zeuscolours_h

class ZeusColours {
	public:
		enum {
			MAP_BACKGROUND = 0,
			MAP_EMPTY1,     MAP_EMPTY2,
			MAP_TREE1,      MAP_TREE2,
			MAP_ROCK1,      MAP_ROCK2,
			MAP_COPPER1,    MAP_COPPER2,
			MAP_SILVER1,    MAP_SILVER2,
			MAP_ORICHALC1,  MAP_ORICHALC2,
			MAP_QUARRY1,    MAP_QUARRY2,
			MAP_ELEVATION1, MAP_ELEVATION2,
			MAP_WATER1,     MAP_WATER2,
			MAP_DEEPWATER1, MAP_DEEPWATER2,
			MAP_BEACH1,     MAP_BEACH2,
			MAP_BEACH_EDGE1,MAP_BEACH_EDGE2,
			MAP_MARSH,      MAP_LAVA,
			MAP_FERTILE1,   MAP_FERTILE2,
			MAP_ROAD,       MAP_WALL,
			MAP_HOUSING_COMMON,
			MAP_HOUSING_ELITE,
			MAP_HUSBANDRY,
			MAP_INDUSTRY,
			MAP_DISTRIBUTION,
			MAP_HYGIENE,
			MAP_ADMINISTRATION,
			MAP_CULTURE,
			MAP_SANCTUARY,
			MAP_MILITARY,
			MAP_AESTHETICS,
			MAP_SPRITES,
			MAP_SIZE // SHOULD ALWAYS BE THE LAST
		};
		
		const static int
			SPRITE_HUMAN   = 0,
			SPRITE_SOLDIER = 1,
			SPRITE_ENEMY   = 2,
			SPRITE_GOD     = 3,
			SPRITE_MONSTER = 4,
			SPRITE_HERO    = 5;
	private:
		int map[MAP_SIZE][8];
		
	public:
		ZeusColours();
		int colour(int type, int number);
};

#endif /* zeuscolours_h */
