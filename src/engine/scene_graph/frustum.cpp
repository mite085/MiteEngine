#include "frustum.h"
//#include <glm/gtc/matrix_access.hpp>

namespace mite {

Frustum::Frustum()
{
  for (int i = 0; i < 6; ++i) {
    planes[i] = Plane();
  }
}

Frustum::Frustum(const glm::mat4 &viewProjection)
{
  Update(viewProjection);
}

void Frustum::Update(const glm::mat4 &viewProjection)
{
  // 提取6个裁剪平面
  ExtractPlane(viewProjection, 0, 1.0f);   // 左平面
  ExtractPlane(viewProjection, 0, -1.0f);  // 右平面
  ExtractPlane(viewProjection, 1, 1.0f);   // 下平面
  ExtractPlane(viewProjection, 1, -1.0f);  // 上平面
  ExtractPlane(viewProjection, 2, 1.0f);   // 近平面
  ExtractPlane(viewProjection, 2, -1.0f);  // 远平面
}

void Frustum::ExtractPlane(const glm::mat4 &matrix, int planeIndex, float sign)
{
  int index = planeIndex * 2 + (sign > 0 ? 0 : 1);

  glm::vec4 planeCoeffs;
  if (planeIndex == 0) {  // x轴平面
    planeCoeffs = matrix[3] + sign * matrix[0];
  }
  else if (planeIndex == 1) {  // y轴平面
    planeCoeffs = matrix[3] + sign * matrix[1];
  }
  else {  // z轴平面
    planeCoeffs = matrix[3] + sign * matrix[2];
  }

  // 标准化平面方程
  float length = glm::length(glm::vec3(planeCoeffs));
  planeCoeffs /= length;

  planes[index] = Plane(glm::vec3(planeCoeffs), planeCoeffs.w);
}

bool Frustum::Contains(const glm::vec3 &point) const
{
  for (int i = 0; i < 6; ++i) {
    if (planes[i].DistanceToPoint(point) < 0.0f) {
      return false;
    }
  }
  return true;
}

IntersectionType Frustum::TestSphere(const Sphere &sphere) const
{
  bool completelyInside = true;

  for (int i = 0; i < 6; ++i) {
    float distance = planes[i].DistanceToPoint(sphere.center);

    if (distance < -sphere.radius) {
      return IntersectionType::Outside;
    }

    if (distance < sphere.radius) {
      completelyInside = false;
    }
  }

  return completelyInside ? IntersectionType::Inside : IntersectionType::Intersect;
}

IntersectionType Frustum::TestAABB(const AABB &aabb) const
{
  IntersectionType result = IntersectionType::Inside;

  for (int i = 0; i < 6; ++i) {
    // 计算AABB在平面法线方向上的投影极值
    glm::vec3 positiveVertex = aabb.min;
    glm::vec3 negativeVertex = aabb.max;

    if (planes[i].normal.x >= 0) {
      positiveVertex.x = aabb.max.x;
      negativeVertex.x = aabb.min.x;
    }
    if (planes[i].normal.y >= 0) {
      positiveVertex.y = aabb.max.y;
      negativeVertex.y = aabb.min.y;
    }
    if (planes[i].normal.z >= 0) {
      positiveVertex.z = aabb.max.z;
      negativeVertex.z = aabb.min.z;
    }

    // 测试负极点
    if (planes[i].DistanceToPoint(negativeVertex) < 0.0f) {
      return IntersectionType::Outside;
    }

    // 测试正极点
    if (planes[i].DistanceToPoint(positiveVertex) < 0.0f) {
      result = IntersectionType::Intersect;
    }
  }

  return result;
}

void Frustum::GetCorners(glm::vec3 corners[8]) const
{
  // 通过平面相交计算视锥体角点（简化实现）
  // 实际应用中通常从投影矩阵反算更准确
  corners[0] = glm::vec3(-1, -1, -1);  // 近左下
  corners[1] = glm::vec3(1, -1, -1);   // 近右下
  corners[2] = glm::vec3(-1, 1, -1);   // 近左上
  corners[3] = glm::vec3(1, 1, -1);    // 近右上
  corners[4] = glm::vec3(-1, -1, 1);   // 远左下
  corners[5] = glm::vec3(1, -1, 1);    // 远右下
  corners[6] = glm::vec3(-1, 1, 1);    // 远左上
  corners[7] = glm::vec3(1, 1, 1);     // 远右上
}

IntersectionType Frustum::TestOBB(const OBB &obb) const
{
  // 将OBB转换到视锥体空间测试
  AABB localAABB = obb.GetAABB();
  return TestAABB(localAABB);
}

}  // namespace mite
