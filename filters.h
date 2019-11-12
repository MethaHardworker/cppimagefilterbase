#pragma once
#include "png_toolkit.h"
#include "structs.h"

class abstract_filter {
public:
	virtual image_data apply(rectangle rect, image_data& imgData) = 0;
	abstract_filter(image_data& imgData);
	image_data&& get_image();
	virtual ~abstract_filter();
protected:
	int ToBlackWhite(int x, int y, image_data& imgData);
protected:
	const int QUAN_OF_COLORS = 3;
	image_data imgCopy;
};

class Red : public abstract_filter {
public:
	image_data apply(rectangle rect, image_data& imgData);
	Red(image_data& imgData) : abstract_filter(imgData) { };
	~Red() { };
};

class convolutional_filter : public abstract_filter {
public:
	convolutional_filter(image_data& imgData) : abstract_filter(imgData) {};
	virtual image_data apply(rectangle rect, image_data& imgData) = 0;
	virtual ~convolutional_filter();
protected:
	kernel ker;
};

class Threshold : public convolutional_filter {
public:
	Threshold(image_data& imgData, int sizeOfKernel = 5);
	image_data apply(rectangle rect, image_data& imgData);
	virtual ~Threshold() { };
protected:
	int find_median(int x, int y, image_data& imgData);
	void zero_below_median(int x, int y, int med, image_data& imgData);
};

class Blur : public convolutional_filter {
public:
	Blur(image_data& imgData, int sizeOfKernel = 5);
	image_data apply(rectangle rect, image_data& imgData);
	virtual ~Blur() { };
};

class Edge : public convolutional_filter {
public:
	Edge(image_data& imgData, int sizeOfKernel = 3);
	image_data apply(rectangle rect, image_data& imgData);
	virtual ~Edge() { };
};