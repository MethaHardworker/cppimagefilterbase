#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <array>
#include <fstream>
#include <iostream>
#include "stb_image_write.h"
#include "png_toolkit.h"

png_toolkit::png_toolkit()
{
}

png_toolkit::~png_toolkit()
{
    stbi_image_free(imgData.pixels);
}

bool png_toolkit::load( const std::string &pictureName )
{
    imgData.pixels = stbi_load(pictureName.c_str(), &imgData.w, &imgData.h, &imgData.compPerPixel, 0);
    return imgData.pixels != nullptr;
}

bool png_toolkit::save( const std::string &pictureName )
{
    return stbi_write_png(pictureName.c_str(),
                   imgData.w, imgData.h,
                   imgData.compPerPixel,
                          imgData.pixels, 0) != 0;
}

image_data png_toolkit::getPixelData( void ) const
{
    return imgData;
}

void png_toolkit::Assign(stbi_uc* new_pixels, rectangle rect)
{
	int w = imgData.w;
	int comp = imgData.compPerPixel;
	int max_comp = 3;
	for (int x = rect.a; x < rect.c; ++x) {
		for (int y = rect.b; y < rect.d; y++) {
			for (int d = 0; d < max_comp; ++d) {
				imgData.pixels[(x + y * w) * comp + d] = new_pixels[(x + y * w) * comp + d];
			}
		}
	}
}

int png_toolkit::ToBlackWhite(int x, int y)
{
	return (int)(0.3 * (double)imgData.pixels[(x + y * imgData.w) * imgData.compPerPixel]
		+ 0.6 * (double)imgData.pixels[(x + y * imgData.w) * imgData.compPerPixel + 1]
		+ 0.1 * (double)imgData.pixels[(x + y * imgData.w) * imgData.compPerPixel + 2]);
}

void png_toolkit::Recolor(rectangle rect) {
	int comp = imgData.compPerPixel;
	int max_comp = 3;
	for (int x = rect.a; x < rect.c; ++x) {
		for (int y = rect.b; y < rect.d; ++y) {
			imgData.pixels[(x + y * imgData.w) * comp] = 255;
			for (int depth = 1; depth < max_comp; depth++) {
				imgData.pixels[(x + y * imgData.w) * comp + depth] = 0;
			}
		}
	}
}

void png_toolkit::Blur(rectangle rect) {
	int h = imgData.h, w = imgData.w;
	int comp = imgData.compPerPixel;
	int max_comp = 3;
	stbi_uc* new_pixels = new stbi_uc[w * h * comp];

	for (int depth = 0; depth < max_comp; depth++) {
		for (int x = rect.a; x < rect.c; x++) {
			for (int y = rect.b; y < rect.d; y++) {
				int point = (x + y * w)* comp + depth;
				int sum = 0;
				for (int s_i = -1; s_i <= 1; ++s_i) {
					for (int s_j = -1; s_j <= 1; ++s_j) {
						int x_pos = x + s_i;
						int y_pos = y + s_j;
						if (!(x_pos < 0 || x_pos >= w || y_pos < 0 || y_pos >= h))
							sum += (int)((double)imgData.pixels[(x_pos + y_pos * w) * comp + depth] / 9);
					}
				}
				new_pixels[point] = sum;
			}
		}
	}
	Assign(new_pixels, rect);
	delete[] new_pixels;
}

void png_toolkit::Threshold(rectangle rect) {
	int w = imgData.w, h = imgData.h;
	int comp = imgData.compPerPixel;
	int max_comp = 3;
	int arr[25];
	stbi_uc* new_pixels = new stbi_uc[w * h * comp];

	for (int x = rect.a; x < rect.c; x++) {
		for (int y = rect.b; y < rect.d; y++) {
			int p = 0;
			for (int s_i = -2; s_i <= 2; ++s_i) {
				for (int s_j = -2; s_j <= 2; ++s_j) {
					int x_pos = x + s_i;
					int y_pos = y + s_j;
					if (!(x_pos < 0 || x_pos >= w || y_pos < 0 || y_pos >= h)) {
						arr[p] = ToBlackWhite(x_pos, y_pos);
					}
					else {
						arr[p] = 0;
					}
					p++;
				}
			}
			qsort(arr, 25, sizeof(int), [](const void* x1, const void* x2) ->int { return (*(int*)x1 - *(int*)x2); });
			int bw = ToBlackWhite(x, y);
			if (bw > arr[25 / 2]) {
				for (int d = 0; d < max_comp; ++d)
					new_pixels[(x + y * w) * comp + d] = bw;
			}
			else {
				for (int d = 0; d < max_comp; ++d)
					new_pixels[(x + y * w) * comp + d] = 0;
			}
			new_pixels[(x + y * w) * comp + comp - 1] = imgData.pixels[(x + y * w) * comp + comp - 1];
		}
	}
	Assign(new_pixels, rect);
	delete[] new_pixels;
}

void png_toolkit::Edge(rectangle rect)
{
	int w = imgData.w, h = imgData.h;
	int comp = imgData.compPerPixel;
	int max_comp = 3;
	int arr[9] = { -1, -1, -1,
				   -1,  8, -1,
				   -1, -1, -1 };
	stbi_uc* new_pixels = new stbi_uc[w * h * comp];

	for (int x = rect.a; x < rect.c; x++) {
		for (int y = rect.b; y < rect.d; y++) {
			int point = (x + y * w) * comp;
			int sum = 0;
			int p = 0;
			for (int s_i = -1; s_i <= 1; ++s_i) {
				for (int s_j = -1; s_j <= 1; ++s_j) {
					int x_pos = x + s_i;
					int y_pos = y + s_j;
					if (!(x_pos < 0 || x_pos >= w || y_pos < 0 || y_pos >= h))
						sum += arr[p] * ToBlackWhite(x_pos, y_pos);
					p++;
				}
			}
			for(int d = 0; d < max_comp; ++d)
				new_pixels[point + d] = sum;
		}
	}
	Assign(new_pixels, rect);
	delete[] new_pixels;
}


void png_toolkit::Parse(char* filename) {
	std::ifstream f("config.txt");
	if (!f.is_open())
		return;
	char text[15];
	while (!f.eof()) {
		f >> text;
		int a, b, c, d;
		f >> b >> a >> d >> c;
		rectangle rect;
		if (a != 0)
			rect.a = imgData.w / a;
		else
			rect.a = 0;
		if (b != 0)
			rect.b = imgData.h / b;
		else
			rect.b = 0;
		if (c != 0)
			rect.c = imgData.w / c;
		else
			rect.c = 0;
		if (d != 0)
			rect.d = imgData.h / d;
		else
			rect.d = 0;
		if (strcmp(text, "Red") == 0)
			Recolor(rect);
		else if (strcmp(text, "Blur") == 0)
			Blur(rect);
		else if (strcmp(text, "Threshold") == 0)
			Threshold(rect);
		else if (strcmp(text, "Edge") == 0)
			Edge(rect);
	}
	f.close();
}
