#include "pngimage.h"
#include <png.h>
#include <iostream>
#include <map>
using namespace std;

PNGImage::PNGImage(int width, int height, int bitdepth) {
	if (height <= 0 || width <= 0) {
		throw "Invalid size";
	}
	this->width = width;
	this->height = height;
	this->bitdepth = bitdepth;
	if (bitdepth != 8) {
		throw "Unsupported bitdepth";
	}
	image = new int*[height];
	for (int i = 0; i < height; i++) {
		image[i] = new int[width * sizeof(int)];
		memset(image[i], 0, width * sizeof(int));
	}
	colours.insert(0);
}

PNGImage::~PNGImage() {
	for (int i = 0; i < height; i++) {
		delete image[i];
	}
	delete image;
}

void PNGImage::setRGB(int x, int y, int color) {
	if (x < 0 || x >= width || y < 0 || y >= height) {
		return;
	}
	
	colours.insert(color);
	image[y][x] = color;
}

void PNGImage::setRGB(int x, int y, int r, int g, int b) {
	
	setRGB(x, y, (r << 16) | (g << 8) | b);
	
	return;
}

bool PNGImage::write(std::string filename) {
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	
	fp = fopen(filename.c_str(), "wb");
	if (fp == NULL) {
		std::cerr << "PNGImage::write: fopen() returned NULL" << std::endl;
		return false;
	}
	
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info_ptr = png_create_info_struct(png_ptr);
	png_init_io(png_ptr, fp);
	png_set_compression_level(png_ptr, 6); // 6 == default compression
	
	png_set_IHDR(png_ptr, info_ptr, this->width, this->height,
		this->bitdepth, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
	// Fix up palette
	int num_colours = colours.size();
	png_color pal[num_colours];
	map<int, int> index;
	set<int>::iterator si;
	
	int i = 0;
	for (si = colours.begin(); si != colours.end(); si++) {
		index[*si] = i;
		pal[i].red = *si >> 16;
		pal[i].green = (*si >> 8) & 0xff;
		pal[i].blue = *si & 0xff;
		i++;
	}
	png_bytepp data = new png_bytep[height];
	for (int y = 0; y < height; y++) {
		data[y] = new png_byte[width];
		for (int x = 0; x < width; x++) {
			data[y][x] = (unsigned char) (index[image[y][x]]);
		}
	}
	png_set_PLTE(png_ptr, info_ptr, pal, num_colours);
	
	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, data);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	
	for (int y = 0; y < height; y++) {
		delete data[y];
	}
	delete data;
	
	return true;
}
