//*****************************************************
// Compute mandelbrot set images
//
// Author: Phil Howard
//*****************************************************

#include <complex>
#include <stdio.h>      // BMP output uses C IO not C++
#include <unistd.h>     // for getopt

#include "bmp.h"        // class for creating BMP files

using std::complex;

//*****************************************************
// Determine if a single point is in the mandelbrot set.
// Params:
//    (x,y): The complex number to make the determination on
//
// Return:
//    zero if the number is in the set
//    number of iterations to conclude it's not in the set
int ComputeMandelbrot(long double x, long double y, int max_iters)
{
    complex<long double> c(x,y), z(0,0);

    for (int ii=0; ii<max_iters; ii++)
    {
        z = z*z + c;
        if (std::abs(z) >= 2.0) return ii+1;
    }

    return 0;
}

//**************************************************
// choose a color for a particular mandelbrot value
// Params:
//     value: value returned by ComputeMandelbrot
//     max_value: the max value returned by ComputeMandelbrot
//                note: this is max_iters
// Return: 8 bit color value to be displayed
inline int ColorizeMono(int value, int max_value)
{
    if (value == 0)
        value = 255;
    else
        value = 0;

    return value;
}

//**************************************************
// choose a color for a particular mandelbrot value
// Params:
//     value: value returned by ComputeMandelbrot
//     max_value: the max value returned by ComputeMandelbrot
//                note: this is max_iters
// Return: 8 bit color value to be displayed
inline int ColorizeScaled(int value, int max_value)
{
    value = value*255/max_value*8;
    if (value > 255) value = 255;

    return value;
}

static const char *HELP_STRING = 
    "mandelbrot <options> where <options> can be the following\n"
    "   -h print this help string\n"
    "   -x <value> the starting x value. Defaults to -2\n"
    "   -X <value> the ending x value. Defaults to +2\n"
    "   -y <value> the starting y value. Defaults to -2\n"
    "   -Y <value> the ending y value. Defaults to +2\n"
    "   -r <value> the number of rows in the resulting image. Default 256.\n"
    "   -c <value> the number of cols in the resulting image. Default 256.\n"
    "   -m <value> the max number of iterations. Default is 1024.\n"
    "   -n <value> the number of threads to use. Default is 1.\n"
    "";

//*************************************************
// Main function to compute mandelbrot set image
// Command line args:
//     -x <value> the starting x value. Defaults to -2
//     -X <value> the ending x value. Defaults to +2
//     -y <value> the starting y value. Defaults to -2
//     -Y <value> the ending y value. Defaults to +2
//     -r <value> the number of rows in the resulting image. Default 256.
//     -c <value> the number of cols in the resulting image. Default 256.
//     -m <value> the max number of iterations. Default is 1024.
//     -n <value> the number of threads to use. Default is 1.
//
// Note: the command line args are not sanity checked. You asked for it, 
//       you got it, even if the result is meaningless.
int main(int argc, char** argv)
{
    int max_iters = 1024;
    //int num_threads = 1;
    int rows = 256;
    int cols = 256;
    long double start_x = -2.0;
    long double end_x = 2.0;
    long double start_y = -2.0;
    long double end_y = 2.0;

    long double x,y,value;

    int opt;

    // get command line args
    while ((opt = getopt(argc, argv, "hx:X:y:Y:r:c:m:n:")) >= 0)
    {
        switch (opt)
        {
            case 'x':
                sscanf(optarg, "%Lf", &start_x);
                break;
            case 'X':
                sscanf(optarg, "%Lf", &end_x);
                break;
            case 'y':
                sscanf(optarg, "%Lf", &start_y);
                break;
            case 'Y':
                sscanf(optarg, "%Lf", &end_y);
                break;
            case 'r':
                rows = atoi(optarg);
                break;
            case 'c':
                cols = atoi(optarg);
                break;
            case 'm':
                max_iters = atoi(optarg);
                break;
            case 'n':
                //num_threads = atoi(optarg);
                break;
            case 'h':
                printf(HELP_STRING);
                break;
            default:
                fprintf(stderr, HELP_STRING);
        }
    }

    // create and compute the image
    Bmp_c image(rows, cols);

    for (int row=0; row<rows; row++)
    {
        y = start_y + (end_y - start_y)/rows * row;
        for (int col=0; col<cols; col++)
        {
            x = start_x + (end_x - start_x)/cols * col;
            value = ComputeMandelbrot(x, y, max_iters);

            // colorize and set the pixel
            value = ColorizeScaled(value, max_iters);
            image.Set_Pixel(row, col, value);
        }
    }

    // define the pallet
    uint32_t pallet[256];
    for (int ii=0; ii<256; ii++)
    {
        pallet[ii] = Bmp_c::Make_Color(ii, 0, ii);
    }

    image.Set_Pallet(pallet);

    // create and write the output
    FILE *output = fopen("image.bmp", "wb");

    image.Write_File(output);

    fclose(output);

    printf("File was written\n");

    return 0;
}

