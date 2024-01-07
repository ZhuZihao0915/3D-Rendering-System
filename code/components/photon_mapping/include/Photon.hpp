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
    // ����
    struct Photon
    {
        Vec3 pos;    // λ��
        Vec3 power;   // ��ͨ����RGB��
        Vec3 dir;    // ����
    };

    // ��Ҫ��ѯ�ĵ�
    struct QueryPoint
    {
        Vec3     pos;        // ��ѯ���λ��
        Vec3     normal;     // ��ѯ��ķ�����
    };

    // ��ѯ�õ���������ӵĽ��
    class NearestResult
    {

    private:
        std::vector<pair<Photon*, float>> photons;  // �洢��ѯ���

    public:
        NearestResult(size_t size): photons()
        {
            photons.reserve(size + 1);
        }

        void push(const pair<Photon*, float>& value)
        {
            photons.emplace_back(value);
            std::push_heap(photons.begin(), photons.end(), 
                // Lambda���ʽ���ȽϾ����ѯ��ľ���
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