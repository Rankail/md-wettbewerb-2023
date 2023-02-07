#include "Application.h"

#include "Renderer.h"

void Application::init() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL failed to initialize! Error: %s\n", SDL_GetError());
		return;
	}

	window = SDL_CreateWindow("Circles", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1000, 1000, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Failed to create SDL_Window! Error: %s\n", SDL_GetError());
	}

	Renderer::init(window);

	layer = new Layer();
}

void Application::shutdown() {
	Renderer::shutdown();
	SDL_DestroyWindow(window);
	SDL_Quit();
}

Application::Application() {
	init();
}

Application::~Application() {
	shutdown();
}

void Application::run() {
	quit = false;
	pTime = SDL_GetTicks();
	while (!quit) {
		events();
		update();
		render();
	}
}

void Application::events() {
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		layer->events(e);
		if (e.type == SDL_QUIT) {
			quit = true;
		} else if (e.type == SDL_KEYDOWN) {
			if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
				quit = true;
			}
		}
	}
}

void Application::update() {
	uint32_t cTime = SDL_GetTicks();
	float dt = (float)(cTime - pTime) / 1000.f;
	pTime = cTime;

	layer->update(dt);
}

void Application::render() {
	layer->render();
}
