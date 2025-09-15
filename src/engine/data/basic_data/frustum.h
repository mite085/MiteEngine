#ifndef MITE_FRUSTUM_H
#define MITE_FRUSTUM_H

#include "basic_data/bounding_volume.h"

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
   * @return 相交类型（不包含Contain，若物体将整个视锥体全部包含在内，那就和Outside没有区别了）
   */
  BoundingVolumeIntersection::IntersectionType TestSphere(
      const BoundingVolumeSphere &sphere) const;
  /**
   * @brief 判断AABB是否在视锥体内
   * @param aabb 轴对齐包围盒
   * @return 相交类型
   */
  BoundingVolumeIntersection::IntersectionType TestAABB(const BoundingVolumeAABB &aabb) const;
  /**
   * @brief 判断OBB是否在视锥体内
   * @param obb 有向包围盒
   * @return 相交类型
   */
  BoundingVolumeIntersection::IntersectionType TestOBB(const BoundingVolumeOBB &obb) const;
  /**
   * @brief 判断OBB是否在视锥体内
   * @param obb 有向包围盒
   * @return 相交类型
   */
  BoundingVolumeIntersection::IntersectionType TestPlane(const BoundingVolumePlane &plane) const;
  /**
   * @brief 判断通用包围体是否在视锥体内
   * @param volume 通用包围体
   * @return 相交类型
   */
  BoundingVolumeIntersection::IntersectionType TestBoundingVolume(
      const BoundingVolume &volume) const;
  

  /**
   * @brief 获取视锥体的6个裁剪平面
   * @return 平面数组的指针
   */
  const BoundingVolumePlane *GetPlanes() const
  {
    return m_Planes;
  }

  /**
   * @brief 获取视锥体的8个角点
   * @param corners 角点数组（输出参数，需要8个元素）
   */
  void GetCorners(glm::vec3 corners[8]) const;

 private:
  enum FrustumPlane { LEFT = 0, RIGHT = 1, BOTTOM = 2, TOP = 3, NEAR = 4, FAR = 5 };

  /**
   * @brief 从矩阵提取裁剪平面
   * @param matrix 视图投影矩阵
   * @param planeIndex 平面索引（0-5）
   * @param sign 符号（1或-1）
   */
  void ExtractPlane(const glm::mat4 &matrix, FrustumPlane plane);

 private:
  BoundingVolumePlane m_Planes[6];  // 6个裁剪平面（左、右、下、上、近、远）
};
}  // namespace mite

#endif  // MITE_FRUSTUM_H
