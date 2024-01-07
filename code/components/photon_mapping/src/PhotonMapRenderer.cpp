#include "server/Server.hpp"

#include "PhotonMapRenderer.hpp"

#include "VertexTransformer.hpp"
#include "intersections/intersections.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace PhotonMap
{
    RGB PhotonMapRenderer::gamma(const RGB& rgb) {
        return glm::sqrt(rgb);
    }

    void PhotonMapRenderer::generatePhoton(Ray* ray, Vec3* flux, AreaLight light) {

        const auto a = 2 * PI * get_float();
        const auto b = (float)(1.0 - 2.0 * get_float());
        // 在区域光源内寻找一个随机点
        const auto origin = light.position + light.u * get_float() + light.v * get_float();
        // 在该点下方附近的随机位置生成一个光子
        const auto pos = origin + Vec3(sqrt(1.0 - b * b) * cos(a), -abs(b), sqrt(1.0 - b * b) * sin(a));
       
        // 相减得到光子方向
        const auto dir = glm::normalize(pos-origin);

        //cout << "position: " << pos.x << " " << pos.y << " " << pos.z << endl;

        *ray = Ray(pos, dir);
        *flux = light.radiance / (float)25; // TODO
    }

    void PhotonMapRenderer::photonTrace(const Ray& ray, const Vec3& flux) {
        Ray traced_ray(ray.origin, ray.direction);
        Vec3 traced_flux = flux;

        // 循环追踪
        while (true)
        {
            // 查看是否击中场景物体
            auto hitObject = closestHitObject(traced_ray);

            if (hitObject == nullopt) {
                return; // 未击中则结束
            }

            auto mtlHandle = hitObject->material;
            // 获取材质类型
            switch (scene.materials[mtlHandle.index()].type) {
            case 0: // Lambertian材质 漫反射
            {
                Photon photon;
                photon.pos = hitObject->hitPoint;
                photon.dir = traced_ray.direction;
                photon.power = traced_flux;

                //cout << photon.pos << " " << photon.dir << " " << photon.dir << endl;
                
                // 将光子存储到photon_map中
                photonMap.store(photon);

                // 获取颜色
                Vec3 color(1.0, 1.0, 1.0);
                auto diffuseColor = scene.materials[mtlHandle.index()].getProperty<Property::Wrapper::RGBType>("diffuseColor");
                if (diffuseColor) color = (*diffuseColor).value;


                // RGB的平均值用作为概率
                auto p = (color.x + color.y + color.z) / 3.0;

                // 生成的随机数小于概率 p，则选择反射
                if (p < get_float())
                {
                    // 构建以 normal 为基准的正交基底
                    Vec3 u, v, w;
                    w = normalize(hitObject->normal);
                    if (abs(w.x) > 0.1)
                    {u = glm::normalize(cross(Vec3(0, 1, 0), w));}
                    else
                    {u = glm::normalize(cross(Vec3(1, 0, 0), w));}
                    v = cross(w, u);

                    // 生成随机角度和随机数
                    const auto a = 2 * PI * get_float();
                    const auto b = get_float();
                    const auto sqrt_b = (float)sqrt(b);

                    // 获取随机的反射方向
                    auto dir = glm::normalize(u * (float)cos(a) * sqrt_b + v * (float)sin(a) * sqrt_b + w * (float)sqrt(1.0 - b));

                    traced_ray = Ray(hitObject->hitPoint, dir);

                    // 光子辐射随着反射次数逐渐减弱，且与物体表面的颜色特性相符合
                    traced_flux *= color;
                }
                // 生成的随机数大于等于概率 p，则选择吸收，终止光线的追踪
                // 多次反射后 p 逐渐减小，被吸收的概率也逐渐变大
                else
                {
                    return;
                }
            }
            break;
            case 1: // Dielectric材质
                    // 需要添加类型为RGB，名为reflectColor的Property
            {
                if (dot(traced_ray.direction, hitObject->normal) > 0) {
                    return; // 内部射入
                }
                // 和漫反射一样，获取表面的颜色，这里一般是（1，1，1）
                Vec3 color{ 1, 1, 1 };
                auto reflectColor = scene.materials[mtlHandle.index()].getProperty<Property::Wrapper::RGBType>("reflectColor");
                if (reflectColor) color = (*reflectColor).value;
                else color = { 1, 1, 1 };

                // 继续追踪反射出的光线
                traced_ray = Ray(hitObject->hitPoint, reflect(traced_ray.direction, hitObject->normal));
                traced_flux *= color;
            }
            break;
            case 2: {
                // TODO：折射等
            }
                  break;
            default: break;
            }
        }
    }

    void PhotonMapRenderer::buildPhotonmap() {
        getServer().logger.log("Building photon map...");
        // 遍历场景中的所有区域光源
        for (auto& a : scene.areaLightBuffer) {
            // 每个光源生成photons个光子并进行追踪
            for (auto i = 0; i < photons; ++i)
            {
                Ray ray;
                Vec3 flux;
                generatePhoton(&ray, &flux, a);
                photonTrace(ray, flux);
            }
        }
        photonMap.build();
    }

    void PhotonMapRenderer::renderTask(RGBA* pixels, int width, int height, int off, int step) {
        // 使用两个嵌套的循环遍历图像区域的像素
        for (int i = off; i < height; i += step) {     // 垂直
            for (int j = 0; j < width; j++) {       // 水平
                Vec3 color{ 0, 0, 0 };
                // 计算当前像素在图像中的位置
                auto r = defaultSamplerInstance<UniformInSquare>().sample2d();
                float rx = r.x;
                float ry = r.y;
                float x = (float(j) + rx) / float(width);
                float y = (float(i) + ry) / float(height);

                // 相机对象（camera）生成一条射线（ray），起始点为当前像素位置
                Ray ray = camera.shoot(x, y);
                
                // 递归调用trace函数
                color = trace(ray, 0);

                // 进行伽马校正
                color = gamma(color);
                // 将最终颜色值存储到图像像素数组（pixels）中，根据像素的位置进行索引计算
                pixels[(height - i - 1) * width + j] = { color, 1 };
            }
        }
    }

    auto PhotonMapRenderer::render() -> RenderResult {
        // shaders
        shaderPrograms.clear();
        ShaderCreator shaderCreator{};

        // 针对场景中的材质，为其创建shader
        for (auto& m : scene.materials) {
            shaderPrograms.push_back(shaderCreator.create(m, scene.textures));
        }

        RGBA* pixels = new RGBA[width * height]{};

        // 局部坐标转换成世界坐标
        VertexTransformer vertexTransformer{};
        vertexTransformer.exec(spScene);

        // phase 1 光子映射的构建
        buildPhotonmap();

        // phase 2 路径追踪
        getServer().logger.log("path tracing...");
        const auto taskNums = 8;
        thread t[taskNums];
        for (int i = 0; i < taskNums; i++) {
            t[i] = thread(&PhotonMapRenderer::renderTask,
                this, pixels, width, height, i, taskNums);
        }
        for (int i = 0; i < taskNums; i++) {
            t[i].join();
        }
        getServer().logger.log("Done...");
        return { pixels, width, height };
    }

    void PhotonMapRenderer::release(const RenderResult& r) {
        auto [p, w, h] = r;
        // BUG !
        // delete[] p;
    }

    HitRecord PhotonMapRenderer::closestHitObject(const Ray& r) {
        HitRecord closestHit = nullopt;
        float closest = FLOAT_INF;
        // 遍历各种物体，返回最近交点的信息
        for (auto& s : scene.sphereBuffer) {
            auto hitRecord = Intersection::xSphere(r, s, 0.000001, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        for (auto& t : scene.triangleBuffer) {
            auto hitRecord = Intersection::xTriangle(r, t, 0.000001, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        for (auto& p : scene.planeBuffer) {
            auto hitRecord = Intersection::xPlane(r, p, 0.000001, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        // TODO：添加其他可能的对象
        return closestHit;
    }

    tuple<float, Vec3> PhotonMapRenderer::closestHitLight(const Ray& r) {
        Vec3 v = {};
        HitRecord closest = getHitRecord(FLOAT_INF, {}, {}, {});
        // 遍历场景中的所有区域光源
        for (auto& a : scene.areaLightBuffer) {
            // 对于每个区域光源，判断射线与光源是否相交，获取交点信息。
            auto hitRecord = Intersection::xAreaLight(r, a, 0.000001, closest->t);
            // 如果存在交点且该交点距离更近，更新最近交点记录和区域光源的辐射强度
            if (hitRecord && closest->t > hitRecord->t) {
                closest = hitRecord;
                v = a.radiance;
            }
        }
        // 返回最近交点的距离t和对应的辐射强度
        return { closest->t, v };
    }

    RGB PhotonMapRenderer::trace(const Ray& r, int currDepth) {
        if (currDepth == depth) return scene.ambient.constant;
        auto hitObject = closestHitObject(r);
        auto [t, emitted] = closestHitLight(r);

        // hit object
        if (hitObject && hitObject->t < t) {
            auto mtlHandle = hitObject->material;

            // 根据不同材质进行不同处理
            switch (scene.materials[mtlHandle.index()].type) {
            case 0:     // Lambertian材质
            {

                // 设置查询参数
                QueryPoint query;
                query.pos = hitObject->hitPoint;
                query.normal = hitObject->normal;

                NearestResult result(result_count);  // 用于存储结果
                photonMap.search(query, result, max_radius2);  // 到光子映射中查找

                Vec3 color(1.0, 1.0, 1.0);
                auto diffuseColor = scene.materials[mtlHandle.index()].getProperty<Property::Wrapper::RGBType>("diffuseColor");
                if (diffuseColor) color = (*diffuseColor).value;

                auto p = max(color.x, max(color.y, color.z));

                Vec3 power(0, 0, 0);
                double max_dist2 = -1;

                // 遍历查询结果，获取最大距离平方
                for (size_t i = 0; i < result.size(); ++i)
                {
                    max_dist2 = max(max_dist2, (double)result[i].second);
                }

                // 圆锥滤波器
                {
                    Vec3 power(0, 0, 0);

                    const auto max_dist = sqrt(max_dist2);
                    const auto k = 1.1;

                    // 遍历搜索得到的光子结果，计算每个光子的权重和对应辐射度
                    for (size_t i = 0; i < result.size(); ++i)
                    {
                        const auto w = 1.0 - (sqrt(result[i].second) / (k * max_dist));
                        const auto v = (color * result[i].first->power) / PI;
                        power += (float)w * v;  // 累加
                    }
                    // 校正圆锥滤波器的权重
                    power /= (1.0 - 2.0 / (3.0 * k));

                    if (max_dist2 > 0)
                    {
                        // 单位面积的辐射度估计值
                        return power / (float)((PI * max_dist2) / p);
                    }
                }
            }// 对于上述漫反射，我们直接用附近的光子进行估计，并不追踪光线，不用递归调用trace()
            break;
            case 1: {   // 镜面反射
                Vec3 color{ 1, 1, 1 };
                auto reflectColor = scene.materials[mtlHandle.index()].getProperty<Property::Wrapper::RGBType>("reflectColor");
                if (reflectColor) color = (*reflectColor).value;
                else color = { 1, 1, 1 };

                if (dot(r.direction, hitObject->normal) > 0) {
                    return color;
                }

                auto p = max(color.x, max(color.y, color.z));
                // 这里我们递归地调用trace()，追踪反射光线
                return color * trace(Ray(hitObject->hitPoint, glm::normalize(reflect(r.direction, hitObject->normal))), depth + 1) / p;
            }
                  break;
            case 2: {
                // TODO：其他材质
            }
                  break;
            }
        }
        // 存在光线与光源的交点，则返回光源发射的辐射强度
        else if (t != FLOAT_INF) {
            return emitted;
        }
        else {
            return Vec3{ 0 };
        }
        return Vec3{ 0 };
    }
}