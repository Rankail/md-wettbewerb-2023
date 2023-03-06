//#include <SDL2/SDL.h>
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

unsigned char* image_data;
int iw, ih, channels;
double w, h;

std::vector<Circle> drawImage(std::vector<Circle> circles, double left, double right, double scale) {
	int32_t circleColors[8] = { 0x000000, 0x9400D3, 0x009E73, 0x56B4E9, 0xE69F00, 0xF0E442, 0x0072B2, 0xE51E10 };
	int32_t colorCounts[8] = { 0 };
	int32_t colorIndices[8] = { 0 };

	double offX = w * (1. - scale) / 2.f;
	double offY = w * (1. - scale) / 2.f;
	std::vector<Circle*> fillers = std::vector<Circle*>();
	for (auto& c : circles) {
		if (c.cx < offX || c.cy < offY || c.cx > offX + scale * w || c.cy > offY + scale * h) {
			//outside off image
			fillers.push_back(&c);
			continue;
		}
		int x = (c.cx - offX) / scale * iw / w;
		int y = ih-(c.cy - offY) / scale * ih / h;
		unsigned char* offset = image_data + (x + y * iw) * channels;
		unsigned char r = offset[0];
		unsigned char g = offset[1];
		unsigned char b = offset[2];
		int idx = 0;
		int32_t color = (r << 16) + (g << 8) + b;
		HSV c1 = rgbToHsv(hexToRGB(color));
		double minDiff = 255 * 3 + 1;
		if (r > 250 && g > 250 && b > 250) {
			if (x < iw / 2) {
				idx = 1;
			} else {
				idx = 2;
			}
		} else {
			for (int i = 0; i < 8; i++) {
				HSV c2 = rgbToHsv(hexToRGB(circleColors[i]));
				int32_t diffH = std::abs(c1.h - c2.h);
				int32_t diffS = std::abs(c1.s - c2.s);
				int32_t diffV = std::abs(c1.v - c2.v);
				int32_t diff = diffH + diffS + diffV;
				if (diff < minDiff) {
					minDiff = diff;
					idx = i;
				}
			}
		}
		colorCounts[idx]++;
		if (colorCounts[idx] > 114) {
			colorIndices[idx]++;
			colorCounts[idx] = 0;
		}
		if (colorIndices[idx]*8+idx > 100) {
			//too many 
			return drawImage(circles, left, scale, left + (scale - left) / 2.);
		}
		c.type = colorIndices[idx]*8 + idx;
	}

	if (right - left > 0.1) {
		return drawImage(circles, scale, right, scale + (scale - left) / 2.);
	}

	int fi = 0;
	for (int i = 0; i < 8; i++) {
		while (colorIndices[i] * 8 + i < 100 && fi < fillers.size()) {
			fillers[fi]->type = colorIndices[i] * 8 + i;
			colorCounts[i]++;
			if (colorCounts[i] >= 114) {
				colorCounts[i] = 0;
				colorIndices[i]++;
			}
			fi++;
		}
	}
	return circles;
}

int main(int argc, char** argv) {
	if (argc != 6 && argc != 1) {
		std::cout << "Usage: ./Images.exe [IMAGE OLD_OUTPUT W H NEW_OUTPUT]" << std::endl;
		return 1;
	}

	std::string image, old_output, new_output;
	if (argc == 6) {
		image = std::string(argv[1]);
		old_output = std::string(argv[2]);
		w = std::atof(argv[3]);
		h = std::atof(argv[4]);
		new_output = std::string(argv[5]);
	} else {
		std::string line;

		std::cout << "Image:      ";
		std::getline(std::cin, image);

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

	image_data = stbi_load(image.c_str(), &iw, &ih, &channels, 0);
	if (image_data == NULL) {
		std::cout << "Failed to read image" << std::endl;
		return 4;
	}

	circles = drawImage(circles, 0., 1., .5);

	stbi_image_free(image_data);

	std::ofstream noutFile;
	noutFile.open(new_output, std::ios::out);
	if (!noutFile.is_open()) {
		std::cout << "Failed to open new output-file!" << std::endl;
		return 3;
	}
	
	for (auto& c : circles) {
		noutFile << c.cx << " " << c.cy << " " << c.r << " " << c.type << std::endl;
	}

	int counts[100] = { 0 };
	for (auto& c : circles) {
		counts[c.type]++;
	}
	for (int i = 0; i < 100; i++) {
		std::cout << i << "\t" << counts[i] << std::endl;
	}

	noutFile.close();
	
	return 0;
}