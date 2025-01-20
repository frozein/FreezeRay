#include <SDL2/SDL.h>
#include <iostream>
#include "rurt/renderer.hpp"
#include "rurt/material/material_single_bxdf.hpp"
#include "rurt/material/material_specular_glass.hpp"
#include "rurt/material/material_metal.hpp"
#include "rurt/material/material_mirror.hpp"
#include "rurt/material/material_plastic.hpp"
#include "rurt/bxdf/brdf_lambertian_diffuse.hpp"
#include "rurt/bxdf/brdf_microfacet.hpp"
#include "rurt/bxdf/brdf_specular.hpp"
#include "rurt/bxdf/btdf_specular.hpp"
#include "rurt/bxdf/btdf_microfacet.hpp"
#include "rurt/fresnel/fresnel_dielectric.hpp"
#include "rurt/fresnel/fresnel_conductor.hpp"
#include "rurt/fresnel/fresnel_constant.hpp"
#include "rurt/microfacet_distribution/microfacet_distribution_beckmann.hpp"
#include "rurt/microfacet_distribution/microfacet_distribution_trowbridge_reitz.hpp"
#include "rurt/light/light_directional.hpp"
#include "rurt/light/light_point.hpp"
#include "rurt/light/light_area.hpp"
#include "rurt/texture/texture_constant.hpp"
#include "rurt/texture/texture_image.hpp"

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

	//create scene:
	//---------------
	std::shared_ptr<const rurt::Mesh> mesh1 = rurt::Mesh::unit_square();
	std::shared_ptr<const rurt::Mesh> mesh2 = rurt::Mesh::unit_sphere(2, true);

	std::shared_ptr<rurt::Texture<vec3>> tex1 = rurt::TextureImage<vec3, uint32_t>::from_file("C:\\Users\\danie\\Pictures\\Saved Pictures\\Memes\\1070087967294631976-1bc48673-3146-4107-864d-a3eef69934ac-571447.png", rurt::TextureRepeatMode::CLAMP_TO_EDGE);
	std::shared_ptr<rurt::Texture<vec3>> tex2 = std::make_shared<rurt::TextureConstant<vec3>>(vec3(1.0f, 0.0f, 0.0));

	std::shared_ptr<const rurt::Material> material1 = std::make_shared<rurt::MaterialSingleBXDF>("", std::make_shared<rurt::BRDFLambertianDiffuse>(), tex1);
	std::shared_ptr<const rurt::Material> material2 = std::make_shared<rurt::MaterialSingleBXDF>("", std::make_shared<rurt::BRDFLambertianDiffuse>(), tex2);
	//std::shared_ptr<const rurt::Material> material2 = std::make_shared<rurt::MaterialPlastic>("", vec3(0.0f), vec3(1.0f), 0.25f);

	std::vector<std::shared_ptr<const rurt::Mesh>> meshList1 = {mesh1};
	std::vector<std::shared_ptr<const rurt::Material>> materialList1 = {material1};
	std::shared_ptr<const rurt::Object> object1 = std::make_shared<rurt::Object>(meshList1, materialList1);
	mat4 objectTransform1 = translate(vec3(0.0f, -1.0f, 0.0f)) * scale(vec3(5.0f, 1.0f, 5.0f));
	
	std::vector<std::shared_ptr<const rurt::Mesh>> meshList2 = {mesh2};
	std::vector<std::shared_ptr<const rurt::Material>> materialList2 = {material2};
	std::shared_ptr<const rurt::Object> object2 = std::make_shared<rurt::Object>(meshList2, materialList2);
	mat4 objectTransform2 = mat4_identity();

	std::vector<rurt::ObjectReference> objectList = {{object1, objectTransform1}};

	std::shared_ptr<const rurt::LightDirectional> light1 = std::make_shared<rurt::LightDirectional>(normalize(vec3(1.0f)), vec3(2.0f));
	std::shared_ptr<const rurt::LightPoint> light2 = std::make_shared<rurt::LightPoint>(vec3(-2.0f, 0.0f, 0.0f), vec3(0.3f, 0.3f, 1.5f));
	
	//std::shared_ptr<const rurt::Mesh> lightMesh1 = rurt::Mesh::unit_square();
	//mat4 lightTransform1 = translate(vec3(0.0f, 1.5f, 0.0f)) * scale(vec3(2.5f, 1.0f, 2.5f));
	//std::shared_ptr<const rurt::LightArea> light1 = std::make_shared<rurt::LightArea>(lightMesh1, lightTransform1, vec3(1.0f));

	std::vector<std::shared_ptr<const rurt::Light>> lightList = {light1};

	std::shared_ptr<const rurt::Scene> scene = std::make_shared<rurt::Scene>(objectList, lightList);

	//create renderer:
	//---------------
	std::shared_ptr<const rurt::Camera> camera = std::make_shared<rurt::Camera>(
		vec3(0.0f, -0.5f, 3.0f), 
		normalize(vec3(0.0f, 0.0f, -1.0f)), 
		vec3(0.0f, 1.0f, 0.0f), 
		60.0f, 
		(float)WINDOW_W / (float)WINDOW_H
	);
	rurt::Renderer* renderer = new rurt::Renderer(scene, camera, WINDOW_W, WINDOW_H, 1);

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