#include "PhotonMap.hpp"

namespace PhotonMap
{
    size_t PhotonMap::size() const {
        return photons.size();
    }
    void PhotonMap::search(QueryPoint& query, NearestResult& result, float max_dist2) {
        for (Photon& photon : photons)
        {
            const auto dir = photon.pos - query.pos;
            const auto dist2 = dot(dir, dir);
            const auto dt = dot(query.normal, dir / sqrt(dist2));
            // ����������ѯλ�õľ���С�������룬���ҹ��ӵķ������ѯ���߼нǽ�С���򽫹��Ӽ���������
            if (dist2 < max_dist2 && fabs(dt) <= max_dist2 * 0.01)
            {
                result.push(pair<Photon*, float>(&photon, dist2));
                if (result.size() > result_count)
                {
                    result.pop();
                    // ������Զ����
                    max_dist2 = result.top().second;
                }
            }
        }
    }
    void PhotonMap::store(const Photon& value) {
        photons.emplace_back(value);
    }
}