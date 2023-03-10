#include "solver.h"

#include <chrono>

int main(int argc, char** argv) {
	std::string input;
	std::string output;
	double weighting;
	unsigned seed;

	std::vector<std::string> args = std::vector<std::string>();
	for (int i = 0; i < argc; i++) {
		args.emplace_back(argv[i]);
	}

	auto it = std::find_if(args.begin(), args.end(), [](const std::string& s) { return s.substr(0, 6) == "--out="; });
	if (it != args.end()) {
		output = it->substr(6);
		args.erase(it);
	}
	
	// Process Command line arguments
	if (args.size() != 1 && args.size() != 4) {
		std::cout << "Usage: ./Solver.exe [INPUTFILE WEIGHTING SEED] [--out=OUTPUTFILE]" << std::endl;
		return 1;
	}
	if (args.size() == 1) {
		std::cout << "Inputfile:  ";
		std::getline(std::cin, input);
		std::cout << "Weighting:  ";
		std::string weighting_str;
		std::getline(std::cin, weighting_str);
		weighting = weighting_str.empty() ? weighting = 0. : std::stod(weighting_str);
		std::cout << "Seed:       ";
		std::string seed_str;
		std::getline(std::cin, seed_str);
		seed = seed_str.empty() ? 0 : std::stoul(seed_str);
		if (output.empty()) {
			std::cout << "Outputfile: ";
			std::getline(std::cin, output);
		}
	} else {
		input = std::string(args[1]);
		weighting = std::stod(args[2]);
		seed = std::stoul(args[3]);
	}

	auto startTime = std::chrono::high_resolution_clock::now();

	// initialize
	Solver s = Solver();
	if (!s.init(input)) {
		std::cout << "Failed to initialize Solver!" << std::endl;
		return 2;
	}

	// run
	auto result = s.run(weighting, seed);
	if (result.circleCountAtMax == -1) {
		std::cout << "An Error occurred during computation!" << std::endl;
		return 3;
	}

	// write result to output-file
	if (!output.empty()) {
		if (!s.writeOutput(result, output)) {
			std::cout << "Failed to save output!" << std::endl;
			return 4;
		}
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