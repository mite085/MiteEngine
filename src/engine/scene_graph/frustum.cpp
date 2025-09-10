#include "frustum.h"

namespace mite {
Frustum::Frustum()
{
  for (int i = 0; i < 6; ++i) {
    m_Planes[i] = Plane();
  }
}

Frustum::Frustum(const glm::mat4 &viewProjection)
{
  Update(viewProjection);
}

void Frustum::Update(const glm::mat4 &viewProjection)
{
  // 提取6个裁剪平面
  ExtractPlane(viewProjection, FrustumPlane::LEFT);    // 左平面
  ExtractPlane(viewProjection, FrustumPlane::RIGHT);   // 右平面
  ExtractPlane(viewProjection, FrustumPlane::BOTTOM);  // 下平面
  ExtractPlane(viewProjection, FrustumPlane::TOP);     // 上平面
  ExtractPlane(viewProjection, FrustumPlane::NEAR);    // 近平面
  ExtractPlane(viewProjection, FrustumPlane::FAR);     // 远平面
}

void Frustum::ExtractPlane(const glm::mat4 &vpMatrix, FrustumPlane plane)
{
  glm::vec4 planeCoeffs;

  switch (plane) {
    case FrustumPlane::LEFT:
      planeCoeffs = glm::row(vpMatrix, 3) + glm::row(vpMatrix, 0);
      break;
    case FrustumPlane::RIGHT:
      planeCoeffs = glm::row(vpMatrix, 3) - glm::row(vpMatrix, 0);
      break;
    case FrustumPlane::BOTTOM:
      planeCoeffs = glm::row(vpMatrix, 3) + glm::row(vpMatrix, 1);
      break;
    case FrustumPlane::TOP:
      planeCoeffs = glm::row(vpMatrix, 3) - glm::row(vpMatrix, 1);
      break;
    case FrustumPlane::NEAR:
      planeCoeffs = glm::row(vpMatrix, 3) + glm::row(vpMatrix, 2);
      break;
    case FrustumPlane::FAR:
      planeCoeffs = glm::row(vpMatrix, 3) - glm::row(vpMatrix, 2);
      break;
  }

  // 标准化平面方程
  float length = glm::length(glm::vec3(planeCoeffs));
  if (length > 0.0f) {
    planeCoeffs /= length;
  }
  // 右手系平面方程：normal.x*x + normal.y*y + normal.z*z + d = 0
  // 其中 d = planeCoeffs.w（与左手系符号相反）
  m_Planes[static_cast<int>(plane)] = Plane(glm::vec3(planeCoeffs), planeCoeffs.w);
}

bool Frustum::Contains(const glm::vec3 &point) const
{
  for (int i = 0; i < 6; ++i) {
    if (m_Planes[i].DistanceToPoint(point) < 0.0f) {
      return false;
    }
  }
  return true;
}

IntersectionType Frustum::TestSphere(const Sphere &sphere) const
{
  bool completelyInside = true;

  for (int i = 0; i < 6; ++i) {
    float distance = m_Planes[i].DistanceToPoint(sphere.center);

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
  bool intersects = false;
  for (int i = 0; i < 6; ++i) {
    // 计算极值点
    glm::vec3 positiveVertex = aabb.min;
    glm::vec3 negativeVertex = aabb.max;
    if (m_Planes[i].normal.x >= 0) {
      positiveVertex.x = aabb.max.x;
      negativeVertex.x = aabb.min.x;
    }
    if (m_Planes[i].normal.y >= 0) {
      positiveVertex.y = aabb.max.y;
      negativeVertex.y = aabb.min.y;
    }
    if (m_Planes[i].normal.z >= 0) {
      positiveVertex.z = aabb.max.z;
      negativeVertex.z = aabb.min.z;
    }

    // 在右手系中，平面方程的符号约定是
    // normal.x *x + normal.y *y + normal.z *z + d = 0
    // 法线指向视锥体内部，所以：
    // 正值表示点在平面正侧（视锥体内部）
    // 负值表示点在平面负侧（视锥体外部）

    // 测试负顶点（最远的点）
    float negativeDistance = m_Planes[i].DistanceToPoint(negativeVertex);
    if (negativeDistance < 0.0f) {
      // 如果最远的点都在外面，AABB完全在这个平面外
      // 但还要确认是否真的完全在外面（测试8个顶点）
      bool allOutside = true;
      glm::vec3 vertices[8] = {{aabb.min.x, aabb.min.y, aabb.min.z},
                               {aabb.min.x, aabb.min.y, aabb.max.z},
                               {aabb.min.x, aabb.max.y, aabb.min.z},
                               {aabb.min.x, aabb.max.y, aabb.max.z},
                               {aabb.max.x, aabb.min.y, aabb.min.z},
                               {aabb.max.x, aabb.min.y, aabb.max.z},
                               {aabb.max.x, aabb.max.y, aabb.min.z},
                               {aabb.max.x, aabb.max.y, aabb.max.z}};
      for (int j = 0; j < 8; ++j) {
        if (m_Planes[i].DistanceToPoint(vertices[j]) >= 0.0f) {
          allOutside = false;
          break;
        }
      }
      // 仅当8个点全部在视锥体外面时返回Outside
      if (allOutside) {
        return IntersectionType::Outside;
      }
      // 否则认为是相交的
      intersects = true;
      continue;
    }
    // 测试正顶点
    float positiveDistance = m_Planes[i].DistanceToPoint(positiveVertex);
    if (positiveDistance < 0.0f) {
      // 与这个平面相交
      intersects = true;
    }
  }
  // 根据intersects，判断结果为Inside完全在内，还是Intersect相交
  return intersects ? IntersectionType::Intersect : IntersectionType::Inside;
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