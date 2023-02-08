#include "solver.h"

int main(int argc, char** argv) {
	Solver s = Solver("../inputs/forest01.txt");

	s.run();
	s.outputCircles("../results/forest01.txt");
	s.display();
	
	return 0;
}