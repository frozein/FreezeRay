#include <SDL2/SDL.h>
#include <iostream>

#include "example_scene.hpp"
#include "freezeray/renderer/fr_renderer_path.hpp"
#include "freezeray/renderer/fr_renderer_bidirectional.hpp"

//-------------------------------------------//

int main(int argc, char** argv)
{
	//load scene:
	//---------------
	ExampleScene scene = example_material_demo("assets/test.hdr");
	//ExampleScene scene = example_cornell_box();
	//ExampleScene scene = example_sponza();
	//ExampleScene scene = example_san_miguel();

	//init SDL and create window:
	//---------------
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("failed to initialize SDL");
		return 0;
	}

	SDL_Window* window;
	window = SDL_CreateWindow("RT", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, scene.windowWidth, scene.windowHeight, SDL_WINDOW_SHOWN);
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

	//create renderer:
	//---------------
	std::unique_ptr<fr::Renderer> renderer = std::make_unique<fr::RendererBidirectional>(
		scene.camera, 
		scene.windowWidth, 
		scene.windowHeight, 
		10,
		10,
		true,
		false
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
		uint32_t writeY = scene.windowHeight - y - 1;

		((uint32_t*)windowSurface->pixels)[x + scene.windowWidth * writeY] = writeColor; //TODO: be more robust about format handling
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