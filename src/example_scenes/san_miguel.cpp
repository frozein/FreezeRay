#include "example_scene.hpp"

#include "freezeray/fr_object.hpp"
#include "freezeray/light/fr_light_directional.hpp"
#include "freezeray/light/fr_light_environment.hpp"
#include "freezeray/bxdf/fr_brdf_lambertian.hpp"

//-------------------------------------------//

#define WINDOW_W 1920
#define WINDOW_H 1080

//-------------------------------------------//

ExampleScene example_san_miguel()
{
	//create objects:
	//---------------
	std::shared_ptr<const fr::Object> sanMiguelObj = fr::Object::from_obj(
		"assets/models/san-miguel/san-miguel-low-poly.obj",
		"assets/models/san-miguel/san-miguel-low-poly.mtl"
	);
	mat4 sanMiguelTransform = mat4_identity();

	std::vector<fr::ObjectReference> objects = {
		{sanMiguelObj, sanMiguelTransform}
	};

	//create lights:
	//---------------
	std::unique_ptr<fr::Light> light1 = 
		std::make_unique<fr::LightEnvironment>("assets/skyboxes/noon_grass_4k.hdr");
		
	std::vector<std::unique_ptr<fr::Light>> lightList;
	lightList.push_back(std::move(light1));
	
	//define scene:
	//---------------
	std::shared_ptr<const fr::Scene> scene = std::make_shared<fr::Scene>(objects, lightList);

	//define camera:
	//---------------
	std::shared_ptr<const fr::Camera> camera = std::make_shared<fr::Camera>(
		vec3(22.0f, 1.0f, 13.0f), 
		normalize(vec3(-1.0f, 0.0f, -1.0f)), 
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