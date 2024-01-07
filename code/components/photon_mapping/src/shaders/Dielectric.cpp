#include "shaders/Dielectric.hpp"
#include "samplers/SamplerInstance.hpp"

#include "Onb.hpp"

namespace PhotonMap
{
    Dielectric::Dielectric(Material& material, vector<Texture>& textures)
        : Shader(material, textures)
    {
        auto reflectColor = material.getProperty<Property::Wrapper::RGBType>("reflectColor");
        if (reflectColor) color = (*reflectColor).value;
        else color = { 1, 1, 1 };
    }
    Scattered Dielectric::shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const {
        return {};
    }
}