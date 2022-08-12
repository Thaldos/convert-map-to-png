/*
 *   CBMappers - create minimaps from Citybuilder scenarios and saved games
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
#ifndef pngimage_h
#define pngimage_h

#include <string>
#include <set>

class PNGImage {
	public:
		PNGImage(int width, int height, int bitdepth = 8);
		~PNGImage();
		void setRGB(int x, int y, int color);
		void setRGB(int x, int y, int r, int g, int b);
		bool write(std::string filename);
	private:
		int width;
		int height;
		int bitdepth;
		int **image;
		std::set<int> colours;
};

#endif /* pngimage_h */
