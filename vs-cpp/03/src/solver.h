#pragma once

#include "utils.h"

class Solver {
public:
	Solver(const std::string& file);
	virtual ~Solver();

	void readInput(const std::string& path);
	void run();

	int getNextType();

	void display() {}

private:
	double w, h;
	std::vector<CircleType> types;

	double numBlocks;

};