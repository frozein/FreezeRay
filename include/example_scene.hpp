/* example_scene.hpp
 *
 * contains declarations for the various built-in examples
 */

#ifndef EXAMPLE_SCENE_H
#define EXAMPLE_SCENE_H

#include "freezeray/fr_scene.hpp"
#include "freezeray/fr_camera.hpp"

//-------------------------------------------//

struct ExampleScene
{
	uint32_t windowWidth;
	uint32_t windowHeight;

	std::shared_ptr<const fr::Scene> scene;
	std::shared_ptr<const fr::Camera> camera;
};

//-------------------------------------------//

//spheres with different materials lit by an environment map
ExampleScene example_material_demo(const std::string& envMap);

//cornell box scene
ExampleScene example_cornell_box();

//sponza palace scene
ExampleScene example_sponza();

//san miguel palace scene
ExampleScene example_san_miguel();

#endif //#ifndef EXAMPLE_SCENE_H