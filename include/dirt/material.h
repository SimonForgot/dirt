/*
    This file is part of Dirt, the Dartmouth introductory ray tracer.

    Copyright (c) 2017-2019 by Wojciech Jarosz

    Dirt is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Dirt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <dirt/fwd.h>
#include <dirt/parser.h>
#include <stdlib.h>

//对于Metal这样的材质，没有合适的pdf，所以使用这个结构体作为flag
struct ScatterRecord
{
	Color3f attenuation;
	Vec3f scattered;
	bool isSpecular;
};

class onb
{
public:
	onb() {}
	inline Vec3f operator[](int i)const { return axis[i]; }
	Vec3f u()const { return axis[0]; }
	Vec3f v()const { return axis[1]; }
	Vec3f w()const { return axis[2]; }
	Vec3f local(float a, float b, float c)const
	{
		return a * u() + b * v() + c * w();
	}
	Vec3f local(const Vec3f& a)const
	{
		return a.x * u() + a.y * v() + a.z * w();
	}
	void build_form_w(const Vec3f&);
	Vec3f axis[3];
};
/// A base class used to represent surface material properties.
class Material
{
public:
	virtual ~Material() = default;

	/// Return a pointer to a global default material
	static shared_ptr<const Material> defaultMaterial();

	/**
	   \brief Compute the scattered direction scattered at a surface hitpoint.

	   The base Material does not scatter any light, so it simply returns false.
	   
	   \param  ray              incoming ray
	   \param  hit              the ray's intersection with the surface
	   \param  attenuation      how much the light should be attenuated
	   \param  scattered        the direction light should be scattered
	   \return bool             True if the surface scatters light
	 */
	virtual bool scatter(const Ray3f &ray, const HitInfo &hit, Color3f &attenuation, Ray3f &scattered) const
	{
		return false;
	}

	/**
	   Compute the amount of emitted light at the surface hitpoint.
	  
	   The base Material class does not emit light, so it simply returns black.
	  
	   \param  ray		the incoming ray
	   \param  hit		the ray's intersection with the surface
	   \return			the emitted color
	 */
	virtual Color3f emitted(const Ray3f &ray, const HitInfo &hit) const
	{
		return Color3f(0,0,0);
	}


	/**
        Return whether or not this Material is emissive.
		This is primarily used to create a global list of emitters for sampling.
     */
    virtual bool isEmissive() const {return false;}
	//brdf*cos
	virtual Color3f eval(const Vec3f& dirIn, const Vec3f& scattered, const HitInfo& hit)const
	{
		return Color3f(0.0f);
	}
	//返回由sample得到的光线方向的概率密度
	virtual float pdf(const Vec3f& dirIn, const Vec3f& scattered, const HitInfo& hit)const
	{
		return 0.0f;
	}
	//接受一个入射光，采样得到一个输出光方向。
	virtual bool sample(const Vec3f& dirIn, const HitInfo& hit, ScatterRecord& srec)const
	{
		return false;
	}
};


/// A perfectly diffuse (Lambertian) material
class Lambertian : public Material
{
public:
	Lambertian(const json & j = json::object());

	bool scatter(const Ray3f &ray, const HitInfo &hit, Color3f &attenuation, Ray3f &scattered) const override;
	Color3f eval(const Vec3f& dirIn, const Vec3f& scattered, const HitInfo& hit)const override;
	float pdf(const Vec3f& dirIn, const Vec3f& scattered, const HitInfo& hit)const override;
	bool sample(const Vec3f& dirIn, const HitInfo& hit, ScatterRecord& srec)const override;
	shared_ptr<const Texture> albedo ;     ///= Texture(0.8f)< The diffuse color (fraction of light that is reflected per color channel).
};

class BlendMaterial :public Material {
public:
	BlendMaterial(const json& j = json::object());
	shared_ptr<const Material>a, b;
	bool scatter(const Ray3f& ray, const HitInfo& hit, Color3f& attenuation, Ray3f& scattered) const override;
	float amount;
};
/// A metallic material that reflects light into the (potentially rough) mirror reflection direction.
class Metal : public Material
{
public:
	Metal(const json & j = json::object());

	bool scatter(const Ray3f &ray, const HitInfo &hit, Color3f &attenuation, Ray3f &scattered) const override;


	shared_ptr<const Texture> albedo;    ///< The reflective color (fraction of light that is reflected per color channel).
	float roughness = 0.f;              ///< A value between 0 and 1 indicating how smooth vs. rough the reflection should be.
};



/// A smooth dielectric surface that reflects and refracts light according to the specified index of refraction #ior
class Dielectric : public Material
{
public:
	Dielectric(const json & j = json::object());

	bool scatter(const Ray3f &ray, const HitInfo &hit, Color3f &attenuation, Ray3f &scattered) const override;


	float ior;      ///< The (relative) index of refraction of the material
};


/// A material that emits light equally in all directions from the front side of a surface.
class DiffuseLight : public Material
{
public:
	DiffuseLight(const json & j = json::object());

	/// Returns a constant Color3f if the ray hits the surface on the front side.
	Color3f emitted(const Ray3f &ray, const HitInfo &hit) const override;

    bool isEmissive() const override {return true;}

	Color3f emit;	///< The emissive color of the light
};

class Phong : public Material
{
	public:
		float pdf(const Vec3f& dirIn, const Vec3f& scattered, const HitInfo& hit) const;
		Color3f eval(const Vec3f& dirIn, const Vec3f& scattered, const HitInfo& hit)const;
		bool sample(const Vec3f& dirIn, const HitInfo& hit, ScatterRecord& srec)const;
		Phong(const json& j = json::object());
		shared_ptr<const Texture> albedo;
		float exponent;
};