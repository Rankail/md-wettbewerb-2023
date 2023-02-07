#include "Renderer.h"

SDL_Renderer* Renderer::renderer = NULL;
SDL_Color Renderer::clearColor = SDL_Color{0, 0, 0, 0xff};

void Renderer::init(SDL_Window* window) {
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	clearColor = SDL_Color{0, 0, 0, 0xFF};
}

void Renderer::shutdown() {
	SDL_DestroyRenderer(renderer);
}

void Renderer::setClearColor(SDL_Color c) {
	clearColor = c;
}

void Renderer::setClearColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	clearColor = SDL_Color{r, g, b, a};
}

void Renderer::clear() {
	SDL_SetRenderDrawColor(renderer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	SDL_RenderClear(renderer);
}

void Renderer::present() {
	SDL_RenderPresent(renderer);
}

void Renderer::setColor(SDL_Color c) {
	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
}

void Renderer::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void Renderer::drawRect(SDL_Rect rect) {
	SDL_RenderDrawRect(renderer, &rect);
}

void Renderer::drawRect(int32_t x, int32_t y, int32_t w, int32_t h) {
	SDL_RenderDrawRect(renderer, new SDL_Rect{x, y, w, h});
}

void Renderer::fillRect(SDL_Rect rect) {
	SDL_RenderFillRect(renderer, &rect);
}

void Renderer::fillRect(int32_t x, int32_t y, int32_t w, int32_t h) {
	SDL_RenderFillRect(renderer, new SDL_Rect{x, y, w, h});
}

void Renderer::drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
	SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

void Renderer::drawPoint(int32_t x, int32_t y) {
	SDL_RenderDrawPoint(renderer, x, y);
}

void Renderer::drawCircle(int32_t cx, int32_t cy, int32_t r) {
	const int32_t d = 2 * r;

	int32_t x = r - 1;
	int32_t y = 0;
	int32_t tx = 1;
	int32_t ty = 1;
	int32_t error = tx - d;

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
			error += tx - d;
		}
	}
}
