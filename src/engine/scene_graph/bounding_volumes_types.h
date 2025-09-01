#ifndef MITE_BOUNDING_VOLUMES_TYPES_H
#define MITE_BOUNDING_VOLUMES_TYPES_H

#include "headers/headers.h"

namespace mite {

/**
 * @brief 轴对齐包围盒 (AABB)
 * 使用min-max表示法，适用于快速相交测试和空间划分
 */
struct AABB {
  glm::vec3 min;
  glm::vec3 max;

  /**
   * @brief 默认构造函数，创建无效的AABB
   */
  AABB() : min(FLT_MAX), max(-FLT_MAX) {}

  /**
   * @brief 通过最小最大点构造AABB
   */
  AABB(const glm::vec3 &min, const glm::vec3 &max) : min(min), max(max) {}

  /**
   * @brief 通过中心点和半长构造AABB
   */
  AABB(const glm::vec3 &center, float halfExtent);

  /**
   * @brief 判断AABB是否有效（min <= max）
   */
  bool IsValid() const
  {
    return min.x <= max.x && min.y <= max.y && min.z <= max.z;
  }

  /**
   * @brief 获取AABB中心点
   */
  glm::vec3 GetCenter() const
  {
    return (min + max) * 0.5f;
  }

  /**
   * @brief 获取AABB尺寸
   */
  glm::vec3 GetSize() const
  {
    return max - min;
  }

  /**
   * @brief 获取AABB半长
   */
  glm::vec3 GetHalfExtents() const
  {
    return GetSize() * 0.5f;
  }

  /**
   * @brief 扩展AABB以包含给定点
   */
  void Expand(const glm::vec3 &point);

  /**
   * @brief 扩展AABB以包含另一个AABB
   */
  void Expand(const AABB &other);

  /**
   * @brief 对AABB应用变换矩阵
   */
  AABB Transform(const glm::mat4 &matrix) const;

  /**
   * @brief 计算AABB的表面积（用于BVH构建）
   */
  float GetSurfaceArea() const;

  /**
   * @brief 计算AABB的体积
   */
  float GetVolume() const;

  /**
   * @brief 判断点是否在AABB内部
   */
  bool Contains(const glm::vec3 &point) const;

  /**
   * @brief 判断AABB是否完全包含另一个AABB
   */
  bool Contains(const AABB &other) const;

  /**
   * @brief 判断两个AABB是否相交
   */
  bool Intersects(const AABB &other) const;

  /**
   * @brief 计算点到AABB的平方距离
   */
  float DistanceToPointSq(const glm::vec3 &point) const;

  /**
   * @brief 合并两个AABB
   */
  static AABB Merge(const AABB &a, const AABB &b);

  /**
   * @brief 创建一个AABB
   */
  static AABB CreateAABBFromPoints(const glm::vec3 *points, uint32_t count);
};

/**
 * @brief 包围球
 * 使用中心和半径表示，适用于旋转不变形的物体
 */
struct Sphere {
  glm::vec3 center;
  float radius;

  Sphere() : center(0.0f), radius(0.0f) {}
  Sphere(const glm::vec3 &center, float radius) : center(center), radius(radius) {}

  /**
   * @brief 通过AABB构造包围球
   */
  static Sphere FromAABB(const AABB &aabb);

  /**
   * @brief 判断点是否在球内部
   */
  bool Contains(const glm::vec3 &point) const;

  /**
   * @brief 判断球是否完全包含另一个球
   */
  bool Contains(const Sphere &other) const;

  /**
   * @brief 判断两个球是否相交
   */
  bool Intersects(const Sphere &other) const;

  /**
   * @brief 对球应用变换（平移和均匀缩放）
   */
  Sphere Transform(const glm::mat4 &matrix) const;
};

/**
 * @brief 有向包围盒 (OBB)
 * 使用中心、半长和方向矩阵表示，适用于精确的旋转物体碰撞检测
 */
struct OBB {
  glm::vec3 center;
  glm::vec3 extents;      // 半长
  glm::mat3 orientation;  // 3x3旋转矩阵

  OBB() : center(0.0f), extents(0.0f), orientation(1.0f) {}
  OBB(const glm::vec3 &center, const glm::vec3 &extents, const glm::mat3 &orientation)
      : center(center), extents(extents), orientation(orientation)
  {
  }

  /**
   * @brief 通过AABB构造OBB（初始方向为单位矩阵）
   */
  static OBB FromAABB(const AABB &aabb);

  /**
   * @brief 获取OBB的8个顶点
   */
  void GetVertices(glm::vec3 vertices[8]) const;

  /**
   * @brief 获取OBB的AABB近似（用于快速剔除）
   */
  AABB GetAABB() const;

  /**
   * @brief 对OBB应用变换
   */
  OBB Transform(const glm::mat4 &matrix) const;

  /**
   * @brief 判断点是否在OBB内部
   */
  bool Contains(const glm::vec3 &point) const;
};

/**
 * @brief 平面定义
 * 用于视锥体裁剪和碰撞检测
 * 
 * 注意：
 * 右手系平面定义：normal·point + d = 0
 * Side的正值：点在平面正侧（法线指向的一侧）
 */
struct Plane {
  glm::vec3 normal;
  float distance;

  Plane() : normal(0.0f, 1.0f, 0.0f), distance(0.0f) {}
  Plane(const glm::vec3 &normal, float distance)
      : normal(glm::normalize(normal)), distance(distance)
  {
  }
  Plane(const glm::vec3 &point, const glm::vec3 &normal);

  /**
   * @brief 计算点到平面的距离
   */
  float DistanceToPoint(const glm::vec3 &point) const;

  /**
   * @brief 判断点在平面的哪一侧
   */
  int GetSide(const glm::vec3 &point) const;
};

/**
 * @brief 相交测试结果枚举
 */
enum class IntersectionType {
  Outside,   // 完全在外
  Inside,    // 完全在内
  Intersect  // 相交
};

}  // namespace mite

#endif  // MITE_BOUNDING_VOLUMES_TYPES_H
