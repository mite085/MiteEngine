#include "bounding_volumes.h"

namespace mite {
AABB BoundingVolumes::CreateAABBFromPoints(const glm::vec3 *points, uint32_t count)
{
  return AABB::CreateAABBFromPoints(points, count);
}

AABB BoundingVolumes::TransformAABB(const AABB &original, const glm::mat4 &transform)
{
  return original.Transform(transform);
}

AABB BoundingVolumes::MergeAABBs(const AABB &a, const AABB &b)
{
  return AABB::Merge(a, b);
}

bool BoundingVolumes::PointInAABB(const glm::vec3 &point, const AABB &aabb)
{
  return aabb.Contains(point);
}

bool BoundingVolumes::AABBIntersectsAABB(const AABB &a, const AABB &b)
{
  return a.Intersects(b);
}

bool BoundingVolumes::SphereIntersectsAABB(const Sphere &sphere, const AABB &aabb)
{
  // 计算球心到AABB最近点的距离平方
  glm::vec3 closestPoint = glm::clamp(sphere.center, aabb.min, aabb.max);
  float distSq = glm::distance(sphere.center, closestPoint) *
                 glm::distance(sphere.center, closestPoint);
  return distSq <= (sphere.radius * sphere.radius);
}

bool BoundingVolumes::SphereIntersectsSphere(const Sphere &a, const Sphere &b)
{
  float dist = glm::distance(a.center, b.center);
  return dist <= (a.radius + b.radius);
}

Sphere BoundingVolumes::CreateSphereFromAABB(const AABB &aabb)
{
  return Sphere::FromAABB(aabb);
}

Sphere BoundingVolumes::TransformSphere(const Sphere &sphere, const glm::mat4 &transform)
{
  return sphere.Transform(transform);
}

OBB BoundingVolumes::CreateOBBFromAABB(const AABB &aabb)
{
  return OBB::FromAABB(aabb);
}

OBB BoundingVolumes::TransformOBB(const OBB &obb, const glm::mat4 &transform)
{
  return obb.Transform(transform);
}

AABB BoundingVolumes::GetAABBFromOBB(const OBB &obb)
{
  return obb.GetAABB();
}
}  // namespace mite
