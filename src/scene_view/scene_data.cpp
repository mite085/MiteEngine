#include "scene_data.h"

namespace mite {

void RenderData::SetCameraData(const CameraData &camera)
{
  m_CameraData = camera;
  m_DataPrepared = false;
}

const CameraData &RenderData::GetCameraData() const
{
  return m_CameraData;
}

void RenderData::AddDirectionalLight(const DirectionalLight &light)
{
  m_DirectionalLights.push_back(light);
}

void RenderData::AddPointLight(const PointLight &light)
{
  m_PointLights.push_back(light);
}

void RenderData::AddRenderItem(RenderPass pass, const RenderItem &item)
{
  switch (pass) {
    case RenderPass::Opaque:
      m_OpaqueItems.push_back(item);
      break;
    case RenderPass::Transparent:
      m_TransparentItems.push_back(item);
      break;
    case RenderPass::Shadow:
      m_ShadowCasters.push_back(item);
      break;
  }
  m_DataPrepared = false;
}

const std::vector<RenderBatch> &RenderData::GetBatches(RenderPass pass) const
{
  static const std::vector<RenderBatch> emptyBatches;

  if (!m_DataPrepared) {
    throw std::runtime_error("RenderData not prepared before use");
  }

  switch (pass) {
    case RenderPass::Opaque:
      return m_OpaqueBatches;
    case RenderPass::Transparent:
      return m_TransparentBatches;
    case RenderPass::Shadow:
      return m_ShadowBatches;
    default:
      return emptyBatches;
  }
}

void RenderData::Prepare()
{
  if (m_DataPrepared)
    return;

  // 1. ִ����׶���޳�
  FrustumCull();

  // 2. ����͸�����嵽����ľ���
  for (auto &item : m_TransparentItems) {
    glm::vec3 position = glm::vec3(item.transform[3]);
    item.distanceToCamera = glm::distance(position, m_CameraData.position);
  }

  // 3. ����
  SortTransparentItems();

  // 4. ������
  BatchItems();

  m_DataPrepared = true;
}

void RenderData::Clear()
{
  m_OpaqueItems.clear();
  m_TransparentItems.clear();
  m_ShadowCasters.clear();
  m_OpaqueBatches.clear();
  m_TransparentBatches.clear();
  m_ShadowBatches.clear();
  m_DirectionalLights.clear();
  m_PointLights.clear();
  m_DataPrepared = false;
}

void RenderData::BatchItems()
{
  // �����������������ȡ��������
  auto getOrCreateBatch = [](std::vector<RenderBatch> &batches,
                             const std::shared_ptr<Material> &material) -> RenderBatch * {
    for (auto &batch : batches) {
      if (batch.material == material) {
        return &batch;
      }
    }
    batches.emplace_back();
    batches.back().material = material;
    return &batches.back();
  };

  // ����͸������
  for (const auto &item : m_OpaqueItems) {
    RenderBatch *batch = getOrCreateBatch(m_OpaqueBatches, item.material);
    batch->items.push_back(item);
  }

  // ����͸������
  for (const auto &item : m_TransparentItems) {
    RenderBatch *batch = getOrCreateBatch(m_TransparentBatches, item.material);
    batch->items.push_back(item);
  }

  // ������ӰͶ������
  for (const auto &item : m_ShadowCasters) {
    RenderBatch *batch = getOrCreateBatch(m_ShadowBatches, item.material);
    batch->items.push_back(item);
  }
}

void RenderData::SortTransparentItems()
{
  std::sort(m_TransparentItems.begin(),
            m_TransparentItems.end(),
            [](const RenderItem &a, const RenderItem &b) {
              return a.distanceToCamera > b.distanceToCamera;  // ��Զ��������
            });
}

void RenderData::FrustumCull()
{
  // �򻯵���׶���޳�ʵ��
  auto isInsideFrustum = [this](const RenderItem &item) -> bool {
    // ��ȡ����İ�Χ������򻯴���ʵ��Ӧ�ô�mesh��ȡ��
    glm::vec3 center = glm::vec3(item.transform[3]);
    float radius = 1.0f;  // ����뾶Ϊ1��ʵ��Ӧʹ��mesh�İ�Χ��뾶

    for (int i = 0; i < 6; ++i) {
      float distance = glm::dot(center, glm::vec3(m_CameraData.frustumPlanes[i])) +
                       m_CameraData.frustumPlanes[i].w;
      if (distance < -radius) {
        return false;
      }
    }
    return true;
  };

  // ��ÿ����Ⱦͨ��ִ���޳�
  auto cullItems = [&isInsideFrustum](std::vector<RenderItem> &items) {
    auto newEnd = std::remove_if(
        items.begin(), items.end(), [&isInsideFrustum](const RenderItem &item) {
          return !isInsideFrustum(item);
        });
    items.erase(newEnd, items.end());
  };

  cullItems(m_OpaqueItems);
  cullItems(m_TransparentItems);
  cullItems(m_ShadowCasters);
}
};  // namespace mite