#include "freezeray/fr_bsdf.hpp"

#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

BSDF::BSDF(const vec3& hitNormal) :
	m_numBxdfs(0)
{
	m_localToWorld = transform_between(FR_UP_DIR, hitNormal);
	m_worldToLocal = transform_between(hitNormal, FR_UP_DIR);
}

vec3 BSDF::f(const vec3& wiWorld, const vec3& woWorld, BXDFflags flags) const
{
	vec3 wi = m_worldToLocal * wiWorld;
	vec3 wo = m_worldToLocal * woWorld;
	
	vec3 f = 0.0f;
	for(uint32_t i = 0; i < m_numBxdfs; i++)
	{
		if((m_bxdfs[i]->get_flags() & flags) != m_bxdfs[i]->get_flags())
			continue;

		f = f + m_colors[i] * m_bxdfs[i]->f(wi, wo);
	}

	return f;
}

vec3 BSDF::sample_f(vec3& wiWorld, const vec3& woWorld, const vec3& u, float& pdf, BXDFflags flags, BXDFflags& sampledFlags) const
{
	//determine how many BXDFs to sample from:
	//---------------
	uint32_t numComponents = 0;
	for(uint32_t i = 0; i < m_numBxdfs; i++)
		if((m_bxdfs[i]->get_flags() & flags) == m_bxdfs[i]->get_flags())
			numComponents++;

	if(numComponents == 0)
	{
		sampledFlags = BXDFflags::NONE;
		pdf = 0.0f;
		return 0.0f;
	}

	//get BXDF to sample from:
	//---------------
	uint32_t component = (uint32_t)std::floorf(u.x * numComponents);
	if(component >= numComponents)
		component = numComponents - 1;

	//sample:
	//---------------
	vec3 wi;
	vec3 wo = m_worldToLocal * woWorld;

	vec3 f;

	uint32_t sampledIdx;
	for(uint32_t i = 0; i < m_numBxdfs; i++)
	{
		if((m_bxdfs[i]->get_flags() & flags) != m_bxdfs[i]->get_flags())
			continue;
		
		if(component == 0)
		{
			f = m_colors[i] * m_bxdfs[i]->sample_f(wi, wo, u.yz(), pdf);
			sampledFlags = m_bxdfs[i]->get_flags();

			sampledIdx = i;
			
			break;
		}

		component--;
	}

	if(pdf == 0.0f)
		return 0.0f;

	//add contrib from all components:
	//---------------
	for(uint32_t i = 0; i < m_numBxdfs; i++)
	{
		if(i == sampledIdx)
			continue;

		if((m_bxdfs[i]->get_flags() & flags) != m_bxdfs[i]->get_flags())
			continue;

		f = f + m_colors[i] * m_bxdfs[i]->f(wi, wo);
		pdf += m_bxdfs[i]->pdf(wi, wo);
	}

	pdf /= numComponents;

	//return:
	//---------------
	wiWorld = m_localToWorld * wi;
	return f;
}

float BSDF::pdf(const vec3& wiWorld, const vec3& woWorld, BXDFflags flags) const
{
	vec3 wi = m_worldToLocal * wiWorld;
	vec3 wo = m_worldToLocal * woWorld;

	uint32_t numComponents = 0;
	float pdf = 0.0f;
	for(uint32_t i = 0; i < m_numBxdfs; i++)
	{
		if((m_bxdfs[i]->get_flags() & flags) != m_bxdfs[i]->get_flags())
			continue;

		pdf += m_bxdfs[i]->pdf(wi, wo);
		numComponents++;
	}

	if(numComponents > 0)
		pdf /= numComponents;

	return pdf;
}

//-------------------------------------------//

BXDFflags BSDF::get_flags() const
{
	BXDFflags flags = BXDFflags::NONE;

	for(uint32_t i = 0; i < m_numBxdfs; i++)
		flags = flags | m_bxdfs[i]->get_flags();

	return flags;
}

bool BSDF::is_delta() const
{
	for(uint32_t i = 0; i < m_numBxdfs; i++)
		if((m_bxdfs[i]->get_flags() & BXDFflags::DELTA) == BXDFflags::NONE)
			return false;

	return true;
}

//-------------------------------------------//

void BSDF::add_bxdf(std::shared_ptr<const BXDF> bxdf, vec3 color)
{
	if(m_numBxdfs >= FR_BSDF_MAX_COMPONENTS)
		throw std::runtime_error("max BXDFs exceeded");

	m_bxdfs[m_numBxdfs] = bxdf;
	m_colors[m_numBxdfs] = color;
	m_numBxdfs++;
}

//-------------------------------------------//

mat3 BSDF::transform_between(const vec3& from, const vec3& to) const
{
	if(dot(from, to) >=  1.0f - FR_EPSILON)
		return mat3_identity();

	if(dot(from, to) <= -1.0f + FR_EPSILON)
	{
		mat3 mat = mat3();
		mat[0][0] = -1.0f;
		mat[1][1] = -1.0f;
		mat[2][2] = 1.0f;
	}

    vec3 v = cross(from, to);
    float c = dot(from, to);
    float k = 1.0f / (1.0f + c);
    
    mat3 rotation;

    rotation.v[0] = vec3(
        v.x * v.x * k + c,
        v.y * v.x * k + v.z,
        v.z * v.x * k - v.y
    );
    
    rotation.v[1] = vec3(
        v.x * v.y * k - v.z,
        v.y * v.y * k + c,
        v.z * v.y * k + v.x
    );
    
    rotation.v[2] = vec3(
        v.x * v.z * k + v.y,
        v.y * v.z * k - v.x,
        v.z * v.z * k + c
    );
    
    return rotation;
}

}; //namespace fr