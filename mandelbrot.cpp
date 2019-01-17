//*****************************************************
// Compute mandelbrot set images
//
// Author: Phil Howard & JinJie Xu
//*****************************************************

#include <complex>
using std::complex;
#include <stdio.h>      // BMP output uses C IO not C++
#include <unistd.h>     // for getopt
#include <pthread.h>    // use for multi-threading
#include <queue>
using std::queue;

#include "bmp.h"        // class for creating BMP files
#include "palette.h"    // class for different color combo

typedef struct
{
    pthread_t id;
    pthread_mutex_t *mutex;
    queue<int> *work_queue;
    int total_row;
    int total_col;
    long double start_x;
    long double end_x;
    long double start_y;
    long double end_y;
    int max_iters;
    Bmp_c *image;
} thread_arg_t;

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
        if (std::abs(z) >= 2.0)
            return ii+1;
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

//*****************************************************
// Work for each thread
// Params:
//    param: it's the struct "thread_arg_t" defined above
//
// Return:
//    nullptr
void *ThreadFunc(void *param)
{
    long double x,y,value;
    int row;

    thread_arg_t *args = static_cast<thread_arg_t*>(param);

    // Lock before accessing queue
    if (pthread_mutex_lock(args->mutex) != 0) 
        pthread_exit((void *)-1);

    // loop if the queue is not empty: means the image is not done
    while (!args->work_queue->empty())
    {
        // Get a row from the queue
        row = args->work_queue->front();
        args->work_queue->pop();

        // Unlock when done accessing queue
        if (pthread_mutex_unlock(args->mutex) != 0)
            pthread_exit((void *)-1);

        // Get one row done, and go back to get another row
        y = args->start_y + 
            (args->end_y - args->start_y)/args->total_row * row;
            
        for (int col = 0; col < args->total_col; col++)
        {
            x = args->start_x + 
                (args->end_x - args->start_x)/args->total_col * col;
            
            // Here is where valgrind is waiting forever
            value = ComputeMandelbrot(x, y, args->max_iters);

            // colorize and set the pixel
            value = ColorizeScaled(value, args->max_iters);
            args->image->Set_Pixel(row, col, value);
        }

        // Lock again before checking empty
        if (pthread_mutex_lock(args->mutex) != 0) 
            pthread_exit((void *)-1);
    }

    // Release the lock when the thread is done
    if (pthread_mutex_unlock(args->mutex) != 0)
        pthread_exit((void *)-1);

    pthread_exit(0);
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
    int num_threads = 1;
    int rows = 256;
    int cols = 256;
    long double start_x = -2.0;
    long double end_x = 2.0;
    long double start_y = -2.0;
    long double end_y = 2.0;
    queue<int> work_queue;
    Palette palette;

    // Create argument for each thread
    thread_arg_t *args;
    // Create a pthread mutex
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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
                num_threads = atoi(optarg);
                break;
            case 'h':
                printf(HELP_STRING);
                break;
            default:
                fprintf(stderr, HELP_STRING);
        }
    }

    // Fill the queue with rows
    for (int i = 0; i < rows; i++)
    {
        work_queue.push(i);
    }

    // create and compute the image
    Bmp_c image(rows, cols);

    args = new thread_arg_t[num_threads];
    
    // create threads
    for (int i = 0; i < num_threads; i++)
    {
        args[i].work_queue = &work_queue;
        args[i].mutex = &mutex;
        args[i].total_row = rows;
        args[i].total_col = cols;
        args[i].start_x = start_x;
        args[i].end_x = end_x;
        args[i].start_y = start_y;
        args[i].end_y = end_y;
        args[i].max_iters = max_iters;
        args[i].image = &image;

        if (pthread_create(&args[i].id, nullptr, 
            ThreadFunc, &args[i]) != 0)
            return -1;
    }

    // Wait for threads to finish
    for (int i = 0; i < num_threads; i++)
    {
        if (pthread_join(args[i].id, nullptr) != 0)
            return -1;
    }

    delete[] args;

    // define the pallet
    uint32_t pallet[256];
    for (int ii=0; ii<256; ii++)
    {
        Color color(palette.GetColor(ii % palette.Count()));
        pallet[ii] = 
            Bmp_c::Make_Color(color.R(), color.G(), color.B());
    }

    image.Set_Pallet(pallet);

    // create and write the output
    FILE *output = fopen("image.bmp", "wb");

    image.Write_File(output);

    fclose(output);

    printf("File was written\n");

    return 0;
}
