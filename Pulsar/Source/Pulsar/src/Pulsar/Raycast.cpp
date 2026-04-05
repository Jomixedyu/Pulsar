#include "Raycast.h"

#include <algorithm>
#include <limits>

namespace pulsar
{
    bool RayUtils::RayAABBIntersect(const Ray& ray, const BoxBounds3f& box, float& outDist)
    {
        float tmin = 0.0f;
        float tmax = std::numeric_limits<float>::max();

        // X axis
        {
            float invD = (ray.Direction.x == 0.0f) ? 1e38f : (1.0f / ray.Direction.x);
            float t0 = (box.Min.x - ray.Origin.x) * invD;
            float t1 = (box.Max.x - ray.Origin.x) * invD;
            if (invD < 0.0f) std::swap(t0, t1);
            tmin = std::max(tmin, t0);
            tmax = std::min(tmax, t1);
            if (tmax < tmin) return false;
        }
        // Y axis
        {
            float invD = (ray.Direction.y == 0.0f) ? 1e38f : (1.0f / ray.Direction.y);
            float t0 = (box.Min.y - ray.Origin.y) * invD;
            float t1 = (box.Max.y - ray.Origin.y) * invD;
            if (invD < 0.0f) std::swap(t0, t1);
            tmin = std::max(tmin, t0);
            tmax = std::min(tmax, t1);
            if (tmax < tmin) return false;
        }
        // Z axis
        {
            float invD = (ray.Direction.z == 0.0f) ? 1e38f : (1.0f / ray.Direction.z);
            float t0 = (box.Min.z - ray.Origin.z) * invD;
            float t1 = (box.Max.z - ray.Origin.z) * invD;
            if (invD < 0.0f) std::swap(t0, t1);
            tmin = std::max(tmin, t0);
            tmax = std::min(tmax, t1);
            if (tmax < tmin) return false;
        }

        outDist = tmin;
        return true;
    }

    bool RayUtils::WorldToScreenPoint(
        const Vector3f& worldPos,
        const Matrix4f& viewProjMat,
        Vector2f vpPos,
        Vector2f vpSize,
        Vector2f& outScreenPos)
    {
        // World -> clip space
        Vector4f clip = viewProjMat * Vector4f{worldPos.x, worldPos.y, worldPos.z, 1.0f};

        // w <= 0 means the point is behind the camera
        if (clip.w <= 0.0f)
            return false;

        // Perspective divide -> NDC [-1, 1]
        float ndcX = clip.x / clip.w;
        float ndcY = clip.y / clip.w;

        // NDC -> screen pixels (flip Y: NDC Y is up, screen Y is down)
        outScreenPos.x = vpPos.x + (ndcX * 0.5f + 0.5f) * vpSize.x;
        outScreenPos.y = vpPos.y + (1.0f - (ndcY * 0.5f + 0.5f)) * vpSize.y;

        return true;
    }

} // namespace pulsar