CPPFLAGS = -g -O0 -Wall -std=c++11 -pthread

all: mandelbrot

clean:
	rm -f mandelbrot
	rm -f *.bmp
	rm -f *.o