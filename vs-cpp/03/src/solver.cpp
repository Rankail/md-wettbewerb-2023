#include "solver.h"

#include "utils.h"

Solver::Solver(const std::string& file) {
	loaded = readInput(file);
}

Solver::~Solver() {
}

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
	numBlocks = 0;
	for (CircleType& t : types) {
		t.sizeMultiplier = t.r / min;
		numBlocks += t.r * t.r * PI;
	}

	return true;
}

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

void Solver::run() {
	int cur = 0;
	
	if (!loaded) return;
	int nextIdx = getNextType();

	circles = std::vector<std::shared_ptr<Circle>>();
	CircleType& t = types[nextIdx];
	std::cout << t << std::endl;
	circles.push_back(Circle::create(t.r, t.r, t.r));
	circles[0]->conns.push_back(Connection(Wall::LEFT));
	circles[0]->conns.push_back(Connection(Wall::UP));
	circles[0]->typeIndex = t.index;
	t.count++;

	while (true) {
		int nextIdx = getNextType();
		t = types[nextIdx];

		std::shared_ptr<Circle> c = getNextCircle(t);
		if (c == nullptr) break;;
		c->typeIndex = t.index;
		std::cout << c << circles.size() << std::endl;
		circles.push_back(c);
		types[nextIdx].count++;
	}

	std::cout << "Result:\n";
	for (auto& c : circles) {
		std::cout << c << std::endl;
	}
}

int Solver::getNextType() {
	int idx = -1;
	double maxWeight = 0.;
	for (int i = 0; i < types.size(); i++) {
		double weight = types[i].sizeMultiplier * types[i].sizeMultiplier * (1. - types[i].count / numBlocks);
		if (weight > maxWeight) {
			maxWeight = weight;
			idx = i;
		}
	}
	if (idx == -1) {
		printf("Failed to find next type\n");
		return -1;
	}
	return idx;
}

std::shared_ptr<Circle> Solver::getNextCircle(CircleType& t) {
	std::vector<Circle> possible = std::vector<Circle>();
	for (auto& circle : circles) {
		double wd = 2 * sqrt(t.r * circle->r);
		Circle c{0,0,0};
		for (auto& conn : circle->conns) {
			if (conn.type == ConnType::WALL) {
				if (conn.wall == Wall::LEFT) {
					c = Circle{t.r, circle->cy - wd, t.r};
					c.conns.push_back(Connection(circle));
					c.conns.push_back(Connection(Wall::LEFT));
					possible.push_back(c);
					c = Circle{t.r, circle->cy + wd, t.r};
					c.conns.push_back(Connection(circle));
					c.conns.push_back(Connection(Wall::LEFT));
					possible.push_back(c);
				} else if (conn.wall == Wall::UP) {
					c = Circle{circle->cx - wd, t.r,t.r};
					c.conns.push_back(Connection(circle));
					c.conns.push_back(Connection(Wall::UP));
					possible.push_back(c);
					c = Circle{circle->cx + wd, t.r, t.r};
					c.conns.push_back(Connection(circle));
					c.conns.push_back(Connection(Wall::UP));
					possible.push_back(c);
				} else if (conn.wall == Wall::RIGHT) {
					c = Circle{w - t.r, circle->cy - wd, t.r};
					c.conns.push_back(Connection(circle));
					c.conns.push_back(Connection(Wall::RIGHT));
					possible.push_back(c);
					c = Circle{w - t.r, circle->cy + wd, t.r};
					c.conns.push_back(Connection(circle));
					c.conns.push_back(Connection(Wall::RIGHT));
					possible.push_back(c);
				} else if (conn.wall == Wall::DOWN) {
					c = Circle{circle->cx - wd, h - t.r, t.r};
					c.conns.push_back(Connection(circle));
					c.conns.push_back(Connection(Wall::DOWN));
					possible.push_back(c);
					c = Circle{circle->cx + wd, h - t.r, t.r};
					c.conns.push_back(Connection(circle));
					c.conns.push_back(Connection(Wall::DOWN));
					possible.push_back(c);
				}
			}
		}
	}

	std::cout << "###################################################\n";
	possible.erase(std::remove_if(possible.begin(), possible.end(), [&](const Circle& c) {
		if (c.cx < t.r) return true;
		if (c.cy < t.r) return true;
		if (c.cx > w - t.r) return true;
		if (c.cy > h - t.r) return true;

		for (auto& c2 : circles) {
			if ((c.cx - c2->cx) * (c.cx - c2->cx) + (c.cy - c2->cy) * (c.cy - c2->cy) < (c.r + c2->r) * (c.r + c2->r)) {
				return true;
			}
		}
		std::cout << c << std::endl;
		return false;
	}), possible.end());


	std::cout << "--------------------------------------------\n";
	for (auto& c : possible) {
		std::cout << c << std::endl;
	}

	if (possible.empty()) {
		std::cout << "empty\n";
		return nullptr;
	}

	return std::make_shared<Circle>(possible[0]);
}

bool Solver::checkPos(const Circle& c) {
	if (c.cx < c.r) return false;
	if (c.cy < c.r) return false;
	if (c.cx > w - c.r) return false;
	if (c.cy > h - c.r) return false;
	return true;
}
