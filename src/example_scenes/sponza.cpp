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
	std::shared_ptr<const fr::Light> light1 = 
		std::make_shared<fr::LightDirectional>(normalize(vec3(0.1f, 1.0f, 0.1f)), vec3(1.0f));
		
	std::shared_ptr<const fr::Mesh> light2Mesh = fr::Mesh::from_obj("assets/models/bunny.obj")[0];
	mat4 light2Transform = translate(vec3(-250.0f, 0.0f, -40.0f)) * rotate(vec3(0.0f, 1.0f, 0.0f), 90.0f) * scale(vec3(75.0f));
	std::shared_ptr<const fr::Light> light2 =
		std::make_shared<fr::LightArea>(light2Mesh, light2Transform, vec3(1.0f, 0.1f, 0.1f));
		
	std::vector<std::shared_ptr<const fr::Light>> lightList = {
		light1
		//light2
	};
	
	//define scene:
	//---------------
	std::shared_ptr<const fr::Scene> scene = std::make_shared<fr::Scene>(objects, lightList);

	//define camera:
	//---------------
	std::shared_ptr<const fr::Camera> camera = std::make_shared<fr::Camera>(
		vec3(0.0f, 100.0f, 0.0f), 
		normalize(vec3(-1.0f, 0.0f, 0.0f)), 
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