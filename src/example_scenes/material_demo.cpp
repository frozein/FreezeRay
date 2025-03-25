#include "example_scene.hpp"

#include "freezeray/bxdf/fr_brdf_lambertian.hpp"
#include "freezeray/bxdf/fr_btdf_microfacet.hpp"
#include "freezeray/bxdf/fr_btdf_lambertian.hpp"
#include "freezeray/microfacet_distribution/fr_microfacet_distribution_trowbridge_reitz.hpp"
#include "freezeray/microfacet_distribution/fr_microfacet_distribution_beckmann.hpp"
#include "freezeray/texture/fr_texture_constant.hpp"
#include "freezeray/texture/fr_texture_image.hpp"
#include "freezeray/material/fr_material_single_bxdf.hpp"
#include "freezeray/material/fr_material_metal.hpp"
#include "freezeray/material/fr_material_glass.hpp"
#include "freezeray/material/fr_material_specular_glass.hpp"
#include "freezeray/material/fr_material_mirror.hpp"
#include "freezeray/material/fr_material_plastic.hpp"
#include "freezeray/light/fr_light_environment.hpp"

//-------------------------------------------//

#define WINDOW_W 1920
#define WINDOW_H 1080

//-------------------------------------------//

ExampleScene example_material_demo(const std::string& envMap)
{
	//create objects:
	//---------------
	std::shared_ptr<const fr::Mesh> planeMesh = fr::Mesh::from_unit_square();
	std::shared_ptr<const fr::Mesh> sphereMesh = fr::Mesh::from_unit_sphere(2, true);

	std::shared_ptr<fr::Texture<vec3>> whiteColorTex = std::make_shared<fr::TextureConstant<vec3>>(vec3(1.0f));
	std::shared_ptr<fr::Texture<vec3>> redColorTex = std::make_shared<fr::TextureConstant<vec3>>(vec3(1.0f, 0.0f, 0.0f));
	std::shared_ptr<fr::Texture<vec3>> yellowColorTex = std::make_shared<fr::TextureConstant<vec3>>(vec3(1.0f, 1.0f, 0.0f));
	std::shared_ptr<fr::Texture<float>> goldSphereRoughnessTex = std::make_shared<fr::TextureConstant<float>>(0.25f);
	std::shared_ptr<fr::Texture<float>> glassSphereRoughnessTex = std::make_shared<fr::TextureConstant<float>>(0.25f);
	std::shared_ptr<fr::Texture<float>> plasticSphereRoughnessTex = std::make_shared<fr::TextureConstant<float>>(0.5f);

	std::shared_ptr<const fr::Material> planeMat = std::make_shared<fr::MaterialSingleBXDF>("", std::make_shared<fr::BRDFLambertian>(), whiteColorTex);
	std::shared_ptr<const fr::Material> goldMat = std::make_shared<fr::MaterialMetal>("", fr::MetalType::GOLD, goldSphereRoughnessTex, goldSphereRoughnessTex);
	std::shared_ptr<const fr::Material> glassMat = std::make_shared<fr::MaterialGlass>("", 1.6f, whiteColorTex, whiteColorTex, glassSphereRoughnessTex, glassSphereRoughnessTex);
	std::shared_ptr<const fr::Material> mirrorMat = std::make_shared<fr::MaterialMirror>("", whiteColorTex);
	std::shared_ptr<const fr::Material> plasticMat = std::make_shared<fr::MaterialPlastic>("", redColorTex, yellowColorTex, plasticSphereRoughnessTex);

	std::shared_ptr<const fr::Object> planeObj = std::make_shared<fr::Object>(planeMesh, planeMat);
	mat4 planeTransform = translate(vec3(0.0f, -1.0f, 0.0f)) * scale(vec3(20.0f, 1.0f, 7.5f));

	std::shared_ptr<const fr::Object> goldSphereObj = std::make_shared<fr::Object>(sphereMesh, goldMat);
	mat4 goldSphereTransform = translate(vec3(-0.65f, -0.5f, 0.0f)) * scale(vec3(0.5f, 0.5f, 0.5f));

	std::shared_ptr<const fr::Object> glassSphereObj = std::make_shared<fr::Object>(sphereMesh, glassMat);
	mat4 glassSphereTransform = translate(vec3(0.65f, -0.5f, 0.0f)) * scale(vec3(0.5f, 0.5f, 0.5f));

	std::shared_ptr<const fr::Object> mirrorSphereObj = std::make_shared<fr::Object>(sphereMesh, mirrorMat);
	mat4 mirrorSphereTransform = translate(vec3(-1.95f, -0.5f, 0.0f)) * scale(vec3(0.5f, 0.5f, 0.5f));

	std::shared_ptr<const fr::Object> plasticSphereObj = std::make_shared<fr::Object>(sphereMesh, plasticMat);
	mat4 plasticSphereTransform = translate(vec3(1.95f, -0.5f, 0.0f)) * scale(vec3(0.5f, 0.5f, 0.5f));

	std::vector<fr::ObjectReference> objects = {
		{planeObj, planeTransform}, 
		{goldSphereObj, goldSphereTransform},
		{glassSphereObj, glassSphereTransform},
		{mirrorSphereObj, mirrorSphereTransform},
		{plasticSphereObj, plasticSphereTransform}
	};

	//create lights:
	//---------------
	std::shared_ptr<const fr::LightEnvironment> light = std::make_shared<fr::LightEnvironment>(envMap);
	std::vector<std::shared_ptr<const fr::Light>> lightList = {light};

	//define scene:
	//---------------
	std::shared_ptr<const fr::Scene> scene = std::make_shared<fr::Scene>(objects, lightList);

	//define camera:
	//---------------
	std::shared_ptr<const fr::Camera> camera = std::make_shared<fr::Camera>(
		vec3(0.0f, 0.0f, 3.0f), 
		normalize(vec3(0.0f, 0.0f, -1.0f)), 
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