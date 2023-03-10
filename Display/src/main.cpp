#include <SDL2/SDL.h>
#include <string>
#include <fstream>
#include <vector>
#include <iostream>

struct Circle {
	double cx, cy, r;
	int type;
};

static void drawCircle(SDL_Renderer* renderer, Circle& c) {
	int32_t circleColors[8] = {0x000000, 0x9400D3, 0x009E73, 0x56B4E9, 0xE69F00, 0xF0E442, 0x0072B2, 0xE51E10}; //colros from website
	int color = circleColors[c.type % 8];
	uint8_t r = (color >> 16) & 0xff;
	uint8_t g = (color >> 8) & 0xff;
	uint8_t b = color & 0xff;
	SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);

	int32_t cx = (int32_t)c.cx;
	int32_t cy = (int32_t)c.cy;
	const int32_t diameter = (int32_t)(c.r * 2.);

	int32_t x = (int32_t)c.r - 1;
	int32_t y = 0;
	int32_t tx = 1;
	int32_t ty = 1;
	int32_t error = tx - diameter;

	while (x >= y)
	{
		SDL_RenderDrawPoint(renderer, cx + x, cy - y);
		SDL_RenderDrawPoint(renderer, cx + x, cy + y);
		SDL_RenderDrawPoint(renderer, cx - x, cy - y);
		SDL_RenderDrawPoint(renderer, cx - x, cy + y);
		SDL_RenderDrawPoint(renderer, cx + y, cy - x);
		SDL_RenderDrawPoint(renderer, cx + y, cy + x);
		SDL_RenderDrawPoint(renderer, cx - y, cy - x);
		SDL_RenderDrawPoint(renderer, cx - y, cy + x);

		if (error <= 0)
		{
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0)
		{
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}
}

int main(int argc, char** argv) {
	if (argc != 1 && argc != 4) {
		std::cout << "Usage: ./Display.exe [FILE W H]" << std::endl;
		return 1;
	}
	std::string path;
	double w, h;
	if (argc == 4) {
		path = std::string(argv[1]);
		w = std::atoi(argv[2]);
		h = std::atoi(argv[3]);
	} else {
		std::cout << "File:   ";
		std::getline(std::cin, path);

		std::string line;
		std::cout << "Width:  ";
		std::getline(std::cin, line);
		w = std::stoi(line);
		std::cout << "Height: ";
		std::getline(std::cin, line);
		h = std::stoi(line);
	}

	std::ifstream file;
	file.open(path, std::ios::in);
	if (!file.is_open()) {
		std::cout << "Failed to open file!" << std::endl;
		return 2;
	}

	std::vector<Circle> circles = std::vector<Circle>();
	double cx, cy, r;
	int type;
	int maxType = 0;
	while (file >> cx) {
		file >> cy;
		file >> r;
		file >> type;

		maxType = std::max(type, maxType);
		circles.emplace_back(Circle{ cx, h - cy, r, type });
	}

	std::vector<long> counts = std::vector<long>();
	for (int i = 0; i <= maxType; i++) {
		counts.push_back(0);
	}

	double size = 0;
	for (auto& c : circles) {
		counts[c.type]++;
		size += c.r * c.r * M_PI;
	}

	double totalCountSquared = 0.;
	double sumCountsSquared = 0.;
	for (int i = 0; i < counts.size(); i++) {
		totalCountSquared += counts[i];
		sumCountsSquared += (double)counts[i] * (double)counts[i];
	}

	double A = size / (w * h);
	double D = 1. - (double)sumCountsSquared / totalCountSquared / totalCountSquared;

	std::cout << "A: " << A << std::endl;
	std::cout << "D: " << D << std::endl;
	std::cout << "B: " << A * D << std::endl;

	// scale down
	while (w > 900. || h > 900.) {
		w /= 2.; h /= 2.;
		for (auto& c : circles) {
			c.r = std::max(1., c.r/2.);
			c.cx /= 2.;
			c.cy /= 2.;
		}
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
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
						std::cout << c.cx << " " << (h - c.cy) << " " << c.r << " " << c.type << std::endl;
					}
				}
			}
		}


		SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
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
	SDL_Quit();
	
	return 0;
}