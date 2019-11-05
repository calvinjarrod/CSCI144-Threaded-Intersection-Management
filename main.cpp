#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <list>
#include <condition_variable>
#include "carThread.h"

using namespace std;

// GLOBAL CONSTANTS -----------------------------------------------------------

const int debug = 1;
list <thread*> north, east, south, west;

// END GLOBAL CONSTANTS --------------------------------------------------------

// FUNCTION PROTOTYPES ---------------------------------------------------------

void carParser(const string fileName);
void Car_Control(void * carClass);

// DEBUGGING FUNCTIONS --------------------------------------------------------

void dbcout(const string str) {
	if (debug == 1) {
		cout<<str;
	}
}

// END DEBUGGING FUNCTIONS ----------------------------------------------------

// CONTIDION VARIABLES AND LOCKS ----------------------------------------------

typedef mutex LOCK;
typedef condition_variable CV;

LOCK	firstInLine[NUM_DIRECTIONS], intersection[NUM_DIRECTIONS];
CV 		Ngoing[NUM_DIRECTIONS],Sgoing[NUM_DIRECTIONS],
			Egoing[NUM_DIRECTIONS],Wgoing[NUM_DIRECTIONS];

// END CONTIDION VARIABLES AND LOCKS ------------------------------------------

// MAIN FUNCTION --------------------------------------------------------------

int main(int argc, char* argv[]) {
	if (argc == 1) {
		cout<<"Please include string name of input file to read with no spaces"<<\
		endl;
		return 1;
	}
	string fileName = argv[1];
	if (fileName.empty()) {
		cout<<"File name empty"<<endl;
		return 1;
	}
	carParser(fileName);
	return 0;
}

void carParser(const string fileName) {
	ifstream in;
	in.open(fileName);
	while (!in.eof()) {
		string line, temp;
		getline(in,line);
		if (line.size() == 0 && in.peek() == EOF) break;
		cout<<line<<endl;
		Car_Thread * newCar = new Car_Thread(line);
		thread * newThread = new thread(Car_Control,(void *)newCar);
		if (newCar->pos == N) north.push_back(newThread);
	}
	while (north.size() != 0) {
		north.front()->join();
		north.pop_front();	
	}
}

void Car_Control(void * carClass) {
	Car_Thread* myClass = (Car_Thread*) carClass;
	cout<<"New Thread!!! This thread's position is "<<myClass->pos<<endl;
	return;
}