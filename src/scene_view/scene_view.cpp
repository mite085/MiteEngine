#include "scene_view.h"


namespace mite {
SceneView::SceneView(const Scene &scene) {}
void SceneView::UpDate() {
  m_RenderData.Clear();

  // TODO���ռ��������
  CameraData camera;
  //camera.viewMatrix = m_ActiveCamera->GetViewMatrix();
  //camera.projectionMatrix = m_ActiveCamera->GetProjectionMatrix();
  //camera.position = m_ActiveCamera->GetPosition();
  // ... ���������������
  m_RenderData.SetCameraData(camera);

  // TODO����������ʵ��
  //m_Scene->GetRegistry().view<TransformComponent, MeshComponent>().each(
  //    [this](auto entity, TransformComponent &transform, MeshComponent &mesh) {
  //      RenderItem item;
  //      item.transform = transform.GetWorldMatrix();
  //      item.mesh = mesh.mesh;
  //      item.material = mesh.material;
  //      item.submeshIndex = 0;  // ����ʹ�õ�һ��������

  //      // ���ݲ������Ծ�����Ⱦͨ��
  //      bool isTransparent = mesh.material->IsTransparent();
  //      m_RenderData.AddRenderItem(isTransparent ? RenderPass::Transparent : RenderPass::Opaque,
  //                                 item);

  //      // �������Ͷ����Ӱ
  //      if (mesh.castShadow) {
  //        m_RenderData.AddRenderItem(RenderPass::Shadow, item);
  //      }
  //    });

  // TODO: �ռ���������
  //m_Scene->GetRegistry().view<DirectionalLightComponent>().each(
  //    [this](auto entity, DirectionalLightComponent &light) {
  //      DirectionalLight renderLight;
  //      renderLight.direction = light.direction;
  //      renderLight.color = light.color;
  //      renderLight.intensity = light.intensity;
  //      m_RenderData.AddDirectionalLight(renderLight);
  //    });

  // ׼����Ⱦ����
  m_RenderData.Prepare();
}
void SceneView::SyncFromSceneCore() {}

RenderData &SceneView::GetRenderData()
{
  return m_RenderData;
}
}  // namespace mite
