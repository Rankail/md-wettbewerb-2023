#include <SDL.h>
#include <string>
#include <fstream>
#include <vector>

struct Circle {
	double cx, cy, r;
	int type;
};



static void drawCircle(SDL_Renderer* renderer, Circle& c) {
	int color = ((c.type >> 16) ^ c.type) * 0x45d9f3b;
	color = ((color >> 16) ^ color) * 0x45d9f3b;
	color = (color >> 16) ^ color;
	SDL_SetRenderDrawColor(renderer, (color >> 16) ^ 0xff, (color >> 8) ^ 0xff, color ^ 0xff, 0xff);

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
	/*if (argc != 2) {
		printf("Usage: display.exe [file]\n");
		return 1;
	}*/

	//std::string path = std::string(argv[1]);
	std::string path = "../results/forest01.txt";
	std::ifstream file;
	file.open(path);
	if (!file.is_open()) {
		printf("Failed to open file!\n");
		return 2;
	}

	std::vector<Circle> circles = std::vector<Circle>();
	double cx, cy, r;
	int type;
	while (file >> cx) {
		file >> cy;
		file >> r;
		file >> type;

		circles.emplace_back(Circle{ cx, cy, r, type });
	}

	double maxX = 0., maxY = 0.;
	for (auto& c : circles) {
		maxX = std::max(maxX, c.cx + c.r);
		maxY = std::max(maxY, c.cy + c.r);
	}

	double w = std::ceil(maxX / 100) * 100;
	double h = std::ceil(maxY / 100) * 100;


	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Failed to initialize SDL! Error: %s\n", SDL_GetError());
		return 3;
	}

	SDL_Window* window = SDL_CreateWindow("Circles", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (int)w, (int)h, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Failed to create SDL_Window! Error: %s\n", SDL_GetError());
		return 4;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		printf("Failed to create SDL_Renderer! Error: %s\n", SDL_GetError());
		return 5;
	}

	bool quit = false;
	while (!quit) {
		SDL_Event e;
		SDL_WaitEvent(&e);
		if (e.type == SDL_QUIT) quit = true;
		if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) quit = true;

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
		SDL_RenderClear(renderer);

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