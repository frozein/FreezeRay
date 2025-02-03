#include <SDL2/SDL.h>
#include <iostream>
#include "freezeray/fr_renderer.hpp"
#include "freezeray/renderer/fr_renderer_path.hpp"
#include "freezeray/material/fr_material_single_bxdf.hpp"
#include "freezeray/material/fr_material_specular_glass.hpp"
#include "freezeray/material/fr_material_metal.hpp"
#include "freezeray/material/fr_material_mirror.hpp"
#include "freezeray/material/fr_material_plastic.hpp"
#include "freezeray/bxdf/fr_brdf_lambertian_diffuse.hpp"
#include "freezeray/bxdf/fr_brdf_microfacet.hpp"
#include "freezeray/bxdf/fr_brdf_specular.hpp"
#include "freezeray/bxdf/fr_btdf_specular.hpp"
#include "freezeray/bxdf/fr_btdf_microfacet.hpp"
#include "freezeray/fresnel/fr_fresnel_dielectric.hpp"
#include "freezeray/fresnel/fr_fresnel_conductor.hpp"
#include "freezeray/fresnel/fr_fresnel_constant.hpp"
#include "freezeray/microfacet_distribution/fr_microfacet_distribution_beckmann.hpp"
#include "freezeray/microfacet_distribution/fr_microfacet_distribution_trowbridge_reitz.hpp"
#include "freezeray/light/fr_light_directional.hpp"
#include "freezeray/light/fr_light_point.hpp"
#include "freezeray/light/fr_light_area.hpp"
#include "freezeray/light/fr_light_environment.hpp"
#include "freezeray/texture/fr_texture_constant.hpp"
#include "freezeray/texture/fr_texture_image.hpp"

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
	std::shared_ptr<const fr::Mesh> mesh1 = fr::Mesh::unit_square();
	std::shared_ptr<const fr::Mesh> mesh2 = fr::Mesh::unit_sphere(2, true);

	std::shared_ptr<fr::Texture<vec3>> tex1 = std::make_shared<fr::TextureConstant<vec3>>(vec3(1.0f));
	std::shared_ptr<fr::Texture<float>> tex2 = std::make_shared<fr::TextureConstant<float>>(0.25f);

	std::shared_ptr<const fr::Material> material1 = std::make_shared<fr::MaterialSingleBXDF>("", std::make_shared<fr::BRDFLambertianDiffuse>(), tex1);
	std::shared_ptr<const fr::Material> material2 = std::make_shared<fr::MaterialMetal>("", fr::MetalType::GOLD, tex2, tex2);
	//std::shared_ptr<const fr::Material> material2 = std::make_shared<fr::MaterialMirror>("", tex1);

	std::vector<std::shared_ptr<const fr::Mesh>> meshList1 = {mesh1};
	std::vector<std::shared_ptr<const fr::Material>> materialList1 = {material1};
	std::shared_ptr<const fr::Object> object1 = std::make_shared<fr::Object>(meshList1, materialList1);
	mat4 objectTransform1 = translate(vec3(0.0f, -1.0f, 0.0f)) * scale(vec3(10.0f, 1.0f, 10.0f));

	std::vector<std::shared_ptr<const fr::Mesh>> meshList2 = {mesh2};
	std::vector<std::shared_ptr<const fr::Material>> materialList2 = {material2};
	std::shared_ptr<const fr::Object> object2 = std::make_shared<fr::Object>(meshList2, materialList2);
	mat4 objectTransform2 = mat4_identity();

	std::vector<fr::ObjectReference> objectList = {{object1, objectTransform1}, {object2, objectTransform2}};

	std::shared_ptr<const fr::LightDirectional> light1 = std::make_shared<fr::LightDirectional>(normalize(vec3(1.0f)), vec3(2.0f));
	std::shared_ptr<const fr::LightPoint> light2 = std::make_shared<fr::LightPoint>(vec3(-2.0f, 0.0f, 0.0f), vec3(0.3f, 0.3f, 1.5f));
	std::shared_ptr<const fr::LightEnvironment> light3 = std::make_shared<fr::LightEnvironment>("assets/test_skybox.png");
	
	//std::shared_ptr<const fr::Mesh> lightMesh1 = fr::Mesh::unit_square();
	//mat4 lightTransform1 = translate(vec3(0.0f, 1.5f, 0.0f)) * scale(vec3(2.5f, 1.0f, 2.5f));
	//std::shared_ptr<const fr::LightArea> light1 = std::make_shared<fr::LightArea>(lightMesh1, lightTransform1, vec3(1.0f));

	std::vector<std::shared_ptr<const fr::Light>> lightList = {light3};

	std::shared_ptr<const fr::Scene> scene = std::make_shared<fr::Scene>(objectList, lightList);

	//create renderer:
	//---------------
	std::shared_ptr<const fr::Camera> camera = std::make_shared<fr::Camera>(
		vec3(0.0f, 0.0f, -3.0f), 
		normalize(vec3(0.0f, 0.0f, 1.0f)), 
		vec3(0.0f, 1.0f, 0.0f), 
		60.0f, 
		(float)WINDOW_W / (float)WINDOW_H
	);
	std::unique_ptr<fr::Renderer> renderer = std::make_unique<fr::RendererPath>(
		camera, 
		WINDOW_W, 
		WINDOW_H, 
		50,
		10,
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
	
	renderer->render(scene, writePixel, display);

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