#include "solver.h"

int main(int argc, char** argv) {
	Solver s = Solver("../inputs/forest01.txt");

	s.run();
	s.display();
	
	return 0;
}