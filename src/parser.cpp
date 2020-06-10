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

#include <dirt/parser.h>
#include <dirt/obj.h>
#include <dirt/bbh.h>
#include <dirt/sphere.h>
#include <dirt/quad.h>
#include <dirt/scene.h>
#include <iostream>
#include <filesystem/resolver.h>
#include<dirt/integrator.h>

void from_json(const json & j, Transform & v)
{
    Mat44f m = v.m;
    if (j.is_array())
    {
        // multiple transformation commands listed in order
        for (auto & element : j)
            m = element.get<Mat44f>() * m;
    }
    else if (j.is_object())
    {
        // a single transformation
        j.get_to(m);
    }
    else
        throw DirtException("'transform' must be either an array or an object here:\n%s", j.dump(4));

    v = Transform(m);
}

// helper function to check for a required string key within a json object
static string getKey(const string & key, const string & parent, const json & j)
{
    try
    {
        return j.at(key).get<string>();
    }
    catch (...)
    {
        throw DirtException("Missing '%s' on '%s' specification:\n%s", key, parent, j.dump(4));
    }
}

shared_ptr<SurfaceGroup> parseAccelerator(const Scene & scene, const json & j)
{
    string type = getKey("type", "accelerator", j);

    if (type == "bbh" || type == "bvh")
        return make_shared<BBH>(scene, j);
    else if (type == "group")
        return make_shared<SurfaceGroup>(scene, j);
    else
        throw DirtException("Unknown 'accelerator' type '%s' here:\n%s.", type, j.dump(4));
}


shared_ptr<Material> parseMaterial(const json & j)
{
    string type = getKey("type", "material", j);
    //cout<<j.dump();
    if (type == "lambertian")
        return make_shared<Lambertian>(j);
    if (type == "phong")
        return make_shared<Phong>(j);
    else if (type == "metal")
        return make_shared<Metal>(j);
    else if (type == "dielectric")
        return make_shared<Dielectric>(j);
    else if (type == "diffuse light")
        return make_shared<DiffuseLight>(j);
    else if (type == "blend")
        return make_shared<BlendMaterial>(j);
	else
		throw DirtException("Unknown 'material' type '%s' here:\n%s.", type, j.dump(4));
}

shared_ptr<Texture> parseTexture(const json& j)
{ 
    
    if(j.find("type")==j.end())
        return make_shared<ConstantTexture>(json{ {"color",j } });
    string type = getKey("type", "albedo", j);
    //string type = j.at("type");
   
    if (type == "constant")
        return make_shared<ConstantTexture>(j);
    if (type == "checker")
        return make_shared<CheckerTexture>(j);
    if (type == "marble")
        return make_shared<MarbleTexture>(j);
    if (type == "image")
        return make_shared<ImageTexture>(j);
    else
        throw DirtException("Unknown 'texture' type '%s' here:\n%s.", type, j.dump(4));
}
shared_ptr<Integrator> parseIntegrator(const json& j)
{

    string type = getKey("type", "albedo", j);
    //string type = j.at("type");

    if (type == "normals")
        return make_shared<NormalIntegrator>();
    if (type == "ao")
        return make_shared<AmbientOcclusionIntegrator>();
    if (type == "path_tracer_mats")
        return make_shared<PathTracerMaterials>(j);
    else
        throw DirtException("Unknown 'integrator' type '%s' here:\n%s.", type, j.dump(4));
}
void parseSurface(const Scene & scene, SurfaceBase * parent, const json & j)
{
    string type = getKey("type", "surface", j);

    if (type == "quad")
        parent->addChild(make_shared<Quad>(scene, j));
    else if (type == "sphere")
        parent->addChild(make_shared<Sphere>(scene, j));
    else if (type == "mesh")
    {
        auto xform = Transform();
        xform = j.value("transform", xform);
        std::string filename = j["filename"];

        auto mesh = make_shared<Mesh>(loadWavefrontOBJ(getFileResolver().resolve(filename).str(), xform));

        if (mesh->empty())
            return;

        mesh->material = scene.findOrCreateMaterial(j);

        for (auto index : range(mesh->F.size()))
            parent->addChild(make_shared<Triangle>(scene, j, mesh, int(index)));
    }
    else
        throw DirtException("Unknown surface type '%s' here:\n%s",
                            type.c_str(), j.dump(5));
}


void Scene::parseFromJSON(const json & j)
{
    message("parsing...\n");

    // first create the scene-wide acceleration structure
    if (j.contains("accelerator"))
        m_surfaces = parseAccelerator(*this, j["accelerator"]);
    else
        // default to a naive accelerator
        m_surfaces = make_shared<SurfaceGroup>(*this, j["accelerator"]);

    // now loop through all keys in the json file and take the appropriate action
    for (auto it = j.begin(); it != j.end(); ++it)
    {
        if (it.key() == "accelerator")
        {
            // already handled above
        }
        else if (it.key() == "camera")
        {
            if (m_camera)
                throw DirtException("There can only be one camera per scene!");
            m_camera = make_shared<Camera>(it.value());
        }
        else if (it.key() == "integrator")
        {
            if (m_integrator)
                throw DirtException("There can only be one integrator per scene!");      
            m_integrator = parseIntegrator(it.value());
        }
        else if (it.key() == "image_samples")
        {
            m_imageSamples = it.value();
        }
        else if (it.key() == "background")
        {
            m_background = it.value();
        }
        else if (it.key() == "materials")
        {
            for (auto & m : it.value())
            {
                auto material = parseMaterial(m);
                m_materials[getKey("name", "material", m)] = material;
            }
        }
        else if (it.key() == "surfaces")
        {
            for (auto & s : it.value())
                parseSurface(*this, this, s);
        }
        else
            throw DirtException("Unsupported key '%s' here:\n%s", it.key(), it.value().dump(4));
    }
    
    // if (!m_integrator)
    //     throw DirtException("No integrator specified in scene!");

    if (!m_camera)
        throw DirtException("No camera specified in scene!");

    m_surfaces->build();
    message("done parsing scene.\n");
}