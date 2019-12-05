run: car.cpp car.h intersection.cpp intersection.h main.cpp
	g++ -std=c++11 intersection.cpp car.cpp main.cpp -o run -ggdb -lpthread
