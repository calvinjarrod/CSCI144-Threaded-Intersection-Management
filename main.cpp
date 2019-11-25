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

// GLOBALS ---------------------------------------------------------------------

const int debug = 1;
list <Car*> north, east, south, west;
list <thread*> thrNorth, thrEast, thrSouth, thrWest;
list <int> waitTimes;
unsigned int currentTime = 1;
unsigned int numCars;

// END GLOBAL CONSTANTS --------------------------------------------------------

// FUNCTION PROTOTYPES ---------------------------------------------------------

void runIntersection();
void calcAverageWait();
void carParser(const string fileName);
void Car_Control(void * carClass);
void sortLists();
bool isHeadofList(Car * check, int dir);
bool arelistsEmpty();
Car * getNorthHead();
Car * getSouthHead();
Car * getEastHead();
Car * getWestHead();
void thread_sleep(double value);
bool existsAndArrived(Car* checkCar);
bool existsArrivedAndWants2Go(Car* checkCar, int dir);
void getWaitTime(Car* doneCar);

// CONTIDION VARIABLES AND LOCKS ----------------------------------------------

typedef mutex LOCK;
typedef condition_variable CV;

LOCK	intersection_mutex[NUM_DIRECTIONS];
//lock_guard<LOCK> intersectionN(intersection_mutex[N]);
//lock_guard<LOCK> intersectionS(intersection_mutex[S]);
//lock_guard<LOCK> intersectionE(intersection_mutex[E]);
//lock_guard<LOCK> intersectionW(intersection_mutex[W]);
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
	/*
	cout<<"North size: "<<north.size()<<endl;
	cout<<"South size: "<<south.size()<<endl;
	cout<<"West size: "<<west.size()<<endl;
	cout<<"East size: "<<east.size()<<endl;
	*/

	// SORT ALL LISTS BASED ON ARRIVAL TIME
	sortLists();

	/*cout<<"North Elements: "<<endl;
	for (list<Car*>::iterator it = north.begin(); it != north.end(); it++) {
		cout<<"Car arrival: "<<(*it)->arrival<<endl;
	}*/
	

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

	runIntersection();
	calcAverageWait();

	return 0;
}

void runIntersection() {
	// MANAGE INTERSECTION
	Car * priorityCar = NULL;
	unsigned int priPos = U;
	while (!arelistsEmpty()) {

		Car *northCar = getNorthHead();
		Car *southCar = getSouthHead();
		Car *eastCar = getEastHead();
		Car *westCar = getWestHead();

		/*
		// print out current head of queues
		cout<<"-----------------------"<<endl;
		cout<<"Current head of queues."<<endl;
		if (northCar != NULL) cout<<"N: "<<northCar->arrival<<endl;
		else cout<<"N: no car"<<endl;
		if (eastCar != NULL) cout<<"E: "<<eastCar->arrival<<endl;
		else cout<<"e: no car"<<endl;
		if (southCar != NULL) cout<<"S: "<<southCar->arrival<<endl;
		else cout<<"S: no car"<<endl;
		if (westCar != NULL) cout<<"W: "<<westCar->arrival<<endl;
		else cout<<"W: no car"<<endl;
		cout<<"-----------------------"<<endl;
		*/

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
		// CHECK IF NORTH CAR EXISTS AND HAS A SMALLER ARRIVAL TIME
			if (northCar!=NULL && priorityCar!=NULL &&\
				 northCar->arrival < priorityCar->arrival) {
				priorityCar = northCar;
				priPos = N;
			}
			// CHECK IF EAST CAR EXISTS AND HAS A SMALLER ARRIVAL TIME
			if (eastCar!=NULL && priorityCar!=NULL &&\
				 eastCar->arrival < priorityCar->arrival) {
				priorityCar = eastCar;
				priPos = E;
			}
			// CHECK IF SOUTH CAR EXISTS AND HAS A SMALLER ARRIVAL TIME
			if (southCar!=NULL && priorityCar!=NULL &&\
				 southCar->arrival < priorityCar->arrival) {
				priorityCar = southCar;
				priPos = S;
			}
			// CHECK IF WEST CAR EXISTS AND HAS A SMALLER ARRIVAL TIME
			if (westCar!=NULL && priorityCar!=NULL &&\
				 westCar->arrival < priorityCar->arrival) {
				priorityCar = westCar;
				priPos = W;
			}
		// =========================================================================	
		// =========================================================================
		// IF THE PRIORITY CAR IS NORTH
		// =========================================================================
		// =========================================================================
		if (priPos == N) {
			front[N].notify_all();
			thread_sleep(0.05); // must wait this long otherwise thread will not synch

			// -----------------------------------------------------------------------
			// AND THE PRIORITY CAR WANTS TO GO SOUTH
			// NORTH GOING STRAIGHT
			// -----------------------------------------------------------------------
			if (priorityCar->want2Go == S) {
				N2[S].notify_one();
				thread_sleep(0.01);

				// IF THERES A SOUTH CAR -----------------------------------------------
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
				// NO SOUTH CAR --------------------------------------------------------
				// IF THERES AN EAST CAR AND NO SOUTH CAR	------------------------------
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
				// NO EAST CAR ---------------------------------------------------------
				// ONLY NORTH CAN GO ---------------------------------------------------
				} else {
					thread_sleep(5);
					currentTime = currentTime+5;
					getWaitTime(northCar);	
					thrNorth.front()->join();
					thrNorth.pop_front();
					north.pop_front();
				}
			// // // // // 	
			// // // // // // 
			// -----------------------------------------------------------------------
			// PRIORITY CAR WANTS TO GO WEST
			// NORTH CAR TURNING RIGHT
			// -----------------------------------------------------------------------
			// // // // // // 
			// // // // // 
			} else if (priorityCar->want2Go == W) {
				N2[W].notify_one();
				thread_sleep(0.01);
				
				// FIRST CHECK IF A STRAIGHT CAR CAN GO AS WELL ------------------------
				// SOUTH CAN WANTS TO GO STRAIGHT --------------------------------------
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
				// OR WEST WANTS TO GO STRAIGHT ----------------------------------------
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
				// NO STRAIGHTS, SO NOW CHECK LEFTS ------------------------------------
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
			// // // // // 
			// // // // // // 
			// -----------------------------------------------------------------------
			// PRIORITY CAR WANTS TO GO EAST
			// NORTH CAR TURNING LEFT
			// -----------------------------------------------------------------------
			// // // // // // 
			// // // // // 
			} else if (priorityCar->want2Go == E) {
				N2[E].notify_one();
				thread_sleep(0.01);

				// WEST CAR THAT WANTS TO GO SOUTH, NON-INTERSECTING RIGHT TURN --------
				if (westCar != NULL && westCar->arrival <= currentTime &&
					westCar->want2Go == S) {
						front[W].notify_all();
						thread_sleep(0.01);
						W2[S].notify_one();
				}
				// ---------------------------------------------------------------------
				// SOUTH CAR THAT WANTS TO GO WEST, NON-INTERSECTING LEFT TURN ---------
				if (southCar != NULL && southCar->arrival <= currentTime &&
					southCar->want2Go == W) {
						front[S].notify_all();
						thread_sleep(0.01);
						S2[W].notify_one();
				}
				// ---------------------------------------------------------------------
				// EAST CAR THAT WANTS TO GO NORTH, NON-INTERSECTING RIGHT TURN --------
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

		// =========================================================================
		// =========================================================================
		// IF THE PRIORITY CAR IS EAST
		// =========================================================================
		// =========================================================================
		} else if (priPos == E) {
			front[E].notify_all();
			thread_sleep(0.05); // must wait this long otherwise thread will not synch
			// // // // // 
			// // // // // // 
			// -----------------------------------------------------------------------
			// PRIORITY CAR WANTS TO GO NORTH
			// EAST CAR TURNING RIGHT
			// -----------------------------------------------------------------------
			// // // // // // 
			// // // // // 
			if (priorityCar->want2Go == N) {
				E2[N].notify_one();
				thread_sleep(0.01);
				
				// FIRST CHECK IF A STRAIGHT CAR CAN GO AS WELL ------------------------
				// WEST CAN WANTS TO GO STRAIGHT ---------------------------------------
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
				// OR NORTH WANTS TO GO STRAIGHT ---------------------------------------
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
				// NO STRAIGHTS, SO NOW CHECK LEFTS ------------------------------------
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
			// // // // // 
			// // // // // // 
			// -----------------------------------------------------------------------
			// PRIORITY CAR WANTS TO GO SOUTH
			// EAST CAR TURNING LEFT
			// -----------------------------------------------------------------------
			// // // // // // 
			// // // // // 
			} else if (priorityCar->want2Go == S) {
				E2[S].notify_one();
				thread_sleep(0.01);

				// NORTH CAR THAT WANTS TO GO WEST, NON-INTERSECTING RIGHT TURN --------
				if (northCar != NULL && northCar->arrival <= currentTime &&
					northCar->want2Go == W) {
						front[N].notify_all();
						thread_sleep(0.01);
						N2[W].notify_one();
				}
				// ---------------------------------------------------------------------
				// SOUTH CAR THAT WANTS TO GO EAST, NON-INTERSECTING RIGHT TURN --------
				if (southCar != NULL && southCar->arrival <= currentTime &&
					southCar->want2Go == E) {
						front[S].notify_all();
						thread_sleep(0.01);
						S2[E].notify_one();
				}
				// ---------------------------------------------------------------------
				// WEST CAR THAT WANTS TO GO NORTH, NON-INTERSECTING LEFT TURN ---------
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
			// // // // // 
			// // // // // //
			// -----------------------------------------------------------------------
			// PRIOTIRY CAR WANTS TO GO WEST
			// EAST GOING STRAIGHT
			// -----------------------------------------------------------------------
			// // // // // // 
			// // // // //
			} else if (priorityCar->want2Go == W) {
				E2[W].notify_one();
				thread_sleep(0.01);

				// IF THERES A WEST CAR ------------------------------------------------
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
				// NO WEST CAR ---------------------------------------------------------
				// IF THERES AN SOUTH CAR AND NO EAST CAR	------------------------------
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
				// NO SOUTH CAR --------------------------------------------------------
				// ONLY EAST CAN GO ----------------------------------------------------
				} else {
					thread_sleep(5);
					currentTime = currentTime+5;
					getWaitTime(eastCar);	
					thrEast.front()->join();
					thrEast.pop_front();
					east.pop_front();
				}
			}

		// =========================================================================
		// =========================================================================
		// IF THE PRIORITY CAR IS SOUTH
		// =========================================================================
		// =========================================================================
		} else if (priPos == S) {
			front[S].notify_all();
			thread_sleep(0.05); // must wait this long otherwise thread will not synch

			// // // // // 
			// // // // // // 
			// -----------------------------------------------------------------------
			// PRIORITY CAR WANTS TO GO NORTH
			// SOUTH GOING STRAIGHT
			// -----------------------------------------------------------------------
			// // // // // // 
			// // // // // 
			if (priorityCar->want2Go == N) {
				S2[N].notify_one();
				thread_sleep(0.01);

				// IF THERES A NORTH CAR -----------------------------------------------
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
				// NO NORTH CAR --------------------------------------------------------
				// IF THERES AN WEST CAR AND NO NORTH CAR	------------------------------
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
				// NO WEST CAR ---------------------------------------------------------
				// ONLY SOUTH CAN GO ---------------------------------------------------
				} else {
					thread_sleep(5);
					currentTime = currentTime+5;
					getWaitTime(southCar);	
					thrSouth.front()->join();
					thrSouth.pop_front();
					south.pop_front();
				}
				// ---------------------------------------------------------------------

			// // // // // 
			// // // // // // 
			// -----------------------------------------------------------------------
			// PRIORITY CAR WANTS TO GO EAST
			// SOUTH TURNING RIGHT
			// -----------------------------------------------------------------------
			// // // // // // 
			// // // // // 
			} else if (priorityCar->want2Go == E) {
				S2[E].notify_one();
				thread_sleep(0.01);
				
				// FIRST CHECK IF A STRAIGHT CAR CAN GO AS WELL ------------------------
				// NORTH CAN WANTS TO GO STRAIGHT --------------------------------------
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
				// OR EAST WANTS TO GO STRAIGHT ----------------------------------------
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
				// NO STRAIGHTS, SO NOW CHECK LEFTS ------------------------------------
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
			// // // // // 
			// // // // // // 
			// -----------------------------------------------------------------------
			// PRIORITY CAR WANTS TO GO WEST
			// SOUTH TURNING LEFT
			// -----------------------------------------------------------------------
			// // // // // // 
			// // // // // 
			} else if (priorityCar->want2Go == W) {
				S2[W].notify_one();
				thread_sleep(0.01);

				// WEST CAR THAT WANTS TO GO SOUTH, NON-INTERSECTING RIGHT TURN --------
				if (westCar != NULL && westCar->arrival <= currentTime &&
					westCar->want2Go == S) {
						front[W].notify_all();
						thread_sleep(0.01);
						W2[S].notify_one();
				}
				// ---------------------------------------------------------------------
				// NORTH CAR THAT WANTS TO GO EAST, NON-INTERSECTING LEFT TURN ---------
				if (northCar != NULL && northCar->arrival <= currentTime &&
					northCar->want2Go == E) {
						front[N].notify_all();
						thread_sleep(0.01);
						N2[E].notify_one();
				}
				// ---------------------------------------------------------------------
				// EAST CAR THAT WANTS TO GO NORTH, NON-INTERSECTING RIGHT TURN --------
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
		// =========================================================================
		// =========================================================================
		// IF THE PRIORITY CAR IS WEST
		// =========================================================================
		// =========================================================================
		} else if (priPos == W) {
			front[W].notify_all();
			thread_sleep(0.05); // must wait this long otherwise thread will not synch

			// // // // // 
			// // // // // // 
			// -----------------------------------------------------------------------
			// PRIORITY CAR WANTS TO GO NORTH
			// WEST TURNING LEFT
			// -----------------------------------------------------------------------
			// // // // // // 
			// // // // // 
			if (priorityCar->want2Go == N) {
				W2[N].notify_one();
				thread_sleep(0.01);

				// NORTH CAR THAT WANTS TO GO WEST, NON-INTERSECTING RIGHT TURN --------
				if (northCar != NULL && northCar->arrival <= currentTime &&
					northCar->want2Go == W) {
						front[N].notify_all();
						thread_sleep(0.01);
						N2[W].notify_one();
				}
				// ---------------------------------------------------------------------
				// SOUTH CAR THAT WANTS TO GO EAST, NON-INTERSECTING RIGHT TURN --------
				if (southCar != NULL && southCar->arrival <= currentTime &&
					southCar->want2Go == E) {
						front[S].notify_all();
						thread_sleep(0.01);
						S2[E].notify_one();
				}
				// ---------------------------------------------------------------------
				// EAST CAR THAT WANTS TO GO SOUTH, NON-INTERSECTING LEFT TURN ---------
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
			// // // // // 
			// // // // // //
			// -----------------------------------------------------------------------
			// PRIORITY CAR WANTS TO GO SOUTH
			// WEST TURNING RIGHT
			// -----------------------------------------------------------------------
			// // // // // //
			// // // // // 
			} else if (priorityCar->want2Go == S) {
				W2[S].notify_one();
				thread_sleep(0.01);
				
				// FIRST CHECK IF A STRAIGHT CAR CAN GO AS WELL ------------------------
				// EAST CAN WANTS TO GO STRAIGHT ---------------------------------------
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
				// OR SOUTH WANTS TO GO STRAIGHT ---------------------------------------
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
				// NO STRAIGHTS, SO NOW CHECK LEFTS ------------------------------------
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
			// // // // // 
			// // // // // // 
			// -----------------------------------------------------------------------
			// PRIORITY CAR WANTS TO GO EAST
			// WEST CAR GOING STRAIGHT
			// -----------------------------------------------------------------------
			// // // // // //
			// // // // // 
			} else if (priorityCar->want2Go == E) {
				W2[E].notify_one();
				thread_sleep(0.01);

				// IF THERES A EAST CAR ------------------------------------------------
				if (existsArrivedAndWants2Go(eastCar,S) ||\
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
				// NO WEST CAR ---------------------------------------------------------
				// IF THERES AN NORTH CAR AND NO WEST CAR	------------------------------
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
				// NO NORTH CAR --------------------------------------------------------
				// ONLY WEST CAN GO ----------------------------------------------------
				} else {
					thread_sleep(5);
					currentTime = currentTime+5;
					getWaitTime(westCar);
					thrWest.front()->join();
					thrWest.pop_front();
					west.pop_front();
				}
				// ---------------------------------------------------------------------
			}
			// // // // //
			// // // // // //
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

	cout<<"Cars finished at time: "<<currentTime<<endl;
	cout<<numCars<<" total cars"<<endl;	
	return;
}

void calcAverageWait() {
	waitTimes.sort();
	cout<<"Wait time entries "<<waitTimes.size()<<endl;
	unsigned int sum = 0;
	for (list<int>::iterator i = waitTimes.begin(); i != waitTimes.end(); i++) {
		sum += (*i);
	}
	double avg = (double)(sum)/(double)(numCars);
	cout<<"Agerage wait time: "<<avg<<endl;
	cout<<"Least wait time: "<<waitTimes.front()<<endl;
	cout<<"Greatest wait time: "<<waitTimes.back()<<endl;
}

void carParser(const string fileName) {
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

void Car_Control(void * carClass) {
	Car* car = (Car*) carClass;
	numCars++;
	cout<<enum2Char(car->pos)<<" "<<car->arrival<<\
		" thread started! Wants to go: "<<enum2Char(car->want2Go)<<endl;
	unique_lock<LOCK> lock(intersection_mutex[car->pos]);
	while (!isHeadofList(car,car->pos) || car->arrival > currentTime) {
		front[car->pos].wait(lock);
	}
	//thread_sleep(0.01);
	if (car->pos == N) {
		cout<<"North #"<<car->arrival<<" car arrived"<<endl;
		N2[car->want2Go].wait(lock);
		thread_sleep(5);
		cout<<"North #"<<car->arrival<<" car left intersection"<<endl;
	} else if (car->pos == S) {
		cout<<"South #"<<car->arrival<<" car arrived"<<endl;
		S2[car->want2Go].wait(lock);
		thread_sleep(5);
		cout<<"South #"<<car->arrival<<" car left intersection"<<endl;
	} else if (car->pos == E) {
		cout<<"East #"<<car->arrival<<" car arrived"<<endl;
		E2[car->want2Go].wait(lock);
		thread_sleep(5);
		cout<<"East #"<<car->arrival<<" car left intersection"<<endl;
	} else if (car->pos == W) {
		cout<<"West #"<<car->arrival<<" car arrived"<<endl;
		W2[car->want2Go].wait(lock);
		thread_sleep(5);
		cout<<"West #"<<car->arrival<<" car left intersection"<<endl;
	}
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

Car * getEastHead() {
	if (east.size() == 0) return NULL;
	else return east.front();
}

Car * getWestHead() {
	if (west.size() == 0) return NULL;
	else return west.front();
}

bool existsAndArrived(Car* checkCar) {
	if (checkCar != NULL && checkCar->arrival <= currentTime) return true;
	else return false;
}

bool existsArrivedAndWants2Go(Car* checkCar, int dir) {
	if (checkCar != NULL && checkCar->arrival <= currentTime &&\
		checkCar->want2Go == dir) return true;
	else return false;
}

void getWaitTime(Car* doneCar) {
	waitTimes.push_back(currentTime-doneCar->arrival);
	return;
} 
