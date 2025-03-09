#include <SDL2/SDL.h>
#include <iostream>

#include "example_scene.hpp"
#include "freezeray/renderer/fr_renderer_path.hpp"

#define WINDOW_W 1920
#define WINDOW_H 1080

//-------------------------------------------//

int main(int argc, char** argv)
{
	//init SDL and create window:
	//---------------
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("failed to initialize SDL");
		return 0;
	}

	SDL_Window* window;
	window = SDL_CreateWindow("RT", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);
	if(!window)
	{
		printf("failed to create SDL window");
		return 0;
	}

	SDL_Surface* windowSurface = SDL_GetWindowSurface(window);
	if(!windowSurface)
	{
		printf("failed to get SDL window surace");
		return 0;
	}

	SDL_PixelFormat windowFormat = *windowSurface->format;

	//clear surface to black before rendering:
	SDL_LockSurface(windowSurface); //TODO: this can technically fail, ensure it succeeds
	memset(windowSurface->pixels, 0, windowSurface->h * windowSurface->pitch);
	SDL_UnlockSurface(windowSurface);

	//load scene:
	//---------------
	ExampleScene scene = example_material_demo("assets/test_skybox.hdr");

	//create renderer:
	//---------------
	std::unique_ptr<fr::Renderer> renderer = std::make_unique<fr::RendererPath>(
		scene.camera, 
		WINDOW_W, 
		WINDOW_H, 
		50,
		100,
		true,
		true
	);

	//start rendering:
	//---------------
	unsigned int startTime = SDL_GetTicks();

	auto writePixel = [&](uint32_t x, uint32_t y, vec3 color) -> void {
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = UINT8_MAX; //each pixel fully opaque

		uint32_t writeColor = SDL_MapRGBA(windowSurface->format, r, g, b, a);
		uint32_t writeY = WINDOW_H - y - 1;

		((uint32_t*)windowSurface->pixels)[x + WINDOW_W * writeY] = writeColor; //TODO: be more robust about format handling
	};

	auto display = [&]() -> void {
		SDL_UnlockSurface(windowSurface);
		SDL_UpdateWindowSurface(window);
		SDL_LockSurface(windowSurface);

		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT)
				exit(0);
		}
	};

	SDL_LockSurface(windowSurface);
	
	renderer->render(scene.scene, writePixel, display, 3);

	SDL_UnlockSurface(windowSurface);

	//print total rendering time and continue updating window until closed:
	//---------------
	unsigned int endTime = SDL_GetTicks();

	std::cout << "RENDER TIME: " << (endTime - startTime) / 1000.0f << "s" << std::endl;

	while(true)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT)
				exit(0);
		}
	}

	//free resources and return:
	//---------------
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}