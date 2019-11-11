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
		int oppositeDir(const int dir);
		void setTurn();
		bool operator<(Car const & rhs);
};
