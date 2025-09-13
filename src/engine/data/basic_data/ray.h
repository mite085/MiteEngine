#ifndef MITE_RAY_H
#define MITE_RAY_H

#include "bounding_volumes_types.h"

namespace mite {

/**
 * @class Ray
 * @brief 射线类，用于射线检测和相交测试
 */
class Ray {
 public:
  glm::vec3 origin;     // 射线起点
  glm::vec3 direction;  // 射线方向（单位向量）
  float tMin;           // 最小距离
  float tMax;           // 最大距离

  /**
   * @brief 默认构造函数
   */
  Ray();

  /**
   * @brief 通过起点和方向构造射线
   * @param origin 射线起点
   * @param direction 射线方向（会自动标准化）
   */
  Ray(const glm::vec3 &origin, const glm::vec3 &direction);

  /**
   * @brief 获取射线上某点的坐标
   * @param t 距离参数
   * @return 射线上对应点的坐标
   */
  glm::vec3 GetPoint(float t) const;

  /**
   * @brief 判断射线是否与AABB相交
   * @param aabb 轴对齐包围盒
   * @param t 相交距离（输出参数）
   * @return 是否相交
   */
  bool Intersects(const AABB &aabb, float &t) const;

  /**
   * @brief 判断射线是否与球相交
   * @param sphere 球体
   * @param t 相交距离（输出参数）
   * @return 是否相交
   */
  bool Intersects(const Sphere &sphere, float &t) const;

  /**
   * @brief 判断射线是否与平面相交
   * @param plane 平面
   * @param t 相交距离（输出参数）
   * @return 是否相交
   */
  bool Intersects(const Plane &plane, float &t) const;

  /**
   * @brief 判断射线是否与三角形相交（Möller–Trumbore算法）
   * @param v0 三角形顶点0
   * @param v1 三角形顶点1
   * @param v2 三角形顶点2
   * @param t 相交距离（输出参数）
   * @param u 重心坐标u（输出参数）
   * @param v 重心坐标v（输出参数）
   * @return 是否相交
   */
  bool Intersects(const glm::vec3 &v0,
                  const glm::vec3 &v1,
                  const glm::vec3 &v2,
                  float &t,
                  float &u,
                  float &v) const;
};

}  // namespace mite

#endif  // MITE_RAY_H
