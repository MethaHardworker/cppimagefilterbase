#pragma once
#include "png_toolkit.h"
#include "structs.h"

class abstract_filter {
public:
	virtual void apply(rectangle rect, image_data& imgData) = 0;
	abstract_filter(image_data& imgData);
	image_data&& get_image();
	virtual ~abstract_filter();
protected:
	bool IsInBorder(rectangle rect, int x, int y);
	int SmartAssignVal(int val);
	int ToBlackWhite(int x, int y, image_data& imgData);
	void AssignToAllLevels(int value, image_data& imgData, int x, int y);
protected:
	const int QUAN_OF_COLORS = 3;
	const int MAX_VAL_OF_COLOR = 255;
	const int MIN_VAL_OF_COLOR = 0;

	image_data imgCopy;
};

class BlackWhite : public abstract_filter {
public:
	void apply(rectangle rect, image_data& imgData);
	BlackWhite(image_data& imgData) : abstract_filter(imgData) { };
	~BlackWhite() { };
};

class Red : public abstract_filter {
public:
	void apply(rectangle rect, image_data& imgData);
	Red(image_data& imgData) : abstract_filter(imgData) { };
	~Red() { };
};

class convolutional_filter : public abstract_filter {
public:
	convolutional_filter(image_data& imgData) : abstract_filter(imgData) {};
	virtual void apply(rectangle rect, image_data& imgData) = 0;
	void convolute(rectangle rect, image_data& imgData, int level);
	virtual ~convolutional_filter();
protected:
	kernel ker;
};

class Threshold : public abstract_filter {
public:
	Threshold(image_data& imgData, int sizeOfKernel = 5);
	void apply(rectangle rect, image_data& imgData);
	virtual ~Threshold() { };
protected:
	kernel ker;
	int find_median(int x, int y, rectangle rect);
};

class Blur : public convolutional_filter {
public:
	Blur(image_data& imgData, int sizeOfKernel = 3);
	void apply(rectangle rect, image_data& imgData);
	virtual ~Blur() { };
};

class Edge : public convolutional_filter {
public:
	Edge(image_data& imgData, int sizeOfKernel = 3);
	void apply(rectangle rect, image_data& imgData);
	virtual ~Edge() { };
};