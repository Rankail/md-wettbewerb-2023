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
	std::cout << "Reading inputfile" << std::endl;
	std::ifstream file;
	file.open(path);
	if (!file.is_open()) {
		std::cout << "Failed to read inputfile!\n" << std::endl;
		return false;
	}

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

	double max = types[0].r;
	double block = 0;
	for (CircleType& t : types) {
		t.sizeMultiplier = t.r / max;
		block += t.r * t.r * PI;
	}
	numBlocks = block / w * h;

	std::cout << "Finished reading inputfile" << std::endl;

	return true;
}

/*
* writes constructed circles to file
*/
void Solver::outputCircles(const std::string& path) {
	std::cout << "Finished computing" << std::endl;
	std::ofstream file;
	file.open(path);
	if (!file.is_open()) {
		printf("Failed to open output-file.\n");
		return;
	}
	std::cout << "Writing to '" << path << "'" << std::endl;

	for (auto& circle : circles) {
		file << std::setprecision(std::numeric_limits<double>::digits10+2) << circle->cx << " " << circle->cy << " " << circle->r << " " << circle->typeIndex << "\n";
	}
	file.close();
	std::cout << "Finished" << std::endl;
}

/*
* constructs circles
*/
void Solver::run() {
	int cur = 0;
	
	if (!loaded) return;

	std::cout << "Starting computation" << std::endl;

	connections = std::vector<std::shared_ptr<Connection>>();
	connections.push_back(Connection::create(Corner::TL));
	connections.push_back(Connection::create(Corner::TR));
	connections.push_back(Connection::create(Corner::BL));
	connections.push_back(Connection::create(Corner::BR));
	circles = std::vector<std::shared_ptr<Circle>>();
	calcMaxRadius();

	while (true) {

		stepWeights();
		for (auto& type : types) {
			if (connections.empty()) goto finished;
			if (type.weight < 1.) continue;
			type.weight--;

			std::shared_ptr<PossibleCircle> pc = getNextCircle(type);
			if (pc == nullptr) continue;
			
			std::shared_ptr<Circle> c = pc->circle;
			c->typeIndex = type.index;
			for (auto& conn : pc->conns) {
				connections.push_back(conn);
			}
			circles.push_back(c);
			calcMaxRadius();
			
			type.count++;
		}
	}
	finished:

	double totalCount = 0.;
	double sumCountSquared = 0.;
	double size = 0;
	for (auto& type : types) {
		size += type.count * type.r * type.r;
		totalCount += (double)type.count;
		sumCountSquared += (double)(type.count * type.count);
	}
	size *= PI;
	std::cout << "Result:\n";
	/*for (auto& c : circles) {
		std::cout << c << std::endl;
	}*/
	double A = size / (w * h);
	double D = 1. - sumCountSquared / totalCount / totalCount;
	std::cout << "size: " << A << std::endl;
	std::cout << "Divers: " << D << std::endl;
	std::cout << "B: " << A * D << std::endl;
}

/*
* Advances weights. Advances by maximum of 1 so no weight ever gets to 2 and would need multiple iterations to reach <1.
*/
void Solver::stepWeights() {
	double maxWeight = 0.;
	std::vector<double> weights = std::vector<double>();
	for (int i = 0; i < types.size(); i++) {
		//double weight = types[i].sizeMultiplier * (1. - types[i].count*types[i].sizeMultiplier / numBlocks);
		double weight = 1.;
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
std::shared_ptr<PossibleCircle> Solver::getNextCircle(CircleType& t) {
	auto possible = std::vector<std::shared_ptr<PossibleCircle>>();
	for (auto& conn : connections) {
		if (conn->maxRadiusLeft >= t.r) {
			if (conn->type == ConnType::CIRCLE) {
				possible.push_back(getCircleCircleLeft(conn->c1, conn->c2, t.r)->setMaxRadius(conn->maxRadiusLeft));
			} else if (conn->type == ConnType::WALL) {
				possible.push_back(circleFromWall(conn, true, t.r)->setMaxRadius(conn->maxRadiusLeft));
			} else  if (conn->type == ConnType::CORNER) {
				possible.push_back(circlFromCorner(conn->corner, t.r)->setMaxRadius(conn->maxRadiusLeft));
			}
			if (conn->maxRadiusLeft == t.r) {
				return possible[possible.size() - 1];
			}
		}
		if (conn->maxRadiusRight >= t.r) {
			if (conn->type == ConnType::CIRCLE) {
				possible.push_back(getCircleCircleRight(conn->c1, conn->c2, t.r)->setMaxRadius(conn->maxRadiusRight));
			} else if (conn->type == ConnType::WALL) {
				possible.push_back(circleFromWall(conn, false, t.r)->setMaxRadius(conn->maxRadiusRight));
			}
			if (conn->maxRadiusRight == t.r) {
				return possible[possible.size() - 1];
			}
		}
	}
	if (possible.empty()) return nullptr;

	return *std::min_element(possible.begin(), possible.end(),
		[](const std::shared_ptr<PossibleCircle>& a, const std::shared_ptr<PossibleCircle>& b) {
			return a->maxRadius < b->maxRadius;
	});
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

void Solver::calcMaxRadiusConnectionCorner(std::shared_ptr<Connection> conn) {
	int i = (int)types.size() - 1;
	while (i >= 0) {
		double r = types[i].r;
		double cx, cy;
		if (conn->corner == Corner::TL) {
			cx = r; cy = r;
		} else if (conn->corner == Corner::TR) {
			cx = w - r; cy = r;
		} else if (conn->corner == Corner::BL) {
			cx = r; cy = h - r;
		} else if (conn->corner == Corner::BR) {
			cx = w - r; cy = h - r;
		}

		if (!checkValid(cx, cy, r)) {
			break;
		}
		i--;
	}
	if (i == types.size() - 1) {
		conn->maxRadiusLeft = 0.;
	} else {
		conn->maxRadiusLeft = types[i + 1].r;
	}

	conn->maxRadiusRight = 0.;
}

void Solver::calcMaxRadiusConnectionWall(std::shared_ptr<Connection> conn) {
	auto& c = conn->c1;
	int i = (int)types.size() - 1;
	while (i >= 0) {
		double r = types[i].r;
		double cx, cy;
		double wd = 2 * std::sqrt(c->r * r);
		if (conn->wall == Wall::UP) {
			cx = c->cx - wd;
			cy = r;
		} else if (conn->wall == Wall::LEFT) {
			cx = r;
			cy = c->cy + wd;
		} else if (conn->wall == Wall::DOWN) {
			cx = c->cx + wd;
			cy = h - r;
		} else if (conn->wall == Wall::RIGHT) {
			cx = w - r;
			cy = c->cy - wd;
		}
		if (!checkValid(cx, cy, r)) {
			break;
		}
		i--;
	}
	if (i == types.size() - 1) {
		conn->maxRadiusLeft = 0;
	} else {
		conn->maxRadiusLeft = types[i + 1].r;
	}

	i = (int)types.size() - 1;
	while (i >= 0) {
		double r = types[i].r;
		double cx, cy;
		double wd = 2 * std::sqrt(c->r * r);
		if (conn->wall == Wall::UP) {
			cx = c->cx + wd;
			cy = r;
		} else if (conn->wall == Wall::LEFT) {
			cx = r;
			cy = c->cy - wd;
		} else if (conn->wall == Wall::DOWN) {
			cx = c->cx - wd;
			cy = h - r;
		} else if (conn->wall == Wall::RIGHT) {
			cx = w - r;
			cy = c->cy + wd;
		}
		if (!checkValid(cx, cy, r)) {
			break;
		}
		i--;
	}
	if (i == types.size() - 1) {
		conn->maxRadiusRight = 0;
	} else {
		conn->maxRadiusRight = types[i + 1].r;
	}
}

void Solver::calcMaxRadiusConnectionCircle(std::shared_ptr<Connection> conn) {
	auto& c = conn->c1;
	int i = (int)types.size() - 1;
	while (i >= 0) {
		double r = types[i].r;
		Point n = intersectionTwoCircles(c->cx, c->cy, c->r + r, conn->c2->cx, conn->c2->cy, conn->c2->r + r);
		if (!checkValid(n.x, n.y, r)) {
			break;
		}
		i--;
	}
	if (i == types.size() - 1) {
		conn->maxRadiusLeft = 0;
	} else {
		conn->maxRadiusLeft = types[i + 1].r;
	}

	i = (int)types.size() - 1;
	while (i >= 0) {
		double r = types[i].r;
		Point n = intersectionTwoCircles(conn->c2->cx, conn->c2->cy, conn->c2->r + r, c->cx, c->cy, c->r + r);
		if (!checkValid(n.x, n.y, r)) {
			break;
		}
		i--;
	}
	if (i == types.size() - 1) {
		conn->maxRadiusRight = 0;
	} else {
		conn->maxRadiusRight = types[i + 1].r;
	}
}

/*
* calculates the largest radius that can be placed on either side of an connection.
*/
void Solver::calcMaxRadius() {
	for (auto& conn : connections) {
		if (conn->type == ConnType::CIRCLE) {
			calcMaxRadiusConnectionCircle(conn);
		} else if (conn->type == ConnType::WALL) {
			calcMaxRadiusConnectionWall(conn);
		} else  if (conn->type == ConnType::CORNER) {
			calcMaxRadiusConnectionCorner(conn);
		}
	}

	if (!connections.empty()) {
		connections.erase(std::remove_if(connections.begin(), connections.end(), [&](const std::shared_ptr<Connection>& conn) {
			return conn->maxRadiusLeft == 0 && conn->maxRadiusRight == 0;
		}), connections.end());
	}
}

std::shared_ptr<PossibleCircle> Solver::circleFromWall(std::shared_ptr<Connection> conn, bool left, double r) {
	double wd = 2 * std::sqrt(conn->c1->r * r) * (left ? 1 : -1);
	std::shared_ptr<Circle> c = nullptr;
	if (conn->wall == Wall::UP) {
		c = Circle::create(conn->c1->cx - wd, r, r);
	} else if (conn->wall == Wall::LEFT) {
		c = Circle::create(r, conn->c1->cy + wd, r);
	} else if (conn->wall == Wall::DOWN) {
		c = Circle::create(conn->c1->cx + wd, h - r, r);
	} else if (conn->wall == Wall::RIGHT) {
		c = Circle::create(w - r, conn->c1->cy - wd, r);
	}
	return PossibleCircle::create(c, std::vector<std::shared_ptr<Connection>>{Connection::create(c, conn->c1), Connection::create(c, conn->wall)});
}

std::shared_ptr<PossibleCircle> Solver::getCircleCircleLeft(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r) {
	auto n = circleFromTwoCircles(c1, c2, r);
	return PossibleCircle::create(n, std::vector <std::shared_ptr<Connection>>{Connection::create(n, c1), Connection::create(n, c2)});
}

std::shared_ptr<PossibleCircle> Solver::getCircleCircleRight(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r) {
	auto n = circleFromTwoCircles(c2, c1, r);
	return PossibleCircle::create(n, std::vector<std::shared_ptr<Connection>>{Connection::create(n, c1), Connection::create(n, c2)});
}

std::shared_ptr<PossibleCircle> Solver::circlFromCorner(Corner corner, double r) {
	std::shared_ptr<PossibleCircle> pc;
	std::shared_ptr<Circle> c;
	std::vector<std::shared_ptr<Connection>> conns = std::vector<std::shared_ptr<Connection>>();
	if (corner == Corner::TL) {
		c = Circle::create(r, r, r);
		conns.push_back(Connection::create(c, Wall::LEFT));
		conns.push_back(Connection::create(c, Wall::UP));
	} else if (corner == Corner::TR) {
		c = Circle::create(w - r, r, r);
		conns.push_back(Connection::create(c, Wall::RIGHT));
		conns.push_back(Connection::create(c, Wall::UP));
	} else if (corner == Corner::BL) {
		c = Circle::create(r, h - r, r);
		conns.push_back(Connection::create(c, Wall::LEFT));
		conns.push_back(Connection::create(c, Wall::DOWN));
	} else if (corner == Corner::BR) {
		c = Circle::create(w - r, h - r, r);
		conns.push_back(Connection::create(c, Wall::RIGHT));
		conns.push_back(Connection::create(c, Wall::DOWN));
	}
	return PossibleCircle::create(c, conns);
}
