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

#include <dirt/sphere.h>
#include <dirt/scene.h>


Sphere::Sphere(float radius,
               shared_ptr<const Material> material,
               const Transform & xform)
    : Surface(xform), m_radius(radius), m_material(material)
{

}

Sphere::Sphere(const Scene & scene, const json & j)
    : Surface(scene, j)
{
	m_radius = j.value("radius", m_radius);
    m_material = scene.findOrCreateMaterial(j);
}

Box3f Sphere::localBBox() const
{
    return Box3f(Vec3f(-m_radius), Vec3f(m_radius));
}



bool Sphere::intersect(const Ray3f &ray, HitInfo &hit) const
{
    INCREMENT_INTERSECTION_TESTS;
    // TODO: Assignment 1: Implement ray-sphere intersection

    //putYourCodeHere("Assignment 1: Insert your ray-sphere intersection code here");

    //return false;
    Ray3f transed_ray = m_xform.inverse().ray(ray);
    //Vec3f center=m_xform.point(Vec3f(0.f,0.f,0.f));
    Vec3f CO = transed_ray.o;
    float A = dot(transed_ray.d, transed_ray.d);
    float B = 2.f * dot(transed_ray.d , CO);
    float C = dot(CO, CO) - m_radius * m_radius;

    float det = B*B-4*A*C;
    if (det < 0)return false;
    det = sqrt(det);
    float t = (-B - det) / (2 * A);
    if (t < ray.mint||t>ray.maxt)
    {
        t = (-B + det) / (2 * A);
        if (t < ray.mint || t>ray.maxt)return false;
    }
    
    // TODO: If the ray misses the sphere, you should return false
    // TODO: If you successfully hit something, you should compute the hit point, 
    //       hit distance, and normal and fill in these values
    auto transed_hitpoint = transed_ray.o + t * transed_ray.d;
    float hitT = t;
    Vec3f hitPoint= m_xform.point(transed_hitpoint);
    Vec3f geometricNormal= normalize(m_xform.normal(normalize(transed_hitpoint)));


    // For this assignment you can leave these values as is
    Vec3f shadingNormal = geometricNormal;
    float phi = atan2(transed_hitpoint.y, transed_hitpoint.x);
    float theta = acos(transed_hitpoint.z/m_radius);
    float u =  (phi + M_PI) / (2 * M_PI);
    float v = (theta) / M_PI;
    Vec2f uvCoordinates = Vec2f(u,v);

    // You should only assign hit and return true if you successfully hit something
    hit = HitInfo(hitT,
            hitPoint,
            geometricNormal,
            shadingNormal,
            uvCoordinates,
            m_material.get(),
            this);

    return true;

}
