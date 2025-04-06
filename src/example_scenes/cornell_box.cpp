#include "example_scene.hpp"

#include "freezeray/bxdf/fr_brdf_lambertian.hpp"
#include "freezeray/texture/fr_texture_constant.hpp"
#include "freezeray/material/fr_material_single_bxdf.hpp"
#include "freezeray/light/fr_light_area.hpp"

//-------------------------------------------//

#define WINDOW_W 1280
#define WINDOW_H 1280

//-------------------------------------------//

ExampleScene example_cornell_box()
{
	//create objects:
	//---------------
	std::shared_ptr<const fr::Mesh> squareMesh = fr::Mesh::from_unit_square();
	std::shared_ptr<const fr::Mesh> cubeMesh = fr::Mesh::from_unit_cube();

	std::shared_ptr<fr::Texture<vec3>> whiteColorTex = std::make_shared<fr::TextureConstant<vec3>>(vec3(0.73f, 0.73f, 0.73f));
	std::shared_ptr<fr::Texture<vec3>> redColorTex = std::make_shared<fr::TextureConstant<vec3>>(vec3(0.65f, 0.05f, 0.05f));
	std::shared_ptr<fr::Texture<vec3>> greenColorTex = std::make_shared<fr::TextureConstant<vec3>>(vec3(0.12f, 0.45f, 0.15f));

	std::shared_ptr<const fr::Material> whiteMat = std::make_shared<fr::MaterialSingleBXDF>("", std::make_shared<fr::BRDFLambertian>(), whiteColorTex);
	std::shared_ptr<const fr::Material> redMat = std::make_shared<fr::MaterialSingleBXDF>("", std::make_shared<fr::BRDFLambertian>(), redColorTex);
	std::shared_ptr<const fr::Material> greenMat = std::make_shared<fr::MaterialSingleBXDF>("", std::make_shared<fr::BRDFLambertian>(), greenColorTex);
		
	mat4 scaleMat = scale(vec3(1.0f  +FR_EPSILON, 1.0f + FR_EPSILON, 1.0f + FR_EPSILON));
	
	std::shared_ptr<const fr::Object> leftWallObj = std::make_shared<fr::Object>(squareMesh, redMat);
	mat4 leftWallTransform = translate(vec3(-0.5f, 0.0f, 0.0f)) * rotate(vec3(0.0f, 0.0f, 1.0f), -90.0f) * scaleMat;

	std::shared_ptr<const fr::Object> rightWallObj = std::make_shared<fr::Object>(squareMesh, greenMat);
	mat4 rightWallTransform = translate(vec3(0.5f, 0.0f, 0.0f)) * rotate(vec3(0.0f, 0.0f, 1.0f), 90.0f) * scaleMat;

	std::shared_ptr<const fr::Object> backWallObj = std::make_shared<fr::Object>(squareMesh, whiteMat);
	mat4 backWallTransform = translate(vec3(0.0f, 0.0f, -0.5f)) * rotate(vec3(1.0f, 0.0f, 0.0f), 90.0f) * scaleMat;

	std::shared_ptr<const fr::Object> floorObj = std::make_shared<fr::Object>(squareMesh, whiteMat);
	mat4 floorTransform = translate(vec3(0.0f, -0.5f, 0.0f)) * scaleMat;
	
	std::shared_ptr<const fr::Object> ceilingObj = std::make_shared<fr::Object>(squareMesh, whiteMat);
	mat4 ceilingTransform = translate(vec3(0.0f, 0.5f, 0.0f)) * scaleMat;
	
	float lightSize = 0.25f;
	float lightHeight = 0.5f - FR_EPSILON;
	mat4 lightTransform = translate(vec3(0.0f, lightHeight, 0.0f)) * scale(vec3(lightSize, 1.0f, lightSize));
	
	std::shared_ptr<const fr::Object> tallBoxObj = std::make_shared<fr::Object>(cubeMesh, whiteMat);
	float tallBoxHeight = 0.55f;
	float tallBoxWidth = 0.3f;
	mat4 tallBoxTransform = translate(vec3(0.25f, -0.5f + tallBoxHeight/2, -0.09f)) * 
	                        rotate(vec3(0.0f, 1.0f, 0.0f), -20.0f) * 
	                        scale(vec3(tallBoxWidth, tallBoxHeight, tallBoxWidth));
	
	std::shared_ptr<const fr::Object> shortBoxObj = std::make_shared<fr::Object>(cubeMesh, whiteMat);
	float shortBoxHeight = 0.27f;
	float shortBoxWidth = 0.3f;
	mat4 shortBoxTransform = translate(vec3(-0.27f, -0.5f + shortBoxHeight/2, 0.15f)) * 
	                         rotate(vec3(0.0f, 1.0f, 0.0f), 18.0f) * 
	                         scale(vec3(shortBoxWidth, shortBoxHeight, shortBoxWidth));

	std::vector<fr::ObjectReference> objects = {
		{leftWallObj, leftWallTransform},
		{rightWallObj, rightWallTransform},
		{backWallObj, backWallTransform},
		{floorObj, floorTransform},
		{ceilingObj, ceilingTransform},
		{tallBoxObj, tallBoxTransform},
		{shortBoxObj, shortBoxTransform}
	};

	//create lights:
	//---------------
	std::unique_ptr<fr::LightArea> areaLight = std::make_unique<fr::LightArea>(
		squareMesh,
		lightTransform,
		10.0f
	);
	
	std::vector<std::unique_ptr<fr::Light>> lightList;
	lightList.push_back(std::move(areaLight));

	//define scene:
	//---------------
	std::shared_ptr<const fr::Scene> scene = std::make_shared<fr::Scene>(objects, lightList);

	//define camera:
	//---------------
	std::shared_ptr<const fr::Camera> camera = std::make_shared<fr::Camera>(
		vec3(0.0f, 0.0f, 2.5f), 
		normalize(vec3(0.0f, 0.0f, -1.0f)),
		vec3(0.0f, 1.0f, 0.0f),
		40.0f,
		(float)WINDOW_W / (float)WINDOW_H
	);

	//return:
	//---------------
	ExampleScene exampleScene;
	exampleScene.windowWidth = WINDOW_W;
	exampleScene.windowHeight = WINDOW_H;
	exampleScene.scene = scene;
	exampleScene.camera = camera;

	return exampleScene;
}