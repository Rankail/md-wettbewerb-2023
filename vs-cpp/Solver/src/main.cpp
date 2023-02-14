#include "solver.h"

#include <chrono>

int main(int argc, char** argv) {
	std::string input;
	std::string output;
	double weighting;
	
	// Process Command line arguments
	std::string arg1(argv[1]);
	if ((arg1 == "--manual" && argc != 2) || (arg1 != "--manual" && (argc < 3  || argc > 4))) {
		std::cout << "Usage: ./04_draw.exe {--manual | INPUTFILE OUTPUTFILE [WEIGHTING]}" << std::endl;
		return 1;
	}
	if (arg1 == "--manual") {
		std::cout << "Inputfile:  ";
		std::getline(std::cin, input);
		std::cout << "Outputfile: ";
		std::getline(std::cin, output);
		std::cout << "Weighting:  ";
		std::string weighting_str;
		std::getline(std::cin, weighting_str);
		weighting = weighting_str.empty() ? weighting = 0. : std::stof(weighting_str);
	} else {
		input = std::string(argv[1]);
		output = std::string(argv[2]);
		weighting = (argc == 4) ? std::atof(argv[3]) : 0.;
	}

	auto startTime = std::chrono::high_resolution_clock::now();

	Solver s = Solver();
	if (!s.init(input, weighting)) {
		std::cout << "Failed to initialize Solver!" << std::endl;
		return 2;
	}

	auto result = s.run();
	if (result.circleCountAtMax == -1) {
		std::cout << "An Error occurred during computation!" << std::endl;
		return 3;
	}

	if (!s.writeOutput(result, output)) {
		std::cout << "Failed to save output!" << std::endl;
		return 4;
	}


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