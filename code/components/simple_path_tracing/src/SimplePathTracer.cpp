#include "server/Server.hpp"

#include "SimplePathTracer.hpp"

#include "VertexTransformer.hpp"
#include "intersections/intersections.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace SimplePathTracer
{
    RGB SimplePathTracerRenderer::gamma(const RGB& rgb) {
        return glm::sqrt(rgb);
    }


    void SimplePathTracerRenderer::renderTask(RGBA* pixels, int width, int height, int off, int step) {
        // 使用两个嵌套的循环遍历图像区域的像素
        for(int i=off; i<height; i+=step) {     // 垂直
            for (int j=0; j<width; j++) {       // 水平
                Vec3 color{0, 0, 0};
                vector<Vec3> record;
                for (int k=0; k < samples; k++) {   // 在每个像素位置进行多次采样
                    // 计算当前像素在图像中的位置
                    auto r = defaultSamplerInstance<UniformInSquare>().sample2d();
                    float rx = r.x;
                    float ry = r.y;
                    float x = (float(j)+rx)/float(width);
                    float y = (float(i)+ry)/float(height);
                    // 相机对象（camera）生成一条射线（ray），起始点为当前像素位置
                    auto ray = camera.shoot(x, y);
                    // 递归调用trace函数
                    color += trace(ray, 0);
                }
                // 取多次采样的平均值
                color /= samples;
                // 对平均颜色进行伽马校正
                color = gamma(color);
                // 将最终颜色值存储到图像像素数组（pixels）中，根据像素的位置进行索引计算
                pixels[(height-i-1)*width+j] = {color, 1};
            }
        }
    }
    

    auto SimplePathTracerRenderer::render() -> RenderResult {
        // shaders
        shaderPrograms.clear();
        ShaderCreator shaderCreator{};
        for (auto& m : scene.materials) {
            shaderPrograms.push_back(shaderCreator.create(m, scene.textures));
        }

        RGBA* pixels = new RGBA[width*height]{};

        // 局部坐标转换成世界坐标
        VertexTransformer vertexTransformer{};
        vertexTransformer.exec(spScene);

        const auto taskNums = 8;
        thread t[taskNums];
        for (int i=0; i < taskNums; i++) {
            t[i] = thread(&SimplePathTracerRenderer::renderTask,
                this, pixels, width, height, i, taskNums);
        }
        for(int i=0; i < taskNums; i++) {
            t[i].join();
        }
        getServer().logger.log("Done...");
        return {pixels, width, height};
    }

    void SimplePathTracerRenderer::release(const RenderResult& r) {
        auto [p, w, h] = r;
        delete[] p;
    }

    HitRecord SimplePathTracerRenderer::closestHitObject(const Ray& r) {
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
        // TODO：添加其他可能的物体
        return closestHit;
    }
    
    tuple<float, Vec3> SimplePathTracerRenderer::closestHitLight(const Ray& r) {
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

    RGB SimplePathTracerRenderer::trace(const Ray& r, int currDepth) {
        if (currDepth == depth) return scene.ambient.constant;
        auto hitObject = closestHitObject(r);
        auto [ t, emitted ] = closestHitLight(r);
        // hit object
        if (hitObject && hitObject->t < t) {
            auto mtlHandle = hitObject->material;   // 获取材质
            auto scattered = shaderPrograms[mtlHandle.index()]->shade(r, hitObject->hitPoint, hitObject->normal);
            auto scatteredRay = scattered.ray;
            auto attenuation = scattered.attenuation;
            auto emitted = scattered.emitted;
            auto next = trace(scatteredRay, currDepth+1);   // 递归
            float n_dot_in = glm::dot(hitObject->normal, scatteredRay.direction);
            float pdf = scattered.pdf;
            /**
             * emitted      - Le(p, w_0)
             * next         - Li(p, w_i)
             * n_dot_in     - cos<n, w_i>
             * atteunation  - BRDF
             * pdf          - p(w)
             **/
            // rendering equation
            return emitted + attenuation * next * n_dot_in / pdf;
        }
        // 存在光线与光源的交点，则返回光源发射的辐射强度
        else if (t != FLOAT_INF) {
            return emitted;
        }
        // 如果光线没有与物体或光源相交，则返回黑色
        else {
            return Vec3{0};
        }
    }
}