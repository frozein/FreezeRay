#include <SDL2/SDL.h>
#include <iostream>
#include <iomanip>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "example_scene.hpp"
#include "freezeray/renderer/fr_renderer_path.hpp"
#include "freezeray/renderer/fr_renderer_bidirectional.hpp"
#include "freezeray/renderer/fr_renderer_metropolis.hpp"
#include "freezeray/texture/stb_image.h"
#include "stb_image_write.h"

//-------------------------------------------//

int main(int argc, char** argv)
{
	//validate:
	//---------------
	if(argc < 2)
	{
		std::cout << "ERROR: no output path specified" << std::endl;
		return -1;
	}

	//load scene:
	//---------------
	ExampleScene scene = example_material_demo("assets/skyboxes/noon_sunny.hdr");
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

	//create output texture:
	//---------------
	std::unique_ptr<uint32_t[]> outTex = std::make_unique<uint32_t[]>(scene.windowWidth * scene.windowHeight);

	//create renderer:
	//---------------
	std::unique_ptr<fr::Renderer> renderer = std::make_unique<fr::RendererPath>(
		scene.camera, 
		scene.windowWidth, 
		scene.windowHeight, 
		10,
		512,
		false,
		false
	);

	/*std::unique_ptr<fr::Renderer> renderer = std::make_unique<fr::RendererMetropolis>(
		scene.camera,
		scene.windowWidth,
		scene.windowHeight,
		10,
		50
	);*/

	//start rendering:
	//---------------
	unsigned int startTime = SDL_GetTicks();

	auto writePixel = [&](uint32_t x, uint32_t y, vec3 color) -> void {
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = UINT8_MAX; //each pixel fully opaque

		uint32_t writeY = scene.windowHeight - y - 1;

		uint32_t writeWindow = SDL_MapRGBA(windowSurface->format, r, g, b, a);
		((uint32_t*)windowSurface->pixels)[x + scene.windowWidth * writeY] = writeWindow;

		uint32_t writeTex = (a << 24) | (b << 16) | (g << 8) | r;
		outTex[x + scene.windowWidth * writeY] = writeTex;
	};

	auto display = [&](float progress) -> void {
		std::cout << "Rendering... " << std::fixed << std::setprecision(2) << progress * 100.0f << "%\r";

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

	//save file to texture:
	//---------------
	stbi_write_png(argv[1], scene.windowWidth, scene.windowHeight, 4, outTex.get(), scene.windowWidth * sizeof(uint32_t));

	//print total rendering time:
	//---------------
	unsigned int endTime = SDL_GetTicks();

	std::cout << "RENDER TIME: " << (endTime - startTime) / 1000.0f << "s" << std::endl;

	//compute MSE if output texture given:
	//---------------
	if(argc >= 3)
	{
		uint32_t width;
		uint32_t height;
		uint32_t channels;
		uint8_t* refImage = stbi_load(argv[2], (int*)&width, (int*)&height, (int*)&channels, 4);

		if(!refImage) 
		{
			std::cout << "error loading reference image: " << argv[2] << std::endl;
			return -1;
		}
		
		if(width != scene.windowWidth || height != scene.windowHeight) 
		{
			std::cout << "reference image's dimensions to not match" << std::endl;
			return -1;
		}
		
		double sumSquaredError = 0.0;
		for(uint32_t y = 0; y < scene.windowHeight; y++)
		for(uint32_t x = 0; x < scene.windowWidth; x++) 
		{
			uint32_t idx = x + y * width;
			
			uint8_t* refPixel = &refImage[idx * 4];
			
			uint8_t renderedR = outTex[idx] & 0xFF;
			uint8_t renderedG = (outTex[idx] >> 8) & 0xFF;
			uint8_t renderedB = (outTex[idx] >> 16) & 0xFF;
			uint8_t renderedA = (outTex[idx] >> 24) & 0xFF;
			
			double diffR = refPixel[0] - renderedR;
			double diffG = refPixel[1] - renderedG;
			double diffB = refPixel[2] - renderedB;
			
			sumSquaredError += (diffR * diffR + diffG * diffG + diffB * diffB) / 3.0;
		}
		
		double mse = sumSquaredError / (width * height);
		std::cout << "MSE: " << mse << std::endl;

		stbi_image_free(refImage);
	}

	//continue updating window:
	//---------------
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