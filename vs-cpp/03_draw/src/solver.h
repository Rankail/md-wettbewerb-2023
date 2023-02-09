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

	void updateConns(std::shared_ptr<Circle> c);

	bool checkValid(double cx, double cy, double r);

	void calcMaxRadius();
	void calcMaxRadiusConnectionCorner(std::shared_ptr<Connection> conn);
	void calcMaxRadiusConnectionWall(std::shared_ptr<Connection> conn);
	void calcMaxRadiusConnectionCircle(std::shared_ptr<Connection> conn);

	std::vector<std::pair<double, std::shared_ptr<Circle>>> getAllPossible(double r);

	std::shared_ptr<Circle> getWallUpLeft(std::shared_ptr<Circle> c, double r, double wd);
	std::shared_ptr<Circle> getWallUpRight(std::shared_ptr<Circle> c, double r, double wd);
	std::shared_ptr<Circle> getWallLeftUp(std::shared_ptr<Circle> c, double r, double wd);
	std::shared_ptr<Circle> getWallLeftDown(std::shared_ptr<Circle> c, double r, double wd);
	std::shared_ptr<Circle> getWallDownLeft(std::shared_ptr<Circle> c, double r, double wd);
	std::shared_ptr<Circle> getWallDownRight(std::shared_ptr<Circle> c, double r, double wd);
	std::shared_ptr<Circle> getWallRightUp(std::shared_ptr<Circle> c, double r, double wd);
	std::shared_ptr<Circle> getWallRightDown(std::shared_ptr<Circle> c, double r, double wd);

	std::shared_ptr<Circle> getCircleCircleLeft(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r);
	std::shared_ptr<Circle> getCircleCircleRight(std::shared_ptr<Circle> c1, std::shared_ptr<Circle> c2, double r);

	std::shared_ptr<Circle> circleFromWall(std::shared_ptr<Connection> conn, bool left, double r);
	std::shared_ptr<Circle> circlFromCorner(Corner corner, double r);

	void render();

private:
	std::string name;
	double w, h;
	std::vector<CircleType> types;

	std::vector<std::shared_ptr<Circle>> circles;
	std::vector<std::shared_ptr<Connection>> connections;

	double numBlocks;

	bool loaded;

	SDL_Window* window;
	SDL_Renderer* renderer;
};

#endif