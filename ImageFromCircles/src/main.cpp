#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct RGB {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

struct HSV {
	unsigned char h;
	unsigned char s;
	unsigned char v;
};

RGB hexToRGB(int32_t hex) {
	RGB out;
	out.r = (hex >> 16) & 0xff;
	out.g = (hex >> 8) & 0xff;
	out.b = hex & 0xff;
	return out;
}

RGB hsvToRgb(HSV hsv) {
	RGB rgb;
	unsigned char region, remainder, p, q, t;

	if (hsv.s == 0) {
		rgb.r = hsv.v;
		rgb.g = hsv.v;
		rgb.b = hsv.v;
		return rgb;
	}

	region = hsv.h / 43;
	remainder = (hsv.h - (region * 43)) * 6;

	p = (hsv.v * (255 - hsv.s)) >> 8;
	q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
	t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

	switch (region) {
		case 0:
			rgb.r = hsv.v; rgb.g = t; rgb.b = p;
			break;
		case 1:
			rgb.r = q; rgb.g = hsv.v; rgb.b = p;
			break;
		case 2:
			rgb.r = p; rgb.g = hsv.v; rgb.b = t;
			break;
		case 3:
			rgb.r = p; rgb.g = q; rgb.b = hsv.v;
			break;
		case 4:
			rgb.r = t; rgb.g = p; rgb.b = hsv.v;
			break;
		default:
			rgb.r = hsv.v; rgb.g = p; rgb.b = q;
			break;
	}

	return rgb;
}

HSV rgbToHsv(RGB rgb) {
	HSV hsv;
	unsigned char rgbMin, rgbMax;

	rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
	rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

	hsv.v = rgbMax;
	if (hsv.v == 0) {
		hsv.h = 0;
		hsv.s = 0;
		return hsv;
	}

	hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
	if (hsv.s == 0) {
		hsv.h = 0;
		return hsv;
	}

	if (rgbMax == rgb.r)
		hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
	else if (rgbMax == rgb.g)
		hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
	else
		hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

	return hsv;
}

struct CircleType {
	int index;
	double radius;
};

struct Circle {
	double cx, cy, r;
	int type;

	Circle(double cx, double cy, double r, int32_t type)
		: cx(cx), cy(cy), r(r), type(type) { }
};

int main(int argc, char** argv) {
	if (argc != 6 && argc != 1) {
		std::cout << "Usage: ./ImagefromCircles [IMAGE INPUT OUTPUT GAP_SCALE SCALE]" << std::endl;
		return 1;
	}

	std::string image, input, output;
	double gapScale, scale;
	if (argc == 6) {
		image = std::string(argv[1]);
		input = std::string(argv[2]);
		output = std::string(argv[3]);
		gapScale = std::atof(argv[4]);
		scale = std::atof(argv[5]);
	} else {
		std::cout << "Image:     ";
		std::getline(std::cin, image);
		std::cout << "Input:     ";
		std::getline(std::cin, input);
		std::cout << "Output:    ";
		std::getline(std::cin, output);

		std::string line;
		std::cout << "Gap-Scale: ";
		std::getline(std::cin, line);
		gapScale = std::stod(line);
		std::cout << "Scale: ";
		std::getline(std::cin, line);
		scale = std::stod(line);
	}

	std::ifstream inFile;
	inFile.open(input, std::ios::in);
	if (!inFile.is_open()) {
		std::cout << "Failed to open input-file!" << std::endl;
		return 2;
	}

	std::vector<CircleType> types = std::vector<CircleType>();

	std::string line;
	std::getline(inFile, line);
	std::getline(inFile, line);
	size_t space = line.find(' ');
	double w = (double)std::stoi(line.substr(0, space));
	double h = (double)std::stoi(line.substr(space + 1));

	int ti = 0;
	while (std::getline(inFile, line)) {
		space = line.find(' ');
		types.emplace_back(ti, std::stod(line.substr(0, space)));
		ti++;
	}

	std::sort(types.begin(), types.end(), [](const CircleType& a, const CircleType& b) { return a.radius < b.radius; });
	//types.erase(types.begin(), types.begin() + 8); // increase circle size -> less circles for same fill

	inFile.close();

	int iw, ih, channels;
	unsigned char* data = stbi_load(image.c_str(), &iw, &ih, &channels, 0);
	if (data == NULL) {
		std::cout << "Failed to read image" << std::endl;
		return 4;
	}

	int32_t circleColors[8] = {0x000000, 0x9400D3, 0x009E73, 0x56B4E9, 0xE69F00, 0xF0E442, 0x0072B2, 0xE51E10};

	// gets all colors present of image
	std::vector<int32_t> colors = std::vector<int32_t>();
	for (int j = 0; j < ih; j++) {
		for (int i = 0; i < iw; i++) {
			unsigned char* offset = data + (i + j * iw) * channels;
			unsigned char r = offset[0];
			unsigned char g = offset[1];
			unsigned char b = offset[2];
			int32_t color = (r << 16) + (g << 8) + b;
			if (color == 0xFFFFFF) continue;
			if (std::find(colors.begin(), colors.end(), color) == colors.end()) {
				colors.push_back(color);
			}
		}
	}

	// creates map for most similiar color based on distance in hsv
	std::unordered_map<int32_t, int32_t> colorMap = std::unordered_map<int32_t, int32_t>();
	for (auto& c : colors) {
		int minDiff = 255 * 3 + 1;
		int idx = 0;
		for (int i = 0; i < 8; i++) {
			HSV c2 = rgbToHsv(hexToRGB(c));
			HSV c1 = rgbToHsv(hexToRGB(circleColors[i]));
			int32_t diffH = std::abs(c1.h - c2.h);
			int32_t diffS = std::abs(c1.s - c2.s);
			int32_t diffV = std::abs(c1.v - c2.v);
			int32_t diff = diffH + diffS + diffV;
			if (diff < minDiff) {
				minDiff = diff;
				idx = i;
			}
		}
		colorMap[c] = idx;
	}
	int maxType = 0;
	for (auto const& [k, v] : colorMap) {
		maxType = std::max(maxType, v);
	}

	//places circles
	double maxDiameter = types[maxType].radius * 2.;
	std::vector<Circle> circles = std::vector<Circle>();
	for (int j = 0; j < ih * scale; j++) {
		for (int i = 0; i < iw * scale; i++) {
			unsigned char* offset = data + ((int)(i / scale) + (int)(j / scale) * iw) * channels;
			unsigned char r = offset[0];
			unsigned char g = offset[1];
			unsigned char b = offset[2];
			int32_t color = (r << 16) + (g << 8) + b;
			if (color == 0xFFFFFF) continue;
			circles.emplace_back((i + .5) * maxDiameter * gapScale, (ih * scale - j + .5) * maxDiameter * gapScale, types[colorMap[color]].radius, types[colorMap[color]].index);
		}
	}

	stbi_image_free(data);

	std::ofstream outFile;
	outFile.open(output, std::ios::out);
	if (!outFile.is_open()) {
		std::cout << "Failed to open output-file!" << std::endl;
		return 3;
	}

	if (iw * scale * maxDiameter * gapScale > w || ih * scale * maxDiameter * gapScale > h) {
		std::cout << "Image or scale is too big" << std::endl;
		return 5;
	}

	for (int i = 0; i < circles.size(); i++) {
		for (int j = i+1; j < circles.size(); j++) {
			auto& c1 = circles[i];
			auto& c2 = circles[j];
			if ((c1.cx - c2.cx) * (c1.cx - c2.cx) + (c1.cy - c2.cy) * (c1.cy - c2.cy) < (c1.r + c2.r) * (c1.r + c2.r)) {
				std::cout << "Circle-Collision: " << i << " " << j << std::endl;
				return 6;
			}
		}
	}

	double offX = (w - iw * scale * maxDiameter * gapScale) / 2.;
	double offY = (h - ih * scale * maxDiameter * gapScale) / 2.;

	for (auto& c : circles) {
		outFile << (offX + c.cx) << " " << (offY + c.cy) << " " << c.r << " " << c.type << std::endl;
	}

	outFile.close();
	
	return 0;
}