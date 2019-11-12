#include <iostream>
#include <fstream>
#include "png_toolkit.h"
#include "structs.h"
#include "filters.h"
#include <crtdbg.h>

class abstract_filter;

class filter_applyer {
public:
	void parse(char* filename, image_data imgData);
private:
	void set_filter(char* fitlername, image_data& newData);
	void Assign(const image_data& newData, rectangle rect);
private:
	abstract_filter* filter;
	image_data image;
};

///////////////////////////////////////////////////////////////||||||||||||||||||||||||||

void filter_applyer::Assign(const image_data& newData, rectangle rect) {
	int w = image.w;
	int comp = image.compPerPixel;
	int max_comp = 3;
	for (int x = rect.a; x < rect.c; x++) {
		for (int y = rect.b; y < rect.d; y++) {
			for (int d = 0; d < max_comp; ++d) {
				image.pixels[(x + y * w) * comp + d] = newData.pixels[(x + y * w) * comp + d];
			}
		}
	}
}

void filter_applyer::set_filter(char* filtername, image_data& imgData) {
	if (strcmp(filtername, "Red") == 0)
		filter = new Red(imgData);
	else if (strcmp(filtername, "Blur") == 0)
		filter = new Blur(imgData);
	else if (strcmp(filtername, "Threshold") == 0)
		filter = new Threshold(imgData);
	else if (strcmp(filtername, "Edge") == 0)
		filter = new Edge(imgData);
	else
		filter = nullptr;
}

void filter_applyer::parse(char* filename, image_data imgData) {
	std::ifstream f(filename);
	if (!f.is_open())
		return;
	char text[15];
	image = imgData;
	while (!f.eof()) {
		f >> text;
		int a, b, c, d;
		f >> b >> a >> d >> c;
		rectangle rect;
		a != 0 ? rect.a = imgData.w / a : rect.a = 0;
		b != 0 ? rect.b = imgData.h / b : rect.b = 0;
		c != 0 ? rect.c = imgData.w / c : rect.c = 0;
		d != 0 ? rect.d = imgData.h / d : rect.d = 0;
		set_filter(text, imgData);
		if (filter != nullptr)
			filter->apply(rect, imgData);
		Assign(filter->get_image(), rect);
		delete filter;
	}
	f.close();
}

int main( int argc, char *argv[] )
{
    // toolkit filter_name base_pic_name sudent_tool student_pic_name limitPix limitMSE
    // toolkit near test images!
    try
    {
        if (argc != 4)
            throw "Not enough arguments";

        png_toolkit studTool;
		studTool.load(argv[2]);
		filter_applyer fa;
		fa.parse(argv[1], studTool.getPixelData());
        studTool.save(argv[3]);

    }
    catch (const char *str)
    {
        std::cout << "Error: " << str << std::endl;
        return 1;
    }
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
	_CrtDumpMemoryLeaks();
    return 0;
}


