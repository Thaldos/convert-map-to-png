#ifndef indexedpngimage_h
#define indexedpngimage_h

#include <string>
#include <set>

class IndexedPNGImage {
	public:
		IndexedPNGImage(int width, int height, int bitdepth = 8);
		~IndexedPNGImage();
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
