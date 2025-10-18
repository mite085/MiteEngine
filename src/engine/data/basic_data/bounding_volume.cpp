#include "bounding_volume.h"

namespace mite {
// ==================== 构造函数 ====================
BoundingVolume::BoundingVolume(BoundingVolumeType type) : m_Type(type)
{
  CreateDefaultVolume(type);
}
BoundingVolume::BoundingVolume(const BoundingVolumeAABB &aabb)
    : m_Type(BoundingVolumeType::AABB), m_Volume(aabb)
{
}
BoundingVolume::BoundingVolume(const BoundingVolumeSphere &sphere)
    : m_Type(BoundingVolumeType::Sphere), m_Volume(sphere)
{
}
BoundingVolume::BoundingVolume(const BoundingVolumeOBB &obb)
    : m_Type(BoundingVolumeType::OBB), m_Volume(obb)
{
}
BoundingVolume::BoundingVolume(const BoundingVolumePlane &plane)
    : m_Type(BoundingVolumeType::Plane), m_Volume(plane)
{
}

// ==================== 类型管理 ====================
BoundingVolumeType BoundingVolume::GetType() const
{
  return m_Type;
}
void BoundingVolume::SetType(BoundingVolumeType type)
{
  if (m_Type != type) {
    m_Type = type;
    CreateDefaultVolume(type);
  }
}

// ==================== 数据访问 ====================
const BoundingVolumeAABB &BoundingVolume::GetAABB() const
{
  if (m_Type != BoundingVolumeType::AABB) {
    throw std::runtime_error("BoundingVolume is not of type AABB");
  }
  return std::get<BoundingVolumeAABB>(m_Volume);
}
const BoundingVolumeSphere &BoundingVolume::GetSphere() const
{
  if (m_Type != BoundingVolumeType::Sphere) {
    throw std::runtime_error("BoundingVolume is not of type Sphere");
  }
  return std::get<BoundingVolumeSphere>(m_Volume);
}
const BoundingVolumeOBB &BoundingVolume::GetOBB() const
{
  if (m_Type != BoundingVolumeType::OBB) {
    throw std::runtime_error("BoundingVolume is not of type OBB");
  }
  return std::get<BoundingVolumeOBB>(m_Volume);
}
const BoundingVolumePlane &BoundingVolume::GetPlane() const
{
  if (m_Type != BoundingVolumeType::Plane) {
    throw std::runtime_error("BoundingVolume is not of type Plane");
  }
  return std::get<BoundingVolumePlane>(m_Volume);
}
void BoundingVolume::SetAABB(const BoundingVolumeAABB &aabb)
{
  m_Type = BoundingVolumeType::AABB;
  m_Volume = aabb;
}
void BoundingVolume::SetSphere(const BoundingVolumeSphere &sphere)
{
  m_Type = BoundingVolumeType::Sphere;
  m_Volume = sphere;
}
void BoundingVolume::SetOBB(const BoundingVolumeOBB &obb)
{
  m_Type = BoundingVolumeType::OBB;
  m_Volume = obb;
}
void BoundingVolume::SetPlane(const BoundingVolumePlane &plane)
{
  m_Type = BoundingVolumeType::Plane;
  m_Volume = plane;
}

// ==================== 统一接口实现 ====================
BoundingVolume BoundingVolume::CreateFromPoints(BoundingVolumeType type,
                                                const std::vector<glm::vec3> &points)
{
  if (points.empty()) {
    return BoundingVolume(type);
  }
  switch (type) {
    case BoundingVolumeType::AABB: {
      BoundingVolumeAABB aabb = BoundingVolumeAABB::CreateAABBFromPoints(
          points.data(), static_cast<uint32_t>(points.size()));
      return BoundingVolume(aabb);
    }
    case BoundingVolumeType::Sphere: {
      BoundingVolumeSphere sphere = BoundingVolumeSphere::CreateSphereFromPoints(points);
      return BoundingVolume(sphere);
    }
    case BoundingVolumeType::OBB: {
      // 先创建AABB再转换为OBB
      BoundingVolumeAABB aabb = BoundingVolumeAABB::CreateAABBFromPoints(
          points.data(), static_cast<uint32_t>(points.size()));
      BoundingVolumeOBB obb = BoundingVolumeOBB::FromAABB(aabb);
      return BoundingVolume(obb);
    }
    default:
      throw std::runtime_error("Unsupported bounding volume type for CreateFromPoints");
  }
}
BoundingVolume BoundingVolume::CreateFromAABB(BoundingVolumeAABB aabb)
{
  return BoundingVolume(aabb);
}

bool BoundingVolume::ContainsPoint(const glm::vec3 &point) const
{
  switch (m_Type) {
    case BoundingVolumeType::AABB:
      return std::get<BoundingVolumeAABB>(m_Volume).Contains(point);
    case BoundingVolumeType::Sphere:
      return std::get<BoundingVolumeSphere>(m_Volume).Contains(point);
    case BoundingVolumeType::OBB:
      return std::get<BoundingVolumeOBB>(m_Volume).Contains(point);
    case BoundingVolumeType::Plane:
      return std::get<BoundingVolumePlane>(m_Volume).DistanceToPoint(point) <= 0;
    default:
      return false;
  }
}
BoundingVolume BoundingVolume::Transform(const glm::mat4 &matrix) const
{
  switch (m_Type) {
    case BoundingVolumeType::AABB: {
      BoundingVolumeAABB transformed = std::get<BoundingVolumeAABB>(m_Volume).Transform(matrix);
      return BoundingVolume(transformed);
    }
    case BoundingVolumeType::Sphere: {
      BoundingVolumeSphere transformed = std::get<BoundingVolumeSphere>(m_Volume).Transform(
          matrix);
      return BoundingVolume(transformed);
    }
    case BoundingVolumeType::OBB: {
      BoundingVolumeOBB transformed = std::get<BoundingVolumeOBB>(m_Volume).Transform(matrix);
      return BoundingVolume(transformed);
    }
    case BoundingVolumeType::Plane: {
      // 平面变换需要特殊处理
      const BoundingVolumePlane &plane = std::get<BoundingVolumePlane>(m_Volume);
      glm::vec4 normal4(plane.normal, 0.0f);
      glm::vec4 pointOnPlane(plane.normal * -plane.distance, 1.0f);

      glm::vec3 transformedNormal = glm::normalize(glm::vec3(matrix * normal4));
      glm::vec3 transformedPoint = glm::vec3(matrix * pointOnPlane);

      BoundingVolumePlane transformedPlane(transformedPoint, transformedNormal);
      return BoundingVolume(transformedPlane);
    }
    default:
      return *this;
  }
}
BoundingVolume BoundingVolume::Merge(BoundingVolumeType resultType,
                                     const BoundingVolume &a,
                                     const BoundingVolume &b)
{
  // 情况1：a和b之间不存在类型转换
  if (a.GetType() == b.GetType()) {
    BoundingVolume temp(a.GetType()), ret(resultType);
    switch (a.GetType()) {
      case BoundingVolumeType::AABB:
        temp = BoundingVolume(BoundingVolumeAABB(a.GetAABB()).Expand(b.GetAABB()));
        break;
      case BoundingVolumeType::Sphere:
        temp = BoundingVolume(BoundingVolumeSphere(a.GetSphere()).Expand(b.GetSphere()));
        break;
      case BoundingVolumeType::OBB:
        temp = BoundingVolume(BoundingVolumeOBB(a.GetOBB()).Expand(b.GetOBB()));
        break;
      default:
        throw std::runtime_error("Unsupported result type for Merge");
    }
    // 将temp转换为需要返回的类型（若一致则使用ConvertTo()的内部优化）
    if (temp.ConvertTo(resultType, ret))
      return ret;
    else
      throw std::runtime_error("Unsupported result type for Merge");
  }
  // 情况2：a和b之间存在类型转换
  else {
    // 统一转换为AABB进行合并
    BoundingVolumeAABB mergedAABB = a.GetAABBApproximation().Expand(b.GetAABBApproximation());

    switch (resultType) {
      case BoundingVolumeType::AABB:
        return BoundingVolume(mergedAABB);
      case BoundingVolumeType::Sphere:
        return BoundingVolume(BoundingVolumeSphere::FromAABB(mergedAABB));
      case BoundingVolumeType::OBB:
        return BoundingVolume(BoundingVolumeOBB::FromAABB(mergedAABB));
      default:
        throw std::runtime_error("Unsupported result type for Merge");
    }
  }
}
BoundingVolumeIntersection::IntersectionType BoundingVolume::Intersects(
    const BoundingVolume &other) const
{
  switch (other.m_Type) {
    case BoundingVolumeType::AABB:
      return IntersectsAABB(other.GetAABB());
    case BoundingVolumeType::Sphere:
      return IntersectsSphere(other.GetSphere());
    case BoundingVolumeType::OBB:
      return IntersectsOBB(other.GetOBB());
    case BoundingVolumeType::Plane:
      return IntersectsPlane(other.GetPlane());
    default:
      return BoundingVolumeIntersection::IntersectionType::Outside;
  }
}
BoundingVolumeAABB BoundingVolume::GetAABBApproximation() const
{
  switch (m_Type) {
    case BoundingVolumeType::AABB:
      return std::get<BoundingVolumeAABB>(m_Volume);
    case BoundingVolumeType::Sphere: {
      // 计算Sphere的外切AABB
      const BoundingVolumeSphere &sphere = std::get<BoundingVolumeSphere>(m_Volume);
      glm::vec3 radiusVec(sphere.radius);
      return BoundingVolumeAABB(sphere.center - radiusVec, sphere.center + radiusVec);
    }
    case BoundingVolumeType::OBB:
      return std::get<BoundingVolumeOBB>(m_Volume).GetAABB();
    case BoundingVolumeType::Plane:
      // 平面没有有意义的AABB，返回无限大AABB，确保包围行为，避免渲染目标丢失
      return BoundingVolumeAABB(glm::vec3(-FLT_MAX), glm::vec3(FLT_MAX));
    default:
      return BoundingVolumeAABB();
  }
}
BoundingVolumeSphere BoundingVolume::GetSphereApproximation() const
{
  switch (m_Type) {
    case BoundingVolumeType::Sphere:
      return std::get<BoundingVolumeSphere>(m_Volume);
    case BoundingVolumeType::AABB: {
      // 计算AABB外切球
      const BoundingVolumeAABB &aabb = std::get<BoundingVolumeAABB>(m_Volume);
      return BoundingVolumeSphere::FromAABB(aabb);
    }
    case BoundingVolumeType::OBB: {
      const BoundingVolumeOBB &obb = std::get<BoundingVolumeOBB>(m_Volume);
      BoundingVolumeAABB aabbApprox = obb.GetAABB();
      return BoundingVolumeSphere::FromAABB(aabbApprox);
    }
    case BoundingVolumeType::Plane:
      // 平面没有有意义的Sphere近似
      return BoundingVolumeSphere(glm::vec3(0.0f), FLT_MAX);
    default:
      return BoundingVolumeSphere();
  }
}
bool BoundingVolume::ConvertTo(BoundingVolumeType targetType, BoundingVolume &result) const
{
  // 类型相同，无需转换，直接复制
  if (targetType == m_Type) {
    result = *this;
    return true;
  }

  try {
    switch (targetType) {
      case BoundingVolumeType::AABB:
        result = BoundingVolume(GetAABBApproximation());
        return true;
      case BoundingVolumeType::Sphere:
        result = BoundingVolume(GetSphereApproximation());
        return true;
      case BoundingVolumeType::OBB:
        // 不同类型转换OBB需要分情况讨论
        switch (m_Type) {
          case BoundingVolumeType::AABB: {
            // AABB -> OBB: 直接转换，方向为单位矩阵
            const BoundingVolumeAABB &aabb = std::get<BoundingVolumeAABB>(m_Volume);
            glm::vec3 center = aabb.GetCenter();
            glm::vec3 extents = aabb.GetHalfExtents();
            BoundingVolumeOBB obb(center, extents, glm::mat3(1.0f));
            result = BoundingVolume(obb);
            return true;
          }

          case BoundingVolumeType::Sphere: {
            // Sphere -> OBB: 创建立方体OBB包围球
            const BoundingVolumeSphere &sphere = std::get<BoundingVolumeSphere>(m_Volume);
            glm::vec3 extents(sphere.radius);
            BoundingVolumeOBB obb(sphere.center, extents, glm::mat3(1.0f));
            result = BoundingVolume(obb);
            return true;
          }

          case BoundingVolumeType::Plane: {
            // Plane -> OBB: 为平面创建一个薄的OBB表示
            const BoundingVolumePlane &plane = std::get<BoundingVolumePlane>(m_Volume);

            // 创建基向量
            glm::vec3 normal = plane.normal;
            glm::vec3 tangent, bitangent;

            // 计算切向量和副切向量
            if (std::abs(normal.x) > std::abs(normal.y)) {
              tangent = glm::normalize(glm::vec3(normal.z, 0, -normal.x));
            }
            else {
              tangent = glm::normalize(glm::vec3(0, -normal.z, normal.y));
            }
            bitangent = glm::normalize(glm::cross(normal, tangent));

            // 创建一个很薄的OBB（厚度为0.1单位）
            glm::mat3 orientation(tangent, bitangent, normal);
            glm::vec3 extents(1000.0f, 1000.0f, 0.1f);  // 大的平面，很小的厚度

            // 平面上的一个点
            glm::vec3 pointOnPlane = normal * -plane.distance;

            BoundingVolumeOBB obb(pointOnPlane, extents, orientation);
            result = BoundingVolume(obb);
            return true;
          }

          default:
            return false;
        }
      case BoundingVolumeType::Plane:
        // AABB等正常包围盒转平面，还是太抽象了
        return false;
      default:
        return false;
    }
  }
  catch (...) {
    return false;
  }
}
// ==================== 工具方法 ====================
bool BoundingVolume::IsValid() const
{
  switch (m_Type) {
    case BoundingVolumeType::AABB:
      return std::get<BoundingVolumeAABB>(m_Volume).IsValid();
    case BoundingVolumeType::Sphere:
      return std::get<BoundingVolumeSphere>(m_Volume).radius >= 0;
    case BoundingVolumeType::OBB:
      return glm::length(std::get<BoundingVolumeOBB>(m_Volume).extents) > 0;
    case BoundingVolumeType::Plane:
      return true;  // 无需多言。怎么会有Invalid的Plane呢
    default:
      return false;
  }
}
float BoundingVolume::GetVolume() const
{
  switch (m_Type) {
    case BoundingVolumeType::AABB: {
      glm::vec3 size = std::get<BoundingVolumeAABB>(m_Volume).GetSize();
      return size.x * size.y * size.z;
    }
    case BoundingVolumeType::Sphere: {
      const BoundingVolumeSphere &sphere = std::get<BoundingVolumeSphere>(m_Volume);
      return (4.0f / 3.0f) * glm::pi<float>() * sphere.radius * sphere.radius * sphere.radius;
    }
    case BoundingVolumeType::OBB: {
      glm::vec3 extents = std::get<BoundingVolumeOBB>(m_Volume).extents;
      // 注意半长轴, OBB体积 = 8 * 半长乘积
      return 8.0f * extents.x * extents.y * extents.z;
    }
    default:
      return 0.0f;
  }
}
float BoundingVolume::GetSurfaceArea() const
{
  switch (m_Type) {
    case BoundingVolumeType::AABB: {
      glm::vec3 size = std::get<BoundingVolumeAABB>(m_Volume).GetSize();
      return 2.0f * (size.x * size.y + size.x * size.z + size.y * size.z);
    }
    case BoundingVolumeType::Sphere: {
      const BoundingVolumeSphere &sphere = std::get<BoundingVolumeSphere>(m_Volume);
      return 4.0f * glm::pi<float>() * sphere.radius * sphere.radius;
    }
    case BoundingVolumeType::OBB: {
      glm::vec3 extents = std::get<BoundingVolumeOBB>(m_Volume).extents;
      // 注意半长轴
      return 8.0f * (extents.x * extents.y + extents.x * extents.z + extents.y * extents.z);
    }
    default:
      return 0.0f;
  }
}
glm::vec3 BoundingVolume::GetCenter() const
{
  switch (m_Type) {
    case BoundingVolumeType::AABB:
      return std::get<BoundingVolumeAABB>(m_Volume).GetCenter();
    case BoundingVolumeType::Sphere:
      return std::get<BoundingVolumeSphere>(m_Volume).center;
    case BoundingVolumeType::OBB:
      return std::get<BoundingVolumeOBB>(m_Volume).center;
    case BoundingVolumeType::Plane:
      return std::get<BoundingVolumePlane>(m_Volume).normal *
             -std::get<BoundingVolumePlane>(m_Volume).distance;
    default:
      return glm::vec3(0.0f);
  }
}
void BoundingVolume::ExpandToInclude(const glm::vec3 &point)
{
  switch (m_Type) {
    case BoundingVolumeType::AABB: {
      BoundingVolumeAABB &aabb = std::get<BoundingVolumeAABB>(m_Volume);
      // 判断新的点是否被AABB包含
      if (!aabb.Contains(point))
        // 不包含时才应当执行扩展操作
        aabb.Expand(point);
      break;
    }
    case BoundingVolumeType::Sphere: {
      BoundingVolumeSphere &sphere = std::get<BoundingVolumeSphere>(m_Volume);
      if (!sphere.Contains(point)) {
        sphere.Expand(point);
      }
      break;
    }
    case BoundingVolumeType::OBB: {
      BoundingVolumeOBB &obb = std::get<BoundingVolumeOBB>(m_Volume);
      if (!obb.Contains(point)) {
        obb.Expand(point);
      }
      break;
    }
    case BoundingVolumeType::Plane:
      // 平面无法有意义地扩展包含点，保持原样
      break;
    default:
      break;
  }
}
void BoundingVolume::ExpandToInclude(const BoundingVolume &other)
{
  switch (m_Type) {
    case BoundingVolumeType::AABB: {
      BoundingVolumeAABB &aabb = std::get<BoundingVolumeAABB>(m_Volume);
      BoundingVolumeAABB otherAABB = other.GetAABBApproximation();
      aabb.Expand(otherAABB);
      break;
    }
    case BoundingVolumeType::Sphere: {
      BoundingVolumeSphere &sphere = std::get<BoundingVolumeSphere>(m_Volume);
      BoundingVolumeSphere otherSphere = other.GetSphereApproximation();
      sphere.Expand(otherSphere);
      break;
    }
    case BoundingVolumeType::OBB: {
      BoundingVolumeOBB &obb = std::get<BoundingVolumeOBB>(m_Volume);
      // 当OBB扩展时，必须使用OBB对其进行扩展，不应当由AABB和Sphere转换而来
      // 原因：精度损失问题。OBB是最精确的有向包围盒，近似转换的操作与OBB设定不符
      if (other.GetType() == BoundingVolumeType::OBB)
        obb.Expand(other.GetOBB());
      else
        LOG_ERROR("OBB can only expand to include OBB");
      break;
    }
    case BoundingVolumeType::Plane:
      // 平面无法有意义地扩展包含其他包围体
      break;
    default:
      break;
  }
}

// ==================== 内部辅助方法 ====================
void BoundingVolume::CreateDefaultVolume(BoundingVolumeType type)
{
  switch (type) {
    case BoundingVolumeType::AABB:
      m_Volume = BoundingVolumeAABB();
      break;
    case BoundingVolumeType::Sphere:
      m_Volume = BoundingVolumeSphere(glm::vec3(0.0f), 1.0f);
      break;
    case BoundingVolumeType::OBB:
      m_Volume = BoundingVolumeOBB(glm::vec3(0.0f), glm::vec3(1.0f), glm::mat3(1.0f));
      break;
    case BoundingVolumeType::Plane:
      m_Volume = BoundingVolumePlane(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);
      break;
    case BoundingVolumeType::None:
      m_Volume = BoundingVolumeAABB();
      break;
  }
}

BoundingVolumeIntersection::IntersectionType BoundingVolume::IntersectsAABB(
    const BoundingVolumeAABB &other) const
{
  switch (m_Type) {
    case BoundingVolumeType::AABB:
      return BoundingVolumeIntersection::Intersects(std::get<BoundingVolumeAABB>(m_Volume), other);
    case BoundingVolumeType::Sphere:
      return BoundingVolumeIntersection::Intersects(std::get<BoundingVolumeSphere>(m_Volume),
                                                    other);
    case BoundingVolumeType::OBB:
      return BoundingVolumeIntersection::Intersects(std::get<BoundingVolumeOBB>(m_Volume), other);
    case BoundingVolumeType::Plane:
      return BoundingVolumeIntersection::Intersects(std::get<BoundingVolumePlane>(m_Volume),
                                                    other);
    default:
      return BoundingVolumeIntersection::IntersectionType::Outside;
  }
}

BoundingVolumeIntersection::IntersectionType BoundingVolume::IntersectsSphere(
    const BoundingVolumeSphere &other) const
{
  // 函数体与IntersectsAABB完全相同，但是other类型不同，所以需要重载
  switch (m_Type) {
    case BoundingVolumeType::AABB:
      return BoundingVolumeIntersection::Intersects(std::get<BoundingVolumeAABB>(m_Volume), other);
    case BoundingVolumeType::Sphere:
      return BoundingVolumeIntersection::Intersects(std::get<BoundingVolumeSphere>(m_Volume),
                                                    other);
    case BoundingVolumeType::OBB:
      return BoundingVolumeIntersection::Intersects(std::get<BoundingVolumeOBB>(m_Volume), other);
    case BoundingVolumeType::Plane:
      return BoundingVolumeIntersection::Intersects(std::get<BoundingVolumePlane>(m_Volume),
                                                    other);
    default:
      return BoundingVolumeIntersection::IntersectionType::Outside;
  }
}

BoundingVolumeIntersection::IntersectionType BoundingVolume::IntersectsOBB(
    const BoundingVolumeOBB &other) const
{
  switch (m_Type) {
    case BoundingVolumeType::AABB:
      return BoundingVolumeIntersection::Intersects(std::get<BoundingVolumeAABB>(m_Volume), other);
    case BoundingVolumeType::Sphere:
      return BoundingVolumeIntersection::Intersects(std::get<BoundingVolumeSphere>(m_Volume),
                                                    other);
    case BoundingVolumeType::OBB:
      return BoundingVolumeIntersection::Intersects(std::get<BoundingVolumeOBB>(m_Volume), other);
    case BoundingVolumeType::Plane:
      return BoundingVolumeIntersection::Intersects(std::get<BoundingVolumePlane>(m_Volume),
                                                    other);
    default:
      return BoundingVolumeIntersection::IntersectionType::Outside;
  }
}

BoundingVolumeIntersection::IntersectionType BoundingVolume::IntersectsPlane(
    const BoundingVolumePlane &other) const
{
  switch (m_Type) {
    case BoundingVolumeType::AABB:
      return BoundingVolumeIntersection::Intersects(std::get<BoundingVolumeAABB>(m_Volume), other);
    case BoundingVolumeType::Sphere:
      return BoundingVolumeIntersection::Intersects(std::get<BoundingVolumeSphere>(m_Volume),
                                                    other);
    case BoundingVolumeType::OBB:
      return BoundingVolumeIntersection::Intersects(std::get<BoundingVolumeOBB>(m_Volume), other);
    default:
      return BoundingVolumeIntersection::IntersectionType::Outside;
  }
}

// // ==================== 静态数学工具（暂时保留） ====================
// BoundingVolumeAABB BoundingVolume::CreateAABBFromPoints(const glm::vec3 *points, uint32_t count)
//{
//  return BoundingVolumeAABB::CreateAABBFromPoints(points, count);
//}
//
// BoundingVolumeAABB BoundingVolume::TransformAABB(const BoundingVolumeAABB &original,
//                                                 const glm::mat4 &transform)
//{
//  return original.Transform(transform);
//}
//
// BoundingVolumeAABB BoundingVolume::MergeAABBs(const BoundingVolumeAABB &a,
//                                              const BoundingVolumeAABB &b)
//{
//  return BoundingVolumeAABB::Merge(a, b);
//}
//
// bool BoundingVolume::PointInAABB(const glm::vec3 &point, const BoundingVolumeAABB &aabb)
//{
//  return aabb.Contains(point);
//}
//
// bool BoundingVolume::AABBIntersectsAABB(const BoundingVolumeAABB &a, const BoundingVolumeAABB
// &b)
//{
//  return a.Intersects(b);
//}
//
// bool BoundingVolume::SphereIntersectsAABB(const BoundingVolumeSphere &sphere,
//                                          const BoundingVolumeAABB &aabb)
//{
//  // 计算球心到AABB最近点的距离平方
//  glm::vec3 closestPoint = glm::clamp(sphere.center, aabb.min, aabb.max);
//  float distSq = glm::distance(sphere.center, closestPoint) *
//                 glm::distance(sphere.center, closestPoint);
//  return distSq <= (sphere.radius * sphere.radius);
//}
//
// bool BoundingVolume::SphereIntersectsSphere(const BoundingVolumeSphere &a,
//                                            const BoundingVolumeSphere &b)
//{
//  float dist = glm::distance(a.center, b.center);
//  return dist <= (a.radius + b.radius);
//}
//
// BoundingVolumeSphere BoundingVolume::CreateSphereFromAABB(const BoundingVolumeAABB &aabb)
//{
//  return BoundingVolumeSphere::FromAABB(aabb);
//}
//
// BoundingVolumeSphere BoundingVolume::TransformSphere(const BoundingVolumeSphere &sphere,
//                                                     const glm::mat4 &transform)
//{
//  return sphere.Transform(transform);
//}
//
// BoundingVolumeOBB BoundingVolume::CreateOBBFromAABB(const BoundingVolumeAABB &aabb)
//{
//  return BoundingVolumeOBB::FromAABB(aabb);
//}
//
// BoundingVolumeOBB BoundingVolume::TransformOBB(const BoundingVolumeOBB &obb,
//                                               const glm::mat4 &transform)
//{
//  return obb.Transform(transform);
//}
//
// BoundingVolumeAABB BoundingVolume::GetAABBFromOBB(const BoundingVolumeOBB &obb)
//{
//  return obb.GetAABB();
//}
}  // namespace mite