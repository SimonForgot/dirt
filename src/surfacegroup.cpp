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

#include <dirt/surfacegroup.h>


Box3f SurfaceGroup::localBBox() const
{
    return m_localBBox;
}

void SurfaceGroup::addChild(shared_ptr<SurfaceBase> surface)
{
    m_surfaces.push_back(surface);

    m_localBBox.enclose(surface->worldBBox());
}

void SurfaceGroup::clear()
{
    m_surfaces.clear();
    m_surfaces.shrink_to_fit();
}

bool SurfaceGroup::intersect(const Ray3f &_ray, HitInfo &hit) const
{
    // copy the ray so we can modify the tmax values as we traverse
    Ray3f ray = _ray;
    bool hitSomething = false;
    
    // This is a linear intersection test that iterates over all primitives
    // within the scene. It's the most naive intersection test and hence very 
    // slow if you have many primitives.

    // foreach primitive
    for (auto surface : m_surfaces)
    {
        if (surface->intersect(ray, hit))
        {
            hitSomething = true;
            ray.maxt = hit.t;
        }
    }

    // record closest intersection
    return hitSomething;
}
