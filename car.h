/**
		CSCI144 - Operating Systems, Semester Project: Threaded Intersection Management
		car.h
		Purpose: 	Use threads to manage an intersection given a list of car with
							starting position, destination and arrival time.

		@author Calvin Jarrod Smith
		@version 1.0 11/25/2019
*/

#pragma <once>

#include <cstdlib>
#include <string>

using namespace std;

const int NUM_DIRECTIONS = 4;

enum direction{
	N,S,W,E,U		// order of these is important!!! if this is changed setTurn()
							// function will not work!!!!!!
};

int char2Enum(const char c);
char enum2Char(const int e);
int oppositeDir(const int dir);

enum turn {
	straight,rightTurn,leftTurn,unknown
};

class Car {
	public:
		int pos;
		int want2Go;
		int want2Turn;		
		unsigned int arrival;
		Car();
		Car(const string str);
		~Car();
		void setTurn();
		bool operator<(Car const & rhs);
};
