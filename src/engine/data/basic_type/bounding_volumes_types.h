#ifndef MITE_BOUNDING_VOLUMES_TYPES_H
#define MITE_BOUNDING_VOLUMES_TYPES_H

#include "headers/headers.h"

namespace mite {
/**
 * @enum BoundingVolumeType
 * @brief 包围体类型枚举
 */
enum class BoundingVolumeType {
  AABB,    // 轴对齐包围盒
  OBB,     // 有向包围盒
  Sphere,  // 包围球
  Plane,   // 平面
  None     // 无包围体
};
/**
 * @brief 轴对齐包围盒 (AABB)
 * 使用min-max表示法，适用于快速相交测试和空间划分
 */
struct BoundingVolumeAABB {
  glm::vec3 min;
  glm::vec3 max;

  /**
   * @brief 默认构造函数，创建无效的AABB
   */
  BoundingVolumeAABB() : min(FLT_MAX), max(-FLT_MAX) {}
  /**
   * @brief 通过最小最大点构造AABB
   */
  BoundingVolumeAABB(const glm::vec3 &min, const glm::vec3 &max) : min(min), max(max) {}
  /**
   * @brief 通过中心点和半长构造AABB
   */
  BoundingVolumeAABB(const glm::vec3 &center, float halfExtent);

  /**
   * @brief 判断AABB是否有效（min <= max）
   */
  bool IsValid() const;
  /**
   * @brief 获取AABB中心点
   */
  glm::vec3 GetCenter() const;
  /**
   * @brief 获取AABB尺寸
   */
  glm::vec3 GetSize() const;
  /**
   * @brief 获取AABB半长
   */
  glm::vec3 GetHalfExtents() const;
  /**
   * @brief 对AABB应用变换矩阵
   * @return 应用变换之后的AABB
   */
  BoundingVolumeAABB Transform(const glm::mat4 &matrix) const;
  /**
   * @brief 计算AABB的表面积（用于BVH构建）
   */
  float GetSurfaceArea() const;
  /**
   * @brief 计算AABB的体积
   */
  float GetVolume() const;

  /**
   * @brief 判断两个AABB是否相交
   */
  bool Intersects(const BoundingVolumeAABB &other) const;
  /**
   * @brief 判断点是否在AABB内部
   */
  bool Contains(const glm::vec3 &point) const;
  /**
   * @brief 判断AABB是否完全包含另一个AABB
   */
  bool Contains(const BoundingVolumeAABB &other) const;
  /**
   * @brief 扩展AABB以包含给定点
   */
  BoundingVolumeAABB Expand(const glm::vec3 &point);
  /**
   * @brief 扩展AABB以包含另一个AABB
   */
  BoundingVolumeAABB Expand(const BoundingVolumeAABB &other);

  /**
   * @brief 计算点到AABB的平方距离
   */
  float DistanceToPointSq(const glm::vec3 &point) const;
  /**
   * @brief 创建一个AABB
   */
  static BoundingVolumeAABB CreateAABBFromPoints(const glm::vec3 *points, uint32_t count);
};

/**
 * @brief 包围球
 * 使用中心和半径表示，适用于旋转不变形的物体
 */
struct BoundingVolumeSphere {
  glm::vec3 center;
  float radius;

  BoundingVolumeSphere() : center(0.0f), radius(0.0f) {}
  BoundingVolumeSphere(const glm::vec3 &center, float radius) : center(center), radius(radius) {}

  /**
   * @brief 通过AABB构造包围球（外切球）
   */
  static BoundingVolumeSphere FromAABB(const BoundingVolumeAABB &aabb);

  /**
   * @brief 判断两个球是否相交
   */
  bool Intersects(const BoundingVolumeSphere &other) const;
  /**
   * @brief 判断点是否在球内部
   */
  bool Contains(const glm::vec3 &point) const;
  /**
   * @brief 判断球是否完全包含另一个球
   */
  bool Contains(const BoundingVolumeSphere &other) const;
  /**
   * @brief 扩展球以包含给定点
   */
  BoundingVolumeSphere Expand(const glm::vec3 &point);
  /**
   * @brief 扩展球以包含另一个球
   */
  BoundingVolumeSphere Expand(const BoundingVolumeSphere &other);

  /**
   * @brief 对球应用变换（平移和均匀缩放）
   * @return 应用变换之后的球
   */
  BoundingVolumeSphere Transform(const glm::mat4 &matrix) const;

  /**
   * @brief 从点集创建最小包围球（Welzl算法实现，最优）
   * @param points 点集
   * @return 最小包围球
   */
  static BoundingVolumeSphere CreateSphereFromPoints(const std::vector<glm::vec3> &points);
  /**
   * @brief 从点集创建近似包围球（Ritter算法，更快但非最优）
   * @param points 点集
   * @return 近似包围球
   */
  static BoundingVolumeSphere CreateSphereFromPointsRitter(const std::vector<glm::vec3> &points);
  /**
   * @brief 从点集创建快速包围球（AABB中心+最远点）
   * @param points 点集
   * @return 快速包围球
   */
  static BoundingVolumeSphere CreateSphereFromPointsFast(const std::vector<glm::vec3> &points);
};

/**
 * @brief 有向包围盒 (OBB)
 * 使用中心、半长和方向矩阵表示，适用于精确的旋转物体碰撞检测
 */
struct BoundingVolumeOBB {
  glm::vec3 center;
  glm::vec3 extents;      // 半长
  glm::mat3 orientation;  // 3x3旋转矩阵

  BoundingVolumeOBB() : center(0.0f), extents(0.0f), orientation(1.0f) {}
  BoundingVolumeOBB(const glm::vec3 &center,
                    const glm::vec3 &extents,
                    const glm::mat3 &orientation)
      : center(center), extents(extents), orientation(orientation)
  {
  }

  /**
   * @brief 通过AABB构造OBB（初始方向为单位矩阵）
   */
  static BoundingVolumeOBB FromAABB(const BoundingVolumeAABB &aabb);

  /**
   * @brief 获取OBB的8个顶点
   */
  void GetVertices(glm::vec3 vertices[8]) const;

  /**
   * @brief 获取OBB的AABB近似（用于快速剔除）
   */
  BoundingVolumeAABB GetAABB() const;

  /**
   * @brief 对OBB应用变换
   * @return 应用变换之后的OBB
   */
  BoundingVolumeOBB Transform(const glm::mat4 &matrix) const;

  /**
   * @brief 判断点是否在OBB内部
   */
  bool Contains(const glm::vec3 &point) const;
  /**
   * @brief 扩展OBB以包含给定点
   */
  BoundingVolumeOBB Expand(const glm::vec3 &point);
  /**
   * @brief 扩展OBB以包含另一个OBB
   */
  BoundingVolumeOBB Expand(const BoundingVolumeOBB &other);
};

/**
 * @brief 平面定义
 * 用于视锥体裁剪和碰撞检测
 *
 * 注意：
 * 右手系平面定义：normal·point + d = 0
 * Side的正值：点在平面正侧（法线指向的一侧）
 */
struct BoundingVolumePlane {
  glm::vec3 normal;
  float distance;

  BoundingVolumePlane() : normal(0.0f, 1.0f, 0.0f), distance(0.0f) {}
  BoundingVolumePlane(const glm::vec3 &normal, float distance)
      : normal(glm::normalize(normal)), distance(distance)
  {
  }
  BoundingVolumePlane(const glm::vec3 &point, const glm::vec3 &normal);

  /**
   * @brief 计算点到平面的距离
   */
  float DistanceToPoint(const glm::vec3 &point) const;

  /**
   * @brief 判断点在平面的哪一侧
   */
  int GetSide(const glm::vec3 &point) const;
};

}  // namespace mite

#endif  // MITE_BOUNDING_VOLUMES_TYPES_H
