#pragma once
#ifndef __PHOTON_MAP_RENDERER_HPP__
#define __PHOTON_MAP_RENDERER_HPP__

#include "scene/Scene.hpp"
#include "Ray.hpp"
#include "Camera.hpp"
#include "intersections/HitRecord.hpp"
#include "shaders/ShaderCreator.hpp"
#include "samplers/Sampler.hpp"
#include "PhotonMap.hpp"
#include "PhotonMapKD.hpp"
#include <tuple>
#include <cmath>
#include <random>

namespace PhotonMap
{
    using namespace NRenderer;
    using namespace std;


    class PhotonMapRenderer
    {
    public:
    private:
        SharedScene spScene;
        Scene& scene;

        // 发射的光子数
        int photons = 5000000;

        unsigned int width;
        unsigned int height;
        unsigned int depth;
        unsigned int samples;

        using SCam = Camera;
        SCam camera;

        // 用于随机浮点数生成
        default_random_engine e;
        uniform_real_distribution<float> u;

        // 选择一个光子映射
        PhotonMapKD photonMap;   // kd-tree
        // PhotonMap photonMap;

        int result_count = 512;
        float max_radius2 = 10.0f;  // 查询时的最大距离平方

        vector<SharedShader> shaderPrograms;
    public:
        PhotonMapRenderer(SharedScene spScene)
            : spScene(spScene)
            , scene(*spScene)
            , camera(spScene->camera)
            , e(time(NULL))
            , u(0, 1)
        {
            width = scene.renderOption.width;
            height = scene.renderOption.height;
            depth = scene.renderOption.depth;
            samples = scene.renderOption.samplesPerPixel;
        }
        ~PhotonMapRenderer() = default;

        using RenderResult = tuple<RGBA*, unsigned int, unsigned int>;
        RenderResult render();
        void release(const RenderResult& r);

        float get_float() { // 0到1的随机浮点数
            return u(e);
        }

    private:
        void renderTask(RGBA* pixels, int width, int height, int off, int step);

        // 关于光子映射的函数
        void buildPhotonmap();
        void generatePhoton(Ray* ray, Vec3* flux, AreaLight light);
        void photonTrace(const Ray& ray, const Vec3& flux);

        RGB gamma(const RGB& rgb);
        RGB trace(const Ray& ray, int currDepth);   // 基础的路径追踪
        HitRecord closestHitObject(const Ray& r);
        tuple<float, Vec3> closestHitLight(const Ray& r);
    };
}

#endif