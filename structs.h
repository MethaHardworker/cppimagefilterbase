#pragma once
#include <vector>
struct rectangle {
	int a;
	int b;
	int c;
	int d;
};

struct kernel {
	int size;
	std::vector<double> ker_matrix;
};