#ifndef SOLVER_H
#define SOLVER_H

#include "utils.h"

class Solver {
public:
	Solver();
	virtual ~Solver();

	bool init(const std::string& inputfile, double weighting);

	void reset();

	bool readInput(const std::string& path);
	bool writeOutput(Result& result, const std::string& outputfile);

	Result run();

	void stepWeights();

	void updateConnections(const std::shared_ptr<Circle>& circle);
	
	std::shared_ptr<ConnectionFuture> getNextCircle(CircleType& t);

	bool checkValid(double cx, double cy, double r);
	bool connInRange(const std::shared_ptr<Circle>& circle, const std::shared_ptr<Connection>& conn);

	void calcRadiiFuture(std::shared_ptr<Connection>& conn, double r);

	void calcMaxRadiusConnection(std::shared_ptr<Connection>& conn);
	void calcMaxRadiusConnectionCorner(std::shared_ptr<Connection>& conn);
	void calcMaxRadiusConnectionWall(std::shared_ptr<Connection>& conn);
	void calcMaxRadiusConnectionCircle(std::shared_ptr<Connection>& conn);


	std::shared_ptr<PossibleCircle> getCircleFromConnection(std::shared_ptr<Connection>& conn, double r);
	std::shared_ptr<PossibleCircle> getCirclFromCorner(Corner corner, double r);
	std::shared_ptr<PossibleCircle> getCircleFromWall(std::shared_ptr<Connection>& conn, double r);
	std::shared_ptr<PossibleCircle> getCircleFromCircle(std::shared_ptr<Circle>& c1, std::shared_ptr<Circle>& c2, double r, bool left);

	void render();

private:
	std::string name;
	double w, h;
	std::vector<CircleType> types;

	std::vector<std::shared_ptr<Circle>> circles;
	std::list<std::shared_ptr<Connection>> conns;
	std::list<std::shared_ptr<Connection>> conns_unknown;
	std::list<std::shared_ptr<Connection>> conns_calculated;

	std::unordered_map<double, int> radiusMap;
	std::vector<double> radii;

	int circleCountAtMax = 0;
	double weighting;

	double numBlocks;

	int iteration;

	bool loaded;

#ifdef DRAW_SDL
	SDL_Window* window;
	SDL_Renderer* renderer;

	double scale;
#endif
};

#endif