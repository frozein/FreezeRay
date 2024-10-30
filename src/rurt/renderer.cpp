#include "renderer.hpp"

#include "ray.hpp"

//-------------------------------------------//

namespace rurt
{

Renderer::Renderer(std::shared_ptr<const Scene> scene, std::shared_ptr<const Camera> cam, uint32_t imageW, uint32_t imageH) : 
	m_scene(scene),
	m_cam(cam), 
	m_imageW(imageW), 
	m_imageH(imageH)
{
	m_camInvView = inverse(m_cam->view());
	m_camInvProj = inverse(m_cam->proj());
}

Renderer::~Renderer()
{

}

void Renderer::draw_scanline(uint32_t y, uint32_t* buf)
{
	//loop over every pixel in scanline:
	//---------------
	for(uint32_t x = 0; x < m_imageW; x++)
	{
		//generate ray for current pixel:
		//---------------
		vec2 pixelCenter = vec2((float)x, (float)y) + vec2(0.5f);
		vec2 pixelUV = pixelCenter / vec2((float)m_imageW, (float)m_imageH);
		vec2 pixelD = pixelUV * 2.0f - vec2(1.0f);

		vec4 rayOrig = m_camInvView * vec4(0.0f, 0.0f, 0.0f, 1.0f);
		vec4 rayTarget = m_camInvProj * vec4(pixelD.x, pixelD.y, 1.0f, 1.0f);
		vec4 rayDir = m_camInvView * vec4(normalize(rayTarget.xyz()), 0.0f);

		//having a normalized ray dir is useful, so we ensure its always normalized
		Ray ray = Ray(rayOrig.xyz(), normalize(rayDir.xyz()));

		//cast ray against scene:
		//---------------
		vec3 color = m_scene->intersect(ray);

		//write color to given buffer:
		//---------------
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = UINT8_MAX; //each pixel fully opaque

		uint32_t writeColor = ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | (uint32_t)a;
		buf[x] = writeColor;
	}
}

}; //namespace rurt