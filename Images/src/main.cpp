//#include <SDL2/SDL.h>
#include <string>
#include <fstream>
#include <vector>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct CircleType {
	int index;
	double radius;
};

struct Circle {
	double cx, cy, r;
	int type;
};

//static void drawCircle(SDL_Renderer* renderer, Circle& c) {
//	int color = ((c.type >> 16) ^ c.type) * 0x45d9f3b;
//	color = ((color >> 16) ^ color) * 0x45d9f3b;
//	color = (color >> 16) ^ color;
//	uint8_t r = 0x80 + ((color >> 16) ^ 0xff) / 2;
//	uint8_t g = 0x80 + ((color >> 8) ^ 0xff) / 2;
//	uint8_t b = 0x80 + (color ^ 0xff) / 2;
//	SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
//
//	int32_t cx = (int32_t)c.cx;
//	int32_t cy = (int32_t)c.cy;
//	const int32_t diameter = (int32_t)(c.r * 2.);
//
//	int32_t x = (int32_t)c.r - 1;
//	int32_t y = 0;
//	int32_t tx = 1;
//	int32_t ty = 1;
//	int32_t error = tx - diameter;
//
//	while (x >= y)
//	{
//		SDL_RenderDrawPoint(renderer, cx + x, cy - y);
//		SDL_RenderDrawPoint(renderer, cx + x, cy + y);
//		SDL_RenderDrawPoint(renderer, cx - x, cy - y);
//		SDL_RenderDrawPoint(renderer, cx - x, cy + y);
//		SDL_RenderDrawPoint(renderer, cx + y, cy - x);
//		SDL_RenderDrawPoint(renderer, cx + y, cy + x);
//		SDL_RenderDrawPoint(renderer, cx - y, cy - x);
//		SDL_RenderDrawPoint(renderer, cx - y, cy + x);
//
//		if (error <= 0)
//		{
//			++y;
//			error += ty;
//			ty += 2;
//		}
//
//		if (error > 0)
//		{
//			--x;
//			tx += 2;
//			error += (tx - diameter);
//		}
//	}
//}

int main(int argc, char** argv) {
	if (argc != 4 && argc != 1) {
		std::cout << "Usage: ./Images.exe [IMAGE INPUT OUTPUT]" << std::endl;
		return 1;
	}

	std::string image, input, output;
	if (argc == 4) {
		image = std::string(argv[1]);
		input = std::string(argv[2]);
		output = std::string(argv[3]);
	} else {
		std::cout << "Image:  ";
		std::getline(std::cin, image);
		std::cout << "Input:  ";
		std::getline(std::cin, input);
		std::cout << "Output: ";
		std::getline(std::cin, output);
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

	double maxRadius = std::max(types[4].radius, std::max(types[5].radius, types[7].radius));

	inFile.close();

	int iw, ih, channels;
	unsigned char* data = stbi_load(image.c_str(), &iw, &ih, &channels, 1);
	if (data == NULL) {
		std::cout << "Failed to read image" << std::endl;
		return 4;
	}

	std::vector<std::string> colors = std::vector<std::string>();
	for (int i = 0; i < iw * ih; i++) {
		unsigned char* offset = data + i * channels;
		unsigned char r = offset[0];
		unsigned char g = offset[1];
		unsigned char b = offset[2];
		std::string color = std::to_string((int)r) + " " + std::to_string((int)g) + " " + std::to_string((int)b);
		if (std::find(colors.begin(), colors.end(), color) == colors.end()) {
			colors.push_back(color);
		}
	}

	for (auto& c : colors) {
		std::cout << c << std::endl;
	}

	stbi_image_free(data);

	std::ofstream outFile;
	outFile.open(output, std::ios::out);
	if (!outFile.is_open()) {
		std::cout << "Failed to open output-file!" << std::endl;
		return 3;
	}

	double x = 0.;
	for (auto& t : types) {
		outFile << x + t.radius << " " << t.radius << " " << t.radius << " " << t.index << std::endl;
		x += t.radius * 2.;
	}

	outFile.close();


	/*std::vector<Circle> circles = std::vector<Circle>();
	double cx, cy, r;
	int type;
	int maxType = 0;
	while (file >> cx) {
		file >> cy;
		file >> r;
		file >> type;

		circles.emplace_back(Circle{ cx, cy, r, type });
	}*/

	/*if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "Failed to initialize SDL! Error: " << SDL_GetError() << std::endl;
		return 3;
	}

	SDL_Window* window = SDL_CreateWindow("Circles", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (int)w, (int)h, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		std::cout << "Failed to create SDL_Window! Error: " << SDL_GetError() << std::endl;
		return 4;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		std::cout << "Failed to create SDL_Renderer! Error: " << SDL_GetError() << std::endl;
		return 5;
	}

	bool slowly = false;

	int delay = (circles.size() < 4000) ? 1 : 0;

	bool quit = false;
	while (!quit) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) quit = true;
			else if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) quit = true;
				if (e.key.keysym.scancode == SDL_SCANCODE_SPACE) slowly = true;
			} else if (e.type == SDL_MOUSEBUTTONDOWN) {
				int x, y;
				SDL_GetMouseState(&x, &y);
				for (auto& c : circles) {
					double dx = c.cx - x;
					double dy = c.cy - y;
					if (dx * dx + dy * dy < c.r * c.r) {
						std::cout << c.cx << c.cy << c.r << c.type << std::endl;
					}
				}
			}
		}


		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
		SDL_RenderClear(renderer);

		for (unsigned int i = 0; i < circles.size(); i++) {
			drawCircle(renderer, circles[i]);
			if (slowly) {
				SDL_RenderPresent(renderer);
				SDL_Delay(delay);
			}
		}
		if (slowly) {
			slowly = false;
			std::cout << "finished re-rendering" << std::endl;
		}

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();*/
	
	return 0;
}