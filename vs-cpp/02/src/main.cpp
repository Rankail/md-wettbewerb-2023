#include "utils.h"

#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <numbers>
#include <random>

int main(int argc, char** argv) {

	std::ifstream file;
	file.open("../inputs/forest01.txt");
	if (!file.is_open()) {
		printf("Failed to open file!");
		return 1;
	}

	std::string name;
	std::getline(file, name);

	std::string line;
	std::getline(file, line);
	int space = line.find(' ');
	double w = (double)std::stoi(line.substr(0, space));
	double h = (double)std::stoi(line.substr(space));
	printf("%d %d\n", w, h);

	int idx = 0;
	std::vector<CircleType> types = std::vector<CircleType>();
	while (std::getline(file, line)) {
		space = line.find(' ');
		int r = std::stoi(line.substr(0, space));
		types.push_back(CircleType{idx, (double)r, line.substr(space)});
		idx++;
	}

	std::sort(types.begin(), types.end(), [](const CircleType& lhs, const CircleType& rhs) {
		return lhs.r > rhs.r;
	});

	double diversityMax = 1. - 1. / types.size();
	double block = 0;
	for (auto& type : types) {
		block += std::numbers::pi * type.r * type.r;
		printf("%f %d %s\n", type.r, type.index, type.name);
	}

	double size = w * h;
	printf("%f %f %f %f\n", diversityMax, block, size, size / block);

	std::vector<Circle> circles = std::vector<Circle>();
	for (int i = 0; i < size / block * .7; i++) {
		for (auto& type : types) {
			circles.push_back(Circle{rand()/double(RAND_MAX) * (w - 2 * type.r) + type.r, rand() / double(RAND_MAX) * (h - 2 * type.r) + type.r, type.r});
		}
	}


	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("Circles", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (int)w, (int)h, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	

	std::unordered_set<Circle> collided = std::unordered_set<Circle>();
	bool quit = false;
	while (!quit) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) quit = true;
			else if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) quit = true;
		}

		bool coll = false;
		for (auto& c1 : circles) {
			for (auto& c2 : circles) {
				if (c1 == c2 || collided.find(c1) != collided.end() || collided.find(c2) != collided.end()) continue;
				double dx = c2.cx - c1.cx;
				double dy = c2.cy - c1.cy;
				double dr = c1.r + c2.r;
				double d = std::sqrt(dx * dx + dy * dy);
				double diff = dr-d;
				if (diff > 0) {
					coll = true;
					collided.insert(c1);
					collided.insert(c2);
					c1.cx -= dx * diff / d / 1;
					c1.cy -= dy * diff / d / 1;
					c2.cx += dx * diff / d / 1;
					c2.cy += dy * diff / d / 1;

					if (c1.cx - c1.r < 0) {
						c1.cx = c1.r;
					}
					if (c1.cy - c1.r < 0) {
						c1.cy = c1.r;
					}
					if (c1.cx + c1.r > w) {
						c1.cx = w - c1.r;
					}
					if (c1.cy + c1.r > h) {
						c1.cy = h - c1.r;
					}

					if (c2.cx - c2.r < 0) {
						c2.cx = c2.r;
					}
					if (c2.cy - c2.r < 0) {
						c2.cy = c2.r;
					}
					if (c2.cx + c2.r > w) {
						c2.cx = w - c2.r;
					}
					if (c2.cy + c2.r > h) {
						c2.cy = h - c2.r;
					}
				}
			}
		}
		if (!coll) {
			printf("Finished\n");
		}
		collided.clear();

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 0, 0xff, 0xff, 0xff);
		for (auto& c : circles) {
			drawCircle(renderer, c);
		}

		SDL_RenderPresent(renderer);

	}


	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}