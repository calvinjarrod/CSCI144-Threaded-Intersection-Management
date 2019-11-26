/**
		CSCI144 - Operating Systems, Semester Project: Threaded Intersection Management
		car.cpp
		Purpose: 	Use threads to manage an intersection given a list of car with
							starting position, destination and arrival time.

		@author Calvin Jarrod Smith
		@version 1.0 11/25/2019
*/

#include <cstdlib>
#include <iostream>
#include <climits>
#include <chrono>
#include <thread>
#include "car.h"

using namespace std;

/**
		Used to convert character of direction: 'N' or 'E', to enum direciton.

		@param Character of direction, 'N','E','S' or 'W'
		@return enum of direction
*/
int char2Enum(const char c) {
	if (c == 'N') return N;
	else if (c == 'S') return S;
	else if (c == 'E') return E;
	else if (c == 'W') return W;
	else return U;
}

/**
		Used to convert an enum direction to a character. Ex. N -> 'N'

		@param enum of direction
		@return character that represents direction
*/
char enum2Char(const int e) {
	if (e == N) return 'N';
	else if (e == S) return 'S';
	else if (e == W) return 'W';
	else if (e == E) return 'E';
	else return 'U';
}

/**
		Used to convert current enum to an enum in opposite direction. Ex. E -> W

		@param enum of direction
		@return enum of opposite direction
*/
int oppositeDir(const int dir) {
	if (dir == N) return S;
	else if (dir == S) return N;
	else if (dir == E) return W;
	else if (dir == W) return E;
	else return U;
}

Car::Car() {
	want2Go = U;
	want2Turn = 0;
	pos = unknown;
	arrival = INT_MAX;
}

/**
		Car constructor used to parse a line from the input file to obtain its 
		position, direciton and arrival time.

		@param string containing Car info from input file
		@return none
*/
Car::Car(const string str) {
	string temp;
	int carDir;
		
	int line_iter = 0;
	while (str[line_iter] != ' ' && str[line_iter] != '.') {
		temp += str[line_iter++];
	}
	while (!isalpha(str[line_iter+1]))
		line_iter++;
	string dirPos = str.substr(line_iter+1);
	while (!isalpha(dirPos.back())) dirPos.pop_back();
	arrival = stoi(temp);
	if (dirPos.size()==1) {
		want2Go = char2Enum(dirPos[0]);
		pos = oppositeDir(char2Enum(dirPos[0]));
		want2Turn = straight;
	// has position and direction
	} else {
		pos = oppositeDir(char2Enum(dirPos[0]));
		want2Go = char2Enum(dirPos[1]);
		setTurn();
	}
}

Car::~Car() {}

/**
		Sets want2Turn field of Car object based on position and direction.

		@param none
		@return none
*/
void Car::setTurn() {
	switch(pos) {
		N: 
			if (want2Go == E) want2Turn = leftTurn;
			else if (want2Go == W) want2Turn = rightTurn;
			break;
		S:
			if (want2Go == E) want2Turn = rightTurn;
			else if (want2Go == W) want2Turn = leftTurn;
			break;
		W: 
			if (want2Go == N) want2Turn = leftTurn;
			else if (want2Go == S) want2Turn = rightTurn;
			break;
		E:
			if (want2Go == N) want2Turn = rightTurn;
			else if (want2Go == S) want2Turn = leftTurn;
			break;
	}
}

bool Car::operator<(Car const & rhs) {
	return arrival < rhs.arrival;
}
