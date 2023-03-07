#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iomanip>

struct Circle {
	double cx, cy, r;
	int type;

	Circle(double cx, double cy, double r, int32_t type)
		: cx(cx), cy(cy), r(r), type(type) { }
};

double w, h;

/*
* Sorts circles based on frame. Black pixel -> show circle
*/
std::vector<Circle>::iterator sortForFrame(const std::string& frame, std::vector<Circle>::iterator start, std::vector<Circle>::iterator end) {
	int iw, ih, channels;
	unsigned char* image = stbi_load(frame.c_str(), &iw, &ih, &channels, 0);
	if (image == NULL) {
		std::cout << "Failed to read image from '" << frame << "'" << std::endl;
		return end;
	}

	auto partition = std::stable_partition(start, end, [&](const Circle& c) {
		int x = (int)(c.cx * iw / w);
		int y = (int)(ih - c.cy * ih / h);
		unsigned char* offset = image + (x + y * iw) * channels;
		unsigned char r = offset[0];
		unsigned char g = offset[1];
		unsigned char b = offset[2];
		return r != 0xFF || g != 0xFF || b != 0xFF;
	});

	stbi_image_free(image);
	return partition;
}

int main(int argc, char** argv) {
	if (argc >= 2 && argc <= 4) {
		std::cout << "Usage: ./FrameAnimation [OLD_OUTPUT W H NEW_OUTPUT FRAMES...]" << std::endl;
		return 1;
	}

	std::string old_output, new_output;
	std::vector<std::string> frames = std::vector<std::string>();
	if (argc > 4) {
		old_output = std::string(argv[1]);
		w = std::atof(argv[2]);
		h = std::atof(argv[3]);
		new_output = std::string(argv[4]);
		for (int i = 5; i < argc; i++) {
			frames.emplace_back(argv[i]);
		}
	} else {
		std::string line;
		std::cout << "Old Output: ";
		std::getline(std::cin, old_output);

		std::cout << "Width:      ";
		std::getline(std::cin, line);
		w = std::stod(line);
		std::cout << "Height:     ";
		std::getline(std::cin, line);
		h = std::stod(line);

		std::cout << "New Output: ";
		std::getline(std::cin, new_output);
		std::cout << "Frames:     ";
		std::getline(std::cin, line);
		size_t space = line.find(' ');
		size_t lastSpace = 0;
		while (space != std::string::npos) {
			frames.emplace_back(line.substr(lastSpace, space - lastSpace));
			lastSpace = space;
			space = line.find(' ', space + 1);
		}
		frames.emplace_back(line.substr(lastSpace));
	}

	std::ifstream ooutFile;
	ooutFile.open(old_output, std::ios::in);
	if (!ooutFile.is_open()) {
		std::cout << "Failed to open old output-file!" << std::endl;
		return 2;
	}

	std::vector<Circle> circles = std::vector<Circle>();

	double cx, cy, r;
	int type;
	while (ooutFile >> cx) {
		ooutFile >> cy;
		ooutFile >> r;
		ooutFile >> type;

		circles.emplace_back(Circle{ cx, cy, r, type });
	}

	ooutFile.close();

	// animation from center 
	std::sort(circles.begin(), circles.end(), [](const Circle& a, const Circle& b) {
		double adx = w / 2 - a.cx;
		double ady = h / 2 - a.cy;
		double bdx = w / 2 - b.cx;
		double bdy = h / 2 - b.cy;
		return adx * adx + ady * ady < bdx * bdx + bdy * bdy;
	});

	auto it = circles.begin();
	for (auto& frame : frames) {
		it = sortForFrame(frame, it, circles.end());
		std::cout << std::distance(circles.begin(), it) << std::endl;
	}

	// animate rest of circles
	std::sort(it, circles.end(), [](const Circle& a, const Circle& b) {
		double adx = w / 2 - a.cx;
		double ady = h / 2 - a.cy;
		double bdx = w / 2 - b.cx;
		double bdy = h / 2 - b.cy;
		return adx * adx + ady * ady >= bdx * bdx + bdy * bdy;
	});

	std::ofstream noutFile;
	noutFile.open(new_output, std::ios::out);
	if (!noutFile.is_open()) {
		std::cout << "Failed to open new output-file!" << std::endl;
		return 3;
	}
	
	for (auto& c : circles) {
		noutFile << std::setprecision(std::numeric_limits<long double>::digits10) << c.cx << " "
			<< std::setprecision(std::numeric_limits<long double>::digits10) << c.cy << " " << c.r << " " << c.type << std::endl;
	}

	noutFile.close();
	
	return 0;
}