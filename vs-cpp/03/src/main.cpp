#include "solver.h"

#include <chrono>

int main(int argc, char** argv) {
	if (argc != 3) {
		printf("Usage: 03.exe [inputfile] [outputfile]");
		return 1;
	}

	auto startTime = std::chrono::high_resolution_clock::now();

	Solver s = Solver(std::string(argv[1]));

	s.run();
	s.outputCircles(std::string(argv[2]));

	auto endTime = std::chrono::high_resolution_clock::now();

	auto diff = endTime - startTime;

	auto minutes = std::chrono::duration_cast<std::chrono::minutes>(diff);
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(diff - minutes);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff - minutes - seconds);

	std::cout << "Finished after ";
	if (minutes.count() > 0) std::cout << minutes.count() << "min ";
	if (minutes.count() > 0 || seconds.count() > 0) std::cout << seconds.count() << "s ";
	std::cout << ms.count() << "ms" << std::endl;
	
	
	return 0;
}