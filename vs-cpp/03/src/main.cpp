#include <vector>
#include <algorithm>
#include <cmath>

struct Connection;

struct Point {
	double x, y;
};

struct Circle {
	double cx, cy, r;
};

struct Connection {
	const Circle& c1;
	const Circle& c2;
	double angle; //(from c1 to c2) => connection-point = c1+sin/cos(angle) * c1.r
};

static Circle leftWallDown(const Circle& c, double r) {
	double cx = r;
	double cy = c.cy + 2. * std::sqrt(c.r * r);
	return Circle{cx, cy, r};
}

static Circle leftWallUp(const Circle& c, double r) {
	double cx = r;
	double cy = c.cy - 2. * std::sqrt(c.r * r);
	return Circle{cx, cy, r};
}

static Circle topWallRight(const Circle& c, double r) {
	double cx = c.cx + 2. * std::sqrt(c.r * r);
	double cy = r;
	return Circle{cx, cy, r};
}

static Circle topWallLeft(const Circle& c, double r) {
	double cx = c.cx - 2. * std::sqrt(c.r * r);
	double cy = r;
	return Circle{cx, cy, r};
}

static Circle bottomWallRight(const Circle& c, double r, double h) {
	double cx = c.cx + 2. * std::sqrt(c.r * r);
	double cy = h - r;
	return Circle{cx, cy, r};
}

static Circle bottomWallLeft(const Circle& c, double r, double h) {
	double cx = c.cx - 2. * std::sqrt(c.r * r);
	double cy = h - r;
	return Circle{cx, cy, r};
}

static Circle rightWallUp(const Circle& c, double r, double w) {
	double cx = w - r;
	double cy = c.cy - 2. * std::sqrt(c.r * r);
	return Circle{cx, cy, r};
}

static Circle rightWallDown(const Circle& c, double r, double w) {
	double cx = w - r;
	double cy = c.cy + 2. * std::sqrt(c.r * r);
	return Circle{cx, cy, r};
}

static Point intersectionTwoCircles(double cx1, double cy1, double cr1, double cx2, double cy2, double cr2) {
	double dx = cx2 - cx1;
	double dy = cy2 - cy1;
	double d = std::sqrt(dx * dx + dy * dy);
	double a = (cr1 * cr1 - cr2 * cr2 + d * d) / (2. * d);
	double h = std::sqrt(cr1 * cr1 - a * a);

	double x3 = cx1 + (a * dx + h * dy) / d;
	double y3 = cy1 + (a * dy + h * dx) / d;

	return Point{x3, y3};
}

static Circle circleFromTwoCircles(const Circle& c1, const Circle& c2, double r) {
	auto p = intersectionTwoCircles(c1.cx, c1.cy, c1.r+r, c2.cx, c2.cy, c2.r+r);
	return Circle{p.x, p.y, r};
}

int main(int argc, char** argv) {
	std::vector<double> sizes = std::vector<double>();
	sizes.push_back(3.);
	sizes.push_back(4.);
	sizes.push_back(6.);
	sizes.push_back(9.);
	sizes.push_back(12.);

	std::sort(sizes.begin(), sizes.end(), [](const double lhs, const double rhs) {
		return rhs > lhs;
	});

	double w = 500., h = 200.;
	double size = w * h;
	double maxDiverse = 1. - 1. / sizes.size();
	double block = 0.;
	for (auto& d : sizes) {
		block += d;
	}
	double possibleBlocks = size / block;

	std::vector<Circle> circles = std::vector<Circle>();
	circles.push_back(Circle{sizes[0], sizes[0], sizes[0]});


	

}