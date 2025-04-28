#include "example_scene.hpp"

#include "freezeray/fr_object.hpp"
#include "freezeray/light/fr_light_directional.hpp"
#include "freezeray/light/fr_light_area.hpp"
#include "freezeray/light/fr_light_environment.hpp"
#include "freezeray/bxdf/fr_brdf_lambertian.hpp"
#include "freezeray/material/fr_material_single_bxdf.hpp"
#include "freezeray/texture/fr_texture_constant.hpp"

//-------------------------------------------//

#define WINDOW_W 1920
#define WINDOW_H 1080

//-------------------------------------------//

ExampleScene example_sponza()
{
	//create objects:
	//---------------
	std::shared_ptr<const fr::Object> sponzaObj = fr::Object::from_obj(
		"assets/models/sponza/sponza.obj",
		"assets/models/sponza/sponza.mtl"
	);
	mat4 sponzaTransform = mat4_identity();

	std::vector<fr::ObjectReference> objects = {
		{sponzaObj, sponzaTransform}
	};

	//create lights:
	//---------------
	std::vector<std::unique_ptr<fr::Light>> lightList;

	std::unique_ptr<fr::Light> envLight = 
		std::make_unique<fr::LightEnvironment>("assets/skyboxes/night.hdr");
	lightList.push_back(std::move(envLight));


	const vec3 bunnyColors[3] = {vec3(1.0f, 0.1f, 0.1f), vec3(0.1f, 1.0f, 0.1f), vec3(0.1f, 0.1f, 1.0f)};
	const float bunnyPositions[3] = {0.0f, -400.0f, -800.0f};
	std::shared_ptr<const fr::Mesh> bunnyMesh = fr::Mesh::from_obj("assets/models/bunny.obj")[0];

	mat4 bunnyTransformBase = rotate(vec3(0.0f, 1.0f, 0.0f), 90.0f) * scale(vec3(100.0f));

	for(uint32_t i = 0; i < 3; i++)
	{
		mat4 topLeftTransform  = translate(vec3(bunnyPositions[i], 450.0f, -400.0f)) * bunnyTransformBase;
		mat4 topRightTransform = translate(vec3(bunnyPositions[i], 450.0f,  400.0f)) * bunnyTransformBase;
		mat4 botLeftTransform  = translate(vec3(bunnyPositions[i],   0.0f, -400.0f)) * bunnyTransformBase;
		mat4 botRightTransform = translate(vec3(bunnyPositions[i],   0.0f,  400.0f)) * bunnyTransformBase;

		lightList.push_back(std::make_unique<fr::LightArea>(bunnyMesh, topLeftTransform , bunnyColors[i]));
		lightList.push_back(std::make_unique<fr::LightArea>(bunnyMesh, topRightTransform, bunnyColors[i]));
		lightList.push_back(std::make_unique<fr::LightArea>(bunnyMesh, botLeftTransform , bunnyColors[i]));
		lightList.push_back(std::make_unique<fr::LightArea>(bunnyMesh, botRightTransform, bunnyColors[i]));
	}

	mat4 centerTopBunnyTransform = translate(vec3(-1300.0f, 450.0f, -50.0f)) * bunnyTransformBase;
	mat4 centerBotBunnyTransform = translate(vec3(-1300.0f,   0.0f, -50.0f)) * bunnyTransformBase;
	
	lightList.push_back(std::make_unique<fr::LightArea>(bunnyMesh, centerTopBunnyTransform, vec3(1.0f)));
	lightList.push_back(std::make_unique<fr::LightArea>(bunnyMesh, centerBotBunnyTransform, vec3(1.0f)));

	//define scene:
	//---------------
	std::shared_ptr<const fr::Scene> scene = std::make_shared<fr::Scene>(objects, lightList);

	//define camera:
	//---------------
	std::shared_ptr<const fr::Camera> camera = std::make_shared<fr::Camera>(
		vec3(500.0f, 700.0f, 0.0f), 
		normalize(vec3(-1.0f, -0.2f, -0.2f)), 
		vec3(0.0f, 1.0f, 0.0f), 
		60.0f, 
		(float)WINDOW_W / (float)WINDOW_H
	);

	//return:
	//---------------
	ExampleScene exampleScene;
	exampleScene.windowWidth  = WINDOW_W;
	exampleScene.windowHeight = WINDOW_H;
	exampleScene.scene = scene;
	exampleScene.camera = camera;

	return exampleScene;
}