#include "filters.h"


abstract_filter::abstract_filter(image_data& imgData) {
	imgCopy.compPerPixel = imgData.compPerPixel;
	imgCopy.w = imgData.w;
	imgCopy.h = imgData.h;
	imgCopy.pixels = new stbi_uc[imgCopy.h * imgCopy.w * imgCopy.compPerPixel];
	for (int x = 0; x < imgData.w; x++) {
		for (int y = 0; y < imgData.h; y++) {
			for (int d = 0; d < QUAN_OF_COLORS; ++d) {
				imgCopy.pixels[(x + y * imgData.w) * imgData.compPerPixel + d] = imgData.pixels[(x + y * imgData.w) * imgData.compPerPixel + d];
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
	delete ker.ker_matrix;
}

image_data Red::apply(rectangle rect, image_data& imgData) {
	int comp = imgCopy.compPerPixel;
	int max_comp = QUAN_OF_COLORS;
	for (int x = rect.a; x < rect.c; x++) {
		for (int y = rect.b; y < rect.d; y++) {
			imgCopy.pixels[(x + y * imgCopy.w) * comp] = 255;
			for (int depth = 1; depth < max_comp; depth++) {
				imgCopy.pixels[(x + y * imgCopy.w) * comp + depth] = 0;
			}
		}
	}
	return imgCopy;
}

Threshold::Threshold(image_data& imgData, int sizeOfKernel) : convolutional_filter(imgData) {
	ker.size = sizeOfKernel;
	ker.ker_matrix = nullptr;
}

Blur::Blur(image_data& imgData, int sizeOfKernel) : convolutional_filter(imgData) {
	ker.size = sizeOfKernel;
	double sq = ker.size * ker.size;
	ker.ker_matrix = new double[sq];
	for (int i = 0; i < sq; ++i) {
		ker.ker_matrix[i] = 1 / sq;
	}
}

Edge::Edge(image_data& imgData, int sizeOfKernel) : convolutional_filter(imgData) {
	ker.size = sizeOfKernel;
	const int sq = ker.size * ker.size;
	ker.ker_matrix = new double[sq];
	for (int i = 0; i < sq; ++i) {
		if (i != sq / 2)
			ker.ker_matrix[i] = -1;
		else
			ker.ker_matrix[i] = sq - 1;
	}
}

image_data Blur::apply(rectangle rect, image_data& imgData) {
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
							sum += (int)(imgData.pixels[(x_pos + y_pos * w) * comp + depth] * ker.ker_matrix[p]);
						p++;
					}
				}
				imgCopy.pixels[point] = sum;
			}
		}
	}
	return imgCopy;
}

int Threshold::find_median(int x, int y, image_data& imgData) {
	int w = imgCopy.w, h = imgCopy.h;
	int arr[25];
	int p = 0;
	for (int s_i = -ker.size / 2; s_i <= ker.size / 2; ++s_i) {
		for (int s_j = -ker.size / 2; s_j <= ker.size / 2; ++s_j) {
			int x_pos = x + s_i;
			int y_pos = y + s_j;
			if (!(x_pos < 0 || x_pos >= w || y_pos < 0 || y_pos >= h)) {
				arr[p] = ToBlackWhite(x_pos, y_pos, imgData);
			}
			else {
				arr[p] = 0;
			}
			p++;
		}
	}
	qsort(arr, ker.size * ker.size, sizeof(int), [](const void* x1, const void* x2) ->int { return (*(int*)x2 - *(int*)x1); });
	return arr[ker.size * ker.size / 2];
}

void Threshold::zero_below_median(int x, int y, int med, image_data& imgData) {
	int w = imgCopy.w, h = imgCopy.h;
	int comp = imgCopy.compPerPixel;
	int max_comp = QUAN_OF_COLORS;
	for (int s_i = -ker.size / 2; s_i <= ker.size / 2; ++s_i) {
		for (int s_j = -ker.size / 2; s_j <= ker.size / 2; ++s_j) {
			int x_pos = x + s_i;
			int y_pos = y + s_j;
			if (!(x_pos < 0 || x_pos >= w || y_pos < 0 || y_pos >= h)) {
				int bw = ToBlackWhite(x_pos, y_pos, imgData);
				if (bw < med) {
					for (int d = 0; d < max_comp; d++) {
						imgCopy.pixels[(x_pos + y_pos * w) * comp + d] = 0;
					}
				}
				else {
					for (int d = 0; d < max_comp; d++) {
						imgCopy.pixels[(x_pos + y_pos * w) * comp + d] = bw;
					}
				}
			}
		}
	}
}

image_data Threshold::apply(rectangle rect, image_data& imgData) {
	for (int x = rect.a + ker.size / 2; x < rect.c + ker.size / 2; x+=ker.size) {
		for (int y = rect.b + ker.size / 2; y < rect.d + ker.size / 2; y+=ker.size) {
			zero_below_median(x, y, find_median(x, y, imgData), imgData);
		}
	}
	return imgCopy;
}

image_data Edge::apply(rectangle rect, image_data& imgData) {
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
						sum += ker.ker_matrix[p] * ToBlackWhite(x_pos, y_pos, imgData);
					p++;
				}
			}
			for (int d = 0; d < max_comp; ++d)
				imgCopy.pixels[point + d] = sum;
		}
	}
	return imgCopy;
}