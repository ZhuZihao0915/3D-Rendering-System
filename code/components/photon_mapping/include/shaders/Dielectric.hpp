#pragma once
#ifndef __DIELECTRIC_HPP__
#define __DIELECTRIC_HPP__

#include "Shader.hpp"

namespace PhotonMap
{
    class Dielectric : public Shader
    {
    private:
    public:
        Vec3 color;
        Dielectric(Material& material, vector<Texture>& textures);
        Scattered shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const;
    };
}

#endif