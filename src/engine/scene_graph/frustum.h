#ifndef MITE_FRUSTUM_H
#define MITE_FRUSTUM_H

#include "bounding_volumes_types.h"

namespace mite {
/**
 * @class Frustum
 * @brief 视锥体类，用于视锥体裁剪和可见性判断
 */
class Frustum {
 public:
  /**
   * @brief 默认构造函数
   */
  Frustum();

  /**
   * @brief 通过视图投影矩阵构造视锥体
   * @param viewProjection 视图投影矩阵
   */
  explicit Frustum(const glm::mat4 &viewProjection);

  /**
   * @brief 通过视图投影矩阵更新视锥体
   * @param viewProjection 视图投影矩阵
   */
  void Update(const glm::mat4 &viewProjection);

  /**
   * @brief 判断点是否在视锥体内
   * @param point 测试点
   * @return 是否在视锥体内
   */
  bool Contains(const glm::vec3 &point) const;

  /**
   * @brief 判断球是否在视锥体内
   * @param sphere 球体
   * @return 相交类型
   */
  IntersectionType TestSphere(const Sphere &sphere) const;

  /**
   * @brief 判断AABB是否在视锥体内
   * @param aabb 轴对齐包围盒
   * @return 相交类型
   */
  IntersectionType TestAABB(const AABB &aabb) const;

  /**
   * @brief 获取视锥体的6个裁剪平面
   * @return 平面数组的指针
   */
  const Plane *GetPlanes() const
  {
    return planes;
  }

  /**
   * @brief 获取视锥体的8个角点
   * @param corners 角点数组（输出参数，需要8个元素）
   */
  void GetCorners(glm::vec3 corners[8]) const;

  /**
   * @brief 判断OBB是否在视锥体内
   * @param obb 有向包围盒
   * @return 相交类型
   */
  IntersectionType TestOBB(const OBB &obb) const;

 private:
  /**
   * @brief 从矩阵提取裁剪平面
   * @param matrix 视图投影矩阵
   * @param planeIndex 平面索引（0-5）
   * @param sign 符号（1或-1）
   */
  void ExtractPlane(const glm::mat4 &matrix, int planeIndex, float sign);

 private:
  Plane planes[6];  ///< 6个裁剪平面（左、右、下、上、近、远）
};
}  // namespace mite

#endif  // MITE_FRUSTUM_H
