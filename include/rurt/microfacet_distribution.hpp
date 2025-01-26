/* microfacet_distribution.hpp
 *
 * contains the definition of the microfacet distribution class,
 * which represents preoperties of a microfacet-based surface
 */

#ifndef RURT_MICROFACET_DISTRIBUTION_H
#define RURT_MICROFACET_DISTRIBUTION_H

#include "rurt/globals.hpp"
#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace rurt
{

class MicrofacetDistribution
{
public:
	virtual float distribution(const vec3& w) const = 0;
	virtual vec3 sample_distribution(const vec3& w, const vec2& u) const = 0;

	inline float proportion_visible(const vec3& w) const
	{
		return 1.0f / (1.0f + invisible_masked_proportion(w));
	}

	inline float proportion_visible(const vec3& wi, const vec3& wo) const
	{
		return 1.0f / (1.0f + invisible_masked_proportion(wi) + invisible_masked_proportion(wo));
	}

	inline float pdf(const vec3& wo, const vec3& wh) const
	{
		return distribution(wh) * std::abs(cos_theta(wh));
	}

protected:
	virtual float invisible_masked_proportion(const vec3& w) const = 0;
};

}; //namespace rurt

#endif //#ifndef RURT_MICROFACET_DISTRIBUTION_H