#pragma once

#ifdef DRAW_SDL
#include <SDL2/SDL.h>

class Renderer {
public:
	static bool Init(double w, double h);
	static void Shutdown();

	static void clear(double r=0xFF, double g=0xFF, double b=0xFF, double a=0xFF);
	static void drawCircle(double cx, double cy, double r, int type);
	static void present();

private:
	static SDL_Window* window;
	static SDL_Renderer* renderer;
	static double scale;
};
#endif