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
#include "zeuscolours.h"

ZeusColours::ZeusColours() {
	int colours[][8] = {{0x0000ff}, // background
		{0xCEAA5A, 0xEFC38C, 0xD6AA63, 0xE7C363}, // empty1
		{0xCEAA52, 0xCEAA7B, 0xD6AA73, 0xC6AA52}, // empty2
		
		{0x395121, 0x4A6921, 0x6B9A42, 0x426918}, // tree 1
		{0x6B9231, 0x5A8229, 0x394921, 0x395921}, // tree 2
		{0xA5B2C6, 0x7B92A5, 0xA5B2C6, 0x6B829C}, // rock 1
		{0x5A718C, 0x8CA2B5, 0x7B92A5, 0x94A2B5}, // rock 2
		
		{0xDE7108, 0xE77108, 0xCE6908, 0xDE7108}, // copper ore 1
		{0xCE6908, 0xCE6908, 0xDE7108, 0xCE6908}, // copper ore 2
		{0x63CBD6, 0x63D3DE, 0x5AB2C6, 0x63CBD6}, // silver ore 1
		{0x5AB2BD, 0x5AB2BD, 0x63CBD6, 0x5AB2C6}, // silver ore 2
		{0xEF0029, 0xEF0029, 0xCE0039, 0xEF0029}, // orichalc ore 1
		{0xCE0039, 0xCE0039, 0xEF0029, 0xCE0039}, // orichalc ore 2
		{0x8C9294, 0x63717B, 0x4A595A, 0x5A6163, 0x5A6163, 0x39494A}, // quarry 1 + black marble
		{0x73797B, 0x7B8A8C, 0x63716B, 0x425152, 0x425152, 0x52595A}, // quarry 2 + black marble
		{0xA5AAAD, 0x8C9A9C, 0xA5B2B5, 0x849294}, // elevation 1
		{0x7B8A8C, 0x94A2A5, 0x8C9A9C, 0x9CA2A5}, // elevation 2
		
		{0x317984, 0x317984, 0x317984, 0x316973}, // water 1
		{0x316973, 0x316973, 0x316973, 0x317984}, // water 2
		{0x18696B, 0x186973, 0x105963, 0x18696B}, // deep water 1
		{0x105963, 0x10595A, 0x18696B, 0x105963}, // deep water 2
		
		{0xF7D3AD, 0xF7DBAD, 0xE7C39C, 0xEFCBA5, 0xF7D3A5, 0xE7C39C, 0xEFCBA5, 0xEFCBA5}, // beach 1
		{0xDEBA94, 0xF7D3A5, 0xEFCBA5, 0xF7D3A5, 0xF7CBA5, 0xF7D3A5, 0xE7CBA5, 0xEFC39C}, // beach 2
		// 00-18     00-48     40-50     50-60      60
		{0xC6AA52, 0x9C8A39, 0xAD924A, 0x948239, 0x948239}, // beach edge & scrub 1
		{0xC6A24A, 0xB59A4A, 0x948242, 0x9C8A39, 0x847931}, // beach edge & scrub 2
		
		{0x395121, 0x42716B, 0x6B9231, 0x5A8A73}, // marsh 1 & 2
		{0x6B595A, 0x636163, 0x5A494A, 0x525152}, // lava 1 & 2
		{0xB59263, 0xA5716B, 0x946984, 0x7B517B}, // fertile 1
		{0xB58A5A, 0x9C7173, 0x846163, 0x7B5973}, // fertile 2
		{0xDECBBD, 0xCEC3B5}, // road
		{0x000000, 0x000000}, // wall
		{0xA54131, 0xCE694A, 0x39927B, 0x52C3AD}, // common housing + poseidon
		{0xCE2808, 0xEF4110, 0x9C18BD, 0xBD28D6}, // elite housing + poseidon
		{0x7B7131, 0x8C8A39}, // husbandry
		{0x9C716B, 0xBD8A84}, // industry
		{0x73BAF7, 0xEFF3FF}, // distribution
		{0x425994, 0x7392CE}, // hygiene, safety
		{0xE7A200, 0xFFDB00}, // administration
		{0x218273, 0x5AE3D6}, // culture
		{0xCE3021, 0xFF5931, 0x636163, 0x525152}, // sanctuaries, including stairs
		{0x424142, 0x73716B}, // military
		{0xD6D3D6, 0xEFEBEF, 0xD6D3D6, 0x0, 0x73BAB5, 0x008294}, // aesthetics + park
		{0x000000, 0xFFFF00, 0x1800FF, 0x00FB00, 0xFF0000, 0x0800FF}, // sprites
	};
	for (int i = 0; i < MAP_SIZE; i++) {
		for (int j = 0; j < 8; j++) {
			map[i][j] = colours[i][j];
		}
	}
}

int ZeusColours::colour(int type, int number) {
	return map[type][number];
}
