#pragma once
#ifndef __PHOTON_MAP_HPP__
#define __PHOTON_MAP_HPP__

#include "geometry/vec.hpp"
#include "Photon.hpp"
#include <queue>
#include <vector>

// ֱ��ʹ��vector�洢��PhotonMap

namespace PhotonMap
{
    using namespace NRenderer;
    using namespace std;

    class PhotonMap
    {
    public:
        vector<Photon> photons;

        int result_count = 128; // ���ؽ����

        size_t size() const;
        void search(QueryPoint& query, NearestResult& result, float max_dist2);
        void store(const Photon& value);
        void build() {};
    };
}

#endif