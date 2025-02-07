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

	std::shared_ptr<fr::Scene> scene;
	std::shared_ptr<fr::Camera> camera;
};

//-------------------------------------------//

ExampleScene example_cornell_box();

//for scratch work
ExampleScene scratch();

#endif //#ifndef EXAMPLE_SCENE_H