#pragma once

#include <SDL.h>
#include <vector>

struct Point {
	int32_t x, y;
};

struct Circle {
	int32_t cx, cy;
	int32_t r;

	Circle(int32_t cx, int32_t cy, int32_t r) : cx(cx), cy(cy), r(r) {}
};

class Layer {
public:
	Layer();
	virtual ~Layer();

	void events(SDL_Event e);
	void update();
	void render();

private:
	std::vector<Circle> circles;
};

