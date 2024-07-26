all: hw2

hw2: hw2.cpp hw2_output.c
	g++ -o hw2 hw2.cpp hw2_output.c -lpthread

