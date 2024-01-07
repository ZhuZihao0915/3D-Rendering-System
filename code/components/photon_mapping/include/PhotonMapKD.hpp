#pragma once
#ifndef __PHOTON_MAP_KD_HPP__
#define __PHOTON_MAP_KD_HPP__

#include "geometry/vec.hpp"
#include "Photon.hpp"
#include <queue>
#include <vector>

// 使用kd-tree存储的PhotonMap

namespace PhotonMap
{
    using namespace NRenderer;
    using namespace std;

    class PhotonMapKD
    {
    public:
        // kd-tree 节点
        struct kdNode
        {
            Photon* p_photon; // 指向该节点的光子
            int axis; // 当前节点分割的维度
            kdNode* leftNode;
            kdNode* rightNode;
        };

        kdNode* root;
        vector<Photon> photons;
        float max_dist2;

        int result_count = 512; // 返回结果数

        size_t size() const;
        void search(QueryPoint& query, NearestResult& result, float max_dist2);
        void store(const Photon& value);
        void build();
        void search(QueryPoint& query, kdNode* node, NearestResult& result);
        kdNode* build(Photon* photons, int count, int depth);
    };
}

#endif