#pragma once
#include "filters.h"
#include "png_toolkit.h"
#include "structs.h"


class filter_applyer {
public:
	void parse(char* filename, image_data imgData);
private:
	void set_filter(std::string fitlername, image_data& newData);

private:
	abstract_filter* filter;
	image_data image;
};