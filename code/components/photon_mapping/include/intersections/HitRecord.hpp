#pragma once
#ifndef __HIT_RECORD_HPP__
#define __HIT_RECORD_HPP__

#include <optional>

#include "geometry/vec.hpp"

namespace PhotonMap
{
    using namespace NRenderer;
    using namespace std;
    struct HitRecordBase
    {
        float t;            // 相交参数（光线起点到相交点的距离）
        Vec3 hitPoint;      // 光线与模型相交的坐标
        Vec3 normal;        // 模型在相交点的表面法线方向
        Handle material;    // 材质
    };
    using HitRecord = optional<HitRecordBase>;
    inline
    HitRecord getMissRecord() {
        return nullopt; // 表示一个空的optional对象
    }
    
    inline
    HitRecord getHitRecord(float t, const Vec3& hitPoint, const Vec3& normal, Handle material) {
        return make_optional<HitRecordBase>(t, hitPoint, normal, material);
    }
}

#endif