/* fr_renderer_metropolis.hpp
 *
 * contains the definition of a renderer that uses
 * the metropolis-hastings sampling algorithm for integration
 */

#ifndef FR_RENDERER_METROPOLIS_H
#define FR_RENDERER_METROPOLIS_H

#include "fr_renderer_bidirectional.hpp"

//-------------------------------------------//

namespace fr
{

class RendererMetropolis : public RendererBidirectional
{
public:
	RendererMetropolis(std::shared_ptr<const Camera> cam, uint32_t imageW, uint32_t imageH, uint32_t maxDepth, 
	                   uint32_t mutationsPerPixel, uint32_t numChains = 128, uint32_t numBootstrap = 16384,
	                   float largeStepProb = 0.3f, float smallStepSize = 0.01f);
	~RendererMetropolis();

	void render(const std::shared_ptr<const Scene>& scene,
	            std::function<void(uint32_t x, uint32_t y, vec3 color)> writePixel, 
	            std::function<void(float progress)> display, uint32_t displayFrequency = 1) override;

private:
	template <uint32_t N>
	class PRNGMetropolis : public PRNG
	{
	public:
		PRNGMetropolis(uint32_t seed, float largeStepProb, float smallStepSize);

		float randf() override;

		void start_iteration();
		void end_iteration(bool accept);

		void start_stream(uint32_t idx);

	private:
		struct Sample
		{
			float value;
			uint32_t lastModified = 0;

			float backupVal;
			uint32_t backupLastModified;
		};

		uint32_t m_streamIdx;
		std::vector<Sample> m_samples[N];
		uint32_t m_streamPosition;
		
		float m_largeStepProb;
		float m_smallStepSize;

		uint32_t m_lastLargeStepIter;
		uint32_t m_curIter;
		bool m_curIterLargeStep;

		PRNG m_prng;
	};

	uint32_t m_mutationsPerPixel;
	uint32_t m_numChains;
	uint32_t m_numBootstrap;

	float m_largeStepProb;
	float m_smallStepSize;

	vec3 l(const std::shared_ptr<PRNGMetropolis<3>>& prng, const std::shared_ptr<const Scene>& scene, uint32_t depth, vec2& uv) const;
};

}; //namespace fr

#endif //#ifndef FR_RENDERER_METROPOLIS_H