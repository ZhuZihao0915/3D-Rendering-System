#include "PhotonMapKD.hpp"

namespace PhotonMap
{
    // �ݹ�ع���kd-tree
    PhotonMapKD::kdNode* PhotonMapKD::build(Photon* photons, int count, int depth)
    {
        if (count <= 0)
        {
            return nullptr;
        }
        // x,y,z����
        const int axis = depth % 3;
        // ��ȡ��ǰ�ڵ�Ĺ���λ��
        const int current = (count - 1) / 2;

        // ��axis��Ϊ��׼�������Ӱ������ϵ�ֵ��������
        nth_element(photons, photons + current, photons + count,
            [axis](const Photon& a, const Photon& b) {return a.pos[axis] < b.pos[axis]; });

        // ������ǰ�ڵ�
        kdNode* node = new(nothrow) PhotonMapKD::kdNode();
        node->axis = axis;
        node->p_photon = &photons[current]; // ��ǰ�ڵ�Ĺ���
        // �ݹ�ع�����������
        node->leftNode = build(photons, current, depth + 1);
        node->rightNode = build(photons + current + 1, count - current - 1, depth + 1);

        return node;
    }

    void PhotonMapKD::search(QueryPoint& query, kdNode* node, NearestResult& result)
    {
        if (node == nullptr)
        {
            return;
        }

        const int  axis = node->axis;
        double     delta = query.pos[axis] - node->p_photon->pos[axis];
        const auto dir = node->p_photon->pos - query.pos;
        const auto dist2 = dot(dir, dir);
        const auto dt = dot(query.normal, dir / sqrt(dist2));

        // ����������ѯλ�õľ���С�������룬���ҹ��ӵķ������ѯ���߼нǽ�С���򽫹��Ӽ���������
        if (dist2 < max_dist2 && fabs(dt) <= max_dist2 * 0.01)
        {
            result.push(pair<Photon*, float>(node->p_photon, dist2));
            if (result.size() > result_count)
            {
                result.pop();
                max_dist2 = result.top().second;
            }
        }

        if (delta > 0.0)
        {
            // �ݹ�����������
            search(query, node->rightNode, result);
            // �����ǰ�ڵ�ķָ����ϵĲ�ֵ��ƽ��С�������룬���������������
            if (delta * delta < max_dist2)
            {
                search(query, node->leftNode, result);
            }
        }
        else
        {
            // �ݹ�����������
            search(query, node->leftNode, result);
            // �����ǰ�ڵ�ķָ����ϵĲ�ֵ��ƽ��С�������룬���������������
            if (delta * delta < max_dist2)
            {
                search(query, node->rightNode, result);
            }
        }
    }

    size_t PhotonMapKD::size() const
    {
        return photons.size();
    }

    void PhotonMapKD::search(QueryPoint& query, NearestResult& result, float max_dist2)
    {
        this->max_dist2 = max_dist2;
        search(query, root, result);
    }

    void PhotonMapKD::store(const Photon& value)
    {
        photons.emplace_back(value);
    }

    void PhotonMapKD::build()
    {
        photons.shrink_to_fit();
        root = build(photons.data(), int(photons.size()), 0);
    }
}