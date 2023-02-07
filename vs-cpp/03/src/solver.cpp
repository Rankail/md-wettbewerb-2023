#include "solver.h"

#include "utils.h"

Solver::Solver(const std::string& file) {
	readInput(file);
}

Solver::~Solver() {
}

void Solver::readInput(const std::string& path) {
	std::ifstream file;
	file.open(path);
	if (!file.is_open()) {
		printf("Failed to read file!\n");
		return;
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
		types.emplace_back(i, (double)std::stoi(line.substr(0, space)));
		i++;
	}

	std::sort(types.begin(), types.end(), [](const CircleType& lhs, const CircleType& rhs) {
		return lhs.r > rhs.r;
	});

	double min = types[types.size() - 1].r;
	numBlocks = 0;
	for (CircleType& t : types) {
		t.sizeMultiplier = t.r / min;
		numBlocks += t.r * t.r * PI;
	}

}

void Solver::run() {
	int nextIdx = getNextType();

	std::vector<std::shared_ptr<Circle>> circles = std::vector<std::shared_ptr<Circle>>();
	CircleType& t = types[nextIdx];
	std::cout << t << std::endl;
	circles.push_back(Circle::create(t.r, t.r, t.r));
	t.count++;

	bool fits = true;
	while (fits) {
		fits = false;
		t = types[getNextType()];

		std::cout << t << std::endl;

		

		break;
	}
}

int Solver::getNextType() {
	int idx = -1;
	double maxWeight = 0.;
	for (int i = 0; i < types.size(); i++) {
		double weight = types[i].sizeMultiplier * types[i].sizeMultiplier * (1. - types[i].count / numBlocks);
		if (weight > maxWeight) {
			maxWeight = weight;
			idx = i;
		}
	}
	if (idx == -1) {
		printf("Failed to find next type\n");
		return -1;
	}
	return idx;
}