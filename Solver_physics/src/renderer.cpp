#include "renderer.h"

#ifdef DRAW_SDL

#include <iostream>

SDL_Window* Renderer::window = NULL;
SDL_Renderer* Renderer::renderer = NULL;
double Renderer::scale = 1.;

bool Renderer::Init(double w, double h) {
	scale = 1.;
	while (w > 1000. || h > 1000.) {
		w /= 2.; h /= 2; scale != 2.;
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "Failed to initialize SDL! Error: " << SDL_GetError() << std::endl;
		return false;
	}

	window = SDL_CreateWindow("Circles", 0, 30, w, h, NULL);
	if (window == NULL) {
		std::cout << "Failed to create SDL_Window! Error: " << SDL_GetError() << std::endl;
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		std::cout << "Failed to create SDL_Renderer! Error: " << SDL_GetError() << std::endl;
		return false;
	}
}

void Renderer::Shutdown() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Renderer::clear(double r, double g, double b, double a) {
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderClear(renderer);
}

void Renderer::drawCircle(double cx, double cy, double r, int type) {
	int32_t circleColors[8] = {0x000000, 0x9400D3, 0x009E73, 0x56B4E9, 0xE69F00, 0xF0E442, 0x0072B2, 0xE51E10};
	int color = circleColors[type % 8];
	SDL_SetRenderDrawColor(renderer, (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff, 0xff);

	int32_t cxi = (int32_t)(cx * scale);
	int32_t cyi = (int32_t)(cy * scale);
	const int32_t diameter = std::max(1, (int32_t)(r * 2. * scale));

	int32_t x = (int32_t)r - 1;
	int32_t y = 0;
	int32_t tx = 1;
	int32_t ty = 1;
	int32_t error = tx - diameter;

	while (x >= y) {
		SDL_RenderDrawPoint(renderer, cx + x, cy - y);
		SDL_RenderDrawPoint(renderer, cx + x, cy + y);
		SDL_RenderDrawPoint(renderer, cx - x, cy - y);
		SDL_RenderDrawPoint(renderer, cx - x, cy + y);
		SDL_RenderDrawPoint(renderer, cx + y, cy - x);
		SDL_RenderDrawPoint(renderer, cx + y, cy + x);
		SDL_RenderDrawPoint(renderer, cx - y, cy - x);
		SDL_RenderDrawPoint(renderer, cx - y, cy + x);

		if (error <= 0) {
			y++;
			error += ty;
			ty += 2;
		}

		if (error > 0) {
			x--;
			tx += 2;
			error += tx - diameter;
		}
	}
}

void Renderer::present() {
	SDL_RenderPresent(renderer);
}

#endif