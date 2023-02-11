#include "solver.h"

int main(int argc, char** argv) {
	Solver s = Solver("../inputs/forest07.txt");

	s.run();
	s.outputCircles("../results/forest07.txt");
	
	return 0;
}