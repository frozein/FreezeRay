#include "freezeray/renderer/fr_renderer_metropolis.hpp"
#include "freezeray/fr_globals.hpp"
#include "freezeray/fr_distribution.hpp"
#include "../fr_thread_pool.hpp"

//-------------------------------------------//

namespace fr
{

//-------------------------------------------//

static float inv_erf(float x)
{
	x = std::min(std::max(x, -.99999f), .99999f);
	float w = -std::log((1 - x) * (1 + x));
	
	float p;
	if(w < 5) 
	{
		w = w - 2.5f;
		p =  2.81022636e-08f;
		p =  3.43273939e-07f + p * w;
		p = -3.5233877e-06f  + p * w;
		p = -4.39150654e-06f + p * w;
		p =  0.00021858087f  + p * w;
		p = -0.00125372503f  + p * w;
		p = -0.00417768164f  + p * w;
		p =  0.246640727f    + p * w;
		p =  1.50140941f     + p * w;
	} 
	else 
	{
		w = std::sqrt(w) - 3;
		p = -0.000200214257f;
		p =  0.000100950558f + p * w;
		p =  0.00134934322f  + p * w;
		p = -0.00367342844f  + p * w;
		p =  0.00573950773f  + p * w;
		p = -0.0076224613f   + p * w;
		p =  0.00943887047f  + p * w;
		p =  1.00167406f     + p * w;
		p =  2.83297682f     + p * w;
	}

    return p * x;
}

RendererMetropolis::RendererMetropolis(std::shared_ptr<const Camera> cam, uint32_t imageW, uint32_t imageH, uint32_t maxDepth, 
                                       uint32_t mutationsPerPixel, uint32_t numChains, uint32_t numBootstrap,
                                       float largeStepProb, float smallStepSize) :
	RendererBidirectional(cam, imageW, imageH, maxDepth, 0, true, true), 
	m_mutationsPerPixel(mutationsPerPixel),
	m_numChains(numChains),
	m_numBootstrap(numBootstrap),
	m_largeStepProb(largeStepProb),
	m_smallStepSize(smallStepSize)
{

}

RendererMetropolis::~RendererMetropolis()
{

}

void RendererMetropolis::render(const std::shared_ptr<const Scene>& scene,
                                std::function<void(uint32_t, uint32_t, vec3)> writePixel, 
                                std::function<void(float)> display, uint32_t displayFrequency)
{
	uint64_t numThreads = std::thread::hardware_concurrency();

	//generate bootstrapping samples:
	//---------------
	std::vector<std::pair<uint32_t, float>> bootstrapSamples(m_numBootstrap * (m_maxDepth + 1));

	std::queue<uint32_t> bootstrapWorkGroups;
	for(uint32_t i = 0; i < m_numBootstrap; i++)
		bootstrapWorkGroups.push(i);

	auto processBootstrapWorkGroup = [&](uint32_t idx) {
		for(uint32_t i = 0; i <= m_maxDepth; i++)
		{
			uint32_t seed = idx * (m_maxDepth + 1) + i;
			std::shared_ptr<PRNGMetropolis<3>> prng = std::make_shared<PRNGMetropolis<3>>(seed, m_largeStepProb, m_smallStepSize);

			vec2 uv;
			bootstrapSamples[seed] = { seed, luminance(l(prng, scene, i, uv)) };
		}
	};

	{
		ThreadPool<uint32_t> bootstrapPool(numThreads, bootstrapWorkGroups, processBootstrapWorkGroup);
	}

	DistributionDiscrete<uint32_t> bootstrapDistribution(bootstrapSamples);

	float bootstrapInt = bootstrapDistribution.get_total_density() * (m_maxDepth + 1);
	bootstrapInt /= (float)bootstrapSamples.size();

	//generate temporary image:
	//---------------
	struct AtomicVec3
	{
		std::atomic<float> x = 0.0f;
		std::atomic<float> y = 0.0f;
		std::atomic<float> z = 0.0f;
	};

	std::shared_ptr<AtomicVec3[]> image = std::make_shared<AtomicVec3[]>(m_imageW * m_imageH);

	auto addSample = [image, this](vec2 uv, vec3 sample) {
		uint32_t x = std::min((uint32_t)(uv.x * m_imageW + 0.5f), m_imageW - 1);
		uint32_t y = std::min((uint32_t)(uv.y * m_imageH + 0.5f), m_imageH - 1);

		AtomicVec3& pixel = image[x + m_imageW * y];
		pixel.x.fetch_add(sample.x);
		pixel.y.fetch_add(sample.y);
		pixel.z.fetch_add(sample.z);
	};

	//run markov chains:
	//---------------
	uint64_t totalMutations = (uint64_t)m_imageW * (uint64_t)m_imageH * (uint64_t)m_mutationsPerPixel;

	std::queue<uint32_t> markovChains;
	for(uint32_t i = 0; i < m_numChains; i++)
		markovChains.push(i);

	auto processMarkovChain = [&](uint32_t idx) {
		uint64_t numMutations = totalMutations / m_numChains;
		if(idx < totalMutations % m_numChains)
			numMutations++;

		PRNG prng(idx);
		float pdf;
		uint32_t bootstrapIdx = bootstrapDistribution.sample(prng.randf(), pdf);
		uint32_t depth = bootstrapIdx % (m_maxDepth + 1);

		std::shared_ptr<PRNGMetropolis<3>> chain = std::make_shared<PRNGMetropolis<3>>(bootstrapIdx, m_largeStepProb, m_smallStepSize);

		vec2 uvCurrent;
		vec3 lCurrent = l(chain, scene, depth, uvCurrent);

		for(uint64_t i = 0; i < numMutations; i++)
		{
			chain->start_iteration();

			vec2 uvProposed;
			vec3 lProposed = l(chain, scene, depth, uvProposed);

			float accept = std::min(luminance(lProposed) / luminance(lCurrent), 1.0f);

			if(accept > 0)
				addSample(uvProposed, lProposed * accept / luminance(lProposed));
			addSample(uvCurrent, lCurrent * (1.0f - accept) / luminance(lCurrent));

			if(prng.randf() < accept)
			{
				uvCurrent = uvProposed;
				lCurrent = lProposed;

				chain->end_iteration(true);
			}
			else
				chain->end_iteration(false);
		}
	};

	ThreadPool<uint32_t> markovChainPool(numThreads, markovChains, processMarkovChain);

	//display periodically:
	//---------------
	auto writeAndDisplay = [&]() {
		for(uint32_t y = 0; y < m_imageH; y++)
		for(uint32_t x = 0; x < m_imageW; x++)
		{
			AtomicVec3& pixelAtomic = image[x + y * m_imageW];

			//it's certainly possible for us to read only a partial update to the overall color,
			//but being just a sample or 2 off isn't a big deal visually

			vec3 pixel;
			pixel.x = pixelAtomic.x.load();
			pixel.y = pixelAtomic.y.load();
			pixel.z = pixelAtomic.z.load();

			pixel = pixel * bootstrapInt;
			pixel = pixel / (float)m_mutationsPerPixel;

			pixel.r = std::max(std::min(pixel.r, 1.0f), 0.0f);
			pixel.g = std::max(std::min(pixel.g, 1.0f), 0.0f);
			pixel.b = std::max(std::min(pixel.b, 1.0f), 0.0f);
			pixel = linear_to_srgb(pixel);

			writePixel(x, y, pixel);
		}

		display(markovChainPool.progress());
	};

	while(!markovChainPool.complete())
	{
		std::this_thread::sleep_for(std::chrono::seconds(displayFrequency));

		writeAndDisplay();
	}

	//display final result:
	//---------------
	writeAndDisplay();
}

vec3 RendererMetropolis::l(const std::shared_ptr<PRNGMetropolis<3>>& prng, const std::shared_ptr<const Scene>& scene, uint32_t depth, vec2& uv) const
{
	//choose strategy:
	//---------------
	prng->start_stream(0);

	uint32_t numCam;
	uint32_t numLight;
	uint32_t numStrategies;
	if(depth <= 1) //TODO: allow numCam == 1
	{
		numStrategies = 1;
		numLight = 0;
		numCam = 2;
	}
	else
	{
		numStrategies = depth;
		numLight = std::min((uint32_t)(prng->randf() * (numStrategies - 1)), numStrategies - 2);
		numCam = numStrategies - numLight;
	}

	//sample point on image:
	//---------------
	uv = prng->rand2f();
	Ray ray = get_camera_ray(uv);

	//generate subpaths:
	//---------------
	std::vector<PathVertex> cameraSubpath = trace_camera_subpath(prng, scene, ray, numCam);
	if(cameraSubpath.size() != numCam)
		return vec3(0.0f);

	prng->start_stream(1);

	std::vector<PathVertex> lightSubpath = trace_light_subpath(prng, scene, ray, numLight);
	if(lightSubpath.size() != numLight)
		return vec3(0.0f);

	//connect:
	//---------------
	prng->start_stream(2);

	PathVertex sampled;
	vec3 l = connect_subpaths(scene, prng, cameraSubpath, lightSubpath, numCam, numLight, sampled);
	if(l != vec3(0.0f))
		l =  l * mis_weight(scene, cameraSubpath, lightSubpath, sampled, numLight, numCam);

	return l * (float)numStrategies;
}

//-------------------------------------------//

template <uint32_t N>
RendererMetropolis::PRNGMetropolis<N>::PRNGMetropolis(uint32_t seed, float largeStepProb, float smallStepSize) :
	m_largeStepProb(largeStepProb),
	m_smallStepSize(smallStepSize),
	m_prng(seed),
	m_streamIdx(0),
	m_streamPosition(0),
	m_curIter(0),
	m_curIterLargeStep(true),
	m_lastLargeStepIter(0)
{

}

template <uint32_t N>
float RendererMetropolis::PRNGMetropolis<N>::randf()
{
	if(m_streamPosition >= m_samples[m_streamIdx].size())
		m_samples[m_streamIdx].resize(m_streamPosition + 1);

	Sample& sample = m_samples[m_streamIdx][m_streamPosition];
	if(sample.lastModified < m_lastLargeStepIter)
	{
		sample.value = m_prng.randf();
		sample.lastModified = m_lastLargeStepIter;
	}

	sample.backupVal = sample.value;
	sample.backupLastModified = sample.lastModified;

	if(m_curIterLargeStep)
		sample.value = m_prng.randf();
	else
	{
		float normal = FR_SQRT_2 * inv_erf(m_prng.randf() * 2.0f - 1.0f);
		float stdDev = m_smallStepSize * std::sqrtf((float)m_curIter - (float)sample.lastModified);

		sample.value += stdDev * normal;
		sample.value -= std::floorf(sample.value);
	}

	sample.lastModified = m_curIter;

	m_streamPosition++;
	return sample.value;
}

template <uint32_t N>
void RendererMetropolis::PRNGMetropolis<N>::start_iteration()
{
	m_curIter++;
	m_curIterLargeStep = m_prng.randf() < m_largeStepProb;
}

template <uint32_t N>
void RendererMetropolis::PRNGMetropolis<N>::end_iteration(bool accept)
{
	if(accept)
	{
		if(m_curIterLargeStep)
			m_lastLargeStepIter = m_curIter;
	}
	else
	{
		for(uint32_t i = 0; i < N; i++)
		for(uint32_t j = 0; j < m_samples[i].size(); j++)
		{
			if(m_samples[i][j].lastModified == m_curIter)
			{
				m_samples[i][j].value = m_samples[i][j].backupVal;
				m_samples[i][j].lastModified = m_samples[i][j].backupLastModified;
			}
		}

		m_curIter--;
	}
}

template <uint32_t N>
void RendererMetropolis::PRNGMetropolis<N>::start_stream(uint32_t idx)
{
	if(idx >= N)
		throw std::invalid_argument("Stream index out of bounds");

	m_streamIdx = idx;
	m_streamPosition = 0;
}

}; //namespace fr