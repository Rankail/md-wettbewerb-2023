#pragma once

#include <SDL.h>
#include <vector>

struct Point {
	double x, y;
};

struct Circle {
	double cx, cy;
	double r;

	Circle(double cx, double cy, double r) : cx(cx), cy(cy), r(r) {}
};

class Layer {
public:
	Layer();
	virtual ~Layer();

	void events(SDL_Event e);
	void update(float dt);
	void render();

private:
	std::vector<Circle> circles;
	Circle* ci1;
	Circle* ci2;
	double r1;
	float cooldown;

	bool paused;
	 
};

