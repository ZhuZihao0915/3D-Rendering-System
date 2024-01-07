#include "PhotonMapKD.hpp"

namespace PhotonMap
{
    // 递归地构建kd-tree
    PhotonMapKD::kdNode* PhotonMapKD::build(Photon* photons, int count, int depth)
    {
        if (count <= 0)
        {
            return nullptr;
        }
        // x,y,z轮流
        const int axis = depth % 3;
        // 获取当前节点的光子位置
        const int current = (count - 1) / 2;

        // 以axis轴为基准，将光子按照轴上的值进行排序
        nth_element(photons, photons + current, photons + count,
            [axis](const Photon& a, const Photon& b) {return a.pos[axis] < b.pos[axis]; });

        // 创建当前节点
        kdNode* node = new(nothrow) PhotonMapKD::kdNode();
        node->axis = axis;
        node->p_photon = &photons[current]; // 当前节点的光子
        // 递归地构建左右子树
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

        // 如果光子与查询位置的距离小于最大距离，并且光子的方向与查询法线夹角较小，则将光子加入结果集合
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
            // 递归搜索右子树
            search(query, node->rightNode, result);
            // 如果当前节点的分割轴上的差值的平方小于最大距离，则继续搜索左子树
            if (delta * delta < max_dist2)
            {
                search(query, node->leftNode, result);
            }
        }
        else
        {
            // 递归搜索左子树
            search(query, node->leftNode, result);
            // 如果当前节点的分割轴上的差值的平方小于最大距离，则继续搜索右子树
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