#pragma once
#ifndef __NR_SCENE_HPP__
#define __NR_SCENE_HPP__

#include "Texture.hpp"
#include "Material.hpp"
#include "Model.hpp"
#include "Light.hpp"
#include "Camera.hpp"

namespace NRenderer
{
    struct RenderOption
    {
        unsigned int width;
        unsigned int height;
        unsigned int depth;     // 光线追踪的最大深度，即光线可以反射或折射的最大次数
        unsigned int samplesPerPixel;   // 每个像素的采样次数，用于抗锯齿和平滑效果
        RenderOption()
            : width             (500)
            , height            (500)
            , depth             (4)
            , samplesPerPixel   (16)
        {}
    };

    struct Ambient      // 定义环境光的参数
    {
        enum class Type
        {
            CONSTANT, ENVIROMENT_MAP
        };
        Type type;
        Vec3 constant = {};     // 当环境光类型为常量光照时的光照强度
        Handle environmentMap = {}; // 当环境光类型为环境贴图时使用的贴图句柄
    };

    struct Scene
    {
        Camera camera;

        RenderOption renderOption;

        Ambient ambient;

        // buffers
        vector<Material> materials;
        vector<Texture> textures;

        vector<Model> models;
        vector<Node> nodes;
        // object buffer
        vector<Sphere> sphereBuffer;
        vector<Triangle> triangleBuffer;
        vector<Plane> planeBuffer;
        vector<Mesh> meshBuffer;

        vector<Light> lights;
        // light buffer
        vector<PointLight> pointLightBuffer;
        vector<AreaLight> areaLightBuffer;
        vector<DirectionalLight> directionalLightBuffer;
        vector<SpotLight> spotLightBuffer;
    };
    using SharedScene = shared_ptr<Scene>;
} // namespace NRenderer


#endif