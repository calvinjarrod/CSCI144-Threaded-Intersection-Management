/**
		CSCI144 - Operating Systems, Semester Project: Threaded Intersection Management
		intersection.H
		Purpose: 	Use threads to manage an intersection given a list of car with
							starting position, destination and arrival time.

		@author Calvin Jarrod Smith
		@version 1.0 11/25/2019
*/		

#pragma<once>

#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <list>
#include <condition_variable>
#include <chrono>
#include "car.h"

#define _DEBUG
#ifdef _DEBUG
#define dbout cout
#else 
#define dbout 0 && cout
#endif


class Intersection{
  private:
    const int debug = 1;
    list <Car*> north, east, south, west;
    list <thread*> thrNorth, thrEast, thrSouth, thrWest;
    list <int> waitTimes;
    unsigned int currentTime = 1;
    unsigned int numCars = 0;

    // LOCKS AND CONDITION VARIABLES
    typedef mutex LOCK;
    typedef condition_variable CV;

    LOCK	intersection_mutex[NUM_DIRECTIONS];
    CV 		front[NUM_DIRECTIONS],
					N2[NUM_DIRECTIONS],S2[NUM_DIRECTIONS],
					E2[NUM_DIRECTIONS],W2[NUM_DIRECTIONS];

	public:
    Intersection();
    Intersection(const string inputFile);
		~Intersection();
    void run();
		void makeThreads();
    void runIntersection();
    void calcAverageWait();
    void carParser(const string fileName);
    void carControl(Car * carClass);
    void sortLists();
    bool isHeadofList(Car * check, int dir);
    bool arelistsEmpty();
    Car * getNorthHead();
    Car * getSouthHead();
    Car * getEastHead();
    Car * getWestHead();
		void pickCarShortestArrival(Car*,Car*,Car*,Car*,Car**,unsigned int*);
    void northCarGoingEast(Car*,Car*,Car*,Car*);
    void northCarGoingSouth(Car*,Car*,Car*,Car*);
    void northCarGoingWest(Car*,Car*,Car*,Car*);
    void eastCarGoingNorth(Car*,Car*,Car*,Car*); 
    void eastCarGoingSouth(Car*,Car*,Car*,Car*);
    void eastCarGoingWest(Car*,Car*,Car*,Car*);
    void southCarGoingNorth(Car*,Car*,Car*,Car*);
    void southCarGoingEast(Car*,Car*,Car*,Car*);
    void southCarGoingWest(Car*,Car*,Car*,Car*);
    void westCarGoingNorth(Car*,Car*,Car*,Car*);
    void westCarGoingEast(Car*,Car*,Car*,Car*);
    void westCarGoingSouth(Car*,Car*,Car*,Car*);
    void thread_sleep(double value);
    bool existsAndArrived(Car* checkCar);
    bool existsArrivedAndWants2Go(Car* checkCar, int dir);
    void getWaitTime(Car* doneCar);
};
