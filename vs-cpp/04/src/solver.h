#ifndef SOLVER_H
#define SOLVER_H

#include "utils.h"

class Solver {
public:
	Solver(const std::string& file, const char* weightening = "0");
	virtual ~Solver();

	bool readInput(const std::string& path);
	void outputCircles(const std::string& path);

	void run();

	void stepWeights();

	std::shared_ptr<PossibleCircle> getCircleFromConnection(std::shared_ptr<Connection> conn, double r);
	std::shared_ptr<PossibleCircle> getNextCircle(CircleType& t);

	bool checkValid(double cx, double cy, double r);

	void calcMaxRadius(const std::shared_ptr<Circle>& circle);
	void calcMaxRadiusConnectionCorner(std::shared_ptr<Connection> conn);
	void calcMaxRadiusConnectionWall(std::shared_ptr<Connection> conn);
	void calcMaxRadiusConnectionCircle(std::shared_ptr<Connection> conn);

	std::vector<std::pair<double, std::shared_ptr<Circle>>> getAllPossible(double r);

	std::shared_ptr<PossibleCircle> getCircleFromCircle(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r, bool left);

	std::shared_ptr<PossibleCircle> circleFromWall(std::shared_ptr<Connection> conn, double r);
	std::shared_ptr<PossibleCircle> circlFromCorner(Corner corner, double r);

private:
	std::string name;
	double w, h;
	std::vector<CircleType> types;

	std::vector<std::shared_ptr<Circle>> circles;
	std::vector<std::shared_ptr<Connection>> conns_unknown;
	std::vector<std::shared_ptr<Connection>> conns_calculated;

	std::unordered_map<double, int> radiusMap;
	std::vector<double> radii;

	int circleCountAtMax = 0;
	double weightening;

	double numBlocks;

	int iteration;

	bool loaded;

};

#endif