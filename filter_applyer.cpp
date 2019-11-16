#include "filter_applyer.h"
#include <fstream>
#include <string>
#include <iostream>

void filter_applyer::set_filter(std::string filtername, image_data& imgData) {
	if (filtername == "Red")
		filter = new Red(imgData);
	else if (filtername == "Blur")
		filter = new Blur(imgData);
	else if (filtername == "Threshold")
		filter = new Threshold(imgData);
	else if (filtername == "Edge")
		filter = new Edge(imgData);
	else
		filter = nullptr;
}

void filter_applyer::parse(char* filename, image_data imgData) {
	std::ifstream f(filename);
	if (!f.is_open()) {
		return;
	}
	image = imgData;
	while (!f.eof()) {
		std::string text;
		try {
			f >> text;
			int a, b, c, d;
			f >> b >> a >> d >> c;
			rectangle rect;
			a == 0 ? rect.a = 0 : rect.a = imgData.w / a;
			b == 0 ? rect.b = 0 : rect.b = imgData.h / b;
			c == 0 ? rect.c = 0 : rect.c = imgData.w / c;
			d == 0 ? rect.d = 0 : rect.d = imgData.h / d;
			set_filter(text, imgData);
			if (filter != nullptr) {
				filter->apply(rect, imgData);
			}
			text.clear();
			delete filter;
		}
		catch (...) {
			break;
		}
	}
	f.close();
}
