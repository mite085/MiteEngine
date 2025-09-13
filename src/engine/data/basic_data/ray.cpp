#include "ray.h"


namespace mite {

Ray::Ray()
    : origin(0.0f),
      direction(0.0f, 0.0f, 1.0f),
      tMin(0.0f),
      tMax(std::numeric_limits<float>::max())
{
}

Ray::Ray(const glm::vec3 &origin, const glm::vec3 &direction)
    : origin(origin),
      direction(glm::normalize(direction)),
      tMin(0.0f),
      tMax(std::numeric_limits<float>::max())
{
}

glm::vec3 Ray::GetPoint(float t) const
{
  return origin + direction * t;
}

bool Ray::Intersects(const AABB &aabb, float &t) const
{
  // SLAB方法进行AABB相交测试
  float tmin = tMin;
  float tmax = tMax;

  for (int i = 0; i < 3; ++i) {
    if (std::abs(direction[i]) < 1e-6f) {
      // 射线与轴平行
      if (origin[i] < aabb.min[i] || origin[i] > aabb.max[i]) {
        return false;
      }
    }
    else {
      float invD = 1.0f / direction[i];
      float t1 = (aabb.min[i] - origin[i]) * invD;
      float t2 = (aabb.max[i] - origin[i]) * invD;

      if (t1 > t2)
        std::swap(t1, t2);
      tmin = std::max(tmin, t1);
      tmax = std::min(tmax, t2);

      if (tmin > tmax)
        return false;
    }
  }

  t = tmin;
  return true;
}

bool Ray::Intersects(const Sphere &sphere, float &t) const
{
  glm::vec3 oc = origin - sphere.center;
  float a = glm::dot(direction, direction);
  float b = 2.0f * glm::dot(oc, direction);
  float c = glm::dot(oc, oc) - sphere.radius * sphere.radius;

  float discriminant = b * b - 4 * a * c;

  if (discriminant < 0.0f)
    return false;

  float sqrtDiscriminant = std::sqrt(discriminant);
  float t0 = (-b - sqrtDiscriminant) / (2.0f * a);
  float t1 = (-b + sqrtDiscriminant) / (2.0f * a);

  if (t0 > t1)
    std::swap(t0, t1);

  if (t0 < tMin) {
    t0 = t1;
    if (t0 < tMin)
      return false;
  }

  if (t0 > tMax)
    return false;

  t = t0;
  return true;
}

bool Ray::Intersects(const Plane &plane, float &t) const
{
  float denom = glm::dot(plane.normal, direction);
  if (std::abs(denom) < 1e-6f)
    return false;  // 射线与平面平行

  t = (plane.distance - glm::dot(plane.normal, origin)) / denom;
  return t >= tMin && t <= tMax;
}

bool Ray::Intersects(const glm::vec3 &v0,
                     const glm::vec3 &v1,
                     const glm::vec3 &v2,
                     float &t,
                     float &u,
                     float &v) const
{
  // Möller–Trumbore算法
  glm::vec3 edge1 = v1 - v0;
  glm::vec3 edge2 = v2 - v0;
  glm::vec3 pvec = glm::cross(direction, edge2);

  float det = glm::dot(edge1, pvec);
  if (std::abs(det) < 1e-6f)
    return false;  // 射线与三角形平行

  float invDet = 1.0f / det;
  glm::vec3 tvec = origin - v0;
  u = glm::dot(tvec, pvec) * invDet;
  if (u < 0.0f || u > 1.0f)
    return false;

  glm::vec3 qvec = glm::cross(tvec, edge1);
  v = glm::dot(direction, qvec) * invDet;
  if (v < 0.0f || u + v > 1.0f)
    return false;

  t = glm::dot(edge2, qvec) * invDet;
  return t >= tMin && t <= tMax;
}

}  // namespace mite
