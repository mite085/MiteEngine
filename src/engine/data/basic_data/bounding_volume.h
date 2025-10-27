#ifndef MITE_BOUNDING_VOLUMES_H
#define MITE_BOUNDING_VOLUMES_H

#include "basic_data/bounding_volumes_intersects.h"

namespace mite {

/**
 * @class BoundingVolume
 * @brief 统一的包围体类，管理多种类型的包围体并提供统一接口
 */
class BoundingVolume {
 public:
  explicit BoundingVolume(BoundingVolumeType type);


  // ==================== 类型管理 ====================
  BoundingVolumeType GetType() const;
  void SetType(BoundingVolumeType type);

  // ==================== 数据访问 ====================
  const BoundingVolumeAABB &GetAABB() const;
  const BoundingVolumeSphere &GetSphere() const;
  const BoundingVolumeOBB &GetOBB() const;
  const BoundingVolumePlane &GetPlane() const;

  void SetAABB(const BoundingVolumeAABB &aabb);
  void SetSphere(const BoundingVolumeSphere &sphere);
  void SetOBB(const BoundingVolumeOBB &obb);
  void SetPlane(const BoundingVolumePlane &plane);
  // ==================== 统一接口 ====================
  /**
   * @brief 从点集创建包围体
   */
  static BoundingVolume CreateFromPoints(BoundingVolumeType type,
                                         const std::vector<glm::vec3> &points);
  /**
   * @brief 从点集创建包围体
   */
  static BoundingVolume CreateFromAABB(BoundingVolumeAABB aabb);
  /**
   * @brief 判断点是否在包围体内
   */
  bool ContainsPoint(const glm::vec3 &point) const;
  /**
   * @brief 变换包围体
   */
  BoundingVolume Transform(const glm::mat4 &matrix) const;
  /**
   * @brief 合并两个包围体
   */
  static BoundingVolume Merge(BoundingVolumeType resultType,
                              const BoundingVolume &a,
                              const BoundingVolume &b);

  /**
   * @brief 与其他包围体相交测试
   */
  BoundingVolumeIntersection::IntersectionType Intersects(const BoundingVolume &other) const;
  /**
   * @brief 获取AABB近似（用于快速测试）
   */
  BoundingVolumeAABB GetAABBApproximation() const;
  /**
   * @brief 获取Sphere近似
   */
  BoundingVolumeSphere GetSphereApproximation() const;

  /**
   * @brief 转换为指定类型
   */
  bool ConvertTo(BoundingVolumeType targetType);

  // ==================== 工具方法 ====================
  bool IsValid() const;
  float GetVolume() const;
  float GetSurfaceArea() const;
  glm::vec3 GetCenter() const;
  /**
   * @brief 扩展包围体以包含点
   */
  void ExpandToInclude(const glm::vec3 &point);

  /**
   * @brief 扩展包围体以包含另一个包围体
   */
  void ExpandToInclude(const BoundingVolume &other);

 private:
  // 私有构造函数
  BoundingVolume(const BoundingVolumeAABB &aabb);
  BoundingVolume(const BoundingVolumeSphere &sphere);
  BoundingVolume(const BoundingVolumeOBB &obb);
  BoundingVolume(const BoundingVolumePlane &plane);
  void CreateDefaultVolume(BoundingVolumeType type);

  std::variant<BoundingVolumeAABB, BoundingVolumeSphere, BoundingVolumeOBB, BoundingVolumePlane>
      m_Volume;
  BoundingVolumeType m_Type = BoundingVolumeType::None; // 默认包围盒类型为None不可用

  // 内部辅助方法
  BoundingVolumeIntersection::IntersectionType IntersectsAABB(
      const BoundingVolumeAABB &other) const;
  BoundingVolumeIntersection::IntersectionType IntersectsSphere(
      const BoundingVolumeSphere &other) const;
  BoundingVolumeIntersection::IntersectionType IntersectsOBB(const BoundingVolumeOBB &other) const;
  BoundingVolumeIntersection::IntersectionType IntersectsPlane(
      const BoundingVolumePlane &other) const;

 // // ==================== 静态数学工具（暂时保留，后续删除） ====================
 //public:
 // /**
 //  * @brief 通过点集创建AABB
 //  * @param points 点集数组
 //  * @param count 点的数量
 //  * @return 包含所有点的最小AABB
 //  */
 // static BoundingVolumeAABB CreateAABBFromPoints(const glm::vec3 *points, uint32_t count);

 // /**
 //  * @brief 通过变换矩阵更新AABB
 //  * @param original 原始AABB
 //  * @param transform 变换矩阵
 //  * @return 变换后的AABB
 //  */
 // static BoundingVolumeAABB TransformAABB(const BoundingVolumeAABB &original, const glm::mat4 &transform);

 // /**
 //  * @brief 计算两个AABB的合并结果
 //  * @param a 第一个AABB
 //  * @param b 第二个AABB
 //  * @return 包含两个AABB的最小AABB
 //  */
 // static BoundingVolumeAABB MergeAABBs(const BoundingVolumeAABB &a, const BoundingVolumeAABB &b);

 // /**
 //  * @brief 判断点是否在AABB内部
 //  * @param point 测试点
 //  * @param aabb 包围盒
 //  * @return 是否在内部
 //  */
 // static bool PointInAABB(const glm::vec3 &point, const BoundingVolumeAABB &aabb);

 // /**
 //  * @brief 判断两个AABB是否相交
 //  * @param a 第一个AABB
 //  * @param b 第二个AABB
 //  * @return 是否相交
 //  */
 // static bool AABBIntersectsAABB(const BoundingVolumeAABB &a, const BoundingVolumeAABB &b);

 // /**
 //  * @brief 判断球是否与AABB相交
 //  * @param sphere 球
 //  * @param aabb 包围盒
 //  * @return 是否相交
 //  */
 // static bool SphereIntersectsAABB(const BoundingVolumeSphere &sphere, const BoundingVolumeAABB &aabb);

 // /**
 //  * @brief 判断两个球是否相交
 //  * @param a 第一个球
 //  * @param b 第二个球
 //  * @return 是否相交
 //  */
 // static bool SphereIntersectsSphere(const BoundingVolumeSphere &a, const BoundingVolumeSphere &b);

 // /**
 //  * @brief 通过AABB创建包围球
 //  * @param aabb 包围盒
 //  * @return 包含AABB的最小球
 //  */
 // static BoundingVolumeSphere CreateSphereFromAABB(const BoundingVolumeAABB &aabb);

 // /**
 //  * @brief 变换球体（支持平移和均匀缩放）
 //  * @param sphere 原始球
 //  * @param transform 变换矩阵
 //  * @return 变换后的球
 //  */
 // static BoundingVolumeSphere TransformSphere(const BoundingVolumeSphere &sphere, const glm::mat4 &transform);

 // /**
 //  * @brief 通过AABB创建OBB
 //  * @param aabb 包围盒
 //  * @return 初始方向的OBB
 //  */
 // static BoundingVolumeOBB CreateOBBFromAABB(const BoundingVolumeAABB &aabb);

 // /**
 //  * @brief 变换OBB
 //  * @param obb 原始OBB
 //  * @param transform 变换矩阵
 //  * @return 变换后的OBB
 //  */
 // static BoundingVolumeOBB TransformOBB(const BoundingVolumeOBB &obb, const glm::mat4 &transform);

 // /**
 //  * @brief 获取OBB的AABB近似（用于快速剔除）
 //  * @param obb 有向包围盒
 //  * @return 包含OBB的轴对齐包围盒
 //  */
 // static BoundingVolumeAABB GetAABBFromOBB(const BoundingVolumeOBB &obb);
};

}  // namespace mite

#endif  // MITE_BOUNDING_VOLUMES_H
