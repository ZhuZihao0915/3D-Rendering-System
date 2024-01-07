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
        unsigned int depth;     // ����׷�ٵ������ȣ������߿��Է���������������
        unsigned int samplesPerPixel;   // ÿ�����صĲ������������ڿ���ݺ�ƽ��Ч��
        RenderOption()
            : width             (500)
            , height            (500)
            , depth             (4)
            , samplesPerPixel   (16)
        {}
    };

    struct Ambient      // ���廷����Ĳ���
    {
        enum class Type
        {
            CONSTANT, ENVIROMENT_MAP
        };
        Type type;
        Vec3 constant = {};     // ������������Ϊ��������ʱ�Ĺ���ǿ��
        Handle environmentMap = {}; // ������������Ϊ������ͼʱʹ�õ���ͼ���
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