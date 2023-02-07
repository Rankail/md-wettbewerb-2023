#include "Layer.h"

#include "Renderer.h"

#include <cmath>

static Circle circleBottomWallToRight(const Circle& connected, int32_t r) {
	int cy = 1000 - r;
	// Pythagoras cx = sqrt((or+r)^2 - (or-r)^2) = sqrt(or^2+2or*r+r^2-or^2+2or*r-r^2) => sqrt(4*or*r) = 2 * sqrt(or*r)
	int cx = connected.cx + 2 * std::sqrt(connected.r * r);
	return Circle{cx, cy, r};
}

static Circle circleLeftWallToBottom(const Circle& connected, int32_t r) {
	int32_t cx = r;
	int32_t cy = connected.cy + 2 * std::sqrt(connected.r * r);
	return Circle{cx, cy, r};
}

static Point circleCircleIntersection(const Circle& c1, const Circle& c2) {
	float dx = (float)(c2.cx - c1.cx);
	float dy = (float)(c2.cy - c1.cy);
	float d = std::sqrt(dx * dx + dy * dy);
	float a = (float)(c1.r * c1.r - c2.r * c2.r + d * d) / (2.f * d);
	float h = std::sqrt(c1.r * c1.r - a * a);

	float x2 = c1.cx + a * dx / d;
	float y2 = c1.cy + a * dy / d;

	float x3 = c1.cx + (a * dx + h * dy) / d;
	float y3 = c1.cy + (a * dy + h * dx) / d;

	return Point{(int32_t)x3, (int32_t)y3};
}

static Circle circleTwo(const Circle& c1, const Circle& c2, int32_t r) {
	Point c = circleCircleIntersection(Circle{c1.cx, c1.cy, c1.r + r}, Circle{c2.cx, c2.cy, c2.r + r});
	return Circle{c.x, c.y, r};
}


Layer::Layer() {
	std::vector<int32_t> sizes = std::vector<int32_t>();
	sizes.push_back(32);
	sizes.push_back(8);
	sizes.push_back(4);
	sizes.push_back(2);
	circles = std::vector<Circle>();
	


}

Layer::~Layer() {
}

void Layer::events(SDL_Event e) {
}

void Layer::update() {
}

void Layer::render() {
	Renderer::clear();

	Renderer::setColor(0, 0xff, 0xff);
	for (auto& circle : circles) {
		Renderer::drawCircle(circle.cx, circle.cy, circle.r);
	}

	Renderer::present();
}
