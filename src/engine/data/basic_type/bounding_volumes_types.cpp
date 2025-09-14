#include "bounding_volumes_types.h"

namespace mite {
// ==================== AABB成员函数实现 ====================

BoundingVolumeAABB::BoundingVolumeAABB(const glm::vec3 &center, float halfExtent)
{
  min = center - glm::vec3(halfExtent);
  max = center + glm::vec3(halfExtent);
}

bool BoundingVolumeAABB::IsValid() const
{
  return min.x <= max.x && min.y <= max.y && min.z <= max.z;
}
glm::vec3 BoundingVolumeAABB::GetCenter() const
{
  return (min + max) * 0.5f;
}
glm::vec3 BoundingVolumeAABB::GetSize() const
{
  return max - min;
}
glm::vec3 BoundingVolumeAABB::GetHalfExtents() const
{
  return GetSize() * 0.5f;
}

BoundingVolumeAABB BoundingVolumeAABB::Expand(const glm::vec3 &point)
{
  min = glm::min(min, point);
  max = glm::max(max, point);

  return *this;
}

BoundingVolumeAABB BoundingVolumeAABB::Expand(const BoundingVolumeAABB &other)
{
  min = glm::min(min, other.min);
  max = glm::max(max, other.max);

  return *this;
}

BoundingVolumeAABB BoundingVolumeAABB::Transform(const glm::mat4 &matrix) const
{
  // 变换AABB的8个顶点并重新计算包围盒
  glm::vec3 vertices[8] = {{min.x, min.y, min.z},
                           {min.x, min.y, max.z},
                           {min.x, max.y, min.z},
                           {min.x, max.y, max.z},
                           {max.x, min.y, min.z},
                           {max.x, min.y, max.z},
                           {max.x, max.y, min.z},
                           {max.x, max.y, max.z}};

  BoundingVolumeAABB result;
  for (int i = 0; i < 8; ++i) {
    glm::vec4 transformed = matrix * glm::vec4(vertices[i], 1.0f);
    result.Expand(glm::vec3(transformed));
  }
  return result;
}

float BoundingVolumeAABB::GetSurfaceArea() const
{
  glm::vec3 size = max - min;
  return 2.0f * (size.x * size.y + size.x * size.z + size.y * size.z);
}

float BoundingVolumeAABB::GetVolume() const
{
  glm::vec3 size = max - min;
  return size.x * size.y * size.z;
}

bool BoundingVolumeAABB::Intersects(const BoundingVolumeAABB &other) const
{
  return (min.x <= other.max.x && max.x >= other.min.x) &&
         (min.y <= other.max.y && max.y >= other.min.y) &&
         (min.z <= other.max.z && max.z >= other.min.z);
}

bool BoundingVolumeAABB::Contains(const glm::vec3 &point) const
{
  return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y &&
         point.z >= min.z && point.z <= max.z;
}

bool BoundingVolumeAABB::Contains(const BoundingVolumeAABB &other) const
{
  return Contains(other.min) && Contains(other.max);
}

float BoundingVolumeAABB::DistanceToPointSq(const glm::vec3 &point) const
{
  float sqDist = 0.0f;

  // 分别计算三个轴上的距离分量
  for (int i = 0; i < 3; ++i) {
    // 点在包围盒左侧
    if (point[i] < min[i]) {
      float diff = min[i] - point[i];
      sqDist += diff * diff;
    }
    // 点在包围盒右侧
    else if (point[i] > max[i]) {
      float diff = point[i] - max[i];
      sqDist += diff * diff;
    }
    // 点在包围盒内部，该轴距离分量为0
  }

  return sqDist;
}

BoundingVolumeAABB BoundingVolumeAABB::CreateAABBFromPoints(const glm::vec3 *points,
                                                            uint32_t count)
{
  BoundingVolumeAABB result;
  if (count == 0)
    return result;

  glm::vec3 minPoint = points[0];
  glm::vec3 maxPoint = points[0];

  for (uint32_t i = 1; i < count; ++i) {
    minPoint = glm::min(minPoint, points[i]);
    maxPoint = glm::max(maxPoint, points[i]);
  }

  result.min = minPoint;
  result.max = maxPoint;
  return result;
}

// ==================== Sphere成员函数实现 ====================
BoundingVolumeSphere BoundingVolumeSphere::FromAABB(const BoundingVolumeAABB &aabb)
{
  glm::vec3 center = aabb.GetCenter();

  // 获取三个轴向的半轴长glm::vec3 GetHalfExtents()，
  // 使用glm::length计算对角线长度，作为外切球的半径
  float radius = glm::length(aabb.GetHalfExtents());
  return BoundingVolumeSphere(center, radius);
}

bool BoundingVolumeSphere::Intersects(const BoundingVolumeSphere &other) const
{
  float dist = glm::distance(center, other.center);
  return dist <= (radius + other.radius);
}
bool BoundingVolumeSphere::Contains(const glm::vec3 &point) const
{
  return (glm::distance(center, point) * glm::distance(center, point)) <= (radius * radius);
}
bool BoundingVolumeSphere::Contains(const BoundingVolumeSphere &other) const
{
  float dist = glm::distance(center, other.center);
  return dist + other.radius <= radius;
}
BoundingVolumeSphere BoundingVolumeSphere::Expand(const glm::vec3 &point)
{
  float dist = glm::distance(center, point);
  if (dist > radius) {
    // 需要扩展球体
    glm::vec3 direction = glm::normalize(point - center);
    glm::vec3 newEdge = center + direction * radius;
    center = (newEdge + point) * 0.5f;
    radius = glm::distance(center, point);
  }

  return *this;
}
BoundingVolumeSphere BoundingVolumeSphere::Expand(const BoundingVolumeSphere &other)
{
  float dist = glm::distance(center, other.center);

  if (dist + other.radius <= radius) {
    // 另一个球完全在当前球内，无需扩展
    return;
  }

  if (dist + radius <= other.radius) {
    // 当前球完全在另一个球内，直接使用另一个球
    center = other.center;
    radius = other.radius;
    return;
  }

  // 两个球相交，计算新的包围球
  float newRadius = (dist + radius + other.radius) * 0.5f;
  glm::vec3 direction = glm::normalize(other.center - center);
  center = center + direction * (newRadius - radius);
  radius = newRadius;

  return *this;
}

BoundingVolumeSphere BoundingVolumeSphere::Transform(const glm::mat4 &matrix) const
{
  // 提取平移和均匀缩放
  glm::vec3 newCenter = glm::vec3(matrix * glm::vec4(center, 1.0f));

  // 假设均匀缩放，取最大缩放值
  glm::vec3 scale = glm::vec3(glm::length(glm::vec3(matrix[0])),
                              glm::length(glm::vec3(matrix[1])),
                              glm::length(glm::vec3(matrix[2])));
  float maxScale = glm::max(glm::max(scale.x, scale.y), scale.z);
  float newRadius = radius * maxScale;

  return BoundingVolumeSphere(newCenter, newRadius);
}
// Welzl算法辅助函数
void WelzlAlgorithm(std::vector<glm::vec3> &points,
                    int numPoints,
                    std::vector<glm::vec3> &support,
                    int numSupport,
                    BoundingVolumeSphere &result)
{
  if (numPoints == 0 || numSupport == 4) {
    switch (numSupport) {
      case 0:
        result = BoundingVolumeSphere(glm::vec3(0.0f), 0.0f);
        break;
      case 1:
        result = BoundingVolumeSphere(support[0], 0.0f);
        break;
      case 2: {
        glm::vec3 center = (support[0] + support[1]) * 0.5f;
        float radius = glm::distance(center, support[0]);
        result = BoundingVolumeSphere(center, radius);
        break;
      }
      case 3: {
        // 三点确定球
        glm::vec3 a = support[0], b = support[1], c = support[2];
        glm::vec3 ab = b - a, ac = c - a;
        glm::vec3 abXac = glm::cross(ab, ac);

        if (glm::length(abXac) < 1e-6f) {
          // 共线点，取最大距离的两点
          float maxDist = 0.0f;
          glm::vec3 p1, p2;
          for (int i = 0; i < 3; ++i) {
            for (int j = i + 1; j < 3; ++j) {
              float dist = glm::distance(support[i], support[j]);
              if (dist > maxDist) {
                maxDist = dist;
                p1 = support[i];
                p2 = support[j];
              }
            }
          }
          glm::vec3 center = (p1 + p2) * 0.5f;
          float radius = maxDist * 0.5f;
          result = BoundingVolumeSphere(center, radius);
        }
        else {
          // 计算外接球中心
          glm::vec3 toCircumcircle = glm::cross(glm::dot(ab, ab) * ac - glm::dot(ac, ac) * ab,
                                                glm::cross(ab, ac)) /
                                     (2.0f * glm::dot(glm::cross(ab, ac), glm::cross(ab, ac)));

          glm::vec3 center = a + toCircumcircle;
          float radius = glm::distance(center, a);
          result = BoundingVolumeSphere(center, radius);
        }
        break;
      }
      case 4: {
        // 四点确定球（四面体外接球）
        glm::vec3 a = support[0], b = support[1], c = support[2], d = support[3];

        glm::vec3 ab = b - a, ac = c - a, ad = d - a;
        glm::mat3 mat(
            glm::vec3(ab.x, ac.x, ad.x), glm::vec3(ab.y, ac.y, ad.y), glm::vec3(ab.z, ac.z, ad.z));

        float det = glm::determinant(mat);
        if (std::abs(det) < 1e-6f) {
          // 退化情况，使用前三点
          std::vector<glm::vec3> tempSupport(support.begin(), support.begin() + 3);
          WelzlAlgorithm(points, 0, tempSupport, 3, result);
          return;
        }

        glm::vec3 rhs = glm::vec3(
            glm::dot(ab, ab) * 0.5f, glm::dot(ac, ac) * 0.5f, glm::dot(ad, ad) * 0.5f);

        glm::vec3 solution = glm::inverse(mat) * rhs;
        glm::vec3 center = a + solution.x * ab + solution.y * ac + solution.z * ad;

        float maxRadius = 0.0f;
        for (int i = 0; i < 4; ++i) {
          float dist = glm::distance(center, support[i]);
          if (dist > maxRadius)
            maxRadius = dist;
        }
        result = BoundingVolumeSphere(center, maxRadius);
        break;
      }
    }
    return;
  }

  // 随机选择一个点
  int index = numPoints - 1;
  glm::vec3 p = points[index];

  // 递归计算不包含该点的球
  WelzlAlgorithm(points, numPoints - 1, support, numSupport, result);

  // 如果点在当前球外，将其加入支撑集
  if (!result.Contains(p)) {
    support[numSupport] = p;
    WelzlAlgorithm(points, numPoints - 1, support, numSupport + 1, result);
  }
}
BoundingVolumeSphere CreateSphereFromPoints(const std::vector<glm::vec3> &points)
{
  if (points.empty()) {
    return BoundingVolumeSphere();
  }

  if (points.size() == 1) {
    return BoundingVolumeSphere(points[0], 0.0f);
  }

  // 使用Welzl算法
  std::vector<glm::vec3> mutablePoints = points;
  std::vector<glm::vec3> support(4);
  BoundingVolumeSphere result;

  // 随机打乱点集以获得更好的平均性能，需要创建随机数引擎（静态创建避免开销）
  static std::mt19937 g(std::random_device{}());
  std::shuffle(mutablePoints.begin(), mutablePoints.end(), g);

  WelzlAlgorithm(mutablePoints, static_cast<int>(mutablePoints.size()), support, 0, result);

  return result;
}
BoundingVolumeSphere CreateSphereFromPointsRitter(const std::vector<glm::vec3> &points)
{
  if (points.empty()) {
    return BoundingVolumeSphere();
  }

  // Ritter算法：先找到距离最远的两个点
  glm::vec3 p1 = points[0], p2 = points[0];
  float maxDist = 0.0f;

  // 第一遍：找到大致范围
  for (size_t i = 0; i < points.size(); ++i) {
    for (size_t j = i + 1; j < points.size(); ++j) {
      float dist = glm::distance(points[i], points[j]);
      if (dist > maxDist) {
        maxDist = dist;
        p1 = points[i];
        p2 = points[j];
      }
    }
  }

  // 初始球：以两点中点为圆心，半距离为半径
  glm::vec3 center = (p1 + p2) * 0.5f;
  float radius = maxDist * 0.5f;
  BoundingVolumeSphere sphere(center, radius);

  // 第二遍：扩展球以包含所有点
  for (const auto &point : points) {
    float distance = glm::distance(sphere.center, point);
    if (distance > sphere.radius) {
      // 扩展球体
      glm::vec3 direction = glm::normalize(point - sphere.center);
      glm::vec3 newEdge = sphere.center + direction * sphere.radius;
      sphere.center = (newEdge + point) * 0.5f;
      sphere.radius = glm::distance(sphere.center, point);
    }
  }

  return sphere;
}
BoundingVolumeSphere CreateSphereFromPointsFast(const std::vector<glm::vec3> &points)
{
  if (points.empty()) {
    return BoundingVolumeSphere();
  }

  // 快速算法：计算点集中心，然后找最远点
  glm::vec3 center(0.0f);
  for (const auto &point : points) {
    center += point;
  }
  center /= static_cast<float>(points.size());

  // 找到离中心最远的点
  float maxDist = 0.0f;
  for (const auto &point : points) {
    float dist = glm::distance(center, point);
    if (dist > maxDist) {
      maxDist = dist;
    }
  }

  return BoundingVolumeSphere(center, maxDist);
}
// ==================== OBB成员函数实现 ====================

BoundingVolumeOBB BoundingVolumeOBB::FromAABB(const BoundingVolumeAABB &aabb)
{
  glm::vec3 center = aabb.GetCenter();
  glm::vec3 extents = aabb.GetHalfExtents();
  return BoundingVolumeOBB(center, extents, glm::mat3(1.0f));
}

void BoundingVolumeOBB::GetVertices(glm::vec3 vertices[8]) const
{
  glm::vec3 axes[3] = {
      orientation[0] * extents.x, orientation[1] * extents.y, orientation[2] * extents.z};

  vertices[0] = center - axes[0] - axes[1] - axes[2];
  vertices[1] = center - axes[0] - axes[1] + axes[2];
  vertices[2] = center - axes[0] + axes[1] - axes[2];
  vertices[3] = center - axes[0] + axes[1] + axes[2];
  vertices[4] = center + axes[0] - axes[1] - axes[2];
  vertices[5] = center + axes[0] - axes[1] + axes[2];
  vertices[6] = center + axes[0] + axes[1] - axes[2];
  vertices[7] = center + axes[0] + axes[1] + axes[2];
}

BoundingVolumeAABB BoundingVolumeOBB::GetAABB() const
{
  glm::vec3 vertices[8];
  GetVertices(vertices);
  return BoundingVolumeAABB::CreateAABBFromPoints(vertices, 8);
}

BoundingVolumeOBB BoundingVolumeOBB::Transform(const glm::mat4 &matrix) const
{
  glm::vec3 newCenter = glm::vec3(matrix * glm::vec4(center, 1.0f));
  glm::mat3 newOrientation = glm::mat3(matrix) * orientation;

  // 计算新的半长（考虑非均匀缩放）
  glm::vec3 scale = glm::vec3(glm::length(glm::vec3(matrix[0])),
                              glm::length(glm::vec3(matrix[1])),
                              glm::length(glm::vec3(matrix[2])));
  glm::vec3 newExtents = extents * scale;

  return BoundingVolumeOBB(newCenter, newExtents, newOrientation);
}

bool BoundingVolumeOBB::Contains(const glm::vec3 &point) const
{
  // 将点转换到OBB局部空间
  glm::vec3 localPoint = glm::transpose(orientation) * (point - center);
  return std::abs(localPoint.x) <= extents.x && std::abs(localPoint.y) <= extents.y &&
         std::abs(localPoint.z) <= extents.z;
}
BoundingVolumeOBB BoundingVolumeOBB::Expand(const glm::vec3 &point)
{
  // 将点转换到OBB局部空间
  glm::vec3 localPoint = glm::transpose(orientation) * (point - center);

  // 检查是否需要扩展每个轴向
  glm::vec3 newExtents = extents;
  bool needUpdate = false;

  for (int i = 0; i < 3; ++i) {
    if (std::abs(localPoint[i]) > extents[i]) {
      newExtents[i] = std::abs(localPoint[i]);
      needUpdate = true;
    }
  }

  if (needUpdate) {
    extents = newExtents;
  }

  return *this;
}
BoundingVolumeOBB BoundingVolumeOBB::Expand(const BoundingVolumeOBB &other)
{
  // 获取两个OBB的所有顶点
  glm::vec3 thisVertices[8];
  glm::vec3 otherVertices[8];

  GetVertices(thisVertices);
  other.GetVertices(otherVertices);

  // 创建一个临时的AABB来包含所有顶点
  BoundingVolumeAABB tempAABB;

  // 添加当前OBB的所有顶点
  for (int i = 0; i < 8; ++i) {
    tempAABB.Expand(thisVertices[i]);
  }

  // 添加另一个OBB的所有顶点
  for (int i = 0; i < 8; ++i) {
    tempAABB.Expand(otherVertices[i]);
  }

  // 从AABB创建新的OBB（保持当前方向）
  center = tempAABB.GetCenter();

  // 计算新的半长（在OBB的局部坐标系中）
  glm::vec3 tempExtents = tempAABB.GetHalfExtents();

  // 将AABB的半长转换到OBB的局部坐标系
  // 由于OBB可能有旋转，我们需要找到在OBB轴向上的投影
  for (int i = 0; i < 3; ++i) {
    glm::vec3 axis = orientation[i];
    // 计算AABB半长在该轴向上的最大投影
    float maxProjection = 0.0f;
    for (int j = 0; j < 3; ++j) {
      float projection = std::abs(glm::dot(axis,
                                           glm::vec3(j == 0 ? tempExtents.x : 0.0f,
                                                     j == 1 ? tempExtents.y : 0.0f,
                                                     j == 2 ? tempExtents.z : 0.0f)));
      maxProjection = glm::max(maxProjection, projection);
    }
    extents[i] = maxProjection;
  }

  return *this;
}
// ==================== Plane成员函数实现 ====================

BoundingVolumePlane::BoundingVolumePlane(const glm::vec3 &point, const glm::vec3 &normal)
    : normal(glm::normalize(normal)), distance(-glm::dot(normal, point))
{
  // 因为平面方程是 normal·point + distance = 0，所以 distance = -normal·point
}

float BoundingVolumePlane::DistanceToPoint(const glm::vec3 &point) const
{
  // 右手系平面方程：normal.x*x + normal.y*y + normal.z*z + distance = 0
  // 所以点到平面的距离 = normal·point + distance
  return glm::dot(normal, point) + distance;
}

int BoundingVolumePlane::GetSide(const glm::vec3 &point) const
{
  float dist = DistanceToPoint(point);
  if (dist > 0.0f)
    return 1;  // 正面
  if (dist < 0.0f)
    return -1;  // 背面
  return 0;     // 在平面上
}
}  // namespace mite