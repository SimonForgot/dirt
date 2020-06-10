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

#include <dirt/scene.h>
#include <dirt/progress.h>
#include <fstream>
#include<dirt/integrator.h>

/// Construct a new scene from a json object
Scene::Scene(const json & j)
{
    parseFromJSON(j);
}

/// Read a scene from a json file
Scene::Scene(const string & filename)
{
    // open file
    std::ifstream stream(filename, std::ifstream::in);
    if (!stream.good())
    	throw DirtException("Cannot open file: %s.", filename);

    json j;
    stream >> j;
    parseFromJSON(j);
}

Scene::~Scene()
{
    m_materials.clear();
}


shared_ptr<const Material> Scene::findOrCreateMaterial(const json & jp, const string& key) const
{
    auto it = jp.find(key);
    if (it == jp.end())
        return Material::defaultMaterial();
    
    auto j = it.value();
    if (j.is_string())
    {
        string name = j.get<string>();
        // find a pre-declared material
        auto i = m_materials.find(name);
        if (i != m_materials.end())
            return i->second;
        else
            throw DirtException("Can't find a material with name '%s' here:\n%s", name, jp.dump(4));
    }
    else if (j.is_object())
    {
	    // create a new material
        return parseMaterial(j);
    }
    else
        throw DirtException("Type mismatch: Expecting either a material or material name here:\n%s", jp.dump(4));
}

// compute the color corresponding to a ray by raytracing
Color3f Scene::recursiveColor(const Ray3f &ray, int depth) const
{
    HitInfo hit;
    const int MaxDepth = 64;
    if (this->intersect(ray, hit))
    {
        Ray3f scattered;
        Vec3f attenuation;
        Color3f emit = hit.mat->emitted(ray, hit);

        //const Ray3f& ray, const HitInfo& hit, Color3f& attenuation, Ray3f& scattered
        if (depth < MaxDepth && hit.mat->scatter(ray, hit, attenuation, scattered)) {
            return emit+attenuation * this->recursiveColor(scattered, depth + 1);
        }
        else return emit;
    }
    else return m_background;
}

// raytrace an image
Image3f Scene::raytrace() const
{
    // allocate an image of the proper size
    auto image = Image3f(m_camera->resolution().x, m_camera->resolution().y);

   
    // Hint: you can create a Progress object (progress.h) to provide a 
    // progress bar during rendering.
    {
        Progress progress("Rendering", image.size());
        // Generate a ray for each pixel in the ray image
        for (auto y : range(image.height()))
        {
            for (auto x : range(image.width()))
            {
                INCREMENT_TRACED_RAYS;
                
                Color3f color = Color3f(0.0f);

                // TODO: Call recursiveColorFunction ``NumSamples'' times and average the
                // results. Assign the average color to ``color''
                for (auto i = 0; i < m_imageSamples; i++)
                {
                    auto ray = m_camera->generateRay(x + randf(), y + randf());
                    if (m_integrator != nullptr)
                    {
                        color += m_integrator->Li(*this,ray);//recursiveColor(ray, 0);
                    }
                    else color += recursiveColor(ray, 0);
                }
                color /= m_imageSamples;
                image(x, y) = color;
                ++progress;
            }
        }
    }

	// return the ray-traced image
    return image;
}

