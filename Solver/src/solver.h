#ifndef SOLVER_H
#define SOLVER_H

#include "utils.h"

class Solver {
public:
	Solver();
	virtual ~Solver();

	bool init(const std::string& inputfile);

	void reset();

	bool readInput(const std::string& path);
	bool writeOutput(Result& result, const std::string& outputfile);

	Result run(double weighting, unsigned seed);

	void stepWeights();

	void updateConnections(const std::shared_ptr<Circle>& circle);
	
	std::shared_ptr<PossibleCircle> getNextCircle(CircleType& t);

	bool checkValid(double cx, double cy, double r);

	void calcMaxRadiusConnectionCorner(std::shared_ptr<Connection> conn);
	void calcMaxRadiusConnectionWall(std::shared_ptr<Connection> conn);
	void calcMaxRadiusConnectionCircle(std::shared_ptr<Connection> conn);

	std::shared_ptr<PossibleCircle> getCircleFromConnection(std::shared_ptr<Connection> conn, double r);
	std::shared_ptr<PossibleCircle> getCirclFromCorner(Corner corner, double r);
	std::shared_ptr<PossibleCircle> getCircleFromWall(std::shared_ptr<Connection> conn, double r);
	std::shared_ptr<PossibleCircle> getCircleFromCircle(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r, bool left);

	void render();

private:
	double w, h;
	std::vector<CircleType> types;

	std::vector<std::shared_ptr<Circle>> circles;
	std::vector<std::shared_ptr<Connection>> conns_calculated;
	std::vector<std::shared_ptr<Connection>> conns_unknown;

	std::unordered_map<double, int> radiusMap;
	std::vector<double> radii;

	int circleCountAtMax = 0;
	double weighting;

	bool loaded;

#ifdef DRAW_SDL
	SDL_Window* window;
	SDL_Renderer* renderer;

	double scale;
#endif
};

#endif