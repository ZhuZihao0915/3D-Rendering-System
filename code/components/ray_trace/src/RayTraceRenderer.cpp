#include "RayTraceRenderer.hpp"

#include "VertexTransformer.hpp"
#include "intersections/intersections.hpp"
namespace RayTrace
{
    void RayTraceRenderer::release(const RenderResult& r) {
        auto [p, w, h] = r;
        delete[] p;
    }
    RGB RayTraceRenderer::gamma(const RGB& rgb) {
        return glm::sqrt(rgb);
    }
    auto RayTraceRenderer::render() -> RenderResult {
        auto width = scene.renderOption.width;
        auto height = scene.renderOption.height;
        auto pixels = new RGBA[width*height];

        VertexTransformer vertexTransformer{};
        vertexTransformer.exec(spScene);

        ShaderCreator shaderCreator{};
        for (auto& mtl : scene.materials) {
            shaderPrograms.push_back(shaderCreator.create(mtl, scene.textures));
        }

        for (int i=0; i<height; i++) {
            for (int j=0; j < width; j++) {
                auto ray = camera.shoot(float(j)/float(width), float(i)/float(height));
                auto color = trace(ray, 0);
                color = clamp(color);
                color = gamma(color);
                pixels[(height-i-1)*width+j] = {color, 1};
            }
        }

        return {pixels, width, height};
    }
    
    RGB RayTraceRenderer::trace(const Ray& r, int depth) {
        if (depth >= 10 && scene.pointLightBuffer.size() < 1) return {0, 0, 0};
        auto& l = scene.pointLightBuffer[0];
        auto closestHitObj = closestHit(r);
        if (closestHitObj) {
            auto& hitRec = *closestHitObj;
            auto out = glm::normalize(l.position - hitRec.hitPoint);
            if (glm::dot(out, hitRec.normal) < 0) {
                return {0, 0, 0};
            }
            auto distance = glm::length(l.position - hitRec.hitPoint);
            auto shadowRay = Ray{hitRec.hitPoint, out};
            auto shadowHit = closestHit(shadowRay);
            auto c = shaderPrograms[hitRec.material.index()]->shade(-r.direction, out, hitRec.normal);
            

            // 获得反射方向
            auto reflect_dir = reflect(r.direction, hitRec.normal);
            // 创建新的光线
            auto reflect_ray = Ray{ hitRec.hitPoint, glm::normalize(reflect_dir)}; 

            // 检查是否具有镜面反射属性
            bool reflectable = scene.materials[hitRec.material.index()].hasProperty("specularEx"); 
            // 计算反射光线的颜色
            auto color = shaderPrograms[hitRec.material.index()]->shade(r.direction, glm::normalize(reflect_dir), hitRec.normal);
            
            // 有反射
            if (reflectable) {
                if ((!shadowHit) || (shadowHit && shadowHit->t > distance)) {
                    return c * l.intensity + trace(reflect_ray, depth + 1) * color;
                }
                else {
                    return trace(reflect_ray, depth + 1) * color;
                }
            }
            // 只有漫反射
            else {
                if ((!shadowHit) || (shadowHit && shadowHit->t > distance)) {
                    return c * l.intensity;
                }
                else {
                    return Vec3{ 0 };
                }
            }
        }
        else {
            return {0, 0, 0};
        }
    }

    HitRecord RayTraceRenderer::closestHit(const Ray& r) {
        HitRecord closestHit = nullopt;
        float closest = FLOAT_INF;
        for (auto& s : scene.sphereBuffer) {
            auto hitRecord = Intersection::xSphere(r, s, 0.01, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        for (auto& t : scene.triangleBuffer) {
            auto hitRecord = Intersection::xTriangle(r, t, 0.01, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        for (auto& p : scene.planeBuffer) {
            auto hitRecord = Intersection::xPlane(r, p, 0.01, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        return closestHit; 
    }
}