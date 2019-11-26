/**
		CSCI144 - Operating Systems, Semester Project: Threaded Intersection Management
		main.cpp
		Purpose: 	Use threads to manage an intersection given a list of car with
							starting position, destination and arrival time.

		@author Calvin Jarrod Smith
		@version 1.0 11/25/2019
*/

#include <cstdlib>
#include <iostream>
#include "intersection.h"

using namespace std;

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
	Intersection threadIntersection(fileName);
	threadIntersection.run();
	return 0;
}
