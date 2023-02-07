#include "Layer.h"

#include "Renderer.h"

#include <cmath>

static Circle circleBottomWallToRight(const Circle& connected, double r) {
	double cy = 1000. - r;
	// Pythagoras cx = sqrt((or+r)^2 - (or-r)^2) = sqrt(or^2+2or*r+r^2-or^2+2or*r-r^2) => sqrt(4*or*r) = 2 * sqrt(or*r)
	double cx = connected.cx + 2 * std::sqrt(connected.r * r);
	return Circle{cx, cy, r};
}

static Circle circleLeftWallToBottom(const Circle& connected, double r) {
	double cx = r;
	double cy = connected.cy + 2. * std::sqrt(connected.r * r);
	return Circle{cx, cy, r};
}

static Point circleCircleIntersection(const Circle& c1, const Circle& c2) {
	double dx = c2.cx - c1.cx;
	double dy = c2.cy - c1.cy;
	double d = std::sqrt(dx * dx + dy * dy);
	double a = (c1.r * c1.r - c2.r * c2.r + d * d) / (2. * d);
	double h = std::sqrt(c1.r * c1.r - a * a);

	double x2 = c1.cx + a * dx / d;
	double y2 = c1.cy + a * dy / d;

	double x3 = c1.cx + (a * dx + h * dy) / d;
	double y3 = c1.cy + (a * dy + h * dx) / d;

	return Point{x3, y3};
}

static Circle circleTwo(const Circle& c1, const Circle& c2, double r) {
	Point c = circleCircleIntersection(Circle{c1.cx, c1.cy, c1.r + r}, Circle{c2.cx, c2.cy, c2.r + r});
	return Circle{c.x, c.y, r};
}

static Point innerSoddyCentre(const Circle& c1, const Circle& c2, const Circle& c3) {
	double x12 = c2.cx - c1.cx;
	double y12 = c2.cy - c1.cy;
	double x23 = c3.cx - c2.cx;
	double y23 = c3.cy - c2.cy;
	double x31 = c1.cx - c3.cx;
	double y31 = c1.cy - c3.cy;
	double c = std::sqrt(x12 * x12 + y12 * y12);
	double a = std::sqrt(x23 * x23 + y23 * y23);
	double b = std::sqrt(x31 * x31 + y31 * y31);

	double s = (a + b + c) / 2.;
	double delta = std::sqrt(s * (s - a) * (s - b) * (s - c));
	double l1 = a + delta / (s - a);
	double l2 = b + delta / (s - b);
	double l3 = c + delta / (s - c);
	double lsum = l1 + l2 + l3;
	l1 /= lsum;
	l2 /= lsum;
	l3 /= lsum;

	double xEDP = l1 * c1.cx + l2 * c2.cx + l3 * c3.cx;
	double yEDP = l1 * c1.cy + l2 * c2.cy + l3 * c3.cy;

	return Point{xEDP, yEDP};
}

static Circle constructSoddyCircle(const Circle& c1, const Circle& c2, const Circle& c3) {
	const Point& centre = innerSoddyCentre(c1, c2, c3);
	double dx = centre.x - c1.cx;
	double dy = centre.y - c1.cy;
	double r = std::sqrt(dx * dx + dy * dy) - c1.r;
	return Circle{centre.x, centre.y, r};
}


Layer::Layer() {
	circles = std::vector<Circle>();

	circles.push_back(Circle{300., 300., 100.});
	circles.push_back(Circle{700., 300., 300.});
	circles.push_back(circleTwo(circles[0], circles[1], 200));
	circles.push_back(constructSoddyCircle(circles[0], circles[1], circles[2]));

	cooldown = 0.;
	paused = true;
}

Layer::~Layer() {
}

void Layer::events(SDL_Event e) {
	if (e.type == SDL_KEYUP) {
		if (e.key.keysym.scancode == SDL_SCANCODE_SPACE) {
			paused = !paused;
		}
	}
}

void Layer::update(float dt) {
}

void Layer::render() {
	Renderer::clear();


	Renderer::setColor(0, 0xff, 0xff);
	for (auto& circle : circles) {
		Renderer::drawCircle(circle.cx, circle.cy, circle.r);
	}

	Renderer::present();
}
