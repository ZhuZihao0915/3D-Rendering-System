#pragma once
#ifndef __NR_LIGHT_HPP__
#define __NR_LIGHT_HPP__

#include "geometry/vec.hpp"
#include <numbers>

#include <memory>
#include <string>

#include "Model.hpp"

namespace NRenderer
{
    struct Light
    {
        enum class Type
        {
            POINT = 0x0,                // 点光
            SPOT,                       // 聚光
            DIRECTIONAL,                // 平行光
            AREA                        // 面积光
        };
        // 类型
        Type type;
        // 光源实体的索引
        Index entity;
        Light(Type type)
            : type                  (type)
            , entity                (0)
        {}
        Light() = default;
    };
    SHARE(Light);

    struct AreaLight
    {
        Vec3 radiance = {1, 1, 1};  // 光源的辐射强度（RGB），默认为最大值白色
        Vec3 position = {};         // 区域光源在世界坐标系中的位置
        // 两个基向量，通常用于定义光源的形状和尺寸
        // 可以用于构建光源的局部坐标系，
        // 通过在局部坐标系内进行参数化，可以方便地计算光源上的点的位置和属性。
        Vec3 u = {};
        Vec3 v = {};
    };
    SHARE(AreaLight);

    struct PointLight
    {
        Vec3 intensity = {1, 1, 1};
        Vec3 position = {};
    };
    SHARE(PointLight);

    struct DirectionalLight
    {
        Vec3 irradiance = {1, 1, 1};
        Vec3 direction = {};
    };
    SHARE(DirectionalLight);

    struct SpotLight
    {
        Vec3 intensity = {1, 1, 1};
        Vec3 position = {};
        Vec3 direction = {};
        float hotSpot = 3.1415926f/4.f;
        float fallout = 3.1415926f/3.f;
    };
    SHARE(SpotLight);
} // namespace Light


#endif