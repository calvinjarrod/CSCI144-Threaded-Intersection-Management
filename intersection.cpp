/**
		CSCI144 - Operating Systems, Semester Project: Threaded Intersection Management
		intersection.cpp
		Purpose: 	Use threads to manage an intersection given a list of car with
							starting position, destination and arrival time.

		@author Calvin Jarrod Smith
		@version 1.0 11/25/2019
*/

#include <fstream>
#include <chrono>
#include <iostream>
#include "intersection.h"

using namespace std;

Intersection::Intersection() {
}

/**
		Intersection constructor, takes an file name as string to parse list of
		cars.

		@param input file name as string
		@return none
*/

Intersection::Intersection(const string inputFile) {
  carParser(inputFile);
}

Intersection::~Intersection() {
}

/**
		High-level function used to run the intersection. Call using main function.
	
		@param none
		@return none
*/
void Intersection::run() {
  sortLists();
  makeThreads();
  runIntersection();
	calcAverageWait();

}

/**
		Creates threads based on contents of north, east, south and west STL lists.
		Pushes pointers to threads into thrNorth, thrEast, thrSouth and thrWest.

		@param none
		@return none
*/
void Intersection::makeThreads() {
	for (auto &i : north) {
		thread *newThread = new thread(&Intersection::carControl,this,dynamic_cast<Car*>(i));
		thrNorth.push_back(newThread);
	}
	for (auto &i : south) {
		thread *newThread = new thread(&Intersection::carControl,this,dynamic_cast<Car*>(i));
		thrSouth.push_back(newThread);
	}
	for (auto &i : east) {
		thread *newThread = new thread(&Intersection::carControl,this,dynamic_cast<Car*>(i));
		thrEast.push_back(newThread);
	}
	for (auto &i : west) {
		thread *newThread = new thread(&Intersection::carControl,this,dynamic_cast<Car*>(i));
		thrWest.push_back(newThread);
	}  
}

/**
		Mid-level function to run the intersection after all cars and threads added
		to corresponding STL lists. Manages intersection and selects which car has
		priority.

		@param none
		@return none
*/
void Intersection::runIntersection() {
	// MANAGE INTERSECTION
	Car * priorityCar = NULL;
	unsigned int priPos = U;

	// USED TO ERASE CONTENTS OF results.txt AFTER RUNNING EACH CASE
	ofstream fout;
	fout.open("results.txt");
	fout.close();
	while (!arelistsEmpty()) {

		Car *northCar = getNorthHead();
		Car *southCar = getSouthHead();
		Car *eastCar = getEastHead();
		Car *westCar = getWestHead();

		// PICK NORTH AS STARTING PRIORITY CAR
		if (priorityCar == NULL && priPos == U) {
			priorityCar = northCar;
			priPos = N;
		} else {
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
		}
		// IF CURRENT PRIORITY CAR IS STILL NULL OR HASENT ARRIVED YET
		// PICK NEXT CAR IN PREFERED ORDER, N->E->S->W,
		// OTHERWISE WAIT A SECOND AND CHECK FOR NEW ARRIVALS
		int changeTimes = 0;
		while (priorityCar == NULL || priorityCar->arrival > currentTime) {
			// IF ALL ARRIVAL TIMES ARE THE SAME, ROUND ROBIN
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
			// NONE ARRIVED YET, SO WAIT A SECOND
			if ((changeTimes % 4) == 3) {
				this_thread::sleep_for(chrono::seconds(1));
				currentTime++;
			}
		}
		// CHECK IF THERE'S A CAR THAT HAS ARRIVED WITH SMALLER ARRIVAL TIME
		//pickCarShortestArrival(northCar,eastCar,southCar,westCar,\
		//	&priorityCar,&priPos);
		
		// =========================================================================
		// INTERSECTION MANAGEMENT -------------------------------------------------
		// =========================================================================
		// PRIORITY CAR IS NORTH ---------------------------------------------------
		if (priPos == N && priorityCar->want2Go == S) {
			northCarGoingSouth(northCar,eastCar,southCar,westCar);
		} else if (priPos == N && priorityCar->want2Go == W) {
			northCarGoingWest(northCar,eastCar,southCar,westCar);
		} else if (priPos == N && priorityCar->want2Go == E) {
			northCarGoingEast(northCar,eastCar,southCar,westCar);
		// PRIORITY CAR IS EAST ----------------------------------------------------
		} else if (priPos == E && priorityCar->want2Go == N) {
			eastCarGoingNorth(northCar,eastCar,southCar,westCar);
		} else if (priPos == E && priorityCar->want2Go == S) {
			eastCarGoingSouth(northCar,eastCar,southCar,westCar);
		} else if (priPos == E && priorityCar->want2Go == W) {
			eastCarGoingWest(northCar,eastCar,southCar,westCar);
		// PRIORITY CAR IS SOUTH ---------------------------------------------------
		} else if (priPos == S && priorityCar->want2Go == N) {
			southCarGoingNorth(northCar,eastCar,southCar,westCar);
		} else if (priPos == S && priorityCar->want2Go == E) {
			southCarGoingEast(northCar,eastCar,southCar,westCar); 
		} else if (priPos == S && priorityCar->want2Go == W) {
			southCarGoingWest(northCar,eastCar,southCar,westCar); 
		// PRIORITY CAR IS WEST ----------------------------------------------------
		} else if (priPos == W && priorityCar->want2Go == N) {
			westCarGoingNorth(northCar,eastCar,southCar,westCar);
		} else if (priPos == W && priorityCar->want2Go == S) {
			westCarGoingSouth(northCar,eastCar,southCar,westCar); 
		} else if (priPos == W && priorityCar->want2Go == E) {
			westCarGoingEast(northCar,eastCar,southCar,westCar);
		}
	// NOTIFY ALL THREADS TO CHECK IF THEY ARE THE CURRENT HEAD
	front[N].notify_all();
	thread_sleep(0.05);
	front[E].notify_all();
	thread_sleep(0.05);
	front[S].notify_all();
	thread_sleep(0.05);
	front[W].notify_all();
	thread_sleep(0.05);
	}

	return;
}

/**
		Calculates average wait time of all cars based on values saved in waitTimes.
		Also outputs total time, minimum and maximum wait times as well as total
		number of cars that passed through intersection

		@param none
		@return none
*/
void Intersection::calcAverageWait() {
	waitTimes.sort();
	cout<<"Cars finished at time: "<<currentTime<<endl;
	cout<<numCars<<" total cars"<<endl;	
	unsigned int sum = 0;
	for (list<int>::iterator i = waitTimes.begin(); i != waitTimes.end(); i++) {
		sum += (*i);
	}
	double avg = (double)(sum)/(double)(numCars);
	cout<<"Agerage wait time: "<<avg<<endl;
	cout<<"Least wait time: "<<waitTimes.front()<<endl;
	cout<<"Greatest wait time: "<<waitTimes.back()<<endl;
}

/**
		Parses input file given in constructor. It reads each line from input file,
		which represents each car, and passes it to a new Car* constructor based on
		which direction the car is at.

		@param none
		@return none
*/
void Intersection::carParser(const string fileName) {
	ifstream in;
	in.open(fileName);
	while (!in.eof()) {
		string line, temp;
		getline(in,line);
		if (line.size() == 0 && in.peek() == EOF) break;
		Car * newCar = new Car(line);
		if (newCar->pos == N) north.push_back(newCar);
		else if (newCar->pos == S) south.push_back(newCar);
		else if (newCar->pos == E) east.push_back(newCar);
		else west.push_back(newCar);
	}
}


/**
		Used as car management thread. This function/thread uses a direction lock
		to secure its position in the interection, once its in the front of the 
		car list (smallest arrival time). Once its arrival time has reached the
		current time it can be allowed to enter the intersection. Condition
		variables front[position] used to signal thread when its the front of the 
		queue. Condition variables N2[], E2[], S2[] and W2[] used to siganl thread
		when it can enter the intersection.

		@param Pointer to car object
		@return none
*/ 
void Intersection::carControl(Car *carClass) {
	Car* car = (Car*) carClass;
	numCars++;
	//cout<<enum2Char(oppositeDir(car->pos))<<" "<<car->arrival<<\
		" thread started! Wants to go: "<<enum2Char(car->want2Go)<<endl;
	cout<<"Thread started"<<endl;
	unique_lock<LOCK> lock(intersection_mutex[car->pos]);
	while (!isHeadofList(car,car->pos) || car->arrival > currentTime) {
		front[car->pos].wait(lock);
	}
	if (car->pos == N) {
		cout<<"South #"<<car->arrival<<" car arrived"<<endl;
		N2[car->want2Go].wait(lock);
		thread_sleep(5);
		cout<<"South #"<<car->arrival<<" car left intersection"<<endl;
	} else if (car->pos == S) {
		cout<<"North #"<<car->arrival<<" car arrived"<<endl;
		S2[car->want2Go].wait(lock);
		thread_sleep(5);
		cout<<"North #"<<car->arrival<<" car left intersection"<<endl;
	} else if (car->pos == E) {
		cout<<"West #"<<car->arrival<<" car arrived"<<endl;
		E2[car->want2Go].wait(lock);
		thread_sleep(5);
		cout<<"West #"<<car->arrival<<" car left intersection"<<endl;
	} else if (car->pos == W) {
		cout<<"East #"<<car->arrival<<" car arrived"<<endl;
		W2[car->want2Go].wait(lock);
		thread_sleep(5);
		cout<<"East #"<<car->arrival<<" car left intersection"<<endl;
	}
	lock.unlock();
	return;
}

/**
		Checks if an input car is the head of a list of cars going a certain
		direction.

		@param Pointer to Car object, direction integer (passed as N,E,S,W)
		@return True if input Car* is the head of list based on dir. False if not.
*/ 
bool Intersection::isHeadofList(Car * check, int dir) {
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

/**
		Sorts all car lists based on their arrival time.

		@param none
		@return none
*/
void Intersection::sortLists() {
	north.sort();
	south.sort();
	west.sort();
	east.sort();
}

/**
		Checks if all lists are empty. Used for main while loop in runInterection()

		@param none
		@return True if all are empty. False if not.
*/
bool Intersection::arelistsEmpty() {
	if (north.size() == 0 && south.size() == 0 
		&& east.size() == 0 && west.size() == 0) return true;
	else return false;
}

/**
		Gets the head of the north list of cars.

		@param none
		@return Returns pointer to head of north list of cars. NULL if list is 
		empty.
*/
Car * Intersection::getNorthHead() {
	if (north.size() == 0) return NULL;
	else return north.front();
}

/**
		Gets the head of the south list of cars.

		@param none
		@return Returns pointer to head of south list of cars. NULL if list is 
		empty.
*/
Car * Intersection::getSouthHead() {
	if (south.size() == 0) return NULL;
	else return south.front();
}

/**
		Gets the head of the east list of cars.

		@param none
		@return Returns pointer to head of east list of cars. NULL if list is 
		empty.
*/
Car * Intersection::getEastHead() {
	if (east.size() == 0) return NULL;
	else return east.front();
}

/**
		Gets the head of the west list of cars.

		@param none
		@return Returns pointer to head of west list of cars. NULL if list is 
		empty.
*/
Car * Intersection::getWestHead() {
	if (west.size() == 0) return NULL;
	else return west.front();
}

/**
		Picks the Car object, out of those passed in, that has the least arrival 
		time. 

		@param Car* to head of north, east, south and west lists, Car** to current
		priority car (used to change where its pointing to) and pointer to priPos.
		@return none
*/
void Intersection::pickCarShortestArrival(Car* northCar, Car* eastCar,\
	 Car* southCar, Car* westCar, Car** priorityCar, unsigned int* priPos) {
	// CHECK IF NORTH CAR EXISTS AND HAS A SMALLER ARRIVAL TIME
	if (northCar!=NULL && (*priorityCar)!=NULL &&\
	 northCar->arrival < (*priorityCar)->arrival) {
		*priorityCar = northCar;
		*priPos = N;
	}
	// CHECK IF EAST CAR EXISTS AND HAS A SMALLER ARRIVAL TIME
	if (eastCar!=NULL && (*priorityCar)!=NULL &&\
		 eastCar->arrival < (*priorityCar)->arrival) {
		*priorityCar = eastCar;
		*priPos = E;
	}
	// CHECK IF SOUTH CAR EXISTS AND HAS A SMALLER ARRIVAL TIME
	if (southCar!=NULL && (*priorityCar)!=NULL &&\
		 southCar->arrival < (*priorityCar)->arrival) {
		*priorityCar = southCar;
		*priPos = S;
	}
	// CHECK IF WEST CAR EXISTS AND HAS A SMALLER ARRIVAL TIME
	if (westCar!=NULL && (*priorityCar)!=NULL &&\
		 westCar->arrival < (*priorityCar)->arrival) {
		*priorityCar = westCar;
		*priPos = W;
	}

}


// =============================================================================	
// =============================================================================
// NORTH CAR INTERSECTION MANAGEMENT
// =============================================================================
// =============================================================================

// -----------------------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// // // // // // --------------------------------------------------------------
// NORTH CAR GOING EAST --------------------------------------------------------
// NORTH CAR TURNING LEFT ------------------------------------------------------
// // // // // // --------------------------------------------------------------
// // // // // -----------------------------------------------------------------
/**
		Determines which cars can enter intersection at the same time when priority 
		car is north going east. 

		@param Car* to head of north, east, south and west lists
		@return none
*/
void Intersection::northCarGoingEast(Car* northCar, Car* eastCar, Car* southCar,\
	Car* westCar) {
	front[N].notify_all();
	thread_sleep(0.05); // must wait this long otherwise thread will not synch

	N2[E].notify_one();
	thread_sleep(0.01);

	// WEST CAR THAT WANTS TO GO SOUTH, NON-INTERSECTING RIGHT TURN --------------
	if (westCar != NULL && westCar->arrival <= currentTime &&
		westCar->want2Go == S) {
			front[W].notify_all();
			thread_sleep(0.01);
			W2[S].notify_one();
	}
	// ---------------------------------------------------------------------------
	// SOUTH CAR THAT WANTS TO GO WEST, NON-INTERSECTING LEFT TURN ---------------
	if (southCar != NULL && southCar->arrival <= currentTime &&
		southCar->want2Go == W) {
			front[S].notify_all();
			thread_sleep(0.01);
			S2[W].notify_one();
	}
	// ---------------------------------------------------------------------------
	// EAST CAR THAT WANTS TO GO NORTH, NON-INTERSECTING RIGHT TURN --------------
	if (eastCar != NULL && eastCar->arrival <= currentTime &&
		eastCar->want2Go == N) {
			front[E].notify_all();
			thread_sleep(0.01);
			E2[N].notify_one();
	}
	// WAIT 5 SECONDS FOR CARS TO LEAVE INTERSECTION
	thread_sleep(5);
	currentTime=currentTime+5;
	// JOIN THREADS THAT WERE ALLOWED IN THE INTERSECTION
	if (westCar != NULL && westCar->arrival <= currentTime &&
		westCar->want2Go == S) {
			getWaitTime(westCar);
			thrWest.front()->join();
			thrWest.pop_front();
			west.pop_front();
	}
	if (southCar != NULL && southCar->arrival <= currentTime &&
		southCar->want2Go == W) {
			getWaitTime(southCar);	
			thrSouth.front()->join();
			thrSouth.pop_front();
			south.pop_front();
	}
	if (eastCar != NULL && eastCar->arrival <= currentTime &&
		eastCar->want2Go == N) {
			getWaitTime(eastCar);	
			thrEast.front()->join();
			thrEast.pop_front();
			east.pop_front();
	}

	getWaitTime(northCar);	
	thrNorth.front()->join();
	thrNorth.pop_front();
	north.pop_front();		

}

// -----------------------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// // // // // // --------------------------------------------------------------
// NORTH CAR GOING SOUTH -------------------------------------------------------
// NORTH CAR GOING STRAIGHT ----------------------------------------------------
// // // // // // --------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// -----------------------------------------------------------------------------
/**
		Determines which cars can enter intersection at the same time when priority 
		car is north going south. 

		@param Car* to head of north, east, south and west lists
		@return none
*/
void Intersection::northCarGoingSouth(Car* northCar, Car* eastCar, Car* southCar,\
	Car* westCar) {
	
	front[N].notify_all();
	thread_sleep(0.05); // must wait this long otherwise thread will not synch

	N2[S].notify_one();
	thread_sleep(0.01);

	// IF THERES A SOUTH CAR -----------------------------------------------------
	if (existsArrivedAndWants2Go(southCar,E) ||\
		existsArrivedAndWants2Go(southCar,N) ) {
		front[S].notify_all();
		thread_sleep(0.01);

		// IF SOUTH CAR WANTS TO GO EAST, EAST CAR AN GO NORTH
		if (southCar->want2Go == E) {
			S2[E].notify_one();
			if (existsArrivedAndWants2Go(eastCar,N)) {

				front[E].notify_all();
				thread_sleep(0.01);
				E2[N].notify_one();
				// SLEEP MAIN THREAD FOR 5 SECONDS
				thread_sleep(5);
				currentTime = currentTime+5;
				// SAVE WAIT TIMES FOR CARS
				getWaitTime(eastCar);
				getWaitTime(southCar);
				getWaitTime(northCar);


				thrEast.front()->join();
				delete thrEast.front();
				thrEast.pop_front();
				east.pop_front();


				thrSouth.front()->join();
				thrSouth.pop_front();
				south.pop_front();


				thrNorth.front()->join();
				thrNorth.pop_front();
				north.pop_front();
			// NO EAST CAR TO GO AT THE SAME TIME
			} else {
				thread_sleep(5);
				currentTime = currentTime+5;
				getWaitTime(southCar);
				getWaitTime(northCar);	
	
	
				thrSouth.front()->join();
				thrSouth.pop_front();
				south.pop_front();
	

				thrNorth.front()->join();
				thrNorth.pop_front();
				north.pop_front();
			}
		// IF SOUTH CAR WANTS TO GO NORTH, ONLY SOUTH CAN GO
		} else if (southCar->want2Go == N) {
			S2[N].notify_one();
			thread_sleep(5);
			currentTime=currentTime+5;
			getWaitTime(southCar);
			getWaitTime(northCar);	


			thrSouth.front()->join();
			thrSouth.pop_front();
			south.pop_front();


			thrNorth.front()->join();
			thrNorth.pop_front();
			north.pop_front();
		}
	// NO SOUTH CAR --------------------------------------------------------------
	// IF THERES AN EAST CAR AND NO SOUTH CAR	------------------------------------
	} else if (existsArrivedAndWants2Go(eastCar,N)) {
		front[E].notify_all();
		thread_sleep(0.01);
		E2[N].notify_one();
		thread_sleep(5);
		currentTime=currentTime+5;
		getWaitTime(eastCar);
		getWaitTime(northCar);	
		

		thrEast.front()->join();
		thrEast.pop_front();
		east.pop_front();


		thrNorth.front()->join();
		thrNorth.pop_front();
		north.pop_front();
	// NO EAST CAR ---------------------------------------------------------------
	// ONLY NORTH CAN GO ---------------------------------------------------------
	} else {
		thread_sleep(5);
		currentTime = currentTime+5;
		getWaitTime(northCar);	


		thrNorth.front()->join();
		thrNorth.pop_front();
		north.pop_front();
	}

}

// -----------------------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// // // // // // --------------------------------------------------------------
// NORTH CAR GOING WEST --------------------------------------------------------
// NORTH CAR GOING LEFT --------------------------------------------------------
// // // // // // --------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// -----------------------------------------------------------------------------
/**
		Determines which cars can enter intersection at the same time when priority 
		car is north going west. 

		@param Car* to head of north, east, south and west lists
		@return none
*/
void Intersection::northCarGoingWest(Car* northCar, Car* eastCar, Car* southCar,\
	Car* westCar) {
	
	front[N].notify_all();
	thread_sleep(0.05); // must wait this long otherwise thread will not synch
				
	N2[W].notify_one();
	thread_sleep(0.01);
	
	// FIRST CHECK IF A STRAIGHT CAR CAN GO AS WELL ------------------------------
	// SOUTH CAN WANTS TO GO STRAIGHT --------------------------------------------
	if (existsArrivedAndWants2Go(southCar, N)) {
		front[S].notify_all();
		thread_sleep(0.01);
		S2[N].notify_one();
		// WEST CAR CAN TURN RIGHT AT THE SAME TIME
		if (existsArrivedAndWants2Go(westCar,S)) {
			front[W].notify_all();
			thread_sleep(0.01);
			W2[S].notify_one();
		}
	// OR WEST WANTS TO GO STRAIGHT ----------------------------------------------
	} else if (existsArrivedAndWants2Go(westCar,E)) {
		front[W].notify_all();
		thread_sleep(0.01);
		W2[E].notify_one();
		// EAST CAR CAN TURN RIGHT AS THE SAME TIME
		if (existsArrivedAndWants2Go(eastCar,N)) {
			front[E].notify_one();
			thread_sleep(0.01);
			E2[N].notify_one();
		}
	// NO STRAIGHTS, SO NOW CHECK LEFTS ------------------------------------------
	// IF EAST OR WEST CARS WANTS TO TURN LEFT
	} else if (existsArrivedAndWants2Go(eastCar,S) ||\
		existsArrivedAndWants2Go(westCar,N)) {
		// EAST CAR CAN GO LEFT
		if (existsArrivedAndWants2Go(eastCar,S)) {
			front[E].notify_all();
			thread_sleep(0.01);
			E2[S].notify_one();
		}
		// WEST CAN ALSO GO LEFT AT SAME TIME
		if (existsArrivedAndWants2Go(westCar,N)) {
			front[W].notify_all();
			thread_sleep(0.01);
			W2[N].notify_one();
		};
		// SOUTH CAN ALSO GO RIGHT AT SAME TIME
		if (existsArrivedAndWants2Go(southCar,E)) {
			front[S].notify_all();
			thread_sleep(0.01);
			S2[E].notify_one();
		}
	// NO LEFT OR STRAIGHTS 
	// ANY RIGHT TURNS CAN GO ALL AT SAME TIME
	} else {
		// WEST CAN GO RIGHT
		if (existsArrivedAndWants2Go(westCar,S)) {
			front[W].notify_all();
			thread_sleep(0.01);
			W2[S].notify_one();
		}
		// AS WELL AS SOUTH
		if (existsArrivedAndWants2Go(southCar,E)) {
			front[S].notify_all();
			thread_sleep(0.01);
			S2[E].notify_one();
		}
		// AS WELL AS EAST
		if (existsArrivedAndWants2Go(eastCar,N)) {
			front[E].notify_all();
			thread_sleep(0.01);
			E2[N].notify_one();
		}
	}

	// WAIT 5 SECONDS FOR CARS TO LEAVE INTERSECTION
	thread_sleep(5);
	currentTime=currentTime+5;
	getWaitTime(northCar);


	thrNorth.front()->join();
	thrNorth.pop_front();
	north.pop_front();				
	
	// USING SAME NESTED CONDITIONS TO ENSURE THE CORRECT JOINING OF THREADS
	if (existsArrivedAndWants2Go(southCar, N)) {
		getWaitTime(southCar);


		thrSouth.front()->join();
		thrSouth.pop_front();
		south.pop_front();				
		if (existsArrivedAndWants2Go(westCar,S)) {
			getWaitTime(westCar);


			thrWest.front()->join();
			thrWest.pop_front();
			west.pop_front();				
		}
	} else if (existsArrivedAndWants2Go(westCar,E)) {
		getWaitTime(westCar);


		thrWest.front()->join();
		thrWest.pop_front();
		west.pop_front();				
		if (existsArrivedAndWants2Go(eastCar,N)) {
			getWaitTime(eastCar);


			thrEast.front()->join();
			thrEast.pop_front();
			east.pop_front();				
		}
	} else if (existsArrivedAndWants2Go(eastCar,S) ||\
		existsArrivedAndWants2Go(westCar,N)) {
		if (existsArrivedAndWants2Go(eastCar,S)) {
			getWaitTime(eastCar);


			thrEast.front()->join();
			thrEast.pop_front();
			east.pop_front();				
		}
		if (existsArrivedAndWants2Go(westCar,N)) {
			getWaitTime(westCar);


			thrWest.front()->join();
			thrWest.pop_front();
			west.pop_front();				
		};
		if (existsArrivedAndWants2Go(southCar,E)) {
			getWaitTime(southCar);


			thrSouth.front()->join();
			thrSouth.pop_front();
			south.pop_front();				
		}
	} else {
		if (existsArrivedAndWants2Go(westCar,S)) {
			getWaitTime(westCar);


			thrWest.front()->join();
			thrWest.pop_front();
			west.pop_front();				
		}
		if (existsArrivedAndWants2Go(southCar,E)) {
			getWaitTime(southCar);


			thrSouth.front()->join();
			thrSouth.pop_front();
			south.pop_front();				
		}
		if (existsArrivedAndWants2Go(eastCar,N)) {
			getWaitTime(eastCar);


			thrEast.front()->join();
			thrEast.pop_front();
			east.pop_front();				
		}
	}


}


// =============================================================================	
// =============================================================================
// EAST CAR INTERSECTION MANAGEMENT
// =============================================================================
// =============================================================================
// -----------------------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// // // // // // --------------------------------------------------------------
// EAST CAR GOING NORTH --------------------------------------------------------
// EAST CAR TURNING RIGHT ------------------------------------------------------
// // // // // // --------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// -----------------------------------------------------------------------------
/**
		Determines which cars can enter intersection at the same time when priority 
		car is east going north. 

		@param Car* to head of north, east, south and west lists
		@return none
*/
void Intersection::eastCarGoingNorth(Car* northCar, Car* eastCar, Car* southCar,\
	Car* westCar) {
	
	front[E].notify_all();
	thread_sleep(0.05); // must wait this long otherwise thread will not synch
				
	E2[N].notify_one();
	thread_sleep(0.01);
	
	// FIRST CHECK IF A STRAIGHT CAR CAN GO AS WELL ------------------------------
	// WEST CAN WANTS TO GO STRAIGHT ---------------------------------------------
	if (existsArrivedAndWants2Go(westCar, E)) {
		front[W].notify_all();
		thread_sleep(0.01);
		W2[E].notify_one();
		// NORTH CAR CAN TURN RIGHT AT THE SAME TIME
		if (existsArrivedAndWants2Go(northCar,W)) {
			front[N].notify_all();
			thread_sleep(0.01);
			N2[W].notify_one();
		}
	// OR NORTH WANTS TO GO STRAIGHT ---------------------------------------------
	} else if (existsArrivedAndWants2Go(northCar,S)) {
		front[N].notify_all();
		thread_sleep(0.01);
		N2[S].notify_one();
		// SOUTH CAR CAN TURN RIGHT AS THE SAME TIME
		if (existsArrivedAndWants2Go(southCar,E)) {
			front[S].notify_one();
			thread_sleep(0.01);
			S2[E].notify_one();
		}
	// NO STRAIGHTS, SO NOW CHECK LEFTS ------------------------------------------
	// IF NORTH OR SOUTH CARS WANTS TO TURN LEFT
	} else if (existsArrivedAndWants2Go(northCar,E) ||\
		existsArrivedAndWants2Go(southCar,W)) {
		// NORTH CAR CAN GO LEFT
		if (existsArrivedAndWants2Go(northCar,E)) {
			front[N].notify_all();
			thread_sleep(0.01);
			N2[E].notify_one();
		}
		// SOUTH CAN ALSO GO LEFT AT SAME TIME
		if (existsArrivedAndWants2Go(southCar,W)) {
			front[S].notify_all();
			thread_sleep(0.01);
			S2[W].notify_one();
		};
		// WEST CAN ALSO GO RIGHT AT SAME TIME
		if (existsArrivedAndWants2Go(westCar,S)) {
			front[W].notify_all();
			thread_sleep(0.01);
			W2[S].notify_one();
		}
	// NO LEFT OR STRAIGHTS 
	// ANY RIGHT TURNS CAN GO ALL AT SAME TIME
	} else {
		// SOUTH CAN GO RIGHT
		if (existsArrivedAndWants2Go(southCar,E)) {
			front[S].notify_all();
			thread_sleep(0.01);
			S2[E].notify_one();
		}
		// AS WELL AS NORTH
		if (existsArrivedAndWants2Go(northCar,W)) {
			front[N].notify_all();
			thread_sleep(0.01);
			N2[W].notify_one();
		}
		// AS WELL AS WEST
		if (existsArrivedAndWants2Go(westCar,S)) {
			front[W].notify_all();
			thread_sleep(0.01);
			W2[S].notify_one();
		}
	}
	// WAIT 5 SECONDS FOR CARS TO LEAVE INTERSECTION	
	thread_sleep(5);
	currentTime=currentTime+5;
	getWaitTime(eastCar);


	thrEast.front()->join();
	thrEast.pop_front();
	east.pop_front();				
	
	// USING SAME NESTED CONDITIONS TO ENSURE THE CORRECT JOINING OF THREADS
	if (existsArrivedAndWants2Go(westCar, E)) {
		getWaitTime(westCar);
		thrWest.front()->join();
		thrWest.pop_front();
		west.pop_front();				
		if (existsArrivedAndWants2Go(northCar,W)) {
			getWaitTime(northCar);
			thrNorth.front()->join();
			thrNorth.pop_front();
			north.pop_front();				
		}
	} else if (existsArrivedAndWants2Go(northCar,S)) {
		getWaitTime(northCar);


		thrNorth.front()->join();
		thrNorth.pop_front();
		north.pop_front();				
		if (existsArrivedAndWants2Go(southCar,E)) {
			getWaitTime(southCar);


			thrSouth.front()->join();
			thrSouth.pop_front();
			south.pop_front();				
		}
	} else if (existsArrivedAndWants2Go(northCar,E) ||\
		existsArrivedAndWants2Go(southCar,W)) {
		if (existsArrivedAndWants2Go(northCar,E)) {
			getWaitTime(northCar);


			thrNorth.front()->join();
			thrNorth.pop_front();
			north.pop_front();				
		}
		if (existsArrivedAndWants2Go(southCar,W)) {
			getWaitTime(southCar);


			thrSouth.front()->join();
			thrSouth.pop_front();
			south.pop_front();				
		}
		if (existsArrivedAndWants2Go(westCar,S)) {
			getWaitTime(westCar);


			thrWest.front()->join();
			thrWest.pop_front();
			west.pop_front();				
		}
	} else {
		if (existsArrivedAndWants2Go(southCar,E)) {
			getWaitTime(southCar);


			thrSouth.front()->join();
			thrSouth.pop_front();
			south.pop_front();				
		}
		if (existsArrivedAndWants2Go(northCar,W)) {
			getWaitTime(northCar);


			thrNorth.front()->join();
			thrNorth.pop_front();
			north.pop_front();				
		}
		if (existsArrivedAndWants2Go(westCar,S)) {
			getWaitTime(westCar);


			thrWest.front()->join();
			thrWest.pop_front();
			west.pop_front();				
		}
	}

}

// -----------------------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// // // // // // --------------------------------------------------------------
// EAST CAR GOING SOUTH --------------------------------------------------------
// EAST CAR TURNING LEFT -------------------------------------------------------
// // // // // // --------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// -----------------------------------------------------------------------------
/**
		Determines which cars can enter intersection at the same time when priority 
		car is east going south. 

		@param Car* to head of north, east, south and west lists
		@return none
*/
void Intersection::eastCarGoingSouth(Car* northCar, Car* eastCar, Car* southCar,\
	Car* westCar) {
	
	front[E].notify_all();
	thread_sleep(0.05); // must wait this long otherwise thread will not synch

	E2[S].notify_one();
	thread_sleep(0.01);

	// NORTH CAR THAT WANTS TO GO WEST, NON-INTERSECTING RIGHT TURN --------------
	if (northCar != NULL && northCar->arrival <= currentTime &&
		northCar->want2Go == W) {
			front[N].notify_all();
			thread_sleep(0.01);
			N2[W].notify_one();
	}
	// ---------------------------------------------------------------------------
	// SOUTH CAR THAT WANTS TO GO EAST, NON-INTERSECTING RIGHT TURN --------------
	if (southCar != NULL && southCar->arrival <= currentTime &&
		southCar->want2Go == E) {
			front[S].notify_all();
			thread_sleep(0.01);
			S2[E].notify_one();
	}
	// ---------------------------------------------------------------------------
	// WEST CAR THAT WANTS TO GO NORTH, NON-INTERSECTING LEFT TURN ---------------
	if (westCar != NULL && westCar->arrival <= currentTime &&
		westCar->want2Go == N) {
			front[W].notify_all();
			thread_sleep(0.01);
			W2[N].notify_one();
	}
	// WAIT 5 SECONDS FOR CARS TO LEAVE INTERSECTION
	thread_sleep(5);
	currentTime=currentTime+5;
	// JOIN THREADS THAT WERE ALLOWED IN THE INTERSECTION
	if (northCar != NULL && northCar->arrival <= currentTime &&
		northCar->want2Go == W) {
			getWaitTime(northCar);	


			thrNorth.front()->join();
			thrNorth.pop_front();
			north.pop_front();
	}
	if (southCar != NULL && southCar->arrival <= currentTime &&
		southCar->want2Go == E) {
			getWaitTime(southCar);	


			thrSouth.front()->join();
			thrSouth.pop_front();
			south.pop_front();
	}
	if (westCar != NULL && westCar->arrival <= currentTime &&
		westCar->want2Go == N){
			getWaitTime(westCar);	


			thrWest.front()->join();
			thrWest.pop_front();
			west.pop_front();
	}
	getWaitTime(eastCar);	


	thrEast.front()->join();
	thrEast.pop_front();
	east.pop_front();


}

// -----------------------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// // // // // // --------------------------------------------------------------
// EAST CAR GOING WEST ---------------------------------------------------------
// EAST CAR GOING STRAIGHT -----------------------------------------------------
// // // // // // --------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// -----------------------------------------------------------------------------
/**
		Determines which cars can enter intersection at the same time when priority 
		car is east going west. 

		@param Car* to head of north, east, south and west lists
		@return none
*/
void Intersection::eastCarGoingWest(Car* northCar, Car* eastCar, Car* southCar,\
	Car* westCar) {
	
	front[E].notify_all();
	thread_sleep(0.05); // must wait this long otherwise thread will not synch

	E2[W].notify_one();
	thread_sleep(0.01);

	// IF THERES A WEST CAR ------------------------------------------------------
	if (existsArrivedAndWants2Go(westCar,S) ||\
		existsArrivedAndWants2Go(westCar,E)) {
		front[W].notify_all();
		thread_sleep(0.01);

		// IF WEST CAR WANTS TO GO SOUTH, SOUTH CAR CAN GO EAST
		if (westCar->want2Go == S) {
			W2[S].notify_one();
			if (existsArrivedAndWants2Go(southCar,E)) {
				front[S].notify_all();
				thread_sleep(0.01);
				S2[E].notify_one();
				// SLEEP MAIN THREAD FOR 5 SECONDS
				thread_sleep(5);
				currentTime = currentTime+5;
				// ADD DELAY CALCULATION
				getWaitTime(southCar);	
				getWaitTime(westCar);	
				getWaitTime(eastCar);	

				thrSouth.front()->join();
				thrSouth.pop_front();
				south.pop_front();
				thrWest.front()->join();
				thrWest.pop_front();
				west.pop_front();
				thrEast.front()->join();
				thrEast.pop_front();
				east.pop_front();
			// NO SOUTH CAR TO GO AT THE SAME TIME
			} else {
				thread_sleep(5);
				currentTime = currentTime+5;
				getWaitTime(westCar);	
				getWaitTime(eastCar);	
				thrWest.front()->join();
				thrWest.pop_front();
				west.pop_front();
				thrEast.front()->join();
				thrEast.pop_front();
				east.pop_front();
			}
		// IF WEST CAR WANTS TO GO EAST, ONLY WEST CAN GO
		} else if (westCar->want2Go == E) {
			W2[E].notify_one();
			thread_sleep(5);
			currentTime=currentTime+5;
			getWaitTime(westCar);	
			getWaitTime(eastCar);	
			thrWest.front()->join();
			thrWest.pop_front();
			west.pop_front();
			thrEast.front()->join();
			thrEast.pop_front();
			east.pop_front();
		}
	// NO WEST CAR ---------------------------------------------------------------
	// IF THERES AN SOUTH CAR AND NO EAST CAR	------------------------------------
	} else if (existsArrivedAndWants2Go(southCar,E)) {
		front[S].notify_all();
		thread_sleep(0.01);
		S2[E].notify_one();
		thread_sleep(5);
		currentTime=currentTime+5;
		getWaitTime(southCar);	
		getWaitTime(eastCar);	
		thrSouth.front()->join();
		thrSouth.pop_front();
		south.pop_front();
		thrEast.front()->join();
		thrEast.pop_front();
		east.pop_front();
	// NO SOUTH CAR --------------------------------------------------------------
	// ONLY EAST CAN GO ----------------------------------------------------------
	} else {
		thread_sleep(5);
		currentTime = currentTime+5;
		getWaitTime(eastCar);	
		thrEast.front()->join();
		thrEast.pop_front();
		east.pop_front();
	}

}

// =============================================================================	
// =============================================================================
// SOUTH CAR INTERSECTION MANAGEMENT
// =============================================================================
// =============================================================================
// -----------------------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// // // // // // --------------------------------------------------------------
// SOUTH CAR GOING NORTH -------------------------------------------------------
// SOUTH CAR GOING STRAIGHT ----------------------------------------------------
// // // // // // --------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// -----------------------------------------------------------------------------
/**
		Determines which cars can enter intersection at the same time when priority 
		car is south going north. 

		@param Car* to head of north, east, south and west lists
		@return none
*/
void Intersection::southCarGoingNorth(Car* northCar, Car* eastCar, Car* southCar,\
	Car* westCar) {
	
	front[S].notify_all();
	thread_sleep(0.05); // must wait this long otherwise thread will not synch

	S2[N].notify_one();
	thread_sleep(0.01);

	// IF THERES A NORTH CAR -----------------------------------------------------
	if (existsArrivedAndWants2Go(northCar,W) ||\
		existsArrivedAndWants2Go(northCar,S)) {
		front[N].notify_all();
		thread_sleep(0.01);

		// IF NORTH CAR WANTS TO GO WEST, WEST CAR AN GO SOUTH
		if (northCar->want2Go == W) {
			N2[W].notify_one();
			if (existsArrivedAndWants2Go(westCar,S)) {

				front[W].notify_all();
				thread_sleep(0.01);
				W2[S].notify_one();
				// SLEEP MAIN THREAD FOR 5 SECONDS
				thread_sleep(5);
				currentTime = currentTime+5;
				// SAVE WAIT TIMES FOR CARS
				getWaitTime(westCar);	
				getWaitTime(northCar);	
				getWaitTime(southCar);	
				thrWest.front()->join();
				thrWest.pop_front();
				west.pop_front();
				thrNorth.front()->join();
				thrNorth.pop_front();
				north.pop_front();
				thrSouth.front()->join();
				thrSouth.pop_front();
				south.pop_front();
			// NO WEST CAR TO GO AT THE SAME TIME
			} else {
				thread_sleep(5);
				currentTime = currentTime+5;
				getWaitTime(northCar);	
				getWaitTime(southCar);	
				thrNorth.front()->join();
				thrNorth.pop_front();
				north.pop_front();
				thrSouth.front()->join();
				thrSouth.pop_front();
				south.pop_front();
			}
		// IF NORTH CAR WANTS TO GO SOUTH, ONLY NORTH CAN GO
		} else if (northCar->want2Go == S) {
			N2[S].notify_one();
			thread_sleep(5);
			currentTime=currentTime+5;
			getWaitTime(northCar);	
			getWaitTime(southCar);	
			thrNorth.front()->join();
			thrNorth.pop_front();
			north.pop_front();
			thrSouth.front()->join();
			thrSouth.pop_front();
			south.pop_front();
		}
	// NO NORTH CAR --------------------------------------------------------------
	// IF THERES AN WEST CAR AND NO NORTH CAR	------------------------------------
	} else if (existsArrivedAndWants2Go(westCar,S)) {
		front[W].notify_all();
		thread_sleep(0.01);
		W2[S].notify_one();
		thread_sleep(5);
		currentTime=currentTime+5;
		getWaitTime(westCar);	
		getWaitTime(southCar);	
		thrWest.front()->join();
		thrWest.pop_front();
		west.pop_front();
		thrSouth.front()->join();
		thrSouth.pop_front();
		south.pop_front();
	// NO WEST CAR ---------------------------------------------------------------
	// ONLY SOUTH CAN GO ---------------------------------------------------------
	} else {
		thread_sleep(5);
		currentTime = currentTime+5;
		getWaitTime(southCar);	
		thrSouth.front()->join();
		thrSouth.pop_front();
		south.pop_front();
	}

}
// -----------------------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// // // // // // --------------------------------------------------------------
// SOUTH CAR GOING EAST --------------------------------------------------------
// SOUTH CAR TURNING RIGHT -----------------------------------------------------
// // // // // // --------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// -----------------------------------------------------------------------------
/**
		Determines which cars can enter intersection at the same time when priority 
		car is south going east. 

		@param Car* to head of north, east, south and west lists
		@return none
*/
void Intersection::southCarGoingEast(Car* northCar, Car* eastCar, Car* southCar,\
	Car* westCar) {
	
	front[S].notify_all();
	thread_sleep(0.05); // must wait this long otherwise thread will not synch

	S2[E].notify_one();
	thread_sleep(0.01);
	
	// FIRST CHECK IF A STRAIGHT CAR CAN GO AS WELL ------------------------------
	// NORTH CAN WANTS TO GO STRAIGHT --------------------------------------------
	if (existsArrivedAndWants2Go(northCar, S)) {
		front[N].notify_all();
		thread_sleep(0.01);
		N2[S].notify_one();
		// EAST CAR CAN TURN RIGHT AT THE SAME TIME
		if (existsArrivedAndWants2Go(eastCar,N)) {
			front[E].notify_all();
			thread_sleep(0.01);
			E2[N].notify_one();
		}
	// OR EAST WANTS TO GO STRAIGHT ----------------------------------------------
	} else if (existsArrivedAndWants2Go(eastCar,W)) {
		front[E].notify_all();
		thread_sleep(0.01);
		E2[W].notify_one();
		// WEST CAR CAN TURN RIGHT AS THE SAME TIME
		if (existsArrivedAndWants2Go(westCar,S)) {
			front[W].notify_one();
			thread_sleep(0.01);
			W2[S].notify_one();
		}
	// NO STRAIGHTS, SO NOW CHECK LEFTS ------------------------------------------
	// IF EAST OR WEST CARS WANTS TO TURN LEFT
	} else if (existsArrivedAndWants2Go(eastCar,S) ||\
		existsArrivedAndWants2Go(westCar,N)) {
		// EAST CAR CAN GO LEFT
		if (existsArrivedAndWants2Go(eastCar,S)) {
			front[E].notify_all();
			thread_sleep(0.01);
			E2[S].notify_one();
		}
		// WEST CAN ALSO GO LEFT AT SAME TIME
		if (existsArrivedAndWants2Go(westCar,N)) {
			front[W].notify_all();
			thread_sleep(0.01);
			W2[N].notify_one();
		};
		// NORTH CAN ALSO GO RIGHT AT SAME TIME
		if (existsArrivedAndWants2Go(northCar,W)) {
			front[N].notify_all();
			thread_sleep(0.01);
			N2[W].notify_one();
		}
	// NO LEFT OR STRAIGHTS 
	// ANY RIGHT TURNS CAN GO ALL AT SAME TIME
	} else {
		// WEST CAN GO RIGHT
		if (existsArrivedAndWants2Go(westCar,S)) {
			front[W].notify_all();
			thread_sleep(0.01);
			W2[S].notify_one();
		}
		// AS WELL AS NORTH
		if (existsArrivedAndWants2Go(northCar,W)) {
			front[N].notify_all();
			thread_sleep(0.01);
			N2[W].notify_one();
		}
		// AS WELL AS EAST
		if (existsArrivedAndWants2Go(eastCar,N)) {
			front[E].notify_all();
			thread_sleep(0.01);
			E2[N].notify_one();
		}
	}

	// WAIT 5 SECONDS FOR CARS TO LEAVE INTERSECTION
	thread_sleep(5);
	currentTime=currentTime+5;
	getWaitTime(southCar);
	thrSouth.front()->join();
	thrSouth.pop_front();
	south.pop_front();				
	
	// USING SAME NESTED CONDITIONS TO ENSURE THE CORRECT JOINING OF THREADS
	if (existsArrivedAndWants2Go(northCar, S)) {
		getWaitTime(northCar);
		thrNorth.front()->join();
		thrNorth.pop_front();
		north.pop_front();		
		if (existsArrivedAndWants2Go(eastCar,N)) {
			getWaitTime(eastCar);
			thrEast.front()->join();
			thrEast.pop_front();
			east.pop_front();		
		}
	} else if (existsArrivedAndWants2Go(eastCar,W)) {
		getWaitTime(eastCar);
		thrEast.front()->join();
		thrEast.pop_front();
		east.pop_front();				
		if (existsArrivedAndWants2Go(westCar,S)) {
			getWaitTime(westCar);
			thrWest.front()->join();
			thrWest.pop_front();
			west.pop_front();				
		}
	} else if (existsArrivedAndWants2Go(eastCar,S) ||\
		existsArrivedAndWants2Go(westCar,N)) {
		if (existsArrivedAndWants2Go(eastCar,S)) {
			getWaitTime(eastCar);
			thrEast.front()->join();
			thrEast.pop_front();
			east.pop_front();				
		}
		if (existsArrivedAndWants2Go(westCar,N)) {
			getWaitTime(westCar);
			thrWest.front()->join();
			thrWest.pop_front();
			west.pop_front();				
		};
		if (existsArrivedAndWants2Go(northCar,W)) {
			getWaitTime(northCar);
			thrNorth.front()->join();
			thrNorth.pop_front();
			north.pop_front();		
		}
	} else {
		if (existsArrivedAndWants2Go(westCar,S)) {
			getWaitTime(westCar);
			thrWest.front()->join();
			thrWest.pop_front();
			west.pop_front();				
		}
		if (existsArrivedAndWants2Go(northCar,W)) {
			getWaitTime(northCar);
			thrNorth.front()->join();
			thrNorth.pop_front();
			north.pop_front();		
		}
		if (existsArrivedAndWants2Go(eastCar,N)) {
			getWaitTime(eastCar);
			thrEast.front()->join();
			thrEast.pop_front();
			east.pop_front();				
		}
	}

}

// -----------------------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// // // // // // --------------------------------------------------------------
// SOUTH CAR GOING WEST --------------------------------------------------------
// SOUTH CAR TURNING LEFT ------------------------------------------------------
// // // // // // --------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// -----------------------------------------------------------------------------
/**
		Determines which cars can enter intersection at the same time when priority 
		car is south going west. 

		@param Car* to head of north, east, south and west lists
		@return none
*/
void Intersection::southCarGoingWest(Car* northCar, Car* eastCar, Car* southCar,\
	Car* westCar) {
	
	front[S].notify_all();
	thread_sleep(0.05); // must wait this long otherwise thread will not synch

	S2[W].notify_one();
	thread_sleep(0.01);

	// WEST CAR THAT WANTS TO GO SOUTH, NON-INTERSECTING RIGHT TURN --------------
	if (westCar != NULL && westCar->arrival <= currentTime &&
		westCar->want2Go == S) {
			front[W].notify_all();
			thread_sleep(0.01);
			W2[S].notify_one();
	}
	// ---------------------------------------------------------------------------
	// NORTH CAR THAT WANTS TO GO EAST, NON-INTERSECTING LEFT TURN ---------------
	if (northCar != NULL && northCar->arrival <= currentTime &&
		northCar->want2Go == E) {
			front[N].notify_all();
			thread_sleep(0.01);
			N2[E].notify_one();
	}
	// ---------------------------------------------------------------------------
	// EAST CAR THAT WANTS TO GO NORTH, NON-INTERSECTING RIGHT TURN --------------
	if (eastCar != NULL && eastCar->arrival <= currentTime &&
		eastCar->want2Go == N) {
			front[E].notify_all();
			thread_sleep(0.01);
			E2[N].notify_one();
	}
	// WAIT 5 SECONDS FOR CARS TO LEAVE INTERSECTION
	thread_sleep(5);
	currentTime=currentTime+5;
	// JOIN THREADS THAT WERE ALLOWED IN THE INTERSECTION
	if (westCar != NULL && westCar->arrival <= currentTime &&
		westCar->want2Go == S) {
			getWaitTime(westCar);	
			thrWest.front()->join();
			thrWest.pop_front();
			west.pop_front();
	}
	if (northCar != NULL && northCar->arrival <= currentTime &&
		northCar->want2Go == E) {
			getWaitTime(northCar);	
			thrNorth.front()->join();
			thrNorth.pop_front();
			north.pop_front();
	}
	if (eastCar != NULL && eastCar->arrival <= currentTime &&
		eastCar->want2Go == N) {
			getWaitTime(eastCar);	
			thrEast.front()->join();
			thrEast.pop_front();
			east.pop_front();
	}
	getWaitTime(southCar);	
	thrSouth.front()->join();
	thrSouth.pop_front();
	south.pop_front();

}

// =============================================================================	
// =============================================================================
// WEST CAR INTERSECTION MANAGEMENT
// =============================================================================
// =============================================================================

// -----------------------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// // // // // // --------------------------------------------------------------
// WEST CAR GOING NORTH --------------------------------------------------------
// WEST CAR TURNING LEFT -------------------------------------------------------
// // // // // // --------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// -----------------------------------------------------------------------------
/**
		Determines which cars can enter intersection at the same time when priority 
		car is west going north. 

		@param Car* to head of north, east, south and west lists
		@return none
*/
void Intersection::westCarGoingNorth(Car* northCar, Car* eastCar, Car* southCar,\
	Car* westCar) {
	
	front[W].notify_all();
	thread_sleep(0.05); // must wait this long otherwise thread will not synch

	W2[N].notify_one();
	thread_sleep(0.01);

	// NORTH CAR THAT WANTS TO GO WEST, NON-INTERSECTING RIGHT TURN --------------
	if (northCar != NULL && northCar->arrival <= currentTime &&
		northCar->want2Go == W) {
			front[N].notify_all();
			thread_sleep(0.01);
			N2[W].notify_one();
	}
	// ---------------------------------------------------------------------------
	// SOUTH CAR THAT WANTS TO GO EAST, NON-INTERSECTING RIGHT TURN --------------
	if (southCar != NULL && southCar->arrival <= currentTime &&
		southCar->want2Go == E) {
			front[S].notify_all();
			thread_sleep(0.01);
			S2[E].notify_one();
	}
	// ---------------------------------------------------------------------------
	// EAST CAR THAT WANTS TO GO SOUTH, NON-INTERSECTING LEFT TURN ---------------
	if (eastCar != NULL && eastCar->arrival <= currentTime &&
		eastCar->want2Go == S) {
			front[E].notify_all();
			thread_sleep(0.01);
			E2[S].notify_one();
	}
	// WAIT 5 SECONDS FOR CARS TO LEAVE INTERSECTION
	thread_sleep(5);
	currentTime=currentTime+5;
	// JOIN THREADS THAT WERE ALLOWED IN THE INTERSECTION
	if (northCar != NULL && northCar->arrival <= currentTime &&
		northCar->want2Go == W) {
			getWaitTime(northCar);	
			thrNorth.front()->join();
			thrNorth.pop_front();
			north.pop_front();
	}
	if (southCar != NULL && southCar->arrival <= currentTime &&
		southCar->want2Go == E) {
			getWaitTime(southCar);	
			thrSouth.front()->join();
			thrSouth.pop_front();
			south.pop_front();
	}
	if (eastCar != NULL && eastCar->arrival <= currentTime &&
		eastCar->want2Go == S){
			getWaitTime(eastCar);	
			thrEast.front()->join();
			thrEast.pop_front();
			east.pop_front();
	}
	getWaitTime(westCar);	
	thrWest.front()->join();
	thrWest.pop_front();
	west.pop_front();

}

// -----------------------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// // // // // // --------------------------------------------------------------
// WEST CAR GOING EAST ---------------------------------------------------------
// WEST CAR GOING STRAIGHT -----------------------------------------------------
// // // // // // --------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// -----------------------------------------------------------------------------
/**
		Determines which cars can enter intersection at the same time when priority 
		car is west going east. 

		@param Car* to head of north, east, south and west lists
		@return none
*/
void Intersection::westCarGoingEast(Car* northCar, Car* eastCar, Car* southCar,\
	Car* westCar) {
	
	front[W].notify_all();
	thread_sleep(0.05); // must wait this long otherwise thread will not synch

	W2[E].notify_one();
	thread_sleep(0.01);

	// IF THERES A EAST CAR ------------------------------------------------------
	if (existsArrivedAndWants2Go(eastCar,N) ||\
		existsArrivedAndWants2Go(eastCar,W)) {
		front[E].notify_all();
		thread_sleep(0.01);

		// IF EAST CAR WANTS TO GO NORTH, NORTH CAR AN GO WEST
		if (eastCar->want2Go == N) {
			E2[N].notify_one();
			if (existsArrivedAndWants2Go(northCar,W)) {

				front[N].notify_all();
				thread_sleep(0.01);
				N2[W].notify_one();
				// SLEEP MAIN THREAD FOR 5 SECONDS
				thread_sleep(5);
				currentTime = currentTime+5;
				// SAVE WAIT TIMES
				getWaitTime(northCar);
				getWaitTime(eastCar);
				getWaitTime(westCar);

				thrNorth.front()->join();
				thrNorth.pop_front();
				north.pop_front();
				thrEast.front()->join();
				thrEast.pop_front();
				east.pop_front();
				thrWest.front()->join();
				thrWest.pop_front();
				west.pop_front();
			// NO NORTH CAR TO GO AT THE SAME TIME
			} else {
				thread_sleep(5);
				currentTime = currentTime+5;
				getWaitTime(eastCar);
				getWaitTime(westCar);
				thrEast.front()->join();
				thrEast.pop_front();
				east.pop_front();
				thrWest.front()->join();
				thrWest.pop_front();
				west.pop_front();
			}
		// IF EAST CAR WANTS TO GO WEST, ONLY EAST CAN GO
		} else if (eastCar->want2Go == W) {
			E2[W].notify_one();
			thread_sleep(5);
			currentTime=currentTime+5;
			getWaitTime(eastCar);
			getWaitTime(westCar);
			thrEast.front()->join();
			thrEast.pop_front();
			east.pop_front();
			thrWest.front()->join();
			thrWest.pop_front();
			west.pop_front();
		}
	// NO EAST CAR ---------------------------------------------------------------
	// IF THERES AN NORTH CAR AND NO WEST CAR	------------------------------------
	} else if (existsArrivedAndWants2Go(northCar,W)) {
		front[N].notify_all();
		thread_sleep(0.01);
		N2[W].notify_one();
		thread_sleep(5);
		currentTime=currentTime+5;
		getWaitTime(northCar);
		getWaitTime(westCar);
		thrNorth.front()->join();
		thrNorth.pop_front();
		north.pop_front();
		thrWest.front()->join();
		thrWest.pop_front();
		west.pop_front();
	// NO NORTH CAR --------------------------------------------------------------
	// ONLY WEST CAN GO ----------------------------------------------------------
	} else {
		thread_sleep(5);
		currentTime = currentTime+5;
		getWaitTime(westCar);
		thrWest.front()->join();
		thrWest.pop_front();
		west.pop_front();
	}

}

// -----------------------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// // // // // // --------------------------------------------------------------
// WEST CAR GOING SOUTH --------------------------------------------------------
// WEST CAR TURNING RIGHT ------------------------------------------------------
// // // // // // --------------------------------------------------------------
// // // // // -----------------------------------------------------------------
// -----------------------------------------------------------------------------
/**
		Determines which cars can enter intersection at the same time when priority 
		car is west going south. 

		@param Car* to head of north, east, south and west lists
		@return none
*/
void Intersection::westCarGoingSouth(Car* northCar, Car* eastCar, Car* southCar,\
	Car* westCar) {
	
	front[W].notify_all();
	thread_sleep(0.05); // must wait this long otherwise thread will not synch
				
	W2[S].notify_one();
	thread_sleep(0.01);
	
	// FIRST CHECK IF A STRAIGHT CAR CAN GO AS WELL ------------------------------
	// EAST CAN WANTS TO GO STRAIGHT ---------------------------------------------
	if (existsArrivedAndWants2Go(eastCar, W)) {
		front[E].notify_all();
		thread_sleep(0.01);
		E2[W].notify_one();
		// SOUTH CAR CAN TURN RIGHT AT THE SAME TIME
		if (existsArrivedAndWants2Go(southCar,E)) {
			front[S].notify_all();
			thread_sleep(0.01);
			S2[E].notify_one();
		}
	// OR SOUTH WANTS TO GO STRAIGHT ---------------------------------------------
	} else if (existsArrivedAndWants2Go(southCar,N)) {
		front[S].notify_all();
		thread_sleep(0.01);
		S2[N].notify_one();
		// NORTH CAR CAN TURN RIGHT AS THE SAME TIME
		if (existsArrivedAndWants2Go(northCar,W)) {
			front[N].notify_one();
			thread_sleep(0.01);
			N2[W].notify_one();
		}
	// NO STRAIGHTS, SO NOW CHECK LEFTS ------------------------------------------
	// IF NORTH OR SOUTH CARS WANTS TO TURN LEFT
	} else if (existsArrivedAndWants2Go(northCar,E) ||\
		existsArrivedAndWants2Go(southCar,W)) {
		// NORTH CAR CAN GO LEFT
		if (existsArrivedAndWants2Go(northCar,E)) {
			front[N].notify_all();
			thread_sleep(0.01);
			N2[E].notify_one();
		}
		// SOUTH CAN ALSO GO LEFT AT SAME TIME
		if (existsArrivedAndWants2Go(southCar,W)) {
			front[S].notify_all();
			thread_sleep(0.01);
			S2[W].notify_one();
		};
		// EAST CAN ALSO GO RIGHT AT SAME TIME
		if (existsArrivedAndWants2Go(eastCar,N)) {
			front[E].notify_all();
			thread_sleep(0.01);
			E2[N].notify_one();
		}
	// NO LEFT OR STRAIGHTS 
	// ANY RIGHT TURNS CAN GO ALL AT SAME TIME
	} else {
		// SOUTH CAN GO RIGHT
		if (existsArrivedAndWants2Go(southCar,E)) {
			front[S].notify_all();
			thread_sleep(0.01);
			S2[E].notify_one();
		}
		// AS WELL AS NORTH
		if (existsArrivedAndWants2Go(northCar,W)) {
			front[N].notify_all();
			thread_sleep(0.01);
			N2[W].notify_one();
		}
		// AS WELL AS EAST
		if (existsArrivedAndWants2Go(eastCar,N)) {
			front[E].notify_all();
			thread_sleep(0.01);
			E2[N].notify_one();
		}
	}

	// WAIT 5 SECONDS FOR CARS TO LEAVE INTERSECTION	
	thread_sleep(5);
	currentTime=currentTime+5;
	getWaitTime(westCar);
	thrWest.front()->join();
	thrWest.pop_front();
	west.pop_front();				
	
	// USING SAME NESTED CONDITIONS TO ENSURE THE CORRECT JOINING OF THREADS
	if (existsArrivedAndWants2Go(eastCar, W)) {
		getWaitTime(eastCar);
		thrEast.front()->join();
		thrEast.pop_front();
		east.pop_front();				
		if (existsArrivedAndWants2Go(southCar,E)) {
			getWaitTime(southCar);
			thrSouth.front()->join();
			thrSouth.pop_front();
			south.pop_front();				
		}
	} else if (existsArrivedAndWants2Go(southCar,N)) {
		getWaitTime(southCar);
		thrSouth.front()->join();
		thrSouth.pop_front();
		south.pop_front();				
		if (existsArrivedAndWants2Go(northCar,W)) {
			getWaitTime(northCar);
			thrNorth.front()->join();
			thrNorth.pop_front();
			north.pop_front();				
		}
	} else if (existsArrivedAndWants2Go(northCar,E) ||\
		existsArrivedAndWants2Go(southCar,W)) {
		if (existsArrivedAndWants2Go(northCar,E)) {
			getWaitTime(northCar);
			thrNorth.front()->join();
			thrNorth.pop_front();
			north.pop_front();				
		}
		if (existsArrivedAndWants2Go(southCar,W)) {
			getWaitTime(southCar);
			thrSouth.front()->join();
			thrSouth.pop_front();
			south.pop_front();				
		}
		if (existsArrivedAndWants2Go(eastCar,N)) {
			getWaitTime(eastCar);
			thrEast.front()->join();
			thrEast.pop_front();
			east.pop_front();				
		}
	} else {
		if (existsArrivedAndWants2Go(southCar,E)) {
			getWaitTime(southCar);
			thrSouth.front()->join();
			thrSouth.pop_front();
			south.pop_front();				
		}
		if (existsArrivedAndWants2Go(northCar,W)) {
			getWaitTime(northCar);
			thrNorth.front()->join();
			thrNorth.pop_front();
			north.pop_front();				
		}
		if (existsArrivedAndWants2Go(eastCar,N)) {
			getWaitTime(eastCar);
			thrEast.front()->join();
			thrEast.pop_front();
			east.pop_front();				
		}
	}

}

/**
		Checks if the car pointer being passed in is not NULL and the arrival time
		is less than or equal to the currentTime.

		@param Car* to check
		@return True if it exists and if arrival time is less than currentTime,
		False otherwise
*/
bool Intersection::existsAndArrived(Car* checkCar) {
	if (checkCar != NULL && checkCar->arrival <= currentTime) return true;
	else return false;
}

/**
		Checks if the car pointer being passed in is not NULL and the arrival time
		is less than or equal to the currentTime and it wants to go in direction
		passed in.

		@param Car* to check, direction to check (N,S,E,W)
		@return True if it exists and if arrival time is less than currentTime and
		car is going in same direction as dir, False otherwise.
*/
bool Intersection::existsArrivedAndWants2Go(Car* checkCar, int dir) {
	if (checkCar != NULL && checkCar->arrival <= currentTime &&\
		checkCar->want2Go == dir) return true;
	else return false;
}

/**
		Takes Car* and uses the arrival time to determine its wait time. Stores 
		result in waitTime list.
			
			Wait Time = currentTime - Car->arrival
	
		@param Car* of completed car
		@return none
*/
void Intersection::getWaitTime(Car* doneCar) {
	ofstream fout;
	fout.open("results.txt",ios_base::app);
	waitTimes.push_back(currentTime-doneCar->arrival);
	fout<<currentTime<<" "<<enum2Char(oppositeDir(doneCar->pos));
	if (oppositeDir(doneCar->pos) != doneCar->want2Go) 
		fout<<enum2Char(doneCar->want2Go)<<endl;
	else fout<<endl;

	fout.close();
	return;
} 

/**
		Mid-level function that takes an integer or float (of less than 1) and 
		calls this_thread::sleep_for for specified amount of time in either seconds
		(if parameter is whole number) or milliseconds (if parameter is double less 
		than 1).

		@param double representing the time in second to wait
		@return none
*/
void Intersection::thread_sleep(double value) {
	if ((int)value > 0) {
		this_thread::sleep_for(chrono::seconds((int)value));
	} else {
		this_thread::sleep_for(chrono::milliseconds((int)(value*1000)));
	}	
}
