#include "solver.h"

#include <chrono>
#include <string>

int main(int argc, char** argv) {
	if (argc < 3 || argc > 4) {
		printf("Usage: 04.exe inputfile outputfile [weightening]");
		return 1;
	}

	auto startTime = std::chrono::high_resolution_clock::now();

	Solver* s;
	if (argc == 4) s = new Solver(std::string(argv[1]), argv[3]);
	else s = new Solver(std::string(argv[1]));
	s->run();
	s->outputCircles(std::string(argv[2]));

	auto endTime = std::chrono::high_resolution_clock::now();
	delete s;

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