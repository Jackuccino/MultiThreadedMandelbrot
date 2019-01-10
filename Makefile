CPPFLAGS = -g -O0 -Wall -std=c++11 -pthread

all: mandelbrot threadsample

clean:
	rm -f mandelbrot
	rm -f threadsample
	rm -f *.bmp
	rm -f *.o
