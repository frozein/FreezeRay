#include <SDL2/SDL.h>
#include <iostream>
#include "rurt/renderer.hpp"

#define WINDOW_W 1920
#define WINDOW_H 1080

//-------------------------------------------//

std::shared_ptr<rurt::Mesh> generatePolyShphere(float rad, uint32_t divs) 
{ 
	//this code is ripped directly from https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-polygon-mesh/ray-tracing-polygon-mesh-part-1.html
	//will replace eventually

	// generate points                                                                                                                                                                                      
	uint32_t numVertices = (divs - 1) * divs + 2; 
	std::shared_ptr<vec3[]> verts(new vec3[numVertices]); 
 
	float u = -(float)M_PI / 2.0f; 
	float v = -(float)M_PI; 
	float du = (float)M_PI / divs; 
	float dv = 2.0f * (float)M_PI / divs; 
 
	verts[0] = vec3(0, -rad, 0); 
	uint32_t k = 1; 
	for (uint32_t i = 0; i < divs - 1; i++) { 
		u += du; 
		v = -(float)M_PI; 
		for (uint32_t j = 0; j < divs; j++) { 
			float x = rad * cos(u) * cos(v); 
			float y = rad * sin(u); 
			float z = rad * cos(u) * sin(v) ; 
			verts[k] = vec3(x, y, z); 
			v += dv, k++; 
		} 
	} 
	verts[k] = vec3(0, rad, 0); 
 
	uint32_t npolys = divs * divs; 
	std::shared_ptr<uint32_t[]> faceIndex(new uint32_t[npolys]); 
	std::shared_ptr<uint32_t[]> vertsIndex(new uint32_t[(6 + (divs - 1) * 4) * divs]); 
 
	// create the connectivity lists                                                                                                                                                                        
	uint32_t vid = 1, numV = 0, l = 0; 
	k = 0; 
	for (uint32_t i = 0; i < divs; i++) { 
		for (uint32_t j = 0; j < divs; j++) { 
			if (i == 0) { 
				faceIndex[k++] = 3; 
				vertsIndex[l] = 0; 
				vertsIndex[l + 1] = j + vid; 
				vertsIndex[l + 2] = (j == (divs - 1)) ? vid : j + vid + 1; 
				l += 3; 
			} 
			else if (i == (divs - 1)) { 
				faceIndex[k++] = 3; 
				vertsIndex[l] = j + vid + 1 - divs; 
				vertsIndex[l + 1] = vid + 1; 
				vertsIndex[l + 2] = (j == (divs - 1)) ? vid + 1 - divs : j + vid + 2 - divs; 
				l += 3; 
			} 
			else { 
				faceIndex[k++] = 4; 
				vertsIndex[l] = j + vid + 1 - divs; 
				vertsIndex[l + 1] = j + vid + 1; 
				vertsIndex[l + 2] = (j == (divs - 1)) ? vid + 1 : j + vid + 2; 
				vertsIndex[l + 3] = (j == (divs - 1)) ? vid + 1 - divs : j + vid + 2 - divs; 
				l += 4; 
			} 
			numV++; 
		} 
		vid = numV; 
	} 
 
	std::shared_ptr<float[]> vertsCasted = std::reinterpret_pointer_cast<float[]>(verts);
	return std::make_shared<rurt::Mesh>(rurt::VERTEX_ATTRIB_POSITION, npolys, faceIndex, vertsIndex, vertsCasted); 
}

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

	//create scene:
	//---------------
	std::shared_ptr<rurt::Scene> scene = std::make_shared<rurt::Scene>(generatePolyShphere(1.0f, 20));

	//create renderer:
	//---------------
	rurt::Camera camera = rurt::Camera(
		vec3(0.0f, 0.0f, 3.0f), 
		normalize(vec3(0.0f, 0.0f, -1.0f)), 
		vec3(0.0f, 1.0f, 0.0f), 
		60.0f, 
		(float)WINDOW_W / (float)WINDOW_H
	);
	rurt::Renderer* renderer = new rurt::Renderer(scene, camera, WINDOW_W, WINDOW_H);

	//draw loop until rendering finished:
	//---------------
	unsigned int startTime = SDL_GetTicks();

	uint32_t scanline[WINDOW_W];

	bool running = true;
	for(int y = 0; y < WINDOW_H; y++)
	{
		renderer->draw_scanline(WINDOW_H - y - 1, scanline);

		SDL_LockSurface(windowSurface);
		for(uint32_t x = 0; x < WINDOW_W; x++)
		{
			uint8_t r = (scanline[x] >> 24) & 0xFF; 
			uint8_t g = (scanline[x] >> 16) & 0xFF; 
			uint8_t b = (scanline[x] >> 8 ) & 0xFF;
			uint8_t a = scanline[x] & 0xFF; 

			uint32_t writeColor = SDL_MapRGBA(windowSurface->format, r, g, b, a);
			((uint32_t*)windowSurface->pixels)[x + WINDOW_W * y] = writeColor; //TODO: be more robust about format handling
		}
		SDL_UnlockSurface(windowSurface);

		SDL_UpdateWindowSurface(window);

		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT)
				running = false;
		}

		if(!running)
			break;
	}

	//print total rendering time and continue updating window until closed:
	//---------------
	unsigned int endTime = SDL_GetTicks();

	std::cout << "RENDER TIME: " << (endTime - startTime) / 1000.0f << "s" << std::endl;

	while(running)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT)
				running = false;
		}
	}

	//free resources and return:
	//---------------
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}