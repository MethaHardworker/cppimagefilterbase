#include <iostream>
#include "png_toolkit.h"

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
		studTool.Parse(argv[1]);
		//studTool.Recolor();
        studTool.save(argv[3]);

    }
    catch (const char *str)
    {
        std::cout << "Error: " << str << std::endl;
        return 1;
    }

    return 0;
}
