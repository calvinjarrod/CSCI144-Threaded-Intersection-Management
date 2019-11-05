run: carThread.cpp main.cpp
	g++ -std=c++11 carThread.cpp main.cpp -o run -ggdb -lpthread
