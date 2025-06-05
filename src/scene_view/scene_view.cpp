#include "scene_view.h"


namespace mite {
SceneView::SceneView(const Scene &scene) {}
void SceneView::UpDate() {
  m_RenderData.Clear();

  // TODO：收集相机数据
  CameraData camera;
  //camera.viewMatrix = m_ActiveCamera->GetViewMatrix();
  //camera.projectionMatrix = m_ActiveCamera->GetProjectionMatrix();
  //camera.position = m_ActiveCamera->GetPosition();
  // ... 设置其他相机参数
  m_RenderData.SetCameraData(camera);

  // TODO：遍历场景实体
  //m_Scene->GetRegistry().view<TransformComponent, MeshComponent>().each(
  //    [this](auto entity, TransformComponent &transform, MeshComponent &mesh) {
  //      RenderItem item;
  //      item.transform = transform.GetWorldMatrix();
  //      item.mesh = mesh.mesh;
  //      item.material = mesh.material;
  //      item.submeshIndex = 0;  // 假设使用第一个子网格

  //      // 根据材质属性决定渲染通道
  //      bool isTransparent = mesh.material->IsTransparent();
  //      m_RenderData.AddRenderItem(isTransparent ? RenderPass::Transparent : RenderPass::Opaque,
  //                                 item);

  //      // 如果物体投射阴影
  //      if (mesh.castShadow) {
  //        m_RenderData.AddRenderItem(RenderPass::Shadow, item);
  //      }
  //    });

  // TODO: 收集光照数据
  //m_Scene->GetRegistry().view<DirectionalLightComponent>().each(
  //    [this](auto entity, DirectionalLightComponent &light) {
  //      DirectionalLight renderLight;
  //      renderLight.direction = light.direction;
  //      renderLight.color = light.color;
  //      renderLight.intensity = light.intensity;
  //      m_RenderData.AddDirectionalLight(renderLight);
  //    });

  // 准备渲染数据
  m_RenderData.Prepare();
}
void SceneView::SyncFromSceneCore() {}

RenderData &SceneView::GetRenderData()
{
  return m_RenderData;
}
}  // namespace mite
