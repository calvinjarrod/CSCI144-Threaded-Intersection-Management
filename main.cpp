#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <list>
#include <condition_variable>
#include <chrono>
#include "car_obj.h"

using namespace std;

// GLOBAL CONSTANTS -----------------------------------------------------------

const int debug = 1;
list <Car*> north, east, south, west;
list <thread*> thrNorth, thrEast, thrSouth, thrWest;

// END GLOBAL CONSTANTS --------------------------------------------------------

// FUNCTION PROTOTYPES ---------------------------------------------------------

void carParser(const string fileName);
void Car_Control(void * carClass);
void sortLists();
bool isHeadofList(Car * check, int dir);
bool arelistsEmpty();
Car * getNorthHead();
Car * getSouthHead();
Car * getEastHead();
Car * getWestHead();

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

LOCK	intersection_mutex[NUM_DIRECTIONS];
lock_guard<LOCK> intersectionN(intersection_mutex[N]);
lock_guard<LOCK> intersectionS(intersection_mutex[S]);
lock_guard<LOCK> intersectionE(intersection_mutex[E]);
lock_guard<LOCK> intersectionW(intersection_mutex[W]);
CV 		front[NUM_DIRECTIONS],
			N2[NUM_DIRECTIONS],S2[NUM_DIRECTIONS],
			E2[NUM_DIRECTIONS],W2[NUM_DIRECTIONS];

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
	
	// PARSE INPUT FILE
	carParser(fileName);
	
	// DEBUG LIST SIZES
	cout<<"North size: "<<north.size()<<endl;
	cout<<"South size: "<<south.size()<<endl;
	cout<<"West size: "<<west.size()<<endl;
	cout<<"East size: "<<east.size()<<endl;

	// SORT ALL LISTS BASED ON ARRIVAL TIME
	sortLists();

	// MAKE THREADS
	for (auto &i : north) {
		thread *newThread = new thread(Car_Control,(void *)i);
		thrNorth.push_back(newThread);
	}
	for (auto &i : south) {
		thread *newThread = new thread(Car_Control,(void *)i);
		thrSouth.push_back(newThread);
	}
	for (auto &i : east) {
		thread *newThread = new thread(Car_Control,(void *)i);
		thrEast.push_back(newThread);
	}
	for (auto &i : west) {
		thread *newThread = new thread(Car_Control,(void *)i);
		thrWest.push_back(newThread);
	}
	
	// MANAGER INTERSECTION
	unsigned int currentTime = 1;
	Car * priorityCar = NULL;
	unsigned int priPos;
	while (!arelistsEmpty()) {
		Car *northCar = getNorthHead();
		Car *southCar = getSouthHead();
		Car *eastCar = getEastHead();
		Car *westCar = getWestHead();
		// PICK NORTH AS STARTING PRIORITY CAR
		if (priorityCar == NULL) {
			priorityCar = northCar;
			priPos = N;
		}
		// GET NEXT CAR IN PREFERED ORDER N->E->S->W
		switch(priPos) {
			case N:
				priorityCar = eastCar;
				priPos = E;
				break;
			case E:
				priorityCar = southCar;
				priPos = S;
				break;
			case S:
				priorityCar = westCar;
				priPos = W;
				break;
			case W:
				priorityCar = northCar;
				priPos = N;
				break;
		}
		// IF CURRENT PRIORITY CAR IS STILL NULL OR HASENT ARRIVED YET
		// PICK NEXT CAR IN PREFERED ORDER, N->E->S->W,
		// OTHERWISE WAIT A SECOND AND CHECK FOR NEW ARRIVALS
		int changeTimes = 0;
		while (priorityCar == NULL || priorityCar->arrival > currentTime) {
			if (priPos == N) {
				priorityCar = eastCar;
				priPos = E;
				changeTimes++;
			} else if (priPos == E) {
				priorityCar = southCar;
				priPos = S;
				changeTimes++;
			} else if (priPos == S) {
				priorityCar = westCar;
				priPos = W;
				changeTimes++;
			} else {
				priorityCar = northCar;
				priPos = N;
				changeTimes++;
			} 
			if ((changeTimes%4) > 3) {
				this_thread::sleep_for(chrono::seconds(1));
				currentTime++;
			}
		}
		// IF THE PRIORITY CAR IS NORTH
		if (priPos == N) {
			front[N].notify_all();
			// AND THE PRIORITY CAR WANTS TO GO SOUTH
			if (priorityCar->want2Go == S) {
				N2[S].notify_one();
				if (southCar != NULL) {
					S2[E].notify_one();
					S2[N].notify_one();
				}
				if (eastCar != NULL) {
					E2[N].notify_one();
				}
			// OR THE PRIORITY CAR WANTS TO GO WEST
			} else if (priorityCar->want2Go == W) {
				if (westCar != NULL) {
					W2[N].notify_one();
					W2[S].notify_one();
					W2[E].notify_one();
				}
				if (southCar != NULL) {
					S2[N].notify_one();
					S2[E].notify_one();
				}
				if (eastCar != NULL) {
					E2[S].notify_one();
					E2[N].notify_one();
				}
			// OR THE PRIORITY CAR WANTS TO GO EAST
			} else if (priorityCar->want2Go == E) {
				if (westCar != NULL) {
					W2[S].notify_one();
				}
				if (southCar != NULL) {
					S2[W].notify_one();
				}
				if (eastCar != NULL) {
					E2[S].notify_one();
				}
			}
		// IF THE PRIORITY CAR IS EAST
		} else if (priPos == E) {
			front[E].notify_all();
			if (priorityCar->want2Go == N) {
				if (westCar != NULL) {
					// need to check if arrived yet. just call a function 
					// notifyOneIfArrived(&W2[E]) to notify if the arrival time is correct

					// also need to check if the car can actually go instead of signalling
					// any potential direction, this is good start but the cars aren't 
					// getting popped off of lists
					W2[E].notify_one();
					W2[S].notify_one();
				}
				if (southCar != NULL) {
					S2[E].notify_one();
					S2[W].notify_one();
				}
				if (northCar != NULL) {
					N2[S].notify_one();
					N2[E].notify_one();
					N2[W].notify_one();
				}
			} else if (priorityCar->want2Go == S) {
				if (northCar != NULL) {
					N2[W].notify_one();
				}
				if (southCar != NULL) {
					S2[E].notify_one();	
				}
				if (westCar != NULL) {
					W2[N].notify_one();
				}
			} else if (priorityCar->want2Go == W) {
				if (southCar != NULL) {
					S2[E].notify_one();
				}
				if (westCar != NULL) {
					W2[E].notify_one();
					W2[S].notify_one();
				}
			}
		// IF THE PRIORITY CAR IS SOUTH
		} else if (priPos == S) {
			front[S].notify_all();
			if (priorityCar->want2Go == N) {
				if (northCar != NULL) {
					N2[S].notify_one();
					N2[W].notify_one();
				}
				if (westCar != NULL) {
					W2[S].notify_one();
				}
			} else if (priorityCar->want2Go == E) {
				if (northCar != NULL) {
					N2[S].notify_one();
					N2[W].notify_one();
				}
				if (westCar != NULL) {
					W2[N].notify_one();
					W2[S].notify_one();
				}
				if (eastCar != NULL) {
					E2[W].notify_one();
					E2[N].notify_one();
					E2[S].notify_one();
				}
			} else if (priorityCar->want2Go == W) {
				if (northCar != NULL) {
					N2[E].notify_one();
				}
				if (westCar != NULL) {
					W2[S].notify_one();
				}
				if (eastCar != NULL) {
					E2[N].notify_one();
				}
			}
		// IF THE PRIORITY CAR IS WEST
		} else if (priPos == W) {
			front[W].notify_all();
			if (priorityCar->want2Go == N) {
				if (northCar != NULL) {
					N2[W].notify_one();
				}
				if (southCar != NULL) {
					S2[E].notify_one();
				}
				if (eastCar != NULL) {
					E2[S].notify_one();
				}
			} else if (priorityCar->want2Go == S) {
				if (northCar != NULL) {
					N2[E].notify_one();
					N2[W].notify_one();
				}
				if (southCar != NULL) {
					S2[W].notify_one();
					S2[N].notify_one();
					S2[E].notify_one();					
				}
				if (eastCar != NULL) {
					E2[W].notify_one();
					E2[N].notify_one();
				}
			} else if (priorityCar->want2Go == E) {
				if (northCar != NULL) {
					N2[W].notify_one();
				}
				if (eastCar != NULL) {
					E2[W].notify_one();
					E2[N].notify_one();
				}
			}
		}
		this_thread::sleep_for(chrono::seconds(5));
		currentTime = currentTime+5;
	}

	
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
		Car * newCar = new Car(line);
		if (newCar->pos == N) north.push_back(newCar);
		else if (newCar->pos == S) south.push_back(newCar);
		else if (newCar->pos == E) east.push_back(newCar);
		else west.push_back(newCar);
	}
	// need to sort cars
}

void Car_Control(void * carClass) {
	Car* car = (Car*) carClass;
	unique_lock<LOCK> lock(intersection_mutex[car->pos]);
	while (!isHeadofList(car,car->pos)) {
		front[car->pos].wait(lock);
	}
	while (isHeadofList(car,car->pos)) {
		if (car->pos == N) N2[car->want2Go].wait(lock);
		else if (car->pos == S) S2[car->want2Go].wait(lock);
		else if (car->pos == E) E2[car->want2Go].wait(lock);
		else if (car->pos == W) W2[car->want2Go].wait(lock);
	}
	this_thread::sleep_for(chrono::seconds(5));
	lock.unlock();
	return;
}

bool isHeadofList(Car * check, int dir) {
	if (dir == N) {
		if (check == north.front()) return true;
		else return false;
	}	else if (dir == W) {
		if (check == west.front()) return true;
		else return false;
	} else if (dir == S) {
		if (check == south.front()) return true;
		else return false;
	} else if (dir == E) {
		if (check == east.front()) return true;
		else return false;
	}
}

void sortLists() {
	// north first
	north.sort();
	south.sort();
	west.sort();
	east.sort();
}

bool arelistsEmpty() {
	if (north.size() == 0 && south.size() == 0 
		&& east.size() == 0 && west.size() == 0) return true;
	else return false;
}

Car * getNorthHead() {
	if (north.size() == 0) return NULL;
	else return north.front();
}

Car * getSouthHead() {
	if (south.size() == 0) return NULL;
	else return south.front();
}

Car * getEastHeader() {
	if (east.size() == 0) return NULL;
	else return east.front();
}

Car * getWestHead() {
	if (west.size() == 0) return NULL;
	else return west.front();
}