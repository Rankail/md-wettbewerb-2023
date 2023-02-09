#include "solver.h"

#include "utils.h"

Solver::Solver(const std::string& file) {
	loaded = readInput(file);
}

Solver::~Solver() {
}

/*
* Reads input from file and calculates some basic statistics
*/
bool Solver::readInput(const std::string& path) {
	std::ifstream file;
	file.open(path);
	if (!file.is_open()) {
		printf("Failed to read file!\n");
		return false;
	}

	auto lastSlash = path.find_last_of("/");
	auto lastDot = path.find_last_of(".");
	this->name = path.substr(lastSlash, lastDot - lastSlash);

	std::string line;
	std::getline(file, line);
	std::getline(file, line);
	size_t space = line.find(' ');
	w = (double)std::stoi(line.substr(0, space));
	h = (double)std::stoi(line.substr(space + 1));

	int i = 0;
	while (std::getline(file, line)) {
		space = line.find(' ');
		types.emplace_back(i, (double)std::stoi(line.substr(0, space)));
		i++;
	}

	std::sort(types.begin(), types.end(), [](const CircleType& lhs, const CircleType& rhs) {
		return lhs.r > rhs.r;
	});

	double min = types[types.size() - 1].r;
	double block = 0;
	for (CircleType& t : types) {
		t.sizeMultiplier = t.r / min;
		block += t.r * t.r * PI;
	}
	numBlocks = block / w * h;


	return true;
}

/*
* writes constructed circles to file
*/
void Solver::outputCircles(const std::string& path) {
	std::ofstream file;
	file.open(path);
	if (!file.is_open()) {
		printf("Failed to open output-file.\n");
		return;
	}

	for (auto& circle : circles) {
		file << std::setprecision(std::numeric_limits<double>::digits10+2) << circle->cx << " " << circle->cy << " " << circle->r << " " << circle->typeIndex << "\n";
	}
}

/*
* constructs circles
*/
void Solver::run() {
	int cur = 0;
	
	if (!loaded) return;

	circles = std::vector<std::shared_ptr<Circle>>();
	CircleType& t = types[0];
	std::cout << t << std::endl;
	circles.push_back(Circle::create(t.r, t.r, t.r));
	circles[0]->conns.push_back(Connection(Wall::LEFT));
	circles[0]->conns.push_back(Connection(Wall::UP));
	circles[0]->typeIndex = t.index;
	calcMaxRadius(circles[0]);
	t.count++;

	while (true) {
		for (auto& type : types) {
			if (type.weight < 1.) continue;
			type.weight--;

			std::shared_ptr<Circle> c = getNextCircle(type);
			if (c == nullptr) continue;
			c->typeIndex = type.index;

			std::cout << c << circles.size() << std::endl;
			circles.push_back(c);
			double maxR = 0.;
			for (auto& cs : circles) {
				calcMaxRadius(cs);
				for (auto& conn : cs->conns) {
					maxR = std::max(maxR, conn.maxRadiusLeft);
					maxR = std::max(maxR, conn.maxRadiusRight);
				}
			}
			if (maxR == 0) goto finished;
			std::cout << maxR << std::endl;
			t.count++;
		}
		stepWeights();
	}
	finished:

	std::cout << "Result:\n";
	for (auto& c : circles) {
		std::cout << c << std::endl;
	}
}

/*
* Advances weights. Advances by maximum of 1 so no weight ever gets to 2 and would need multiple iterations to reach <1.
*/
void Solver::stepWeights() {
	double maxWeight = 0.;
	std::vector<double> weights = std::vector<double>();
	for (int i = 0; i < types.size(); i++) {
		double weight = types[i].sizeMultiplier * (1. - types[i].count / numBlocks);
		weights.push_back(weight);
		maxWeight = std::max(maxWeight, weight);
	}

	for (int i = 0; i < types.size(); i++) {
		types[i].weight += weights[i] / maxWeight;
	}
}


/*
* Finds all possible next circles and returns the first.
*/
std::shared_ptr<Circle> Solver::getNextCircle(CircleType& t) {
	std::vector<std::pair<double, Circle>> possible = std::vector<std::pair<double, Circle>>();
	for (auto& circle : circles) {
		checkCircle(possible, circle, t.r);
	}

	std::cout << "--------------------------------------------\n";

	std::sort(possible.begin(), possible.end(), [](const std::pair<double, Circle>& a, const std::pair<double, Circle>& b) {
		return a.first < b.first;
	});
	for (auto& c : possible) {
		std::cout << c.second << std::endl;
	}
	std::cout << "############################################\n";

	if (possible.empty()) {
		std::cout << "empty\n";
		return nullptr;
	}

	return std::make_shared<Circle>(possible[0].second);
}

/*
* checks if a circle is in bounds and does not collide with any other circle
*/
bool Solver::checkValid(double cx, double cy, double r) {
	if (cx < r) return false;
	if (cy < r) return false;
	if (cx + r > w) return false;
	if (cy + r > h) return false;

	for (auto& c : circles) {
		if ((c->cx - cx) * (c->cx - cx) + (c->cy - cy) * (c->cy - cy) < (r + c->r) * (c->r + r)-0.0000000001) return false;
	}
	return true;
}

/*
* calculates the largest radius that can be placed on either side of an connection.
*/
void Solver::calcMaxRadius(std::shared_ptr<Circle>& c) {
	for (auto& conn : c->conns) {
		if (conn.type == ConnType::WALL) {
			int i = types.size()-1;
			while (i >= 0) {
				double r = types[i].r;
				double cx, cy;
				double wd = 2 * std::sqrt(c->r * r);
				if (conn.wall == Wall::UP) {
					cx = c->cx - wd;
					cy = r;
				} else if (conn.wall == Wall::LEFT) {
					cx = r;
					cy = c->cy + wd;
				} else if (conn.wall == Wall::DOWN) {
					cx = c->cx + wd;
					cy = h - r;
				} else if (conn.wall == Wall:: RIGHT) {
					cx = w - r;
					cy = c->cx - wd;
				}
				if (!checkValid(cx, cy, r)) {
					break;
				}
				i--;
			}
			if (i == types.size()-1) {
				conn.maxRadiusLeft = 0;
			} else {
				conn.maxRadiusLeft = types[i+1].r;
			}

			i = types.size() - 1;
			while (i >= 0) {
				double r = types[i].r;
				double cx, cy;
				double wd = 2 * std::sqrt(c->r * r);
				if (conn.wall == Wall::UP) {
					cx = c->cx + wd;
					cy = r;
				} else if (conn.wall == Wall::LEFT) {
					cx = r;
					cy = c->cy - wd;
				} else if (conn.wall == Wall::DOWN) {
					cx = c->cx - wd;
					cy = h - r;
				} else if (conn.wall == Wall::RIGHT) {
					cx = w - r;
					cy = c->cx + wd;
				}
				if (!checkValid(cx, cy, r)) {
					break;
				}
				i--;
			}
			if (i == types.size() - 1) {
				conn.maxRadiusRight = 0;
			} else {
				conn.maxRadiusRight = types[i+1].r;
			}
		} else if (conn.type == ConnType::CIRCLE) {
			int i = types.size() - 1;
			while (i >= 0) {
				double r = types[i].r;
				if (circles.size() > 40) {
					std::cout << "a";
				}
				Point n = intersectionTwoCircles(c->cx, c->cy, c->r + r, conn.other->cx, conn.other->cy, conn.other->r + r);
				if (!checkValid(n.x, n.y, r)) {
					break;
				}
				i--;
			}
			if (i == types.size() - 1) {
				conn.maxRadiusLeft = 0;
			} else {
				conn.maxRadiusLeft = types[i+1].r;
			}

			i = types.size() - 1;
			while (i >= 0) {
				double r = types[i].r;
				if (circles.size() > 40) {
					std::cout << "a";
				}
				Point n = intersectionTwoCircles2(c->cx, c->cy, c->r + r, conn.other->cx, conn.other->cy, conn.other->r + r);
				if (!checkValid(n.x, n.y, r)) {
					break;
				}
				i--;
			}
			if (i == types.size() - 1) {
				conn.maxRadiusRight = 0;
			} else {
				conn.maxRadiusRight = types[i+1].r;
			}
		}
	}
}

/*
* searches for possible Positions around circle (no checks needed because of max-circle
*/
void Solver::checkCircle(std::vector<std::pair<double, Circle>>& possible, std::shared_ptr<Circle> c, double r) {
	double wd = 2 * sqrt(r * c->r);
	for (auto& conn : c->conns) {
		if (conn.maxRadiusLeft >= r) {
			if (conn.type == ConnType::WALL) {
				if (conn.wall == Wall::LEFT) {
					possible.push_back(std::make_pair(conn.maxRadiusLeft, getWallLeftDown(c, r, wd)));
				} else if (conn.wall == Wall::RIGHT) {
					possible.push_back(std::make_pair(conn.maxRadiusLeft, getWallRightUp(c, r, wd)));
				} else if (conn.wall == Wall::UP) {
					possible.push_back(std::make_pair(conn.maxRadiusLeft, getWallUpLeft(c, r, wd)));
				} else if (conn.wall == Wall::DOWN) {
					possible.push_back(std::make_pair(conn.maxRadiusLeft, getWallDownRight(c, r, wd)));
				}
			} else if (conn.type == ConnType::CIRCLE) {
				possible.push_back(std::make_pair(conn.maxRadiusLeft, getCircleCircleLeft(c, conn.other, r)));
			}
		}
		if (conn.maxRadiusRight >= r) {
			if (conn.type == ConnType::WALL) {
				if (conn.wall == Wall::LEFT) {
					possible.push_back(std::make_pair(conn.maxRadiusRight, getWallLeftUp(c, r, wd)));
				} else if (conn.wall == Wall::RIGHT) {
					possible.push_back(std::make_pair(conn.maxRadiusRight, getWallRightDown(c, r, wd)));
				} else if (conn.wall == Wall::UP) {
					possible.push_back(std::make_pair(conn.maxRadiusRight, getWallUpRight(c, r, wd)));
				} else if (conn.wall == Wall::DOWN) {
					possible.push_back(std::make_pair(conn.maxRadiusRight, getWallDownLeft(c, r, wd)));
				}
			} else if (conn.type == ConnType::CIRCLE) {
				possible.push_back(std::make_pair(conn.maxRadiusRight, getCircleCircleRight(c, conn.other, r)));
			}
		}
	}
}

Circle Solver::getWallUpLeft(std::shared_ptr<Circle> c, double r, double wd) {
	Circle n = Circle(c->cx - wd, r, r);
	n.conns.push_back(c);
	n.conns.emplace_back(Wall::UP);
	return n;
}

Circle Solver::getWallUpRight(std::shared_ptr<Circle> c, double r, double wd) {
	Circle n = Circle(c->cx + wd, r, r);
	n.conns.push_back(c);
	n.conns.emplace_back(Wall::UP);
	return n;
}

Circle Solver::getWallLeftUp(std::shared_ptr<Circle> c, double r, double wd) {
	Circle n = Circle(r, c->cy - wd, r);
	n.conns.push_back(c);
	n.conns.emplace_back(Wall::LEFT);
	return n;
}

Circle Solver::getWallLeftDown(std::shared_ptr<Circle> c, double r, double wd) {
	Circle n = Circle(r, c->cy + wd, r);
	n.conns.push_back(c);
	n.conns.emplace_back(Wall::LEFT);
	return n;
}

Circle Solver::getWallDownLeft(std::shared_ptr<Circle> c, double r, double wd) {
	Circle n = Circle(c->cx - wd, h - r, r);
	n.conns.push_back(c);
	n.conns.emplace_back(Wall::DOWN);
	return n;
}

Circle Solver::getWallDownRight(std::shared_ptr<Circle> c, double r, double wd) {
	Circle n = Circle(c->cx + wd, h - r, r);
	n.conns.push_back(c);
	n.conns.emplace_back(Wall::DOWN);
	return n;
}

Circle Solver::getWallRightUp(std::shared_ptr<Circle> c, double r, double wd) {
	Circle n = Circle(w - r, c->cy - wd, r);
	n.conns.push_back(c);
	n.conns.emplace_back(Wall::RIGHT);
	return n;
}

Circle Solver::getWallRightDown(std::shared_ptr<Circle> c, double r, double wd) {
	Circle n = Circle(w - r, c->cy + wd, r);
	n.conns.push_back(c);
	n.conns.emplace_back(Wall::RIGHT);
	return n;
}

Circle Solver::getCircleCircleLeft(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r) {
	Circle n = circleFromTwoCircles(c1, c2, r);
	n.conns.push_back(Connection(c1));
	n.conns.push_back(Connection(c2));
	return n;
}

Circle Solver::getCircleCircleRight(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r) {
	Circle n = circleFromTwoCircles2(c1, c2, r);
	n.conns.push_back(Connection(c1));
	n.conns.push_back(Connection(c2));
	return n;
}