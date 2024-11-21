#include "rurt/material.hpp"

#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

Material::Material(const std::string& name, bool delta, BXDFType type) :
	m_name(name), m_delta(delta), m_type(type)
{

}

const std::string& Material::get_name() const 
{ 
	return m_name; 	
};

void Material::set_name(const std::string& name) 
{ 
	m_name = name; 
};

bool Material::bsdf_is_delta() const
{
	return m_delta;
}

BXDFType Material::bsdf_type() const
{
	return m_type;
}

//-------------------------------------------//

template<typename... BXDFs>
vec3 Material::multi_bxdf_f(const vec3& normal, const vec3& wiWorld, const vec3& woWorld, const BXDFs&... bxdfs)
{
	//TODO: TEST THIS!!!

	//ensure all BXDFs are subclasses of BXDF:
	//---------------
    static_assert((std::is_base_of_v<BXDF, std::remove_reference_t<BXDFs>> && ...), 
                  "All BXDFs must be derived from the BXDF base class");

	//convert to local coordinates:
	//---------------
    vec3 wi = world_to_local(normal, wiWorld);
	vec3 wo = world_to_local(normal, woWorld);
    bool reflect = dot(wiWorld, normal) * dot(woWorld, normal) > 0.0f;

	//accumulate all bxdfs:
	//---------------
    vec3 f = vec3(0.0f);
	auto process_bxdf = [&](const auto&... args) {
		([&](const auto& bxdf) {
			BXDFType type = bxdf.type();
			if((reflect && type == BXDFType::TRANSMISSION) || (!reflect && type == BXDFType::REFLECTION))
				return;
			
			f = f + bxdf.f(wi, wo);
		}(args), ...);
	};

	process_bxdf(bxdfs...);

	//return:
	//---------------
    return f;
}

template<typename... BXDFs>
vec3 Material::multi_bxdf_sample_f(const vec3& normal, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf, const BXDFs&... bxdfs)
{
	//TODO: TEST THIS!!!

	//ensure all BXDFs are subclasses of BXDF:
	//---------------
    static_assert((std::is_base_of_v<BXDF, std::remove_reference_t<BXDFs>> && ...), 
                  "All BXDFs must be derived from the BXDF base class");

	//get BXDFs as vector:
	//---------------
	auto bxdfVec = { std::cref(bxdfs)... };
	if(bxdfVec.size() == 0)
	{
		wiWorld = local_to_world(normal, RURT_UP_DIR);
		pdf = 0.0f;
		return vec3(0.0f);
	}

	//sample from random BXDF:
	//---------------
	uint32_t idxToSample = std::min((uint32_t)std::floor(u.x * bxdfVec.size()), bxdfVec.size() - 1);
	const BXDF& toSample = bxdfVec[idxToSample].get();

	vec3 wi;
	vec3 wo = world_to_local(normal, woWorld);
	vec3 f = toSample.sample_f(wi, wo, pdf);

	wiWorld = local_to_world(normal, wi);

	//compute total f and pdf:
	//---------------
    bool reflect = dot(wiWorld, normal) * dot(woWorld, normal) > 0.0f;

	for(uint32_t i = 0; i < bxdfVec.size(); i++)
	{
		if(i == idxToSample)
			continue;

		const BXDF& bxdf = bxdfVec[i].get();

		pdf += bxdf.pdf(wi, wo);

		BXDFType type = bxdf.type();
		if((reflect && type == BXDFType::TRANSMISSION) || (!reflect && type == BXDFType::REFLECTION))
			continue;

		f = f + bxdf.f(wi, wo);
	}

	pdf /= (float)bxdfVec.size();

	//return:
	//---------------
	return f;
}

template<typename... BXDFs>
float Material::multi_bxdf_pdf(const vec3& normal, const vec3& wiWorld, const vec3& woWorld, const BXDFs&... bxdfs)
{
	//TODO: TEST THIS!!!

	//ensure all BXDFs are subclasses of BXDF:
	//---------------
    static_assert((std::is_base_of_v<BXDF, std::remove_reference_t<BXDFs>> && ...), 
                  "All BXDFs must be derived from the BXDF base class");

	//convert to local coordinates:
	//---------------
    vec3 wi = world_to_local(normal, wiWorld);
	vec3 wo = world_to_local(normal, woWorld);
    bool reflect = dot(wiWorld, normal) * dot(woWorld, normal) > 0.0f;

	//accumulate all bxdfs:
	//---------------
    float pdf;
	uint32_t pdfCount = 0;
	auto process_bxdf = [&](const auto&... args) {
		([&](const auto& bxdf) {
			pdfCount++;
			pdf += bxdf.pdf(wi, wo);
		}(args), ...);
	};

	process_bxdf(bxdfs...);

	//return:
	//---------------
	if(pdfCount == 0)
		return 0.0f;
	else
    	return pdf / (float)pdfCount;
}

//-------------------------------------------//

vec3 Material::world_to_local(const vec3& normal, const vec3& v)
{
	return transform_between(normal, RURT_UP_DIR) * v;
}

vec3 Material::local_to_world(const vec3& normal, const vec3& v)
{
	return transform_between(RURT_UP_DIR, normal) * v;
}

mat3 Material::transform_between(const vec3& from, const vec3& to) 
{
	//equal
	if(dot(from, to) >=  1.0f - RURT_EPSILON)
		return mat3_identity();

	//opposite
	if(dot(from, to) <= -1.0f + RURT_EPSILON)
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

}; //namespace rurt