#include "filters.h"
#include <algorithm>


abstract_filter::abstract_filter(image_data& imgData) {
	imgCopy.compPerPixel = imgData.compPerPixel;
	imgCopy.w = imgData.w;
	imgCopy.h = imgData.h;
	imgCopy.pixels = new stbi_uc[imgCopy.h * imgCopy.w * imgCopy.compPerPixel];
	for (int x = 0; x < imgData.w; x++) {
		for (int y = 0; y < imgData.h; y++) {
			for (int d = 0; d < imgData.compPerPixel; ++d) {
				imgCopy.pixels[(x + y * imgData.w) * imgData.compPerPixel + d] = imgData.pixels[(x + y * imgData.w) * imgData.compPerPixel + d];
			}
		}
	}
}

void abstract_filter::Assign(const image_data& imgData, rectangle rect) {
	int w = imgCopy.w;
	int comp = imgCopy.compPerPixel;
	int max_comp = 3;
	for (int x = rect.a; x < rect.c; x++) {
		for (int y = rect.b; y < rect.d; y++) {
			for (int d = 0; d < max_comp; ++d) {
				imgData.pixels[(x + y * w) * comp + d] = imgCopy.pixels[(x + y * w) * comp + d];
			}
		}
	}
}

abstract_filter::~abstract_filter() {
	delete[] imgCopy.pixels;
}

int abstract_filter::ToBlackWhite(int x, int y, image_data& imgData) {
	return (int)(
		0.3 * (double)imgData.pixels[(x + y * imgData.w) * imgData.compPerPixel + 0]
		+ 0.6 * (double)imgData.pixels[(x + y * imgData.w) * imgData.compPerPixel + 1]
		+ 0.1 * (double)imgData.pixels[(x + y * imgData.w) * imgData.compPerPixel + 2]);
}

image_data&& abstract_filter::get_image() {
	return std::move(imgCopy);
}

convolutional_filter::~convolutional_filter()
{
	ker.ker_matrix.clear();
}

Threshold::Threshold(image_data& imgData, int sizeOfKernel) : convolutional_filter(imgData) {
	ker.size = sizeOfKernel;
}

Blur::Blur(image_data& imgData, int sizeOfKernel) : convolutional_filter(imgData) {
	ker.size = sizeOfKernel;
	double sq = ker.size * ker.size;
	for (int i = 0; i < sq; ++i) {
		ker.ker_matrix.push_back(1 / sq);
	}
}

Edge::Edge(image_data& imgData, int sizeOfKernel) : convolutional_filter(imgData) {
	ker.size = sizeOfKernel;
	const int sq = ker.size * ker.size;
	for (int i = 0; i < sq; ++i) {
		if (i != sq / 2)
			ker.ker_matrix.push_back(-1);
		else
			ker.ker_matrix.push_back(sq);
	}
}

void Red::apply(rectangle rect, image_data& imgData) {
	int comp = imgCopy.compPerPixel;
	int max_comp = QUAN_OF_COLORS;
	for (int x = rect.a; x < rect.c; x++) {
		for (int y = rect.b; y < rect.d; y++) {
			imgData.pixels[(x + y * imgData.w) * comp] = 255;
			for (int color = 1; color < max_comp; color++) {
				imgData.pixels[(x + y * imgData.w) * comp + color] = 0;
			}
		}
	}
}

void Blur::apply(rectangle rect, image_data& imgData) {
	int h = imgCopy.h, w = imgCopy.w;
	int comp = imgCopy.compPerPixel;
	int max_comp = QUAN_OF_COLORS;

	for (int depth = 0; depth < max_comp; depth++) {
		for (int x = rect.a + ker.size / 2; x < rect.c - ker.size / 2; x++) {
			for (int y = rect.b + ker.size / 2; y < rect.d - ker.size / 2; y++) {
				int point = (x + y * w) * comp + depth;
				int sum = 0;
				int p = 0;
				for (int s_i = -ker.size / 2; s_i <= ker.size / 2; ++s_i) {
					for (int s_j = -ker.size / 2; s_j <= ker.size / 2; ++s_j) {
						int x_pos = x + s_i;
						int y_pos = y + s_j;
						if (!(x_pos < 0 || x_pos >= w || y_pos < 0 || y_pos >= h))
							sum += (int)(imgCopy.pixels[(x_pos + y_pos * w) * comp + depth] * ker.ker_matrix[p]);
						p++;
					}
				}
				imgData.pixels[point] = sum;
			}
		}
	}
	//Assign(imgData, rect);
}

int Threshold::find_median(int x, int y, image_data& imgData) {
	int w = imgCopy.w, h = imgCopy.h;
	int res;
	for (int s_i = -ker.size / 2; s_i <= ker.size / 2; ++s_i) {
		for (int s_j = -ker.size / 2; s_j <= ker.size / 2; ++s_j) {
			int x_pos = x + s_i;
			int y_pos = y + s_j;
			if (!(x_pos < 0 || x_pos >= w || y_pos < 0 || y_pos >= h)) {
				ker.ker_matrix.push_back(ToBlackWhite(x_pos, y_pos, imgCopy));
			}
			else {
				
			}
		}
	}
	std::sort(ker.ker_matrix.begin(), ker.ker_matrix.end());
	res = ker.ker_matrix[ker.ker_matrix.size() / 2];
	ker.ker_matrix.clear();
	//[](const void* x1, const void* x2) ->int { return (*(int*)x2 - *(int*)x1); };
	return res;
}

void Threshold::zero_below_median(int x, int y, int med, image_data& imgData) {
	int w = imgCopy.w, h = imgCopy.h;
	int comp = imgCopy.compPerPixel;
	int max_comp = QUAN_OF_COLORS;
	int max_count = ker.ker_matrix.size() / 2;
	int counter = 0;
	for (int s_i = -ker.size / 2; s_i <= ker.size / 2; ++s_i) {
		for (int s_j = -ker.size / 2; s_j <= ker.size / 2; ++s_j) {
			int x_pos = x + s_i;
			int y_pos = y + s_j;
			if (!(x_pos < 0 || x_pos >= w || y_pos < 0 || y_pos >= h)) {
				int bw = ToBlackWhite(x_pos, y_pos, imgCopy);
				if (bw < med) {
					for (int d = 0; d < max_comp; d++) {
						imgData.pixels[(x_pos + y_pos * w) * comp + d] = 0;
					}
					counter++;
				}
				else {
					for (int d = 0; d < max_comp; d++) {
						imgData.pixels[(x_pos + y_pos * w) * comp + d] = bw;
					}
				}
			}
		}
	}
	if (counter < max_count) {
		for (int s_i = -ker.size / 2; s_i <= ker.size / 2; ++s_i) {
			for (int s_j = -ker.size / 2; s_j <= ker.size / 2; ++s_j) {
				int x_pos = x + s_i;
				int y_pos = y + s_j;
				if (counter == max_count)
					return;
				if (!(x_pos < 0 || x_pos >= w || y_pos < 0 || y_pos >= h)) {
					if (imgCopy.pixels[(x_pos + y_pos * w) * comp] == med) {
						for (int d = 0; d < max_comp; d++) {
							imgData.pixels[(x_pos + y_pos * w) * comp + d] = 0;
						}
						counter++;
					}
				}
			}
		}
	}
}

void Threshold::apply(rectangle rect, image_data& imgData) {
	for (int x = rect.a + ker.size / 2; x < rect.c + ker.size / 2; x+=ker.size) {
		for (int y = rect.b + ker.size / 2; y < rect.d + ker.size / 2; y+=ker.size) {
			zero_below_median(x, y, find_median(x, y, imgData), imgData);
		}
	}
}

void Edge::apply(rectangle rect, image_data& imgData) {
	int w = imgCopy.w, h = imgCopy.h;
	int comp = imgCopy.compPerPixel;
	int max_comp = QUAN_OF_COLORS;
	for (int y = rect.b; y < rect.d; y++) {
		for (int x = rect.a; x < rect.c; x++) {
			int point = (x + y * w) * comp;
			int sum = 0;
			int p = 0;
			for (int s_j = -ker.size / 2; s_j <= ker.size / 2; ++s_j) {
				for (int s_i = -ker.size / 2; s_i <= ker.size / 2; ++s_i) {
					int x_pos = x + s_i;
					int y_pos = y + s_j;
					if (!(x_pos < 0 || x_pos >= w || y_pos < 0 || y_pos >= h))
						sum += ker.ker_matrix[p] * ToBlackWhite(x_pos, y_pos, imgCopy);
					p++;
				}
			}
			for (int d = 0; d < max_comp; ++d)
				imgData.pixels[point + d] = sum;
		}
	}
}