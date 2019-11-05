#include <cstdlib>
#include <iostream>
#include <climits>
#include "carThread.h"

using namespace std;

Car_Thread::Car_Thread() {
	want2Go = U;
	want2Turn = 0;
	pos = unknown;
	arrival = INT_MAX;
}

Car_Thread::Car_Thread(const string str) {
	string temp;
	int carDir;
		
	int line_iter = 0;
	while (str[line_iter] != '.' && str[line_iter] != ' ') {
		temp += str[line_iter++];
	}
	while (!isalpha(str[line_iter+1]))
		line_iter++;
	string dirPos = str.substr(line_iter+1);
	arrival = stoi(temp);
	if (dirPos.size()==1) {
		want2Go = char2Enum(dirPos[0]);
		pos = oppositeDir(char2Enum(dirPos[0]));
		want2Turn = straight;
	// has position and direction
	} else {
		pos = char2Enum(dirPos[0]);
		want2Go = char2Enum(dirPos[1]);
		setTurn();
	}
}

Car_Thread::~Car_Thread() {}

int Car_Thread::char2Enum(const char c) {
	if (c == 'N') return N;
	else if (c == 'S') return S;
	else if (c == 'E') return E;
	else if (c == 'W') return W;
	else return U;
}

int Car_Thread::oppositeDir(const int dir) {
	if (dir == N) return S;
	else if (dir == S) return N;
	else if (dir == E) return W;
	else if (dir == W) return E;
	else return U;
}

void Car_Thread::setTurn() {
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
