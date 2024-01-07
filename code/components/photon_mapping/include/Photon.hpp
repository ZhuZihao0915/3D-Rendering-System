#pragma once
#ifndef __PHOTON_HPP__
#define __PHOTON_HPP__

#include "geometry/vec.hpp"
#include <vector>
#include <queue>

namespace PhotonMap
{
    using namespace NRenderer;
    using namespace std;
    // 光子
    struct Photon
    {
        Vec3 pos;    // 位置
        Vec3 power;   // 光通量（RGB）
        Vec3 dir;    // 方向
    };

    // 需要查询的点
    struct QueryPoint
    {
        Vec3     pos;        // 查询点的位置
        Vec3     normal;     // 查询点的法向量
    };

    // 查询得到的最近光子的结果
    class NearestResult
    {

    private:
        std::vector<pair<Photon*, float>> photons;  // 存储查询结果

    public:
        NearestResult(size_t size): photons()
        {
            photons.reserve(size + 1);
        }

        void push(const pair<Photon*, float>& value)
        {
            photons.emplace_back(value);
            std::push_heap(photons.begin(), photons.end(), 
                // Lambda表达式，比较距离查询点的距离
                [](pair<Photon*, float> a, pair<Photon*, float> b) {return a.second < b.second;});  
        }

        bool empty() const
        {
            return photons.empty();
        }

        size_t size() const
        {
            return photons.size();
        }

        const pair<Photon*, float>& top() const
        {
            return photons.front();
        }

        void pop()
        {
            std::pop_heap(photons.begin(), photons.end(), 
                [](pair<Photon*, float> a, pair<Photon*, float> b) {return a.second < b.second; });
            photons.pop_back();
        }

        const pair<Photon*, float>& operator[](int index)
        {
            return photons[index];
        }

    };
}

#endif