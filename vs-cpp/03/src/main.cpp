#include "solver.h"

int main(int argc, char** argv) {
	if (argc != 3) {
		printf("Usage: 03.exe [inputfile] [outputfile]");
		return 1;
	}
	Solver s = Solver(std::string(argv[1]));

	s.run();
	s.outputCircles(std::string(argv[2]));
	
	return 0;
}