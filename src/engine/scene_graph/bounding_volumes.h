#ifndef MITE_BOUNDING_VOLUMES_H
#define MITE_BOUNDING_VOLUMES_H

#include "bounding_volumes_types.h"

namespace mite {

/**
 * @class BoundingVolumes
 * @brief 包围盒工具类，提供各种包围体的创建、变换和相交测试方法
 */
class BoundingVolumes {
 public:
  /**
   * @brief 通过点集创建AABB
   * @param points 点集数组
   * @param count 点的数量
   * @return 包含所有点的最小AABB
   */
  static AABB CreateAABBFromPoints(const glm::vec3 *points, uint32_t count);

  /**
   * @brief 通过变换矩阵更新AABB
   * @param original 原始AABB
   * @param transform 变换矩阵
   * @return 变换后的AABB
   */
  static AABB TransformAABB(const AABB &original, const glm::mat4 &transform);

  /**
   * @brief 计算两个AABB的合并结果
   * @param a 第一个AABB
   * @param b 第二个AABB
   * @return 包含两个AABB的最小AABB
   */
  static AABB MergeAABBs(const AABB &a, const AABB &b);

  /**
   * @brief 判断点是否在AABB内部
   * @param point 测试点
   * @param aabb 包围盒
   * @return 是否在内部
   */
  static bool PointInAABB(const glm::vec3 &point, const AABB &aabb);

  /**
   * @brief 判断两个AABB是否相交
   * @param a 第一个AABB
   * @param b 第二个AABB
   * @return 是否相交
   */
  static bool AABBIntersectsAABB(const AABB &a, const AABB &b);

  /**
   * @brief 判断球是否与AABB相交
   * @param sphere 球
   * @param aabb 包围盒
   * @return 是否相交
   */
  static bool SphereIntersectsAABB(const Sphere &sphere, const AABB &aabb);

  /**
   * @brief 判断两个球是否相交
   * @param a 第一个球
   * @param b 第二个球
   * @return 是否相交
   */
  static bool SphereIntersectsSphere(const Sphere &a, const Sphere &b);

  /**
   * @brief 通过AABB创建包围球
   * @param aabb 包围盒
   * @return 包含AABB的最小球
   */
  static Sphere CreateSphereFromAABB(const AABB &aabb);

  /**
   * @brief 变换球体（支持平移和均匀缩放）
   * @param sphere 原始球
   * @param transform 变换矩阵
   * @return 变换后的球
   */
  static Sphere TransformSphere(const Sphere &sphere, const glm::mat4 &transform);

  /**
   * @brief 通过AABB创建OBB
   * @param aabb 包围盒
   * @return 初始方向的OBB
   */
  static OBB CreateOBBFromAABB(const AABB &aabb);

  /**
   * @brief 变换OBB
   * @param obb 原始OBB
   * @param transform 变换矩阵
   * @return 变换后的OBB
   */
  static OBB TransformOBB(const OBB &obb, const glm::mat4 &transform);

  /**
   * @brief 获取OBB的AABB近似（用于快速剔除）
   * @param obb 有向包围盒
   * @return 包含OBB的轴对齐包围盒
   */
  static AABB GetAABBFromOBB(const OBB &obb);
};

}  // namespace mite

#endif  // MITE_BOUNDING_VOLUMES_H
