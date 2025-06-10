#ifndef MITE_SCENE_DATA
#define MITE_SCENE_DATA

#include "headers/headers.h"

namespace mite {
// 前置声明
class Mesh;
class Material;

// 渲染项类型枚举
enum class RenderPass {
  Opaque,
  Transparent,
  Shadow,
  // 未来可扩展：Wireframe, Debug等
};

// 单个可渲染对象的数据
struct RenderItem {
  glm::mat4 transform;                 // 世界变换矩阵
  std::shared_ptr<Mesh> mesh;          // 网格数据
  std::shared_ptr<Material> material;  // 材质数据
  uint32_t submeshIndex;               // 子网格索引
  float distanceToCamera;              // 用于排序
  // 渲染标志位
  union {
    struct {
      bool castShadow : 1;
      bool receiveShadow : 1;
      bool isStatic : 1;
    };
    uint32_t flags;
  };
};

// 渲染批次（相同材质/着色器的项目批量渲染）
struct RenderBatch {
  std::shared_ptr<Material> material;
  std::vector<RenderItem> items;
};

// 渲染视图数据
struct CameraData {
  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
  glm::vec3 position;
  float nearPlane;
  float farPlane;
  // 视锥体平面用于剔除
  glm::vec4 frustumPlanes[6];
};

// 光照数据
struct DirectionalLight {
  glm::vec3 direction;
  glm::vec3 color;
  float intensity;
};

struct PointLight {
  glm::vec3 position;
  glm::vec3 color;
  float radius;
  float intensity;
};

// 主渲染数据结构
class RenderData {
 public:
  // 设置相机数据
  void SetCameraData(const CameraData &camera);
  const CameraData &GetCameraData() const;

  // 添加光照
  void AddDirectionalLight(const DirectionalLight &light);
  void AddPointLight(const PointLight &light);

  // 添加渲染项
  void AddRenderItem(RenderPass pass, const RenderItem &item);

  // 获取指定渲染通道的批次数据
  const std::vector<RenderBatch> &GetBatches(RenderPass pass) const;

  // 准备渲染数据（排序、批处理等）
  void Prepare();

  // 清空当前帧数据
  void Clear();

 private:
  // 按材质对渲染项进行批处理
  void BatchItems();

  // 按距离排序透明物体（从后向前）
  void SortTransparentItems();

  // 执行视锥体剔除
  void FrustumCull();

 private:
  CameraData m_CameraData;
  std::vector<DirectionalLight> m_DirectionalLights;
  std::vector<PointLight> m_PointLights;

  // 按渲染通道分类的原始渲染项
  std::vector<RenderItem> m_OpaqueItems;
  std::vector<RenderItem> m_TransparentItems;
  std::vector<RenderItem> m_ShadowCasters;

  // 批处理后的渲染数据
  std::vector<RenderBatch> m_OpaqueBatches;
  std::vector<RenderBatch> m_TransparentBatches;
  std::vector<RenderBatch> m_ShadowBatches;

  // 其他渲染相关数据...
  bool m_DataPrepared = false;
};

};  // namespace mite

#endif
