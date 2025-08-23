#include "bounding_volumes_types.h"

namespace mite {

// ==================== AABB成员函数实现 ====================

AABB::AABB(const glm::vec3 &center, float halfExtent)
{
  min = center - glm::vec3(halfExtent);
  max = center + glm::vec3(halfExtent);
}

void AABB::Expand(const glm::vec3 &point)
{
  min = glm::min(min, point);
  max = glm::max(max, point);
}

void AABB::Expand(const AABB &other)
{
  min = glm::min(min, other.min);
  max = glm::max(max, other.max);
}

AABB AABB::Transform(const glm::mat4 &matrix) const
{
  // 变换AABB的8个顶点并重新计算包围盒
  glm::vec3 vertices[8] = {{min.x, min.y, min.z},
                           {min.x, min.y, max.z},
                           {min.x, max.y, min.z},
                           {min.x, max.y, max.z},
                           {max.x, min.y, min.z},
                           {max.x, min.y, max.z},
                           {max.x, max.y, min.z},
                           {max.x, max.y, max.z}};

  AABB result;
  for (int i = 0; i < 8; ++i) {
    glm::vec4 transformed = matrix * glm::vec4(vertices[i], 1.0f);
    result.Expand(glm::vec3(transformed));
  }
  return result;
}

float AABB::GetSurfaceArea() const
{
  glm::vec3 size = max - min;
  return 2.0f * (size.x * size.y + size.x * size.z + size.y * size.z);
}

float AABB::GetVolume() const
{
  glm::vec3 size = max - min;
  return size.x * size.y * size.z;
}

bool AABB::Contains(const glm::vec3 &point) const
{
  return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y &&
         point.z >= min.z && point.z <= max.z;
}

bool AABB::Contains(const AABB &other) const
{
  return Contains(other.min) && Contains(other.max);
}

bool AABB::Intersects(const AABB &other) const
{
  return (min.x <= other.max.x && max.x >= other.min.x) &&
         (min.y <= other.max.y && max.y >= other.min.y) &&
         (min.z <= other.max.z && max.z >= other.min.z);
}

AABB AABB::Merge(const AABB &a, const AABB &b)
{
  return AABB(glm::min(a.min, b.min), glm::max(a.max, b.max));
}

AABB AABB::CreateAABBFromPoints(const glm::vec3 *points, uint32_t count)
{
  AABB result;
  if (count == 0)
    return result;

  glm::vec3 minPoint = points[0];
  glm::vec3 maxPoint = points[0];

  for (uint32_t i = 1; i < count; ++i) {
    minPoint = glm::min(minPoint, points[i]);
    maxPoint = glm::max(maxPoint, points[i]);
  }

  result.min = minPoint;
  result.max = maxPoint;
  return result;
}

// ==================== Sphere成员函数实现 ====================

Sphere Sphere::FromAABB(const AABB &aabb)
{
  glm::vec3 center = aabb.GetCenter();
  float radius = glm::length(aabb.GetHalfExtents());
  return Sphere(center, radius);
}

bool Sphere::Contains(const glm::vec3 &point) const
{
  return (glm::distance(center, point) * glm::distance(center, point)) <= (radius * radius);
}

bool Sphere::Contains(const Sphere &other) const
{
  float dist = glm::distance(center, other.center);
  return dist + other.radius <= radius;
}

bool Sphere::Intersects(const Sphere &other) const
{
  float dist = glm::distance(center, other.center);
  return dist <= (radius + other.radius);
}

Sphere Sphere::Transform(const glm::mat4 &matrix) const
{
  // 提取平移和均匀缩放
  glm::vec3 newCenter = glm::vec3(matrix * glm::vec4(center, 1.0f));

  // 假设均匀缩放，取x轴缩放作为半径缩放
  float scale = glm::length(glm::vec3(matrix[0]));
  float newRadius = radius * scale;

  return Sphere(newCenter, newRadius);
}

// ==================== OBB成员函数实现 ====================

OBB OBB::FromAABB(const AABB &aabb)
{
  glm::vec3 center = aabb.GetCenter();
  glm::vec3 extents = aabb.GetHalfExtents();
  return OBB(center, extents, glm::mat3(1.0f));
}

void OBB::GetVertices(glm::vec3 vertices[8]) const
{
  glm::vec3 axes[3] = {
      orientation[0] * extents.x, orientation[1] * extents.y, orientation[2] * extents.z};

  vertices[0] = center - axes[0] - axes[1] - axes[2];
  vertices[1] = center - axes[0] - axes[1] + axes[2];
  vertices[2] = center - axes[0] + axes[1] - axes[2];
  vertices[3] = center - axes[0] + axes[1] + axes[2];
  vertices[4] = center + axes[0] - axes[1] - axes[2];
  vertices[5] = center + axes[0] - axes[1] + axes[2];
  vertices[6] = center + axes[0] + axes[1] - axes[2];
  vertices[7] = center + axes[0] + axes[1] + axes[2];
}

AABB OBB::GetAABB() const
{
  glm::vec3 vertices[8];
  GetVertices(vertices);
  return AABB::CreateAABBFromPoints(vertices, 8);
}

OBB OBB::Transform(const glm::mat4 &matrix) const
{
  glm::vec3 newCenter = glm::vec3(matrix * glm::vec4(center, 1.0f));
  glm::mat3 newOrientation = glm::mat3(matrix) * orientation;

  // 计算新的半长（考虑非均匀缩放）
  glm::vec3 scale = glm::vec3(glm::length(glm::vec3(matrix[0])),
                              glm::length(glm::vec3(matrix[1])),
                              glm::length(glm::vec3(matrix[2])));
  glm::vec3 newExtents = extents * scale;

  return OBB(newCenter, newExtents, newOrientation);
}

bool OBB::Contains(const glm::vec3 &point) const
{
  // 将点转换到OBB局部空间
  glm::vec3 localPoint = glm::transpose(orientation) * (point - center);
  return std::abs(localPoint.x) <= extents.x && std::abs(localPoint.y) <= extents.y &&
         std::abs(localPoint.z) <= extents.z;
}

// ==================== Plane成员函数实现 ====================

Plane::Plane(const glm::vec3 &point, const glm::vec3 &normal)
    : normal(glm::normalize(normal)), distance(glm::dot(normal, point))
{
}

float Plane::DistanceToPoint(const glm::vec3 &point) const
{
  return glm::dot(normal, point) - distance;
}

int Plane::GetSide(const glm::vec3 &point) const
{
  float dist = DistanceToPoint(point);
  if (dist > 0.0f)
    return 1;  // 正面
  if (dist < 0.0f)
    return -1;  // 背面
  return 0;     // 在平面上
}

}  // namespace mite
