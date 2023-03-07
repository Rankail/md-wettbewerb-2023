#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>

struct Circle {
	double cx, cy, r;
	std::string line;
};

void scrambleRandom(std::vector<std::string>& lines) {
	std::random_device dv;
	std::mt19937 g(dv());
	std::shuffle(lines.begin(), lines.end(), g);
}

void sortCenterDistance(std::vector<std::string>& lines, double w, double h) {
	std::vector<Circle> distances = std::vector<Circle>();
	double maxX = 0., maxY = 0.;
	for (auto& line : lines) {
		size_t s1 = line.find(' ');
		size_t s2 = line.find(' ', s1 + 1);
		size_t s3 = line.find(' ', s2 + 1);
		double cx = std::stod(line.substr(0, s1));
		double cy = std::stod(line.substr(s1, s2));
		double r = std::stod(line.substr(s2, s3));
		distances.emplace_back(Circle{cx, cy, r, line});
	}

	std::sort(distances.begin(), distances.end(), [&](const Circle& a, const Circle& b) {
		double adx = std::abs(a.cx - w / 2.);
		double ady = std::abs(a.cy - h / 2.);
		double bdx = std::abs(b.cx - w / 2.);
		double bdy = std::abs(b.cy - h / 2.);
		return adx * adx + ady * ady - a.r * a.r < bdx * bdx + bdy * bdy - b.r * b.r;
	});

	lines.clear();
	for (auto& pair : distances) {
		lines.push_back(pair.line);
	}
}

int main(int argc, char** argv) {
	if (argc != 1 && argc != 4 && argc != 5) {
		std::cout << "Usage: ./Scramble.exe [FILE W H] [option]" << std::endl;
		return 1;
	}

	std::string path;
	double w, h;
	if (argc == 1) {
		std::cout << "File:   ";
		std::getline(std::cin, path);

		std::string line;
		std::cout << "Width:  ";
		std::getline(std::cin, line);
		w = (double)std::stoi(line);
		std::cout << "Height: ";
		std::getline(std::cin, line);
		h = (double)std::stoi(line);
	} else {
		path = argv[1];
		w = (double)std::atoi(argv[2]);
		h = (double)std::atoi(argv[3]);
	}

	std::ifstream ifile;
	ifile.open(path, std::ios::in);
	if (!ifile.is_open()) {
		std::cout << "Failed to open file for reading!" << std::endl;
		return 2;
	}

	std::vector<std::string> lines = std::vector<std::string>();
	std::string line;
	while (std::getline(ifile, line)) {
		lines.push_back(line);
	}
	ifile.close();

	std::string option;
	if (argc == 5) {
		option = argv[4];
		if (option != "1" && option != "2") {
			std::cout << "Please enter a valid option (1,2)" << std::endl;
			argc = 4;
		}
	}
	if (argc < 5) {
		std::cout << "[1] Random\n[2] Distance from Center" << std::endl;
		std::string s;
		std::getline(std::cin, s);
		while (!(s == "1" || s == "2")) {
			std::cout << "Please enter a valid option (1,2)" << std::endl;
			std::getline(std::cin, s);
		}
		option = s;
	}

	if (option == "1") {
		std::cout << "Random" << std::endl;
		scrambleRandom(lines);
	} else if (option == "2") {
		std::cout << "Distance from Center" << std::endl;
		sortCenterDistance(lines, w, h);
	}

	std::ofstream ofile;
	ofile.open(path, std::ios::out | std::ios::trunc);
	if (!ofile.is_open()) {
		std::cout << "Failed to open file for writing!" << std::endl;
		return 3;
	}
	for (auto& l : lines) {
		ofile << l << "\n";
	}

	ofile.close();

	std::cout << "Finished" << std::endl;

	return 0;
}