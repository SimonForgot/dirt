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

#include <dirt/material.h>
#include <dirt/parser.h>
#include <dirt/scene.h>
#include <dirt/surface.h>
#include<iostream>

namespace
{

	auto g_defaultMaterial = nullptr;/*make_shared<Lambertian>(json{
		{"albedo",
		{{"type","constant"},{"color",0.8}}
		}
		});*/
} // namespace


shared_ptr<const Material> Material::defaultMaterial()
{
	//std::cout << "defaultMaterial: ";
	return g_defaultMaterial;
}


Lambertian::Lambertian(const json & j)
{
	//cout << j.dump();
	//std::cout << j.at("albedo").dump() << std::endl;

	albedo = parseTexture(j.at("albedo"));
}
Color3f Lambertian::eval(const Vec3f& dirIn, const Vec3f& scattered, const HitInfo& hit) const
{
	return albedo->value(hit)* max(0.f, dot(scattered, hit.gn)) / M_PI;
}

float Lambertian::pdf(const Vec3f& dirIn, const Vec3f& scattered, const HitInfo& hit) const
{
	return max(0.f, dot(scattered, hit.gn)) / M_PI;
}
Vec3f   randomCosineDirection()
{
	float phi = randf() * 2 * M_PI;
	float cosTheta = sqrt(randf());
	float 	sinTheta = sqrt(1 - cosTheta * cosTheta);
	float	x = cos(phi) * sinTheta;
	float	y = sin(phi) * sinTheta;
	float	z = cosTheta;
	return Vec3f(x, y, z);
}
bool Lambertian::sample(const Vec3f& dirIn, const HitInfo& hit, ScatterRecord& srec) const
{
	onb uvw;
	uvw.build_form_w(hit.gn);
	Vec3f direction = uvw.local(randomCosineDirection());
	srec.scattered= direction;
	srec.isSpecular = false;
	return true;
}

bool Lambertian::scatter(const Ray3f &ray, const HitInfo &hit, Color3f &attenuation, Ray3f &scattered) const
{
	// TODO: Implement Lambertian reflection
	//       You should assign the albedo to ``attenuation'', and
	//       you should assign the scattered ray to ``scattered''
	//       The origin of the scattered ray should be at the hit point,
	//       and the scattered direction is the shading normal plus a random
	//       point on a sphere (please look at the text book for this)

	attenuation = this->albedo->value(hit);
	scattered.o = hit.p;
	scattered.d = normalize(normalize(randomInUnitSphere()) + hit.sn);
	//       You can get the hit point using hit.p, and the shading normal using hit.sn

	//       Hint: You can use the function randomInUnitSphere() to get a random
	//       point in a sphere. IMPORTANT: You want to add a random point *on*
	//       a sphere, not *in* the sphere (the text book gets this wrong)
	//       If you normalize the point, you can force it to be on the sphere always, so
	//       add normalize(randomInUnitSphere()) to your shading normal
	return true;
}


Metal::Metal(const json & j)
{
	//albedo = j.value("albedo", albedo);
	albedo = parseTexture(j.at("albedo"));
	roughness = clamp(j.value("roughness", roughness), 0.f, 1.f);
	
}

Vec3f reflect(const Vec3f& v, const Vec3f& n)
{
	return v - 2 * dot(v, n) * n;
}
bool Metal::scatter(const Ray3f &ray, const HitInfo &hit, Color3f &attenuation, Ray3f &scattered) const
{
	// TODO: Implement metal reflection
	//       This function proceeds similar to the lambertian material, except that the
	//       scattered direction is different.
	//       Instead of adding a point on a sphere to the normal as before, you should add the point
	//       to the *reflected ray direction*.
	//       You can reflect a vector by the normal using reflect(vector, hit.sn); make sure the vector is normalized.
	//       Different to before you can't just use randomInUnitSphere directly; the sphere should be scaled by roughness.
	//       (see text book). In other words, if roughness is 0, the scattered direction should just be the reflected direction.
	//       
	attenuation = this->albedo->value(hit);
	//scattered.o = hit.p;
	///Vec3f ray_proj_n = dot(-ray.d, hit.sn) * hit.sn;
	Vec3f refl = reflect(ray.d,hit.sn);
	refl += roughness * randomInUnitSphere();
	//       This procedure could produce directions below the surface. Handle this by returning false if the scattered direction and the shading normal
	//       point in different directions (i.e. their dot product is negative)
	if(dot(refl,hit.sn)<0)
	return false;
	//scattered.d = normalize(refl);
	scattered = Ray3f(hit.p, normalize(refl));
	return true;
}

bool refract(const Vec3f& v, const Vec3f& n, float ni_over_nt, Vec3f& refracted)
{
	Vec3f uv = normalize(v);
	float dt = dot(uv, n);
	float discriminant = 1.f - ni_over_nt * ni_over_nt * (1 - dt * dt);
	if (discriminant > 0)
	{
		refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
		return true;
	}
	else return false;
}
Dielectric::Dielectric(const json & j)
{
	ior = j.value("ior", ior);
}

float schlick(float cosine, float ref_idx)
{
	float r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow(1-cosine,5);
}
bool Dielectric::scatter(const Ray3f &ray, const HitInfo &hit, Color3f &attenuation, Ray3f &scattered) const
{
	// TODO: Implement dielectric scattering
	
	Vec3f outward_normal;
	//Vec3f reflected = reflect(ray.d, hit.sn);
	float ni_over_nt;
	Vec3f refracted;
	attenuation = Vec3f(1.f,1.f,1.f);
	float reflect_prob;
	float cosnine;
	if (dot(ray.d, hit.sn) > 0)
	{
		outward_normal = -hit.sn;
		ni_over_nt = ior;
		cosnine = ior*dot(ray.d,hit.sn) / length(ray.d);
	}
	else {
		outward_normal = hit.sn;
		ni_over_nt = 1.f / ior;
		cosnine = -dot(ray.d,hit.sn)/length(ray.d);
	}
	Vec3f reflected = reflect(ray.d, outward_normal);
	if (refract(ray.d, outward_normal, ni_over_nt, refracted)) {
		reflect_prob = schlick(cosnine, ior);
	}
	else {
		reflect_prob = 1.f;
	}
	if (randf() < reflect_prob) {
		scattered = Ray3f(hit.p, reflected);
	}
	else {
		scattered = Ray3f(hit.p, refracted);
	}
	return true;
}


DiffuseLight::DiffuseLight(const json & j)
{
	emit = j.value("emit", emit);
}

Color3f DiffuseLight::emitted(const Ray3f &ray, const HitInfo &hit) const
{
	// only emit from the normal-facing side
	if (dot(ray.d, hit.sn) > 0)
		return Color3f(0,0,0);
	else
		return emit;
}

BlendMaterial::BlendMaterial(const json& j)
{
	a = parseMaterial(j.at("a"));
	b = parseMaterial(j.at("b"));
	amount = j.at("amount");
}

bool BlendMaterial::scatter(const Ray3f& ray, const HitInfo& hit, Color3f& attenuation, Ray3f& scattered) const
{
	auto new_amount=amount * 0.212671f + amount * 0.715160f + amount * 0.072169f;
	if (randf() < new_amount)return this->b->scatter(ray, hit, attenuation, scattered);
	else return this->a->scatter(ray, hit, attenuation, scattered);
	//return true;
}

void onb::build_form_w(const Vec3f& n)
{
	axis[2] = normalize(n);
	Vec3f a;
	if (fabs(w().x) > 0.9)
		a = Vec3f(0, 1, 0);
	else a = Vec3f(1, 0, 0);
	axis[1] = normalize(cross(w(), a));
	axis[0] = cross(w(), v());
}
float Phong::pdf(const Vec3f& dirIn, const Vec3f& scattered, const HitInfo& hit) const
{
	auto constant = (exponent + 1) / (2 * M_PI);
	auto mirrorDir = normalize(reflect(dirIn, hit.gn));
	auto cosine = max(dot(normalize(scattered), mirrorDir), 0.f); 
	return constant * powf(cosine, exponent);
}
Color3f Phong::eval(const Vec3f& dirIn, const Vec3f& scattered, const HitInfo& hit) const
{
	return albedo->value(hit) * pdf(dirIn, scattered, hit);
}
Vec3f  randomCosinePowerHemisphere(float e)
{
	float phi = randf() * 2 * M_PI;
	float cosTheta = powf(randf(), 1 / (e + 1));
	float 	sinTheta = sqrt(1 - cosTheta * cosTheta);
	float	x = cos(phi) * sinTheta;
	float	y = sin(phi) * sinTheta;
	float	z = cosTheta;
	return Vec3f(x, y, z);
}
bool Phong::sample(const Vec3f& dirIn, const HitInfo& hit, ScatterRecord& srec) const
{
	onb uvw;
	auto mirrorDir = normalize(reflect(dirIn, hit.gn));
	uvw.build_form_w(mirrorDir);
	Vec3f direction = uvw.local(randomCosinePowerHemisphere(this->exponent));
	srec.scattered = direction;
	srec.isSpecular = false;
	if(dot(hit.gn,direction) < 0)return false;
	return true;
}
Phong::Phong(const json& j)
{
	albedo = parseTexture(j.at("albedo"));//albedo->value(hit)
	exponent = j.at("exponent");
}
