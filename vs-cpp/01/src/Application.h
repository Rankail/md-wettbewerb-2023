#pragma once

#include <SDL.h>
#include <memory>
#include "Layer.h"

class Application {
private:
	void init();
	void shutdown();

public:
	Application();
	virtual ~Application();

	void run();
	void events();
	void update();
	void render();

private:
	SDL_Window* window;
	Layer* layer;
	bool quit;
};