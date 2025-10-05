#ifndef MITE_BOUNDING_VOLUMES_INTERSECTIONS_H
#define MITE_BOUNDING_VOLUMES_INTERSECTIONS_H

#include "basic_type/bounding_volumes_types.h"

namespace mite {
/**
 * @brief 相交测试工具类
 */
class BoundingVolumeIntersection {
 public:

  /**
   * @brief 相交测试结果枚举
   *
   * Intersects函数的第一个参数aabb1为主体，第二个参数为aabb2为被测对象
   * 所有的Intersection语义均为aabb2被测对象 相对于 aabb1主体的状态描述
   */
  enum class IntersectionType {
    Outside,    // 完全在外 （aabb2与aabb1完全不相干）
    Inside,     // 完全在内 （aabb2在aabb1的内部）
    Intersect,  // 相交     （aabb2与aabb1局部重合）
    Covered     // 被包含   （aabb2完全包含了aabb1）
  };

  // ==================== AABB相交测试声明 ====================
  static IntersectionType Intersects(const BoundingVolumeAABB &aabb, const BoundingVolumeSphere &sphere);
  static IntersectionType Intersects(const BoundingVolumeAABB &aabb, const BoundingVolumeOBB &obb);
  static IntersectionType Intersects(const BoundingVolumeAABB &aabb,
                                     const BoundingVolumePlane &plane);
  static IntersectionType Intersects(const BoundingVolumeAABB &aabb1,
                                     const BoundingVolumeAABB &aabb2);

  // ==================== Sphere相交测试声明 ====================
  static IntersectionType Intersects(const BoundingVolumeSphere &sphere,
                                     const BoundingVolumeAABB &aabb);
  static IntersectionType Intersects(const BoundingVolumeSphere &sphere,
                                     const BoundingVolumeOBB &obb);
  static IntersectionType Intersects(const BoundingVolumeSphere &sphere,
                              const BoundingVolumePlane &plane);
  static IntersectionType Intersects(const BoundingVolumeSphere &sphere1,
                              const BoundingVolumeSphere &sphere2);

  // ==================== OBB相交测试声明 ====================
  static IntersectionType Intersects(const BoundingVolumeOBB &obb, const BoundingVolumeAABB &aabb);
  static IntersectionType Intersects(const BoundingVolumeOBB &obb,
                                     const BoundingVolumeSphere &sphere);
  static IntersectionType Intersects(const BoundingVolumeOBB &obb,
                                     const BoundingVolumePlane &plane);
  static IntersectionType Intersects(const BoundingVolumeOBB &obb1, const BoundingVolumeOBB &obb2);

  // ==================== Plane相交测试声明 ====================
  static IntersectionType Intersects(const BoundingVolumePlane &plane,
                                     const BoundingVolumeAABB &aabb);
  static IntersectionType Intersects(const BoundingVolumePlane &plane,
                              const BoundingVolumeSphere &sphere);
  static IntersectionType Intersects(const BoundingVolumePlane &plane,
                                     const BoundingVolumeOBB &obb);
};
}  // namespace mite

#endif  // MITE_BOUNDING_VOLUMES_TYPES_H