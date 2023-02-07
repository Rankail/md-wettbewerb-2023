#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <numbers>
#include <memory>
#include <iostream>
#include <cmath>
#include <exception>

struct Connection;

struct CircleType {
	int index;
	double r;
	double sizeMultiplier;
	int count;

	CircleType(int index, double r)
		: index(index), r(r), sizeMultiplier(0), count(0) {
	}

	friend std::ostream& operator<<(std::ostream& os, const CircleType& ct) {
		return os << "<CircleType " << ct.index << " r=" << ct.r << " count=" << ct.count << ">";
	}
};


struct Point {
	double x, y;

	friend std::ostream& operator<<(std::ostream& os, const Point& p) {
		return os << "(" << p.x << ";" << p.y << ")";
	}
};


struct Circle {
	double cx, cy, r;
	std::vector<Connection> conns;
	Circle(double cx, double cy, double r) : cx(cx), cy(cy), r(r) {}

	static std::shared_ptr<Circle> create(double cx, double cy, double r) {
		return std::make_shared<Circle>(cx, cy, r);
	}

	friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Circle>& c) {
		return os << "<Circle cx=" << c->cx << " cy=" << c->cy << " r=" << c->r << ">";
	}
};


enum class ConnType {
	CIRCLE,
	WALL
};

enum class Wall {
	UP, RIGHT, DOWN, LEFT
};

struct Connection {
	ConnType type;
	std::shared_ptr<Circle> other;
	Wall wall = Wall::LEFT;
	double angle; //(from c1 to c2) => connection-point = c1+sin/cos(angle) * c1.r

	Connection(std::shared_ptr<Circle> self, std::shared_ptr<Circle> other)
		: type(ConnType::CIRCLE), other(other) {
		angle = std::atan2(other->cy - self->cy, other->cx - self->cx);
	}

	Connection(Wall wall)
		: type(ConnType::WALL), wall(wall), angle(0) { }
};

static Circle leftWallDown(std::shared_ptr<Circle> c, double r) {
	double cx = r;
	double cy = c->cy + 2. * std::sqrt(c->r * r);
	return Circle{cx, cy, r};
}

static Circle leftWallUp(std::shared_ptr<Circle> c, double r) {
	double cx = r;
	double cy = c->cy - 2. * std::sqrt(c->r * r);
	return Circle{cx, cy, r};
}

static Circle topWallRight(std::shared_ptr<Circle> c, double r) {
	double cx = c->cx + 2. * std::sqrt(c->r * r);
	double cy = r;
	return Circle{cx, cy, r};
}

static Circle topWallLeft(std::shared_ptr<Circle> c, double r) {
	double cx = c->cx - 2. * std::sqrt(c->r * r);
	double cy = r;
	return Circle{cx, cy, r};
}

static Circle bottomWallRight(std::shared_ptr<Circle> c, double r, double h) {
	double cx = c->cx + 2. * std::sqrt(c->r * r);
	double cy = h - r;
	return Circle{cx, cy, r};
}

static Circle bottomWallLeft(std::shared_ptr<Circle> c, double r, double h) {
	double cx = c->cx - 2. * std::sqrt(c->r * r);
	double cy = h - r;
	return Circle{cx, cy, r};
}

static Circle rightWallUp(std::shared_ptr<Circle> c, double r, double w) {
	double cx = w - r;
	double cy = c->cy - 2. * std::sqrt(c->r * r);
	return Circle{cx, cy, r};
}

static Circle rightWallDown(std::shared_ptr<Circle> c, double r, double w) {
	double cx = w - r;
	double cy = c->cy + 2. * std::sqrt(c->r * r);
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

static Circle circleFromTwoCircles(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r) {
	auto p = intersectionTwoCircles(c1->cx, c1->cy, c1->r + r, c2->cx, c2->cy, c2->r + r);
	return Circle{p.x, p.y, r};
}
