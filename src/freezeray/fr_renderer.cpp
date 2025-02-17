#include "freezeray/fr_renderer.hpp"
#include "freezeray/fr_ray.hpp"
#include "freezeray/fr_globals.hpp"
#include <math.h>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>

//-------------------------------------------//

namespace fr
{

#define WORKGROUP_SIZE 128

//-------------------------------------------//

struct WorkGroup
{
	uint32_t startX;
	uint32_t startY;
	uint32_t endX;
	uint32_t endY;
};

class ThreadPool
{
public:
	ThreadPool(uint64_t numWorkers, const std::queue<WorkGroup>& workGroups, std::function<void(const WorkGroup&)> process) :
		m_workGroups(workGroups), m_process(process)
	{
		m_activeThreads = numWorkers;

		for(uint64_t i = 0; i < numWorkers; i++)
		{
			m_workers.emplace_back(
				[this] {
					while(true)
					{
						WorkGroup group;

						{
							std::unique_lock<std::mutex> lock(m_queueMutex);
							
							if(m_workGroups.empty())
							{
								m_activeThreads.fetch_sub(1);
								return;
							}

							group = m_workGroups.front();
							m_workGroups.pop();
						}

						m_process(group);
					}
				}
			);
		}
	}

	bool complete()
	{
		return m_activeThreads.load() == 0;
	}

	~ThreadPool()
	{
		for(uint64_t i = 0; i < m_workers.size(); i++)
			m_workers[i].join();
	}

private:
	std::vector<std::thread> m_workers;
	std::atomic<uint64_t> m_activeThreads = 0;

	std::queue<WorkGroup> m_workGroups;
	std::mutex m_queueMutex;
	std::function<void(const WorkGroup&)> m_process;
};

//-------------------------------------------//

Renderer::Renderer(const std::shared_ptr<const Camera>& cam, uint32_t imageW, uint32_t imageH) : 
	m_cam(cam),
	m_camInvView(inverse(m_cam->view())),
	m_camInvProj(inverse(m_cam->proj())),
	m_imageW(imageW), 
	m_imageH(imageH)
{

}

Renderer::~Renderer()
{

}

void Renderer::render(const std::shared_ptr<const Scene>& scene, std::function<void(uint32_t, uint32_t, vec3)> writePixel, std::function<void()> display, uint32_t displayFrequency)
{
	//generate workgroups:
	//---------------
	std::queue<WorkGroup> workGroups;

	uint32_t xDivs = (m_imageW + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
	uint32_t yDivs = (m_imageH + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;

	uint32_t xBaseSize = m_imageW / xDivs;
	uint32_t xRemain   = m_imageW % xDivs;

	uint32_t yBaseSize = m_imageH / yDivs;
	uint32_t yRemain   = m_imageH % yDivs;

	for(uint32_t y = 0; y < yDivs; y++)
	for(uint32_t x = 0; x < xDivs; x++)
	{
		uint32_t idx = x + xDivs * y;

		uint32_t xMin = x * xBaseSize + std::min(x, xRemain);
		uint32_t yMin = y * yBaseSize + std::min(y, yRemain);

		uint32_t xMax = xMin + xBaseSize + (x < xRemain ? 1 : 0) - 1;
		uint32_t yMax = yMin + yBaseSize + (y < yRemain ? 1 : 0) - 1;

		workGroups.push( {xMin, yMin, xMax, yMax} );
	}

	//define processing func:
	//---------------
	bool shouldDisplay = false;
	std::condition_variable displayCV;
	std::condition_variable displayCVmain;
	std::mutex displayMutex;

	std::atomic<uint64_t> threadsRendering = 0;

	auto processWorkgroup = [&](const WorkGroup& group) {
		for(int32_t y = group.endY; y >= (int32_t)group.startY; y--)
		{
			//wait for main thread to display
			{
				std::unique_lock<std::mutex> lock(displayMutex);
				displayCV.wait(lock, [&]{ return !shouldDisplay; });
			}

			//increment threads rendering
			threadsRendering.fetch_add(1);

			for(uint32_t x = group.startX; x <= group.endX; x++)
			{
				//generate ray for current pixel
				Ray cameraRay = get_camera_ray(x, (uint32_t)y);
				Ray cameraRayDifferentialX = get_camera_ray(x + 1, (uint32_t)y);
				Ray cameraRayDifferentialY = get_camera_ray(x, (uint32_t)y + 1);
	
				cameraRay = Ray(cameraRay, cameraRayDifferentialX, cameraRayDifferentialY);
	
				//get color of pixel
				vec3 color = li(scene, cameraRay);
	
				//write color to given buffer
				color.r = std::max(std::min(color.r, 1.0f), 0.0f);
				color.g = std::max(std::min(color.g, 1.0f), 0.0f);
				color.b = std::max(std::min(color.b, 1.0f), 0.0f);
				color = linear_to_srgb(color);
	
				writePixel(x, (uint32_t)y, color);
			}
	
			//decrement threads rendering, notify main thread
			if(threadsRendering.fetch_sub(1) == 1)
				displayCVmain.notify_one();
		}
	};

	//start thread groups, display periodically:
	//---------------
	uint64_t numThreads = std::thread::hardware_concurrency();
	ThreadPool pool(numThreads, workGroups, processWorkgroup);

	while(!pool.complete())
	{
		std::this_thread::sleep_for(std::chrono::seconds(displayFrequency));

		shouldDisplay = true;

		std::unique_lock<std::mutex> lock(displayMutex);
		displayCVmain.wait(lock, [&]{ return threadsRendering.load() == 0; });

		display();

		shouldDisplay = false;
		displayCV.notify_all();
	}

	//display final result:
	//---------------
	display();
}

//-------------------------------------------//

vec3 Renderer::sample_one_light(const std::shared_ptr<const Scene>& scene, const IntersectionInfo& hitInfo, const vec3& wo) const
{
	//choose random light index:
	//---------------
	uint32_t numLights = (uint32_t)scene->get_lights().size();
	if(numLights == 0)
		return vec3(0.0f);

	uint32_t lightIdx = rand() % numLights;

	//sample li:
	//---------------
	vec3 u = vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
	vec3 wi;
	VisibilityTestInfo visInfo;
	float pdf;

	const std::shared_ptr<const Light>& light = scene->get_lights()[lightIdx];
	vec3 li = light->sample_li(hitInfo, u, wi, visInfo, pdf);

	if(pdf == 0.0f)
		return vec3(0.0f);

	pdf /= (float)numLights;

	//compute bsdf f:
	//---------------
	vec3 f = hitInfo.material->bsdf_f(hitInfo, wi, wo) * std::abs(dot(wi, hitInfo.worldNormal));

	//trace visibility ray, return:
	//---------------
	if(trace_visibility_ray(scene, hitInfo, wi, wo, visInfo))
		return f * li / pdf;
	else
		return vec3(0.0f);
}

vec3 Renderer::sample_one_light_mis(const std::shared_ptr<const Scene>& scene, const IntersectionInfo& hitInfo, const vec3& wo) const
{
	//choose random light index:
	//---------------
	uint32_t numLights = (uint32_t)scene->get_lights().size();
	if(numLights == 0)
		return vec3(0.0f);

	uint32_t lightIdx = rand() % numLights;
	const std::shared_ptr<const Light>& light = scene->get_lights()[lightIdx];

	float pdfLightSample = 1.0f / (float)numLights;

	//sampling vars:
	//---------------
	vec3 li;
	vec3 wi;
	vec3 f;
	float pdfLight;
	float pdfScattering;

	vec3 ld = vec3(0.0f);

	//sample from light:
	//---------------
	VisibilityTestInfo visInfo;
	vec3 uLight = vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
	li = light->sample_li(hitInfo, uLight, wi, visInfo, pdfLight);

	f = hitInfo.material->bsdf_f(hitInfo, wi, wo) * std::abs(dot(wi, hitInfo.worldNormal));
	pdfScattering = hitInfo.material->bsdf_pdf(hitInfo, wi, wo);

	if(!trace_visibility_ray(scene, hitInfo, wi, wo, visInfo))
		li = vec3(0.0f);

	if(pdfLight > 0.0f && pdfScattering > 0.0f)
	{
		if(light->is_delta())
			ld = ld + li * f / pdfLight;
		else
		{
			float weight = mis_power_heuristic(1, pdfLight, 1, pdfScattering);
			ld = ld + weight * (li * f / pdfLight);
		}
	}

	//sample from bsdf:
	//---------------
	if(!light->is_delta())
	{
		//sample from bsdf
		vec3 uBsdf = vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
		f = hitInfo.material->bsdf_sample_f(hitInfo, wi, wo, uBsdf, pdfScattering);
		f = f * std::abs(dot(wi, hitInfo.worldNormal));

		//get light pdf
		pdfLight = light->pdf_li(hitInfo, wi);

		if(pdfLight > 0.0f && pdfScattering > 0.0f)
		{
			//compute mis weight
			float weight = mis_power_heuristic(1, pdfScattering, 1, pdfLight);

			//trace ray, get light contrib
			vec3 rayPos = hitInfo.worldPos;
			if(dot(wi, hitInfo.worldNormal) > 0.0f)
				rayPos = rayPos + hitInfo.worldNormal * FR_EPSILON;
			else
				rayPos = rayPos - hitInfo.worldNormal * FR_EPSILON;

			Ray ray(rayPos, wi);
			IntersectionInfo hitInfoBsdf;
			
			if(scene->intersect(ray, hitInfoBsdf))
			{
				if(hitInfoBsdf.light.get() == light.get())
					li = light->le(hitInfoBsdf, -1.0f * wi);
				else
					li = vec3(0.0f);
			}
			else
			{
				if(light->is_infinite())
					li = light->le(hitInfoBsdf, -1.0f * wi);
				else
					li = vec3(0.0f);
			}

			//add light contrib
			ld = ld + weight * (li * f / pdfScattering);
		}
	}

	//return:
	//---------------
	return ld / pdfLightSample;
}

bool Renderer::trace_visibility_ray(const std::shared_ptr<const Scene>& scene, const IntersectionInfo& initialHitInfo, const vec3& wi, const vec3& wo, const VisibilityTestInfo& visInfo) const
{
	vec3 rayPos = initialHitInfo.worldPos;
	if(dot(wo, initialHitInfo.worldNormal) > 0.0f)
		rayPos = rayPos + initialHitInfo.worldNormal * FR_EPSILON;
	else
		rayPos = rayPos - initialHitInfo.worldNormal * FR_EPSILON;

	Ray ray;
	if(visInfo.infinite)
		ray = Ray(rayPos, wi);
	else
	{
		vec3 toEnd = visInfo.endPos - rayPos;
		ray = Ray(rayPos, toEnd);
	}

	IntersectionInfo hitInfo;
	bool hit = scene->intersect(ray, hitInfo);

	if(visInfo.infinite)
		return !hit;
	else
	{
		if(!hit)
			return true;

		float minDist = distance(initialHitInfo.worldPos, visInfo.endPos);
		float dist = distance(initialHitInfo.worldPos, hitInfo.worldPos);

		return (dist + FR_EPSILON) > minDist;
	}
}

Ray Renderer::get_camera_ray(uint32_t x, uint32_t y) const
{
	vec2 pixelCenter = vec2((float)x, (float)y) + vec2(0.5f);
	vec2 pixelUV = pixelCenter / vec2((float)m_imageW, (float)m_imageH);
	vec2 pixelD = pixelUV * 2.0f - vec2(1.0f);

	vec4 rayOrig = m_camInvView * vec4(0.0f, 0.0f, 0.0f, 1.0f);
	vec4 rayTarget = m_camInvProj * vec4(pixelD.x, pixelD.y, 1.0f, 1.0f);
	vec4 rayDir = m_camInvView * vec4(normalize(rayTarget.xyz()), 0.0f);

	return Ray(rayOrig.xyz(), normalize(rayDir.xyz()));	
}

vec3 Renderer::random_dir_sphere()
{
	vec3 randUnitSphere;
	while(true)
	{
		randUnitSphere.x = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
		randUnitSphere.y = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
		randUnitSphere.z = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;

		float lenSqr = dot(randUnitSphere, randUnitSphere);
		if(lenSqr > FR_EPSILON && lenSqr <= 1.0f)
		{
			randUnitSphere = randUnitSphere / std::sqrtf(lenSqr);
			break;
		}
	}

	return randUnitSphere;
}

vec3 Renderer::random_dir_hemisphere(const vec3& normal)
{
	vec3 randUnitSphere;
	while(true)
	{
		randUnitSphere.x = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
		randUnitSphere.y = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
		randUnitSphere.z = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;

		float lenSqr = dot(randUnitSphere, randUnitSphere);
		if(lenSqr > FR_EPSILON && lenSqr <= 1.0f)
		{
			randUnitSphere = randUnitSphere / std::sqrtf(lenSqr);
			break;
		}
	}

	if(dot(randUnitSphere, normal) < 0.0f)
		randUnitSphere = randUnitSphere * -1.0f;

	return randUnitSphere;
}

float Renderer::mis_power_heuristic(uint32_t nf, float pdff, uint32_t ng, float pdfg)
{
	float f = nf * pdff;
	float g = ng * pdfg;

	return f * f / (f * f + g * g);
}

}; //namespace fr