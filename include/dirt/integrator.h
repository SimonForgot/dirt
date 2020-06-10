#pragma once

#include<dirt/scene.h>
class Integrator
{
public:
	virtual Color3f Li(const Scene& scene, const Ray3f& ray,int depth=0)const
	{
		return Color3f(1, 0, 1);
	}
};

class NormalIntegrator :public Integrator
{
public:
	Color3f Li(const Scene& scene, const Ray3f& ray,int depth=0)const
	{
		HitInfo hit;
		auto f = scene.intersect(ray, hit);
		if (f)return abs(hit.gn);
		else return scene.background();
	}
};
class AmbientOcclusionIntegrator :public Integrator
{
public:
	Color3f Li(const Scene& scene, const Ray3f& ray,int depth=0)const
	{
		HitInfo hit;
		ScatterRecord srec;
		auto f = scene.intersect(ray, hit);
		if (f)
		{
			hit.mat->sample(-ray.d, hit, srec);
			Ray3f scattered(hit.p,srec.scattered);
			if (scene.intersect(scattered, hit))
				return Color3f(0,0,0);
			else return Color3f(1,1,1);
		}
		else return scene.background();
	}
};
class PathTracerMaterials :public Integrator
{
	int MaxDepth;
public:
	PathTracerMaterials(const json& j) {
		MaxDepth = j.at("max_bounces");
	}
	Color3f Li(const Scene& scene, const Ray3f& ray,int depth=0)const
	{
		HitInfo hit;	
		if (scene.intersect(ray, hit))
		{
			Color3f emit = hit.mat->emitted(ray, hit);
			ScatterRecord srec;
			bool f=hit.mat->sample(ray.d, hit, srec);
			if (f&&depth < MaxDepth)
			{
				auto eval_val=hit.mat->eval(ray.d, srec.scattered, hit);
				auto pdf_val=hit.mat->pdf(ray.d, srec.scattered, hit);
				return this->Li(scene, Ray3f(hit.p,srec.scattered), depth + 1)
					* eval_val / pdf_val;
			}
			else
			{
				Ray3f scattered;
				Vec3f attenuation;
				if (depth < MaxDepth && hit.mat->scatter(ray, hit, attenuation, scattered)) {
					return emit + attenuation * this->Li(scene,scattered, depth + 1);
				}
				else return emit;
			}	
		}
		else return scene.background();
	}
};
