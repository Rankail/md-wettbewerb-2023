#ifndef SOLVER_H
#define SOLVER_H

#include "utils.h"

class Solver {
public:
	Solver(const std::string& file);
	virtual ~Solver();

	bool readInput(const std::string& path);
	void outputCircles(const std::string& path);

	void run();


	void stepWeights();
	std::shared_ptr<Circle> getNextCircle(CircleType& t);

	bool checkValid(double cx, double cy, double r);
	void calcMaxRadius(std::shared_ptr<Circle>& c);

	void checkCircle(std::vector<std::pair<double, Circle>>& possible, std::shared_ptr<Circle> c, double r);

	Circle getWallUpLeft(	std::shared_ptr<Circle> c, double r, double wd);
	Circle getWallUpRight(	std::shared_ptr<Circle> c, double r, double wd);
	Circle getWallLeftUp(	std::shared_ptr<Circle> c, double r, double wd);
	Circle getWallLeftDown(	std::shared_ptr<Circle> c, double r, double wd);
	Circle getWallDownLeft(	std::shared_ptr<Circle> c, double r, double wd);
	Circle getWallDownRight(std::shared_ptr<Circle> c, double r, double wd);
	Circle getWallRightUp(	std::shared_ptr<Circle> c, double r, double wd);
	Circle getWallRightDown(std::shared_ptr<Circle> c, double r, double wd);

	Circle getCircleCircleLeft(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r);
	Circle getCircleCircleRight(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r);

	void display() {}

private:
	std::string name;
	double w, h;
	std::vector<CircleType> types;

	std::vector<std::shared_ptr<Circle>> circles;

	double numBlocks;

	bool loaded;
};

#endif