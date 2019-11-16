#include "png_toolkit.h"
#include "structs.h"
#include "filters.h"
#include "filter_applyer.h"
#include <iostream>

int main(int argc, char *argv[])
{
    // toolkit filter_name base_pic_name student_tool student_pic_name limitPix limitMSE
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
    return 0;
}


