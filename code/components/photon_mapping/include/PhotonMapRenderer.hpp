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

        // ����Ĺ�����
        int photons = 5000000;

        unsigned int width;
        unsigned int height;
        unsigned int depth;
        unsigned int samples;

        using SCam = Camera;
        SCam camera;

        // �����������������
        default_random_engine e;
        uniform_real_distribution<float> u;

        // ѡ��һ������ӳ��
        PhotonMapKD photonMap;   // kd-tree
        // PhotonMap photonMap;

        int result_count = 512;
        float max_radius2 = 10.0f;  // ��ѯʱ��������ƽ��

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

        float get_float() { // 0��1�����������
            return u(e);
        }

    private:
        void renderTask(RGBA* pixels, int width, int height, int off, int step);

        // ���ڹ���ӳ��ĺ���
        void buildPhotonmap();
        void generatePhoton(Ray* ray, Vec3* flux, AreaLight light);
        void photonTrace(const Ray& ray, const Vec3& flux);

        RGB gamma(const RGB& rgb);
        RGB trace(const Ray& ray, int currDepth);   // ������·��׷��
        HitRecord closestHitObject(const Ray& r);
        tuple<float, Vec3> closestHitLight(const Ray& r);
    };
}

#endif