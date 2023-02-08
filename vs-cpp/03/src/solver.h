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


	int getNextType();
	std::shared_ptr<Circle> getNextCircle(CircleType& t);
	bool checkPos(const Circle& c);

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