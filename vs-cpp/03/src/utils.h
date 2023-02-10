#ifndef UTILS_H
#define UTILS_H

#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <iostream>
#include <cmath>
#include <iomanip>

#define PI 3.1415926535897932384626433832795028841971

struct Connection;

struct CircleType {
	int index;
	double r;
	double sizeMultiplier;
	int count;
	double weight;

	CircleType(int index, double r)
		: index(index), r(r), sizeMultiplier(0.), count(0), weight(0.) {
	}

	friend std::ostream& operator<<(std::ostream& os, const CircleType& ct) {
		return os << "<CircleType " << ct.index << " r=" << ct.r << " count=" << ct.count << ">";
	}
};

struct Input {
	std::string name;
	std::vector<CircleType> types;
};

struct Point {
	double x, y;

	friend std::ostream& operator<<(std::ostream& os, const Point& p) {
		return os << "(" << p.x << ";" << p.y << ")";
	}
};

enum class ConnType {
	CIRCLE,
	WALL,
	CORNER
};

enum class Wall {
	UP, RIGHT, DOWN, LEFT
};

enum class Corner {
	TL, TR, BL, BR
};

struct Circle;

struct Connection {
	ConnType type;
	std::shared_ptr<Circle> c1;
	union {
		std::shared_ptr<Circle> c2;
		Wall wall = Wall::LEFT;
		Corner corner;
	};
	double maxRadiusLeft = 0;
	double maxRadiusRight = 0;

	Connection(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2)
		: type(ConnType::CIRCLE), c1(c1), c2(c2) {}

	Connection(std::shared_ptr<Circle> c1, Wall wall)
		: type(ConnType::WALL), c1(c1), wall(wall) {}

	Connection(Corner corner)
		: type(ConnType::CORNER), c1(nullptr), corner(corner) {}

	virtual ~Connection() {}

	static std::shared_ptr<Connection> create(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2) {
		return std::make_shared<Connection>(c1, c2);
	}

	static std::shared_ptr<Connection> create(std::shared_ptr<Circle> c1, Wall wall) {
		return std::make_shared<Connection>(c1, wall);
	}

	static std::shared_ptr<Connection> create(Corner corner) {
		return std::make_shared<Connection>(corner);
	}

	friend std::ostream& operator<<(std::ostream& os, const Connection& c) {
		os << "<Connection ";
		if (c.type == ConnType::CIRCLE) {
			os << "Circle " << c.c2 << ">";
		} else if (c.type == ConnType::WALL) {
			os << "Wall ";
			if (c.wall == Wall::DOWN) os << "DOWN";
			else if (c.wall == Wall::LEFT) os << "LEFT";
			else if (c.wall == Wall::RIGHT) os << "RIGHT";
			else if (c.wall == Wall::UP) os << "UP";
		} else if (c.type == ConnType::CORNER) {
			os << "Corner ";
			if (c.corner == Corner::TL) os << "TL";
			else if (c.corner == Corner::TR) os << "TR";
			else if (c.corner == Corner::BL) os << "BL";
			else if (c.corner == Corner::BR) os << "BR";
		}
		os << " lr=" << c.maxRadiusLeft;
		os << " rr=" << c.maxRadiusRight;
		os << ">";
		return os;
	}

	friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Connection>& c) {
		os << "<Connection ";
		if (c->type == ConnType::CIRCLE) {
			os << "Circle " << c->c2.get() << ">";
		} else if (c->type == ConnType::WALL) {
			os << "Wall ";
			if (c->wall == Wall::DOWN) os << "DOWN";
			else if (c->wall == Wall::LEFT) os << "LEFT";
			else if (c->wall == Wall::RIGHT) os << "RIGHT";
			else if (c->wall == Wall::UP) os << "UP";
		} else if (c->type == ConnType::CORNER) {
			os << "Corner ";
			if (c->corner == Corner::TL) os << "TL";
			else if (c->corner == Corner::TR) os << "TR";
			else if (c->corner == Corner::BL) os << "BL";
			else if (c->corner == Corner::BR) os << "BR";
		}
		os << " lr=" << c->maxRadiusLeft;
		os << " rr=" << c->maxRadiusRight;
		os << ">";
		return os;
	}
};

struct Circle {
	int typeIndex;
	double cx, cy, r;
	std::vector<std::shared_ptr<Connection>> conns;
	Circle(double cx, double cy, double r) : cx(cx), cy(cy), r(r) {}

	static std::shared_ptr<Circle> create(double cx, double cy, double r) {
		return std::make_shared<Circle>(cx, cy, r);
	}

	friend std::ostream& operator<<(std::ostream& os, const Circle& c) {
		os << "<Circle cx=" << c.cx << " cy=" << c.cy << " r=" << c.r;
		for (auto& conn : c.conns) {
			os << " " << *conn;
		}
		os << ">";
		return os;
	}

	friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Circle>& c) {
		if (c == nullptr) return os << "<Circle nullptr>";
		os << "<Circle cx=" << c->cx << " cy=" << c->cy << " r=" << c->r;
		for (auto& conn : c->conns) {
			os << " " << *conn;
		}
		os << ">";
		return os;
	}
};

struct PossibleCircle : std::enable_shared_from_this<PossibleCircle> {
	std::vector<std::shared_ptr<Connection>> conns;
	std::shared_ptr<Circle> circle;
	double maxRadius = 0.;

	PossibleCircle(std::shared_ptr<Circle> circle, std::vector<std::shared_ptr<Connection>> conns)
		: conns(conns), circle(circle) { }

	static std::shared_ptr<PossibleCircle> create(std::shared_ptr<Circle> circle, std::vector<std::shared_ptr<Connection>> conns) {
		return std::make_shared<PossibleCircle>(circle, conns);
	}

	std::shared_ptr<PossibleCircle> setMaxRadius(double maxRadius) {
		this->maxRadius = maxRadius;
		return shared_from_this();
	}
};

static Point intersectionTwoCircles(double cx1, double cy1, double cr1, double cx2, double cy2, double cr2) {
	double dx = cx2 - cx1;
	double dy = cy2 - cy1;
	double d = std::sqrt(dx * dx + dy * dy);
	double a = (cr1 * cr1 - cr2 * cr2 + d * d) / (2. * d);
	double h = std::sqrt(cr1 * cr1 - a * a);

	double x3 = cx1 + (a * dx + h * dy) / d;
	double y3 = cy1 + (a * dy - h * dx) / d;

	return Point{x3, y3};
}

static std::shared_ptr<Circle> circleFromTwoCircles(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r) {
	auto p = intersectionTwoCircles(c1->cx, c1->cy, c1->r + r, c2->cx, c2->cy, c2->r + r);
	return Circle::create(p.x, p.y, r);
}

#endif
