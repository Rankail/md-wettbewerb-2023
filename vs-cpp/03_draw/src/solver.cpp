#include "solver.h"

#include "utils.h"

Solver::Solver(const std::string& file) {
	loaded = readInput(file);

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Failed to initialize SDL! Error: %s\n", SDL_GetError());
		return;
	}

	window = SDL_CreateWindow("Draw Circles", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (int)w, (int)h, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Failed to create SDL_Window! Error: %s\n", SDL_GetError());
		return;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		printf("Failed to create SDL_Renderer! Error: %s\n", SDL_GetError());
		return;
	}
}

Solver::~Solver() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
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

	double max = types[0].r;
	double block = 0;
	for (CircleType& t : types) {
		t.sizeMultiplier = t.r / max;
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

	connections = std::vector<std::shared_ptr<Connection>>();
	connections.push_back(Connection::create(Corner::TL));
	connections.push_back(Connection::create(Corner::TR));
	connections.push_back(Connection::create(Corner::BL));
	connections.push_back(Connection::create(Corner::BR));
	circles = std::vector<std::shared_ptr<Circle>>();
	calcMaxRadius();

	int i = 0;
	while (i < 1000) {

		stepWeights();
		for (auto& type : types) {
			if (connections.empty()) goto finished;
			if (type.weight < 1.) continue;
			type.weight--;

			std::shared_ptr<Circle> c = getNextCircle(type);
			if (c == nullptr) continue;
			c->typeIndex = type.index;
			for (auto& conn : c->conns) {
				connections.push_back(conn);
			}
			circles.push_back(c);
			calcMaxRadius();
			
			//std::cout << c << circles.size() << std::endl;
			type.count++;
		}
		i++;
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
	size *= M_PI;
	std::cout << "Result:\n";
	/*for (auto& c : circles) {
		std::cout << c << std::endl;
	}*/
	double A = size / (w * h);
	double D = 1. - sumCountSquared / totalCount / totalCount;
	std::cout << "size: " << A << std::endl;
	std::cout << "Divers: " << D << std::endl;
	std::cout << "B: " << A * D << std::endl;

	render();
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
std::shared_ptr<Circle> Solver::getNextCircle(CircleType& t) {
	auto possible = getAllPossible(t.r);

	//std::cout << "--------------------------------------------\n";

	std::sort(possible.begin(), possible.end(), [](const std::pair<double, std::shared_ptr<Circle>>& a, const std::pair<double, std::shared_ptr<Circle>>& b) {
		return a.first < b.first;
	});
	/*for (auto& c : possible) {
		std::cout << c.second << std::endl;
	}
	std::cout << "############################################\n";*/
	if (possible.empty()) return nullptr;

	return possible[0].second;
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

	/*std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	for (auto& conn : connections) {
		std::cout << *conn << std::endl;
	}
	std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";*/

	if (!connections.empty()) {
		connections.erase(std::remove_if(connections.begin(), connections.end(), [&](const std::shared_ptr<Connection>& conn) {
			return conn->maxRadiusLeft == 0 && conn->maxRadiusRight == 0;
		}), connections.end());
	}
}

/*
* searches for possible Positions around circle (no checks needed because of max-circle
*/
std::vector<std::pair<double, std::shared_ptr<Circle>>> Solver::getAllPossible(double r) {
	auto possible = std::vector<std::pair<double, std::shared_ptr<Circle>>>();
	for (auto& conn : connections) {
		if (conn->maxRadiusLeft >= r) {
			if (conn->type == ConnType::CIRCLE) {
				possible.push_back(std::make_pair(conn->maxRadiusLeft, getCircleCircleLeft(conn->c1, conn->c2, r)));
			} else if (conn->type == ConnType::WALL) {
				possible.push_back(std::make_pair(conn->maxRadiusLeft, circleFromWall(conn, true, r)));
			} else  if (conn->type == ConnType::CORNER) {
				possible.push_back(std::make_pair(conn->maxRadiusLeft, circlFromCorner(conn->corner, r)));
			}
		}
		if (conn->maxRadiusRight >= r) {
			if (conn->type == ConnType::CIRCLE) {
				possible.push_back(std::make_pair(conn->maxRadiusRight, getCircleCircleRight(conn->c1, conn->c2, r)));
			} else if (conn->type == ConnType::WALL) {
				possible.push_back(std::make_pair(conn->maxRadiusRight, circleFromWall(conn, false, r)));
			}
		}
	}

	return possible;
}

std::shared_ptr<Circle> Solver::circleFromWall(std::shared_ptr<Connection> conn, bool left, double r) {
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
	c->conns.push_back(Connection::create(c, conn->c1));
	c->conns.push_back(Connection::create(c, conn->wall));
	return c;
}

std::shared_ptr<Circle> Solver::getCircleCircleLeft(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r) {
	auto n = circleFromTwoCircles(c1, c2, r);
	n->conns.push_back(Connection::create(n, c1));
	n->conns.push_back(Connection::create(n, c2));
	return n;
}

std::shared_ptr<Circle> Solver::getCircleCircleRight(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r) {
	auto n = circleFromTwoCircles(c2, c1, r);
	n->conns.push_back(Connection::create(n, c1));
	n->conns.push_back(Connection::create(n, c2));
	return n;
}

std::shared_ptr<Circle> Solver::circlFromCorner(Corner corner, double r) {
	std::shared_ptr<Circle> c = nullptr;
	if (corner == Corner::TL) {
		c = Circle::create(r, r, r);
		c->conns.push_back(Connection::create(c, Wall::LEFT));
		c->conns.push_back(Connection::create(c, Wall::UP));
	} else if (corner == Corner::TR) {
		c = Circle::create(w - r, r, r);
		c->conns.push_back(Connection::create(c, Wall::RIGHT));
		c->conns.push_back(Connection::create(c, Wall::UP));
	} else if (corner == Corner::BL) {
		c = Circle::create(r, h - r, r);
		c->conns.push_back(Connection::create(c, Wall::LEFT));
		c->conns.push_back(Connection::create(c, Wall::DOWN));
	} else if (corner == Corner::BR) {
		c = Circle::create(w - r, h - r, r);
		c->conns.push_back(Connection::create(c, Wall::RIGHT));
		c->conns.push_back(Connection::create(c, Wall::DOWN));
	}
	return c;
}

void Solver::render() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
	for (auto& circle : circles) {
		drawCircle(renderer, circle);
	}

	SDL_RenderPresent(renderer);

	bool c = false;
	while (!c) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) c = true;
			if (e.type == SDL_KEYUP && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) c = true;
		}
	}
}
