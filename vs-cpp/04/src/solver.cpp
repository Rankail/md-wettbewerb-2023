#include "solver.h"

#include "utils.h"
#include <limits>

//#define BINARY_SEARCH_RADIUS

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

	for (int i = 0; i < circleCountAtMax; i++) {
		auto& c = circles[i];
		file << std::setprecision(std::numeric_limits<double>::digits10+2) << c->cx << " " << c->cy << " " << c->r << " " << c->typeIndex << "\n";
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

	conns_unknown = std::vector<std::shared_ptr<Connection>>();
	conns_calculated = std::vector<std::shared_ptr<Connection>>();

	conns_unknown.push_back(Connection::create(Corner::TL));
	conns_unknown.push_back(Connection::create(Corner::TR));
	conns_unknown.push_back(Connection::create(Corner::BL));
	conns_unknown.push_back(Connection::create(Corner::BR));
	circles = std::vector<std::shared_ptr<Circle>>();

	double size = 0.;
	double maxB = 0.;
	double mA = 0.;
	double mD = 0.;
	circleCountAtMax = 0;

	int i = 0;
	while (i < 1000) {

		stepWeights();
		for (auto& type : types) {
			if (conns_unknown.empty() && conns_calculated.empty()) goto finished;
			if (type.weight < 1.) continue;
			type.weight--;

			std::shared_ptr<PossibleCircle> pc = getNextCircle(type);
			if (pc == nullptr) continue;
			std::shared_ptr<Circle> circle = pc->circle;

			// move conns to unknown if they are near the new circle
			auto partition = std::stable_partition(conns_calculated.begin(), conns_calculated.end(), [&](const std::shared_ptr<Connection>& conn) {
				double dx, dy, r;
				if (conn->type == ConnType::CORNER) {
					r = conn->maxRadius + circle->r;
					switch (conn->corner) {
					case Corner::TL: {
						dx = std::abs(circle->cx - r);
						dy = std::abs(circle->cy - r);
						break;
					}
					case Corner::TR: {
						dx = std::abs(circle->cx - w + r);
						dy = std::abs(circle->cy - r);
						break;
					}
					case Corner::BL: {
						dx = std::abs(circle->cx - r);
						dy = std::abs(circle->cy - h + r);
						break;
					}
					case Corner::BR: {
						dx = std::abs(circle->cx - w + r);
						dy = std::abs(circle->cy - h + r);
						break;
					}
					}
				} else if (conn->type == ConnType::WALL) {
					r = circle->r + conn->maxRadius * 2 + conn->c1->r;
					dx = std::abs(circle->cx - conn->c1->cx);
					dy = std::abs(circle->cy - conn->c1->cy);
				} else if (conn->type == ConnType::CIRCLE) {
					r = circle->r + conn->maxRadius * 2 + std::max(conn->c1->r, conn->c2->r);
					dx = std::min(std::abs(circle->cx - conn->c1->cx), std::abs(circle->cx - conn->c2->cx));
					dy = std::min(std::abs(circle->cy - conn->c1->cy), std::abs(circle->cy - conn->c2->cy));
				}
				return dx * dx + dy * dy > r * r;
			});
			conns_unknown.insert(conns_unknown.end(), std::make_move_iterator(partition), std::make_move_iterator(conns_calculated.end()));
			conns_calculated.erase(partition, conns_calculated.end());

			// newly formed connections
			for (auto& conn : pc->conns) {
				conns_unknown.push_back(conn);
			}

			std::sort(conns_calculated.begin(), conns_calculated.end(), [](const std::shared_ptr<Connection>& a, const std::shared_ptr<Connection>& b) {
				return a->maxRadius < b->maxRadius;
			});

			circle->typeIndex = type.index;
			circles.push_back(circle);
			/*std::sort(connections.begin(), connections.end(), [](const std::shared_ptr<Connection>& a, const std::shared_ptr<Connection>& b) {
				return std::min(a->maxRadiusLeft, a->maxRadiusRight) < std::min(b->maxRadiusLeft, b->maxRadiusRight);
			});*/

			type.count++;

			// calculate stats to find maximum
			size += circle->r * circle->r * PI;
			double sumCountSquared = 0.;
			double totalCount = 0.;
			for (auto& t : types) {
				sumCountSquared += type.count * type.count;
				totalCount += type.count;
			}
			double A = size / (w * h);
			double D = 1. - sumCountSquared / (totalCount * totalCount);
			double B = A * D;

			if (B > maxB) {
				mA = A;
				mD = D;
				maxB = B;
				circleCountAtMax = circles.size();
			}

			if (circles.size() % 1000 == 0) {
				std::cout << "Max: " << maxB << " = " << mA << " * " << mD << " (" << circleCountAtMax << " circles)" << std::endl;
			}
		}
		i++;
	}
	finished:

	double totalCount = 0.;
	double sumCountSquared = 0.;
	for (auto& type : types) {
		totalCount += (double)type.count;
		sumCountSquared += (double)(type.count * type.count);
	}
	std::cout << "Result:\n";
	/*for (auto& c : circles) {
		std::cout << c << std::endl;
	}*/
	double A = size / (w * h);
	double D = 1. - sumCountSquared / totalCount / totalCount;
	std::cout << "A: " << A << std::endl;
	std::cout << "D: " << D << std::endl;
	std::cout << "B: " << A * D << std::endl;

	std::cout << "Max: " << maxB << " = " << mA << " * " << mD << " (" << circleCountAtMax << " circles)" << std::endl;
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

std::shared_ptr<PossibleCircle> Solver::getCircleFromConnection(std::shared_ptr<Connection> conn, double r) {
	if (conn->type == ConnType::CIRCLE) {
		return getCircleCircleLeft(conn->c1, conn->c2, r);
	} else if (conn->type == ConnType::WALL) {
		return circleFromWall(conn, true, r);
	} else  if (conn->type == ConnType::CORNER) {
		return circlFromCorner(conn->corner, r);
	}
	return nullptr;
}

/*
* Finds all possible next circles and returns the first.
*/
std::shared_ptr<PossibleCircle> Solver::getNextCircle(CircleType& t) {
	auto calcIt = std::lower_bound(conns_calculated.begin(), conns_calculated.end(), t.r, [](const std::shared_ptr<Connection>& a, double r) {
		return a->maxRadius < r;
		});
	// perfect matching Connection was already calculated?
	if (calcIt != conns_calculated.end() && (*calcIt)->maxRadius == t.r) {
		if ((*calcIt)->maxRadius >= t.r) {
			return getCircleFromConnection(*calcIt, t.r);
		}
	}

	// calculate until perfect match found
	for (auto it = conns_unknown.rbegin(); it != conns_unknown.rend(); ++it) {
		auto conn = *it;
		if (conn->type == ConnType::CIRCLE) {
			calcMaxRadiusConnectionCircle(conn);
		} else if (conn->type == ConnType::WALL) {
			calcMaxRadiusConnectionWall(conn);
		} else  if (conn->type == ConnType::CORNER) {
			calcMaxRadiusConnectionCorner(conn);
		}
		// add to calculated if maxRadius > 0 (if not it will get deleted with the call of erase or clear)
		if (conn->maxRadius > 0) {
			conns_calculated.push_back(conn);
		}
		//found perfect match?
		if (conn->maxRadius == t.r) {
			conns_unknown.erase(std::next(it).base(), conns_unknown.end());
			return getCircleFromConnection(conn, t.r);
		}
	}
	// calculated all
	conns_unknown.clear();

	// no perfect match => find next best
	auto nextBest = std::lower_bound(conns_calculated.begin(), conns_calculated.end(), t.r, [](const std::shared_ptr<Connection>& conn, double r) {
		return conn->maxRadius < r;
	});

	if (nextBest == conns_calculated.end()) {
		return nullptr;
	}
	return getCircleFromConnection(*nextBest, t.r);
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
#ifdef BINARY_SEARCH_RADIUS
	auto it = std::lower_bound(types.begin(), types.end(), conn, [&](const CircleType& t, const std::shared_ptr<Connection>& conn) {
		double r = t.r;
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

		return !checkValid(cx, cy, r);
	});

	if (it == types.end()) conn->maxRadiusLeft = 0.;
	else conn->maxRadiusLeft = it->r;

#else
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
	if (i == types.size() - 1) conn->maxRadius = 0.;
	else conn->maxRadius = types[i + 1].r;

#endif
}

void Solver::calcMaxRadiusConnectionWall(std::shared_ptr<Connection> conn) {
#ifdef BINARY_SEARCH_RADIUS
	auto it = std::lower_bound(types.begin(), types.end(), conn, [&](const CircleType& t, const std::shared_ptr<Connection>& conn) {
		auto& c = conn->c1;
		double r = t.r;
		double cx, cy;
		double wd = 2 * std::sqrt(c->r * r) * (conn->left ? 1 : -1);
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
		return !checkValid(cx, cy, r);
	});
	if (it == types.end()) conn->maxRadius = 0.;
	else conn->maxRadius = it->r;

#else
	auto& c = conn->c1;
	int i = (int)types.size() - 1;
	while (i >= 0) {
		double r = types[i].r;
		double cx, cy;
		double wd = 2 * std::sqrt(c->r * r) * (conn->left ? 1 : -1);
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
		conn->maxRadius = 0;
	} else {
		conn->maxRadius = types[i + 1].r;
	}
#endif
}

void Solver::calcMaxRadiusConnectionCircle(std::shared_ptr<Connection> conn) {
#ifdef BINARY_SEARCH_RADIUS
	auto it = std::lower_bound(types.begin(), types.end(), conn, [&](const CircleType& t, const std::shared_ptr<Connection>& conn) {
		auto& c = conn->c1;
		double r = t.r;
		Point n = intersectionTwoCircles(c->cx, c->cy, c->r + r, conn->c2->cx, conn->c2->cy, conn->c2->r + r);
		return !checkValid(n.x, n.y, r);
	});
	if (it == types.end()) conn->maxRadiusLeft = 0.;
	else conn->maxRadiusLeft = it->r;

	it = std::lower_bound(types.begin(), types.end(), conn, [&](const CircleType& t, const std::shared_ptr<Connection>& conn) {
		auto& c = conn->c1;
		double r = t.r;
		Point n = intersectionTwoCircles(conn->c2->cx, conn->c2->cy, conn->c2->r + r, c->cx, c->cy, c->r + r);
		return !checkValid(n.x, n.y, r);
	});
	if (it == types.end()) conn->maxRadiusRight = 0.;
	else conn->maxRadiusRight = it->r;

#else
	std::shared_ptr<Circle> c1 = conn->c1;
	std::shared_ptr<Circle> c2 = conn->c2;
	if (!conn->left) {
		c1 = conn->c2;
		c2 = conn->c1;
	}

	int i = (int)types.size() - 1;
	while (i >= 0) {
		double r = types[i].r;
		Point n = intersectionTwoCircles(c1->cx, c1->cy, c1->r + r, c2->cx, c2->cy, c2->r + r);
		if (!checkValid(n.x, n.y, r)) {
			break;
		}
		i--;
	}
	if (i == types.size() - 1) conn->maxRadius = 0;
	else conn->maxRadius = types[i + 1].r;
#endif
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
	std::vector<std::shared_ptr<Connection>> conns = std::vector<std::shared_ptr<Connection>>();
	conns.emplace_back(Connection::create(c, conn->c1, true));
	conns.emplace_back(Connection::create(c, conn->c1, false));
	conns.emplace_back(Connection::create(c, conn->wall, true));
	conns.emplace_back(Connection::create(c, conn->wall, false));
	return PossibleCircle::create(c, conns);
}

std::shared_ptr<PossibleCircle> Solver::getCircleCircleLeft(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r) {
	auto n = circleFromTwoCircles(c1, c2, r);
	std::vector<std::shared_ptr<Connection>> conns = std::vector<std::shared_ptr<Connection>>();
	conns.emplace_back(Connection::create(n, c1, true));
	conns.emplace_back(Connection::create(n, c1, false));
	conns.emplace_back(Connection::create(n, c2, true));
	conns.emplace_back(Connection::create(n, c2, false));
	return PossibleCircle::create(n, conns);
}

std::shared_ptr<PossibleCircle> Solver::getCircleCircleRight(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r) {
	auto n = circleFromTwoCircles(c2, c1, r);
	std::vector<std::shared_ptr<Connection>> conns = std::vector<std::shared_ptr<Connection>>();
	conns.emplace_back(Connection::create(n, c1, true));
	conns.emplace_back(Connection::create(n, c1, false));
	conns.emplace_back(Connection::create(n, c2, true));
	conns.emplace_back(Connection::create(n, c2, false));
	return PossibleCircle::create(n, conns);
}

std::shared_ptr<PossibleCircle> Solver::circlFromCorner(Corner corner, double r) {
	std::shared_ptr<PossibleCircle> pc;
	std::shared_ptr<Circle> c;
	std::vector<std::shared_ptr<Connection>> conns = std::vector<std::shared_ptr<Connection>>();
	if (corner == Corner::TL) {
		c = Circle::create(r, r, r);
		conns.push_back(Connection::create(c, Wall::LEFT, true));
		conns.push_back(Connection::create(c, Wall::LEFT, false));
		conns.push_back(Connection::create(c, Wall::UP, true));
		conns.push_back(Connection::create(c, Wall::UP, false));
	} else if (corner == Corner::TR) {
		c = Circle::create(w - r, r, r);
		conns.push_back(Connection::create(c, Wall::RIGHT, true));
		conns.push_back(Connection::create(c, Wall::RIGHT, false));
		conns.push_back(Connection::create(c, Wall::UP, true));
		conns.push_back(Connection::create(c, Wall::UP, false));
	} else if (corner == Corner::BL) {
		c = Circle::create(r, h - r, r);
		conns.push_back(Connection::create(c, Wall::LEFT, true));
		conns.push_back(Connection::create(c, Wall::LEFT, false));
		conns.push_back(Connection::create(c, Wall::DOWN, true));
		conns.push_back(Connection::create(c, Wall::DOWN, false));
	} else if (corner == Corner::BR) {
		c = Circle::create(w - r, h - r, r);
		conns.push_back(Connection::create(c, Wall::RIGHT, true));
		conns.push_back(Connection::create(c, Wall::RIGHT, false));
		conns.push_back(Connection::create(c, Wall::DOWN, true));
		conns.push_back(Connection::create(c, Wall::DOWN, false));
	}
	return PossibleCircle::create(c, conns);
}
