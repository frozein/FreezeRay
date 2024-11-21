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

vec3 Material::world_to_local(const vec3& normal, const vec3& v)
{
	return transform_between(normal, RURT_UP_DIR) * v;
}

void Material::world_to_local(const vec3& normal, const vec3& wiWorld, const vec3& woWorld, vec3& wi, vec3& wo)
{
	mat3 transform = transform_between(normal, RURT_UP_DIR);
	wi = transform * wiWorld;
	wo = transform * woWorld;
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