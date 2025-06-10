#ifndef MITE_SCENE_DATA
#define MITE_SCENE_DATA

#include "headers/headers.h"

namespace mite {
// ǰ������
class Mesh;
class Material;

// ��Ⱦ������ö��
enum class RenderPass {
  Opaque,
  Transparent,
  Shadow,
  // δ������չ��Wireframe, Debug��
};

// ��������Ⱦ���������
struct RenderItem {
  glm::mat4 transform;                 // ����任����
  std::shared_ptr<Mesh> mesh;          // ��������
  std::shared_ptr<Material> material;  // ��������
  uint32_t submeshIndex;               // ����������
  float distanceToCamera;              // ��������
  // ��Ⱦ��־λ
  union {
    struct {
      bool castShadow : 1;
      bool receiveShadow : 1;
      bool isStatic : 1;
    };
    uint32_t flags;
  };
};

// ��Ⱦ���Σ���ͬ����/��ɫ������Ŀ������Ⱦ��
struct RenderBatch {
  std::shared_ptr<Material> material;
  std::vector<RenderItem> items;
};

// ��Ⱦ��ͼ����
struct CameraData {
  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
  glm::vec3 position;
  float nearPlane;
  float farPlane;
  // ��׶��ƽ�������޳�
  glm::vec4 frustumPlanes[6];
};

// ��������
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

// ����Ⱦ���ݽṹ
class RenderData {
 public:
  // �����������
  void SetCameraData(const CameraData &camera);
  const CameraData &GetCameraData() const;

  // ��ӹ���
  void AddDirectionalLight(const DirectionalLight &light);
  void AddPointLight(const PointLight &light);

  // �����Ⱦ��
  void AddRenderItem(RenderPass pass, const RenderItem &item);

  // ��ȡָ����Ⱦͨ������������
  const std::vector<RenderBatch> &GetBatches(RenderPass pass) const;

  // ׼����Ⱦ���ݣ�����������ȣ�
  void Prepare();

  // ��յ�ǰ֡����
  void Clear();

 private:
  // �����ʶ���Ⱦ�����������
  void BatchItems();

  // ����������͸�����壨�Ӻ���ǰ��
  void SortTransparentItems();

  // ִ����׶���޳�
  void FrustumCull();

 private:
  CameraData m_CameraData;
  std::vector<DirectionalLight> m_DirectionalLights;
  std::vector<PointLight> m_PointLights;

  // ����Ⱦͨ�������ԭʼ��Ⱦ��
  std::vector<RenderItem> m_OpaqueItems;
  std::vector<RenderItem> m_TransparentItems;
  std::vector<RenderItem> m_ShadowCasters;

  // ����������Ⱦ����
  std::vector<RenderBatch> m_OpaqueBatches;
  std::vector<RenderBatch> m_TransparentBatches;
  std::vector<RenderBatch> m_ShadowBatches;

  // ������Ⱦ�������...
  bool m_DataPrepared = false;
};

};  // namespace mite

#endif
