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
	int norm;
	std::vector<int> ker_matrix;
};