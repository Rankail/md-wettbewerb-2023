#include "solver.h"

#include "utils.h"

/*
* Plan:
* Connections: map<eadius, PossiblCircle+score>
* Not calcultaed -> no key
* Needs recalculation -> score == -1
* Not placable -> score == 0
* 
* Placing:
* Break-score (as cmd argument?)
* Break-score can't have been calculated (would have been selected instantly
* For all connections:
* score == 0 -> skip
* no key -> caclulate full
* score == -1 -> recalculate
* if score >= break-score -> return that
* after calculating all -> take best
* 
* Clearing after palcing:
* Circle in range -> score == -1
* else any conn in range -> score == -1
* 
* Calculating:
* Create circle of radius with conn -> create all subconns and save in map -> calc score
* -> score == 0; zeroCount == 4 -> delete conn (only with new calc; needs to really check all on recalc 0)
* 
* Recalculating (score == -1):
* circle collides? -> remove radius
* else -> calc "maxRadius?" for all subconns
* 
* 
* Dynamic-Break-Score:
* - based on radius?
* 
*/

/*
* Creates Solver Object and initializes SDL
*/
Solver::Solver() {
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
* Destroys Solver Object and SDL-Instance
*/
Solver::~Solver() {
#ifdef DRAW_SDL
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
#endif
}

/*
* Initializes Solver with Data from the specified inputfile
*/
bool Solver::init(const std::string& inputfile, double weighting)
{
	loaded = readInput(inputfile);
	if (!loaded) return false;

	if (weighting > 2. || 0 > weighting) {
		std::cout << "Weightening must be between 0 and 2" << std::endl;
		loaded = false;
		return false;
	}
	this->weighting = weighting;

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
* Resets the data for computation
*/
void Solver::reset() {
	for (auto& type : types) {
		type.count = 0;
	}

	conns_remove = std::vector<std::shared_ptr<Connection>>();
	conns = std::vector<std::shared_ptr<Connection>>();

	conns.push_back(Connection::create(Corner::TL));
	conns.push_back(Connection::create(Corner::TR));
	conns.push_back(Connection::create(Corner::BL));
	conns.push_back(Connection::create(Corner::BR));
	circles = std::vector<std::shared_ptr<Circle>>();
}

/*
* Reads input from file and calculates some basic data
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
	double block = 0;
	double r = 0;
	radii = std::vector<double>();
	for (CircleType& t : types) {
		if (r != t.r) {
			radii.push_back(t.r);
		}
		r = t.r;
		t.sizeMultiplier = t.r / max;
		block += t.r * t.r * PI;
	}
	numBlocks = block * PI / w * h;

	// Used for deduplication during max-radius-test
	radiusMap = std::unordered_map<double, int>();
	for (auto i = 0; i < radii.size(); i++) {
		if (radiusMap.find(radii[i]) == radiusMap.end()) {
			radiusMap[radii[i]] = i;
		}
	}
	radiusMap[0.] = radii.size();

	return true;
}

/*
* takes a Result-Object and writes it to the outputfile
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
		file << std::setprecision(std::numeric_limits<double>::digits10 + 2) << c->cx << " " << c->cy << " " << c->r << " " << c->typeIndex << "\n";
	}
	file.close();
	return true;
}

/*
* runs the algorithm
*/
Result Solver::run() {

	if (!loaded) {
		std::cout << "Could not run because the last Initialization failed" << std::endl;
		return Result();
	};

	double size = 0.;
	double maxB = 0.;
	double maxA = 0.;
	double maxD = 0.;
	circleCountAtMax = 0;

	int iteration = 0;
	double lastMax = 0.;
	int sameFor = 0;
	while (true) {

		stepWeights();
		for (auto& type : types) {
			if (conns.empty()) goto finished;
			if (type.weight < 1.) continue;
			type.weight--;

			auto pc = getNextCircle(type);
			if (pc == nullptr) continue;
			std::shared_ptr<Circle> circle = pc->circle;

			updateConnections(circle);

			for (auto& conn : pc->conns) {
				conns.push_back(conn);
			}

			// sort calculated connections for faster finding
			std::sort(conns.begin(), conns.end(), [](const std::shared_ptr<Connection>& a, const std::shared_ptr<Connection>& b) {
				if (a->maxRadius != b->maxRadius) return a->maxRadius < b->maxRadius;
				if (a->type != b->type) return a->type < b->type;
				if (a->type == ConnType::CORNER) return false;
				return a->c1->index < b->c1->index;
			});

			circles.push_back(circle);

			circle->typeIndex = type.index;
			type.count++;

			// calculate stats to find maximum
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

			if (circles.size() % 1000 == 0) {
				if (lastMax == maxB) sameFor++;
				else sameFor = 0;
				lastMax = maxB;
				std::cout << "Max: " << maxB << " = " << maxA << " * " << maxD << " at "
					<< circleCountAtMax << " circles; Current: " << circles.size() << " circles B=" << B << std::endl;
				if (sameFor > 1) goto finished;
			}

			iteration++;
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
* Calculates weight for every Circletype
*/
void Solver::stepWeights() {
	double maxWeight = 0.;
	for (auto& t : types) {
		double weight;
		if (weighting <= 1.) weight = (weighting * weighting * t.r) + (1. - weighting * weighting);
		else weight = t.r * (std::pow(weighting-1., 5.) * t.r + (1. - std::pow(weighting-1., 5.)));
		t.weight = weight;
		maxWeight = std::max(maxWeight, weight);
	}

	for (auto& t : types) {
		t.weight += t.weight / maxWeight;
	}
}

// marks Connections as unkown if they are possibly clliding with the provided circle
void Solver::updateConnections(const std::shared_ptr<Circle>& circle) {
	for (auto& conn : conns) {
		conn->maxRadius = 0;
		for (auto& [r, fc] : conn->cache) {
			double dx = 0., dy = 0., r = 0.;

			auto& fcir = fc->circle;
			dx = circle->cx - fcir->cx;
			dx = circle->cy - fcir->cy;
			r = circle->r + r;
			if (dx * dx + dy * dy > r * r) {
				fc->score = 0;
				continue;
			}

			conn->maxRadius = std::max(conn->maxRadius, r);

			for (auto& sc : fc->conns) {
				if (sc->type == ConnType::CORNER) {
					r = sc->maxRadius * 2 + circle->r;
					switch (sc->corner) {
						case Corner::TL: {
							dx = std::abs(circle->cx - sc->maxRadius);
							dy = std::abs(circle->cy - sc->maxRadius);
							break;
						}
						case Corner::TR: {
							dx = std::abs(circle->cx - (w - sc->maxRadius));
							dy = std::abs(circle->cy - sc->maxRadius);
							break;
						}
						case Corner::BL: {
							dx = std::abs(circle->cx - sc->maxRadius);
							dy = std::abs(circle->cy - (h - sc->maxRadius));
							break;
						}
						case Corner::BR: {
							dx = std::abs(circle->cx - (w - sc->maxRadius));
							dy = std::abs(circle->cy - (h - sc->maxRadius));
							break;
						}
					}
				} else if (sc->type == ConnType::WALL) {
					r = circle->r + sc->maxRadius * 2 + sc->c1->r;
					dx = std::abs(circle->cx - sc->c1->cx);
					dy = std::abs(circle->cy - sc->c1->cy);
				} else if (sc->type == ConnType::CIRCLE) {
					r = circle->r + sc->maxRadius * 2 + std::max(sc->c1->r, sc->c2->r);
					dx = std::min(std::abs(circle->cx - sc->c1->cx), std::abs(circle->cx - sc->c2->cx));
					dy = std::min(std::abs(circle->cy - sc->c1->cy), std::abs(circle->cy - sc->c2->cy));
				}
				if (dx * dx + dy * dy > r * r) continue;

				conn->cache[r]->score = -1;
			}
		}
	}

	conns.erase(std::remove_if(conns.begin(), conns.end(), [](const std::shared_ptr<Connection>& c) { c->maxRadius == 0; }), conns.end());
}

/*
* tries to find the best position for a Circle of the provided type
*/
std::shared_ptr<ConnectionFuture> Solver::getNextCircle(CircleType& t) {
	const double break_score = std::numeric_limits<double>::max();
	//only needed if break_score is dynamic
	/*for (auto& conn : conns) { 
		if (conn->cache[t.r]->score >= break_score) {
			return conn->cache[t.r];
		}
	}*/

	// calculate until break-score is found
	for (auto it = conns.rbegin(); it != conns.rend(); ++it) {
		auto& conn = *it;
		
		// recalc, calc, skip based on value
		auto fc = conn->cache.find(t.r);
		if (fc == conn->cache.end()) calcRadiiFuture(conn, t.r);
		else if(fc->second->score == -1) recalcRadiiFuture(conn, t.r);
		else if (fc->second->score == 0) continue;

		//found break-Score?
		if (conn->cache[t.r]->score >= break_score) {
			return conn->cache[t.r];
		}
	}


	// no break-score => find next best
	auto nextBest = std::max_element(conns.begin(), conns.end(), [&](const std::shared_ptr<Connection>& conn) {
		return conn->cache[t.r]->score;
	});

	if (nextBest == conns.end()) return nullptr;

	return (*nextBest)->cache[t.r];
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
		if ((c->cx - cx) * (c->cx - cx) + (c->cy - cy) * (c->cy - cy) < (r + c->r) * (c->r + r) - 0.0000000001) return false;
	}
	return true;
}

bool Solver::connInRange(const std::shared_ptr<Circle>& circle, const std::shared_ptr<Connection>& conn) {
	return false;
}

void Solver::calcRadiiFuture(std::shared_ptr<Connection>& conn, double r) {
	auto fc = conn->cache[r];

	auto pc = getCircleFromConnection(conn, r);
	double score = 0;
	fc->circle = pc->circle;
	fc->conns = pc->conns;
	fc->score = 0.;
	for (auto& c : pc->conns) {
		calcMaxRadiusConnection(conn);
		fc->score += c->maxRadius;
	}

	//delete?
	if (fc->score != 0) return;
	for (auto const& [r, sfc] : conn->cache) {
		if (sfc->score != 0) return;
	}
	//delete!
	conn->maxRadius = 0;
}

void Solver::recalcRadiiFuture(std::shared_ptr<Connection>& conn, double r) {
	auto& fc = conn->cache[r];
	if (!checkValid(fc->circle->cx, fc->circle->cy, fc->circle->r)) {
		fc->score = 0;
	} else {
		fc->score = 0;
		for (auto& c : fc->conns) {
			calcMaxRadiusConnection(c);
			fc->score += c->maxRadius;
		}
	}

	//delete?
	if (fc->score != 0) return;
	for (auto const& [r, sfc] : conn->cache) {
		if (sfc->score != 0) return;
	}
	//delete!
	conn->maxRadius = 0;
}

void Solver::calcMaxRadiusConnection(std::shared_ptr<Connection>& conn) {
	if (conn->type == ConnType::CORNER) calcMaxRadiusConnectionCorner(conn);
	else if (conn->type == ConnType::WALL) calcMaxRadiusConnectionWall(conn);
	else if (conn->type == ConnType::CIRCLE) calcMaxRadiusConnectionCircle(conn);
}

/*
* Tests the max-radius for a corner-Connection
*/
void Solver::calcMaxRadiusConnectionCorner(std::shared_ptr<Connection>& conn) {
	int i = (int)radii.size() - 1;

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
* Tests the max-radius for a wall-circle-Connection
*/
void Solver::calcMaxRadiusConnectionWall(std::shared_ptr<Connection>& conn) {
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
* Tests the max-radius for a circle-circle-Connection
*/
void Solver::calcMaxRadiusConnectionCircle(std::shared_ptr<Connection>& conn) {
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
* Constructs a Circle and its connections from another Connection
*/
std::shared_ptr<ConnectionFuture> Solver::getCircleFromConnection(std::shared_ptr<Connection>& conn, double r) {
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
* Constructs a circle from a corner-connection
*/
std::shared_ptr<ConnectionFuture> Solver::getCirclFromCorner(Corner corner, double r) {
	std::shared_ptr<ConnectionFuture> pc;
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
	c->index = iteration;
	return ConnectionFuture::create(c, conns);
}

/*
* Constructs a circle from a wall-circle-connection
*/
std::shared_ptr<ConnectionFuture> Solver::getCircleFromWall(std::shared_ptr<Connection>& conn, double r) {
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
	c->index = iteration;
	std::vector<std::shared_ptr<Connection>> conns = std::vector<std::shared_ptr<Connection>>();
	conns.emplace_back(Connection::create(c, conn->c1, true));
	conns.emplace_back(Connection::create(c, conn->c1, false));
	conns.emplace_back(Connection::create(c, conn->wall, true));
	conns.emplace_back(Connection::create(c, conn->wall, false));
	return ConnectionFuture::create(c, conns);
}

/*
* Constructs a circle from a circle-circle-connection
*/
std::shared_ptr<ConnectionFuture> Solver::getCircleFromCircle(std::shared_ptr<Circle>& c1, std::shared_ptr<Circle>& c2, double r, bool left) {
	std::shared_ptr<Circle> n;
	if (left) n = circleFromTwoCircles(c1, c2, r);
	else n = circleFromTwoCircles(c2, c1, r);
	n->index = iteration;
	std::vector<std::shared_ptr<Connection>> conns = std::vector<std::shared_ptr<Connection>>();
	conns.emplace_back(Connection::create(n, c1, true));
	conns.emplace_back(Connection::create(n, c1, false));
	conns.emplace_back(Connection::create(n, c2, true));
	conns.emplace_back(Connection::create(n, c2, false));
	return ConnectionFuture::create(n, conns);
}

/*
* Renders the computed circles
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
