#include <SDL2/SDL.h>
#include <string>
#include <fstream>
#include <vector>
#include <iostream>

struct Circle {
	double cx, cy, r;
	int type;

	Circle(double cx, double cy, double r, int type)
		: cx(cx), cy(cy), r(r), type(type) {}
};

struct CircleType {
	int index;
	double radius;
	int count;

	CircleType(int index, double radius) : index(index), radius(radius), count(0) {}
};

int main(int argc, char** argv) {
	if (argc != 1 && argc != 3) {
		std::cout << "Usage: ./Checker.exe [INPUT OUTPUT]" << std::endl;
		return 1;
	}
	std::string input, output;
	if (argc == 3) {
		input = std::string(argv[1]);
		output = std::string(argv[2]);
	} else {
		std::cout << "Input-File:    ";
		std::getline(std::cin, input);
		std::cout << "Output-File:   ";
		std::getline(std::cin, output);
	}

	std::ifstream inFile;
	inFile.open(input, std::ios::in);
	if (!inFile.is_open()) {
		std::cout << "Failed to open input-file!" << std::endl;
		return 2;
	}

	std::string line;
	std::getline(inFile, line);
	std::getline(inFile, line);
	int space = line.find(' ');
	double w = std::stod(line.substr(0, space));
	double h = std::stod(line.substr(space));
	auto types = std::vector<CircleType>();
	int i = 0;
	while (std::getline(inFile, line)) {
		space = line.find(' ');
		types.emplace_back(i, std::stod(line.substr(0, space)));
		i++;
	}
	inFile.close();

	std::ifstream outFile;
	outFile.open(output, std::ios::in);
	if (!outFile.is_open()) {
		std::cout << "Failed to open output-file!" << std::endl;
		return 2;
	}
	std::vector<Circle> circles = std::vector<Circle>();
	{
		double cx, cy, r;
		int type;
		int lineNum = 1;
		while (outFile >> cx) {
			outFile >> cy;
			outFile >> r;
			outFile >> type;

			if (types[type].radius != r) {
				std::cout << "Radius for type! Line: " << lineNum << std::endl;
			} else {
				types[type].count++;
			}
			circles.emplace_back(cx, cy, r, type);
			lineNum++;
		}
	}
	outFile.close();

	double size = 0;
	double totalCountSquared = 0.;
	double sumCountsSquared = 0.;
	for (auto const& type : types) {
		size += type.count * type.radius * type.radius;
		totalCountSquared += type.count;
		sumCountsSquared += (double)type.count * (double)type.count;
	}
	totalCountSquared *= totalCountSquared;
	size *= M_PI;

	double A = size / (w * h);
	double D = 1. - (double)sumCountsSquared / totalCountSquared;

	std::cout << "A: " << A << std::endl;
	std::cout << "D: " << D << std::endl;
	std::cout << "B: " << A * D << std::endl;

	double maxDiff = 0.;

	int collCount = 0;
	for (int i = 0; i < circles.size(); i++) {
		auto& c1 = circles[i];
		if (c1.cx < c1.r) {
			std::cout << "Bound: LEFT " << (c1.r - c1.cx) << std::endl;
		}
		if (c1.cy < c1.r) {
			std::cout << "Bound: BOT " << (c1.r - c1.cy) << std::endl;
		}
		if (c1.cx > w - c1.r) {
			std::cout << "Bound: RIGHT " << (c1.r + c1.cx - w) << std::endl;
		}
		if (c1.cy > h - c1.r) {
			std::cout << "Bound: TOP " << (c1.r + c1.cy - h) << std::endl;
		}
		for (int j = i+1; j < circles.size(); j++) {
			auto& c2 = circles[j];
			double dx = c2.cx - c1.cx;
			double dy = c2.cy - c1.cy;
			double r = c2.r + c1.r;
			double diff = r - std::sqrt(dx * dx + dy * dy);
			maxDiff = std::max(maxDiff, diff);
			if (dx * dx + dy * dy < r * r - 1e-10) {
				collCount++;
				std::cout << "Collision: " << "(" << c1.cx << ";" << c1.cy << ";" << c1.r
					<< ") (" << c2.cx << ";" << c2.cy << ";" << c2.r
					<< ") difference=" << diff << std::endl;
			}
		}
	}
	std::cout << "Total collisions: " << collCount << std::endl
		<< "Max Overlap: " << maxDiff << std::endl;
	
	return 0;
}