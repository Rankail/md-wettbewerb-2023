#pragma once

#include <SDL.h>

class Renderer {
public:
	static void init(SDL_Window* window);
	static void shutdown();

	static void setClearColor(SDL_Color c);
	static void setClearColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF);

	static void clear();
	static void present();

	static void setColor(SDL_Color c);
	static void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF);

	static void drawRect(SDL_Rect rect);
	static void drawRect(int32_t x, int32_t y, int32_t w, int32_t h);

	static void fillRect(SDL_Rect rect);
	static void fillRect(int32_t x, int32_t y, int32_t w, int32_t h);

	static void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

	static void drawPoint(int32_t x, int32_t y);

	static void drawCircle(int32_t cx, int32_t cy, int32_t r);


private:
	static SDL_Renderer* renderer;
	static SDL_Color clearColor;
};

