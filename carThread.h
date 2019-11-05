#pragma <once>

#include <cstdlib>
#include <string>

using namespace std;

const int NUM_DIRECTIONS = 4;

enum direction{
	N,S,W,E,U		// order of these is important!!! if this is changed setTurn()
							// function will not work!!!!!!
};

enum turn {
	straight,rightTurn,leftTurn,unknown
};

class Car_Thread {
	public:
		int pos;
		int want2Go;
		int want2Turn;		
		unsigned int arrival;
		Car_Thread();
		Car_Thread(const string str);
		~Car_Thread();
		int char2Enum(const char c);
		int oppositeDir(const int dir);
		void setTurn();
};
