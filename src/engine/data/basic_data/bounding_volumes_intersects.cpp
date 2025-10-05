#include "bounding_volumes_intersects.h"

namespace mite {
// ==================== AABB相交测试实现 ====================

BoundingVolumeIntersection::IntersectionType BoundingVolumeIntersection::Intersects(
    const BoundingVolumeAABB &aabb, const BoundingVolumeSphere &sphere)
{
  float distSq = aabb.DistanceToPointSq(sphere.center);
  float radiusSq = sphere.radius * sphere.radius;

  if (distSq > radiusSq)
    return IntersectionType::Outside;

  // 检查球是否完全在AABB内部
  glm::vec3 aabbMin = aabb.min + glm::vec3(sphere.radius);
  glm::vec3 aabbMax = aabb.max - glm::vec3(sphere.radius);
  bool sphereInsideAABB = (sphere.center.x >= aabbMin.x && sphere.center.x <= aabbMax.x) &&
                          (sphere.center.y >= aabbMin.y && sphere.center.y <= aabbMax.y) &&
                          (sphere.center.z >= aabbMin.z && sphere.center.z <= aabbMax.z);

  if (sphereInsideAABB)
    return IntersectionType::Inside;

  // 检查AABB是否完全在球内部
  bool aabbInsideSphere = true;
  glm::vec3 aabbVertices[8] = {aabb.min,
                               {aabb.min.x, aabb.min.y, aabb.max.z},
                               {aabb.min.x, aabb.max.y, aabb.min.z},
                               {aabb.min.x, aabb.max.y, aabb.max.z},
                               {aabb.max.x, aabb.min.y, aabb.min.z},
                               {aabb.max.x, aabb.min.y, aabb.max.z},
                               {aabb.max.x, aabb.max.y, aabb.min.z},
                               aabb.max};

  for (int i = 0; i < 8; ++i) {
    if (glm::distance2(sphere.center, aabbVertices[i]) > radiusSq) {
      aabbInsideSphere = false;
      break;
    }
  }

  if (aabbInsideSphere)
    return IntersectionType::Covered;
  return IntersectionType::Intersect;
}

BoundingVolumeIntersection::IntersectionType BoundingVolumeIntersection::Intersects(
    const BoundingVolumeAABB &aabb, const BoundingVolumeOBB &obb)
{
  // 将OBB转换为AABB进行近似测试
  BoundingVolumeAABB obbAABB = obb.GetAABB();
  return Intersects(aabb, obbAABB);
}

BoundingVolumeIntersection::IntersectionType BoundingVolumeIntersection::Intersects(
    const BoundingVolumeAABB &aabb, const BoundingVolumePlane &plane)
{
  // 计算AABB在平面法线方向上的投影
  glm::vec3 center = aabb.GetCenter();
  glm::vec3 extents = aabb.GetHalfExtents();

  float r = extents.x * std::abs(plane.normal.x) + extents.y * std::abs(plane.normal.y) +
            extents.z * std::abs(plane.normal.z);

  float distance = plane.DistanceToPoint(center);

  if (distance > r)
    return IntersectionType::Outside;
  if (distance < -r)
    return IntersectionType::Inside;
  return IntersectionType::Intersect;
}

BoundingVolumeIntersection::IntersectionType BoundingVolumeIntersection::Intersects(
    const BoundingVolumeAABB &aabb1, const BoundingVolumeAABB &aabb2)
{
  if (!aabb1.Intersects(aabb2))
    return IntersectionType::Outside;
  if (aabb1.Contains(aabb2))
    return IntersectionType::Inside;
  if (aabb2.Contains(aabb1))
    return IntersectionType::Covered;
  return IntersectionType::Intersect;
}

// ==================== Sphere相交测试实现 ====================

BoundingVolumeIntersection::IntersectionType BoundingVolumeIntersection::Intersects(
    const BoundingVolumeSphere &sphere, const BoundingVolumeAABB &aabb)
{
  // 利用对称性，但需要交换Inside和Covered
  IntersectionType result = Intersects(aabb, sphere);
  if (result == IntersectionType::Inside)
    return IntersectionType::Covered;
  if (result == IntersectionType::Covered)
    return IntersectionType::Inside;
  return result;
}

BoundingVolumeIntersection::IntersectionType BoundingVolumeIntersection::Intersects(
    const BoundingVolumeSphere &sphere, const BoundingVolumeOBB &obb)
{
  // 将点转换到OBB局部空间
  glm::vec3 localPoint = glm::transpose(obb.orientation) * (sphere.center - obb.center);

  // 计算OBB局部空间中的最近点
  glm::vec3 closestPoint;
  closestPoint.x = glm::clamp(localPoint.x, -obb.extents.x, obb.extents.x);
  closestPoint.y = glm::clamp(localPoint.y, -obb.extents.y, obb.extents.y);
  closestPoint.z = glm::clamp(localPoint.z, -obb.extents.z, obb.extents.z);

  // 转换回世界空间
  glm::vec3 worldClosest = obb.center + obb.orientation * closestPoint;
  float distSq = glm::distance2(sphere.center, worldClosest);
  float radiusSq = sphere.radius * sphere.radius;

  if (distSq > radiusSq)
    return IntersectionType::Outside;

  // 检查球是否完全在OBB内部
  bool sphereInsideOBB = (std::abs(localPoint.x) <= obb.extents.x - sphere.radius) &&
                         (std::abs(localPoint.y) <= obb.extents.y - sphere.radius) &&
                         (std::abs(localPoint.z) <= obb.extents.z - sphere.radius);

  if (sphereInsideOBB)
    return IntersectionType::Inside;

  // 检查OBB是否完全在球内部
  glm::vec3 obbVertices[8];
  obb.GetVertices(obbVertices);
  bool obbInsideSphere = true;

  for (int i = 0; i < 8; ++i) {
    if (glm::distance2(sphere.center, obbVertices[i]) > radiusSq) {
      obbInsideSphere = false;
      break;
    }
  }

  if (obbInsideSphere)
    return IntersectionType::Covered;
  return IntersectionType::Intersect;
}

BoundingVolumeIntersection::IntersectionType BoundingVolumeIntersection::Intersects(
    const BoundingVolumeSphere &sphere, const BoundingVolumePlane &plane)
{
  float distance = plane.DistanceToPoint(sphere.center);

  if (distance > sphere.radius)
    return IntersectionType::Outside;
  if (distance < -sphere.radius)
    return IntersectionType::Inside;
  return IntersectionType::Intersect;
}

BoundingVolumeIntersection::IntersectionType BoundingVolumeIntersection::Intersects(
    const BoundingVolumeSphere &sphere1, const BoundingVolumeSphere &sphere2)
{
  float centerDist = glm::distance(sphere1.center, sphere2.center);
  float radiusSum = sphere1.radius + sphere2.radius;
  float radiusDiff = std::abs(sphere1.radius - sphere2.radius);

  if (centerDist > radiusSum)
    return IntersectionType::Outside;
  if (centerDist < radiusDiff) {
    if (sphere1.radius > sphere2.radius) {
      return IntersectionType::Inside;  // sphere2在sphere1内部
    }
    else {
      return IntersectionType::Covered;  // sphere1在sphere2内部
    }
  }
  return IntersectionType::Intersect;
}

// ==================== OBB相交测试实现 ====================

BoundingVolumeIntersection::IntersectionType BoundingVolumeIntersection::Intersects(
    const BoundingVolumeOBB &obb, const BoundingVolumeAABB &aabb)
{
  // 利用对称性，但需要交换Inside和Covered
  IntersectionType result = Intersects(aabb, obb);
  if (result == IntersectionType::Inside)
    return IntersectionType::Covered;
  if (result == IntersectionType::Covered)
    return IntersectionType::Inside;
  return result;
}

BoundingVolumeIntersection::IntersectionType BoundingVolumeIntersection::Intersects(
    const BoundingVolumeOBB &obb, const BoundingVolumeSphere &sphere)
{
  // 利用对称性，但需要交换Inside和Covered
  IntersectionType result = Intersects(sphere, obb);
  if (result == IntersectionType::Inside)
    return IntersectionType::Covered;
  if (result == IntersectionType::Covered)
    return IntersectionType::Inside;
  return result;
}

BoundingVolumeIntersection::IntersectionType BoundingVolumeIntersection::Intersects(
    const BoundingVolumeOBB &obb, const BoundingVolumePlane &plane)
{
  // 计算OBB在平面法线方向上的投影范围
  float r = obb.extents.x * std::abs(glm::dot(plane.normal, obb.orientation[0])) +
            obb.extents.y * std::abs(glm::dot(plane.normal, obb.orientation[1])) +
            obb.extents.z * std::abs(glm::dot(plane.normal, obb.orientation[2]));

  float distance = plane.DistanceToPoint(obb.center);

  if (distance > r)
    return IntersectionType::Outside;
  if (distance < -r)
    return IntersectionType::Inside;
  return IntersectionType::Intersect;
}

BoundingVolumeIntersection::IntersectionType BoundingVolumeIntersection::Intersects(
    const BoundingVolumeOBB &obb1, const BoundingVolumeOBB &obb2)
{
  // 使用分离轴定理(SAT)进行精确的OBB-OBB相交测试
  //
  // 特点：
  // 完整的SAT测试：测试所有15个可能的分离轴（6个面法线轴 + 9个边叉积轴）
  // 精确的包含判断：通过将OBB转换到对方局部空间来精确判断包含关系
  // 数值稳定性：处理接近零长度的叉积轴，避免除零错误
  // 完整的相交类型判断：能够区分Outside、Inside、Covered和Intersect四种状态

  glm::vec3 centerDiff = obb2.center - obb1.center;
  float ra, rb;
  glm::vec3 axis;

  // 测试OBB1的3个轴
  for (int i = 0; i < 3; ++i) {
    axis = obb1.orientation[i];
    ra = obb1.extents[i];
    rb = obb2.extents[0] * std::abs(glm::dot(axis, obb2.orientation[0])) +
         obb2.extents[1] * std::abs(glm::dot(axis, obb2.orientation[1])) +
         obb2.extents[2] * std::abs(glm::dot(axis, obb2.orientation[2]));

    float distance = std::abs(glm::dot(centerDiff, axis));
    if (distance > ra + rb)
      return IntersectionType::Outside;
  }

  // 测试OBB2的3个轴
  for (int i = 0; i < 3; ++i) {
    axis = obb2.orientation[i];
    rb = obb2.extents[i];
    ra = obb1.extents[0] * std::abs(glm::dot(axis, obb1.orientation[0])) +
         obb1.extents[1] * std::abs(glm::dot(axis, obb1.orientation[1])) +
         obb1.extents[2] * std::abs(glm::dot(axis, obb1.orientation[2]));

    float distance = std::abs(glm::dot(centerDiff, axis));
    if (distance > ra + rb)
      return IntersectionType::Outside;
  }

  // 测试9个边叉积轴
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      axis = glm::cross(obb1.orientation[i], obb2.orientation[j]);
      if (glm::length2(axis) < 1e-6f)
        continue;

      axis = glm::normalize(axis);
      ra = obb1.extents[0] * std::abs(glm::dot(axis, obb1.orientation[0])) +
           obb1.extents[1] * std::abs(glm::dot(axis, obb1.orientation[1])) +
           obb1.extents[2] * std::abs(glm::dot(axis, obb1.orientation[2]));

      rb = obb2.extents[0] * std::abs(glm::dot(axis, obb2.orientation[0])) +
           obb2.extents[1] * std::abs(glm::dot(axis, obb2.orientation[1])) +
           obb2.extents[2] * std::abs(glm::dot(axis, obb2.orientation[2]));

      float distance = std::abs(glm::dot(centerDiff, axis));
      if (distance > ra + rb)
        return IntersectionType::Outside;
    }
  }

  // 检查包含关系
  glm::mat3 invOrientation1 = glm::transpose(obb1.orientation);
  glm::vec3 center2InLocal1 = invOrientation1 * (obb2.center - obb1.center);
  glm::mat3 orientation2InLocal1 = invOrientation1 * obb2.orientation;

  bool obb2InsideObb1 = true;
  for (int i = 0; i < 8; ++i) {
    glm::vec3 vertex = center2InLocal1;
    vertex.x += orientation2InLocal1[0].x * obb2.extents.x * (i & 1 ? 1.0f : -1.0f);
    vertex.y += orientation2InLocal1[1].y * obb2.extents.y * (i & 2 ? 1.0f : -1.0f);
    vertex.z += orientation2InLocal1[2].z * obb2.extents.z * (i & 4 ? 1.0f : -1.0f);

    if (std::abs(vertex.x) > obb1.extents.x || std::abs(vertex.y) > obb1.extents.y ||
        std::abs(vertex.z) > obb1.extents.z)
    {
      obb2InsideObb1 = false;
      break;
    }
  }

  if (obb2InsideObb1)
    return IntersectionType::Inside;

  glm::mat3 invOrientation2 = glm::transpose(obb2.orientation);
  glm::vec3 center1InLocal2 = invOrientation2 * (obb1.center - obb2.center);
  glm::mat3 orientation1InLocal2 = invOrientation2 * obb1.orientation;

  bool obb1InsideObb2 = true;
  for (int i = 0; i < 8; ++i) {
    glm::vec3 vertex = center1InLocal2;
    vertex.x += orientation1InLocal2[0].x * obb1.extents.x * (i & 1 ? 1.0f : -1.0f);
    vertex.y += orientation1InLocal2[1].y * obb1.extents.y * (i & 2 ? 1.0f : -1.0f);
    vertex.z += orientation1InLocal2[2].z * obb1.extents.z * (i & 4 ? 1.0f : -1.0f);

    if (std::abs(vertex.x) > obb2.extents.x || std::abs(vertex.y) > obb2.extents.y ||
        std::abs(vertex.z) > obb2.extents.z)
    {
      obb1InsideObb2 = false;
      break;
    }
  }

  if (obb1InsideObb2)
    return IntersectionType::Covered;
  return IntersectionType::Intersect;
}

// ==================== Plane相交测试实现 ====================

BoundingVolumeIntersection::IntersectionType BoundingVolumeIntersection::Intersects(
    const BoundingVolumePlane &plane, const BoundingVolumeAABB &aabb)
{
  // 平面与AABB的相交测试，平面总是包含或被包含
  return Intersects(aabb, plane);
}

BoundingVolumeIntersection::IntersectionType BoundingVolumeIntersection::Intersects(
    const BoundingVolumePlane &plane, const BoundingVolumeSphere &sphere)
{
  // 平面与球的相交测试，平面总是包含或被包含
  return Intersects(sphere, plane);
}

BoundingVolumeIntersection::IntersectionType BoundingVolumeIntersection::Intersects(
    const BoundingVolumePlane &plane, const BoundingVolumeOBB &obb)
{
  // 平面与OBB的相交测试，平面总是包含或被包含
  return Intersects(obb, plane);
}
}  // namespace mite