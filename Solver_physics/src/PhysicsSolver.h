#pragma once

#include "solver.h"
#include "renderer.h"

struct Circle_p {
	double cx, cy, cx_prev, cy_prev;
	double ax, ay;
	double radius;
	int typeIndex;

	Circle_p(double cx, double cy, double r, int typeIndex)
		: cx(cx), cy(cy), cx_prev(cx), cy_prev(cy), ax(0.), ay(0.), radius(r), typeIndex(typeIndex)
	{ }

	void update() {
		//double disx = cx - cx_prev;
		//double disy = cy - cy_prev;

		cx_prev = cx;
		cy_prev = cy;
		cx = cx + ax;
		cy = cy + ay;
		ax = 0.; ay = 0.;
	}

	void accelerate(double ax, double ay) {
		this->ax += ax;
		this->ay += ay;
	}
};

class PhysicsSolver {
public:
	PhysicsSolver(std::vector<std::shared_ptr<Circle>> circles, double w, double h)
		: w(w), h(h) {
		this->circles = std::vector<Circle_p>();
		for (auto& c : circles) {
			this->circles.emplace_back(c->cx, c->cy, c->r, c->typeIndex);
		}
	}
	virtual ~PhysicsSolver() {}

	std::vector<Circle_p> run(unsigned iterations) {
		zzCount = 0;
		unsigned i = 0;
		while (true) {
			applyGravity();
			updateObjects();
			for (int j = 0; j < 50; j++) {
				applyConstraints();
				checkCollisions();
				if (i == iterations) { //update would move all objects by gravity -> out of bounds
					std::cout << "zzCount: " << zzCount << std::endl;
					return circles;
				}
				i++;
			}
			Renderer::clear();
			for (auto& c : circles) {
				Renderer::drawCircle(c.cx, c.cy, c.radius, c.typeIndex);
			}
			Renderer::present();
		}
	}

	void applyGravity() {
		for (auto& c : circles) {
			c.accelerate(.1, .1);
		}
	}

	void checkCollisions() {
		const unsigned obj_count = circles.size();
		for (unsigned i = 0; i < obj_count; i++) {
			auto& c1 = circles[i];
			for (unsigned j = i + 1; j < obj_count; j++) {
				auto& c2 = circles[j];
				double dx = c1.cx - c2.cx;
				double dy = c1.cy - c2.cy;
				double r = c2.radius + c1.radius;
				/*if (dx == 0. && dy == 0.) {
					dx = 1e-10;
					dy = 1e-10;
					zzCount++;
				}*/
				double d2 = dx * dx + dy * dy;
				if (d2 >= r * r) continue;
				const double dist = std::sqrt(d2);
				const double nx = dx / dist;
				const double ny = dy / dist;

				const double mRatio1 = 1.;
				const double mRatio2 = 1.;
				const double delta = .5 * (dist - r);
				c1.cx -= nx * (mRatio2 * delta);
				c1.cy -= ny * (mRatio2 * delta);
				c2.cx += nx * (mRatio1 * delta);
				c2.cy += ny * (mRatio1 * delta);
			}
		}
	}

	void applyConstraints() {
		for (auto& c : circles) {
			if (c.cx < c.radius) {
				c.cx = c.radius;
			} else if (c.cx + c.radius > w) {
				c.cx = w - c.radius;
			}
			if (c.cy < c.radius) {
				c.cy = c.radius;
			} else if (c.cy + c.radius > h) {
				c.cy = h - c.radius;
			}
		}
	}

	void updateObjects() {
		for (auto& c : circles) {
			c.update();
		}
	}

	bool writeOutput(const std::string& outFile) {
		std::ofstream file;
		file.open(outFile, std::ios::out);
		if (!file.is_open()) {
			std::cout << "Failed to open output-file.\n" << std::endl;
			return false;
		}
		std::cout << "Writing to '" << outFile << "'" << std::endl;

		for (auto& c : circles) {
			file  << c.cx << " " << c.cy << " " << (c.radius) << " " << c.typeIndex << "\n";
		}
		file.close();
		return true;
	}

private:
	double w, h;
	std::vector<Circle_p> circles;
	unsigned zzCount;
};