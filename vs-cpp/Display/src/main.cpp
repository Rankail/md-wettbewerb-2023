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
	if (argc != 2) {
		printf("Usage: Display.exe [file]\n");
		return 1;
	}

	std::string path = std::string(argv[1]);
	std::ifstream file;
	file.open(path);
	if (!file.is_open()) {
		printf("Failed to open file!\n");
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
		circles.emplace_back(Circle{ cx, cy, r, type });
	}

	std::vector<long> counts = std::vector<long>();
	for (int i = 0; i <= maxType; i++) {
		counts.push_back(0);
	}

	double size = 0;
	double maxX = 0., maxY = 0.;
	for (auto& c : circles) {
		maxX = std::max(maxX, c.cx + c.r);
		maxY = std::max(maxY, c.cy + c.r);
		counts[c.type]++;
		size += c.r * c.r * M_PI;
	}

	for (auto& c : counts) {
		printf("%d\n", c);
	}

	double totalCountSquared = 0.;
	double sumCountsSquared = 0.;
	for (int i = 0; i < counts.size(); i++) {
		totalCountSquared += counts[i];
		sumCountsSquared += (double)counts[i] * (double)counts[i];
	}

	double w = std::ceil(maxX / 100) * 100;
	double h = std::ceil(maxY / 100) * 100;

	double A = size / (w * h);
	double D = 1. - (double)sumCountsSquared / totalCountSquared / totalCountSquared;

	printf("A: %f\n", A);
	printf("D: %f\n", D);
	printf("B: %f\n", A * D);

	while (w > 1000. || h > 1000.) {
		w /= 2.; h /= 2.;
		for (auto& c : circles) {
			c.r /= 2.;
			c.cx /= 2.;
			c.cy /= 2.;
		}
	}

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
		if (e.type == SDL_MOUSEBUTTONDOWN) {
			int x, y;
			SDL_GetMouseState(&x, &y);
			for (auto& c : circles) {
				double dx = c.cx - x;
				double dy = c.cy - y;
				if (dx * dx + dy * dy < c.r * c.r) {
					printf("%f %f %f %d\n", c.cx, c.cy, c.r, c.type);
				}
			}
		}

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