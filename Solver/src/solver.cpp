#include "solver.h"

#include "utils.h"

/*
Initialize SDL
*/
Solver::Solver()
	: loaded(false), w(0.), h(0.), weighting(0.) {
#ifdef DRAW_SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "Failed to initialize SDL! Error: " << SDL_GetError() << std::endl;
		return;
	}

	window = SDL_CreateWindow("Circles", 0, 30, 600, 600, NULL);
	if (window == NULL) {
		std::cout << "Failed to create SDL_Window! Error: " << SDL_GetError() << std::endl;
		return;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		std::cout << "Failed to create SDL_Renderer! Error: " << SDL_GetError() << std::endl;
		return;
	}
#endif
}

/*
Destroy SDL-Instance
*/
Solver::~Solver() {
#ifdef DRAW_SDL
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
#endif
}

/*
Initialize Solver with data from inputfile
*/
bool Solver::init(const std::string& inputfile)
{
	loaded = readInput(inputfile);
	if (!loaded) return false;

#ifdef DRAW_SDL
	if (w > 1000 || h > 1000) {
		scale = std::max(w, h) / 1000.;
	} else {
		scale = 1.;
	}

	SDL_SetWindowSize(window, (int)w, (int)h);
	SDL_ShowWindow(window);
#endif

	reset();

	return true;
}

/*
Reset data for computation
*/
void Solver::reset() {
	for (auto& type : types) {
		type.count = 0;
	}

	conns_unknown = std::vector<std::shared_ptr<Connection>>();
	conns_calculated = std::vector<std::shared_ptr<Connection>>();

	conns_unknown.push_back(Connection::create(Corner::TL));
	conns_unknown.push_back(Connection::create(Corner::TR));
	conns_unknown.push_back(Connection::create(Corner::BL));
	conns_unknown.push_back(Connection::create(Corner::BR));
	circles = std::vector<std::shared_ptr<Circle>>();
}

/*
Read input from file
*/
bool Solver::readInput(const std::string& path) {
	std::ifstream file;
	file.open(path, std::ios::in);
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
		types.emplace_back(i, std::stod(line.substr(0, space)));
		i++;
	}

	std::sort(types.begin(), types.end(), [](const CircleType& lhs, const CircleType& rhs) {
		return lhs.r > rhs.r;
	});

	double max = types[0].r;
	double r = 0;
	radii = std::vector<double>();
	for (CircleType& t : types) {
		if (r != t.r) {
			radii.push_back(t.r);
		}
		r = t.r;
	}

	// Used for deduplication during max-radius-test
	radiusMap = std::unordered_map<double, int>();
	for (auto i = 0; i < radii.size(); i++) {
		if (radiusMap.find(radii[i]) == radiusMap.end()) {
			radiusMap[radii[i]] = i;
		}
	}
	radiusMap[0.] = 0;

	return true;
}

/*
Write Result-object to outputfile
*/
bool Solver::writeOutput(Result& result, const std::string& outputfile) {
	std::ofstream file;
	file.open(outputfile, std::ios::out);
	if (!file.is_open()) {
		std::cout << "Failed to open output-file.\n" << std::endl;
		return false;
	}
	std::cout << "Writing to '" << outputfile << "'" << std::endl;

	for (int i = 0; i < result.circleCountAtMax; i++) {
		auto& c = result.circles[i];
		file << std::setprecision(std::numeric_limits<double>::digits10) << c->cx << " " << c->cy << " " << c->r << " " << c->typeIndex << "\n";
	}
	file.close();
	return true;
}

/*
Run algorithm
*/
Result Solver::run(double weighting, unsigned seed) {
	if (weighting > 2. || 0 > weighting) {
		std::cout << "Weightening must be between 0 and 2" << std::endl;
		loaded = false;
		return Result();
	}
	this->weighting = weighting;

	std::srand(seed);

	if (!loaded) {
		std::cout << "Could not run because the last Initialization failed" << std::endl;
		return Result();
	};

	double size = 0.;
	double maxB = 0.;
	double maxA = 0.;
	double maxD = 0.;
	circleCountAtMax = 0;

	double lastMax = 0.;
	int sameFor = 0;
	while (true) {

		stepWeights();
		for (auto& type : types) {
			if (conns_unknown.empty() && conns_calculated.empty()) goto finished;
			if (type.weight < 1.) continue;
			type.weight--;

			std::shared_ptr<PossibleCircle> pc = getNextCircle(type);
			if (pc == nullptr) continue;
			std::shared_ptr<Circle> circle = pc->circle;
			circle->index = std::rand();

			updateConnections(circle);

			for (auto& conn : pc->conns) {
				conns_unknown.push_back(conn);
			}

			// sort calculated connections for faster finding
			std::sort(conns_calculated.begin(), conns_calculated.end(), [](const std::shared_ptr<Connection>& a, const std::shared_ptr<Connection>& b) {
				if (a->maxRadius != b->maxRadius) return a->maxRadius < b->maxRadius;
				if (a->type != b->type) return a->type < b->type;
				if (a->type == ConnType::CORNER) return false;
				return a->c1->index < b->c1->index; // only place that the randomizer affects
			});

			circles.push_back(circle);

			circle->typeIndex = type.index;
			type.count++;

			// calculate stats to find maximum
			// should have deleted most common circle after finishing instead
			size += circle->r * circle->r * PI;
			double sumCountSquared = 0.;
			for (auto& t : types) {
				sumCountSquared += (double)t.count * (double)t.count;
			}
			double A = size / (w * h);
			double D = 1. - sumCountSquared / (circles.size() * circles.size());
			double B = A * D;

			if (B > maxB) {
				maxA = A;
				maxD = D;
				maxB = B;
				circleCountAtMax = (int)circles.size();
			}

			// end algorithm if no new maximum after 3000 new circles
			if (circles.size() % 1000 == 0) {
				if (lastMax == maxB) sameFor++;
				else sameFor = 0;
				lastMax = maxB;
				std::cout << "Max: " << maxB << " = " << maxA << " * " << maxD << " at "
					<< circleCountAtMax << " circles; Current: " << circles.size() << " circles B=" << B << std::endl;
				if (sameFor > 1) goto finished;
			}
		}
		render();
	}
finished:

	std::cout << "Result:\n";

	std::cout << "Max: " << maxB << " = " << maxA << " * " << maxD << " (" << circleCountAtMax << " circles)" << std::endl;
	std::cout << "C: " << maxB * types.size() / (types.size() - 1) << std::endl;

#ifdef DRAW_SDL
	bool c = false;
	while (!c) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) c = true;
			if (e.type == SDL_KEYUP && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) c = true;
		}
		render();
	}
#endif

	return Result(circles, maxA, maxD, maxB, circleCountAtMax);
}

/*
Calculate weight for every circletype
*/
void Solver::stepWeights() {
	double maxWeight = 0.;
	std::vector<double> weights = std::vector<double>();
	for (int i = 0; i < types.size(); i++) {
		double weight;
		// interpolate somewhat smoothly between 1, r, r^2
		if (weighting <= 1.) weight = (weighting * weighting * types[i].r) + (1. - weighting * weighting);
		else weight = types[i].r * (std::pow(weighting-1., 5.) * types[i].r + (1. - std::pow(weighting-1., 5.)));
		weights.push_back(weight);
		maxWeight = std::max(maxWeight, weight);
	}

	// normalize weights so maximum is 1
	for (int i = 0; i < types.size(); i++) {
		types[i].weight += weights[i] / maxWeight;
	}
}

/*
Mark connections as unkown if they are possibly colliding with the newly placed circle
*/
void Solver::updateConnections(const std::shared_ptr<Circle>& circle) {
	auto partition = std::stable_partition(conns_calculated.begin(), conns_calculated.end(), [&](const std::shared_ptr<Connection>& conn) {
			double dx = 0., dy = 0., r = 0.;
		if (conn->type == ConnType::CORNER) {
			r = conn->maxRadius * 2 + circle->r;
			switch (conn->corner) {
			case Corner::TL: {
				dx = std::abs(circle->cx - conn->maxRadius);
				dy = std::abs(circle->cy - conn->maxRadius);
				break;
			}
			case Corner::TR: {
				dx = std::abs(circle->cx - (w - conn->maxRadius));
				dy = std::abs(circle->cy - conn->maxRadius);
				break;
			}
			case Corner::BL: {
				dx = std::abs(circle->cx - conn->maxRadius);
				dy = std::abs(circle->cy - (h - conn->maxRadius));
				break;
			}
			case Corner::BR: {
				dx = std::abs(circle->cx - (w - conn->maxRadius));
				dy = std::abs(circle->cy - (h - conn->maxRadius));
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
}

/*
Try to find a good position for a circle of the provided type
*/
std::shared_ptr<PossibleCircle> Solver::getNextCircle(CircleType& t) {
	auto calcIt = std::lower_bound(conns_calculated.begin(), conns_calculated.end(), t.r, [](const std::shared_ptr<Connection>& a, double r) {
		return a->maxRadius < r;
	});
	// Connection with max-radius equal to radius of provided type was already calculated?
	if (calcIt != conns_calculated.end() && (*calcIt)->maxRadius == t.r) {
		if ((*calcIt)->maxRadius >= t.r) {
			return getCircleFromConnection(*calcIt, t.r);
		}
	}

	// calculate until good connection found
	for (auto it = conns_unknown.rbegin(); it != conns_unknown.rend(); ++it) {
		auto& conn = *it;
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
		// found perfect match?
		if (conn->maxRadius == t.r) {
			conns_unknown.erase(std::next(it).base(), conns_unknown.end());
			return getCircleFromConnection(conn, t.r);
		}
	}
	// calculated all connections
	conns_unknown.clear();

	// no good connection => find next best
	auto nextBest = std::lower_bound(conns_calculated.begin(), conns_calculated.end(), t.r, [](const std::shared_ptr<Connection>& conn, double r) {
		return conn->maxRadius < r;
	});

	if (nextBest == conns_calculated.end()) {
		return nullptr;
	}
	return getCircleFromConnection(*nextBest, t.r);
}

/*
Check if circle collides
*/
bool Solver::checkValid(double cx, double cy, double r) {
	if (cx < r) return false;
	if (cy < r) return false;
	if (cx + r > w) return false;
	if (cy + r > h) return false;

	for (auto& c : circles) {
		if ((c->cx - cx) * (c->cx - cx) + (c->cy - cy) * (c->cy - cy) < (r + c->r) * (c->r + r) - 0.0000000001) return false;
	}
	return true;
}

/*
Calculate the max-radius for a corner-connection
*/
void Solver::calcMaxRadiusConnectionCorner(std::shared_ptr<Connection> conn) {
	int i = (int)radii.size() - 1;

	// only test up to current max-radius
	// i thought a smaller circle not fitting would mean a bigger on would not fit either. I was wrong.
	while (i >= radiusMap[conn->maxRadius]) {
		double r = radii[i];
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
	if (i == radii.size() - 1) conn->maxRadius = 0.;
	else conn->maxRadius = radii[i + 1];
}

/*
Calculate the max-radius for a wall-circle-connection
*/
void Solver::calcMaxRadiusConnectionWall(std::shared_ptr<Connection> conn) {
	auto& c = conn->c1;
	int i = (int)radii.size() - 1;

	while (i >= radiusMap[conn->maxRadius]) {
		double r = radii[i];
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
	if (i == radii.size() - 1) conn->maxRadius = 0;
	else conn->maxRadius = radii[i + 1];
}

/*
Calculate the max-radius for a circle-circle-connection
*/
void Solver::calcMaxRadiusConnectionCircle(std::shared_ptr<Connection> conn) {
	std::shared_ptr<Circle> c1 = conn->c1;
	std::shared_ptr<Circle> c2 = conn->c2;
	if (!conn->left) {
		c1 = conn->c2;
		c2 = conn->c1;
	}

	int i = (int)radii.size() - 1;
	while (i >= radiusMap[conn->maxRadius]) {
		double r = radii[i];
		Point n = intersectionTwoCircles(c1->cx, c1->cy, c1->r + r, c2->cx, c2->cy, c2->r + r);
		if (!checkValid(n.x, n.y, r)) {
			break;
		}
		i--;
	}
	if (i == radii.size() - 1) conn->maxRadius = 0;
	else conn->maxRadius = radii[i + 1];
}

/*
Construct a circle and its connections from a connection
*/
std::shared_ptr<PossibleCircle> Solver::getCircleFromConnection(std::shared_ptr<Connection> conn, double r) {
	if (conn->type == ConnType::CIRCLE) {
		return getCircleFromCircle(conn->c1, conn->c2, r, conn->left);
	} else if (conn->type == ConnType::WALL) {
		return getCircleFromWall(conn, r);
	} else  if (conn->type == ConnType::CORNER) {
		return getCirclFromCorner(conn->corner, r);
	}
	return nullptr;
}

/*
Construct a circle from a corner-connection
*/
std::shared_ptr<PossibleCircle> Solver::getCirclFromCorner(Corner corner, double r) {
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

/*
Construct a circle from a wall-circle-connection
*/
std::shared_ptr<PossibleCircle> Solver::getCircleFromWall(std::shared_ptr<Connection> conn, double r) {
	double wd = 2 * std::sqrt(conn->c1->r * r) * (conn->left ? 1 : -1);
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

/*
Construct a circle from a circle-circle-connection
*/
std::shared_ptr<PossibleCircle> Solver::getCircleFromCircle(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r, bool left) {
	std::shared_ptr<Circle> n;
	if (left) n = circleFromTwoCircles(c1, c2, r);
	else n = circleFromTwoCircles(c2, c1, r);
	std::vector<std::shared_ptr<Connection>> conns = std::vector<std::shared_ptr<Connection>>();
	conns.emplace_back(Connection::create(n, c1, true));
	conns.emplace_back(Connection::create(n, c1, false));
	conns.emplace_back(Connection::create(n, c2, true));
	conns.emplace_back(Connection::create(n, c2, false));
	return PossibleCircle::create(n, conns);
}

/*
Render the computed circles
*/
void Solver::render() {
#ifdef DRAW_SDL
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
	for (auto& circle : circles) {
		drawCircle(renderer, circle, scale);
	}

	SDL_RenderPresent(renderer);

	/*bool c = false; //Uncomment to pause after every render
	while (!c) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) c = true;
			if (e.type == SDL_KEYUP && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) c = true;
		}
	}*/
#endif
}
