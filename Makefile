CC=g++
CFLAGS=`pkg-config --cflags opencv4`
LDFLAGS=`pkg-config --libs opencv4 OpenCL`

all: main clean

main: main.o cse.o fractal.o matrix_ops.o
	$(CC) main.o cse.o fractal.o matrix_ops.o -o main $(CFLAGS) $(LDFLAGS)

main.o: main.cc
	$(CC) -c main.cc $(CFLAGS) $(LDFLAGS)

cse.o: cse.cc cse.hh
	$(CC) -c cse.cc $(CFLAGS) $(LDFLAGS)

fractal.o: fractal.cc fractal.hh
	$(CC) -c fractal.cc $(CFLAGS) $(LDFLAGS)

matrix_ops.o: matrix_ops.cc matrix_ops.hh
	$(CC) -c matrix_ops.cc

clean:
	rm *.o