#include "solver.h"

#include <chrono>

int main(int argc, char** argv) {
	std::string input;
	std::string output;
	Weighting weighting = Weighting();

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
	if (args.size() != 1 && args.size() != 6) {
		std::cout << "Usage: ./Solver.exe [INPUTFILE RADIUS_EXPONENT RADIUS_FACTOR COUNT_EXPONENT COUNT_FACTOR]\n\t\t[--out=OUTPUTFILE]" << std::endl;
		return 1;
	}
	if (args.size() == 1) {
		std::cout << "Inputfile:       ";
		std::getline(std::cin, input);

		std::string weighting_str;
		std::cout << "Radius-Exponent: ";
		std::getline(std::cin, weighting_str);
		weighting.radiusExponent = weighting_str.empty() ? 0 : std::stoi(weighting_str);

		std::cout << "Radius-Factor:   ";
		std::getline(std::cin, weighting_str);
		weighting.radiusFactor = weighting_str.empty() ? 1. : std::stod(weighting_str);

		std::cout << "Count-Exponent:  ";
		std::getline(std::cin, weighting_str);
		weighting.countExponent = weighting_str.empty() ? 0 : std::stoi(weighting_str);

		std::cout << "Count-Factor:    ";
		std::getline(std::cin, weighting_str);
		weighting.countFactor = weighting_str.empty() ? 1. : std::stod(weighting_str);

		if (output.empty()) {
			std::cout << "Outputfile:      ";
			std::getline(std::cin, output);
		}
	} else {
		input = std::string(args[1]);
		weighting.radiusExponent = std::stoi(args[2]);
		weighting.radiusFactor = std::stod(args[3]);
		weighting.countExponent = std::stoi(args[4]);
		weighting.countFactor = std::stod(args[5]);
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