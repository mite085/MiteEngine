#include "inspector_panel.h"
#include <imgui.h>

namespace mite {
InspectorPanel::InspectorPanel() : UIPanel("Inspector")
{
  // 订阅实体选择事件
  EventBus::Subscribe(this, &InspectorPanel::OnEntitySelected);
}

void InspectorPanel::DrawContent()
{
  if (m_currentEntity == entt::null) {
    ImGui::Text("未选择任何实体");
    return;
  }

  auto &registry = SceneCore::GetRegistry();
  if (!registry.valid(m_currentEntity)) {
    m_currentEntity = entt::null;
    return;
  }

  // 1. 显示实体基本信息
  ImGui::Text("实体ID: %d", static_cast<int>(m_currentEntity));
  ImGui::SameLine();
  if (ImGui::Button("删除实体")) {
    registry.destroy(m_currentEntity);
    m_currentEntity = entt::null;
    return;
  }

  // 2. 绘制所有组件
  DrawTransformComponent(registry, m_currentEntity);
  if (registry.has<MeshComponent>(m_currentEntity)) {
    DrawMeshComponent(registry, m_currentEntity);
  }

  // 3. 添加组件按钮
  ImGui::Separator();
  if (ImGui::Button("+ 添加组件")) {
    ImGui::OpenPopup("AddComponentPopup");
  }
  if (ImGui::BeginPopup("AddComponentPopup")) {
    DrawAddComponentMenu();
    ImGui::EndPopup();
  }
}

void InspectorPanel::DrawTransformComponent(entt::registry &registry, entt::entity entity)
{
  if (ImGui::CollapsingHeader("变换", ImGuiTreeNodeFlags_DefaultOpen)) {
    auto &transform = registry.get<TransformComponent>(entity);

    ImGui::DragFloat3("位置", &transform.position.x, 0.1f);
    ImGui::DragFloat3("旋转", &transform.rotation.x, 1.0f);
    ImGui::DragFloat3("缩放", &transform.scale.x, 0.1f, 0.01f);

    if (ImGui::Button("重置")) {
      transform = TransformComponent{};
    }
  }
}

void InspectorPanel::DrawMeshComponent(entt::registry &registry, entt::entity entity)
{
  if (ImGui::CollapsingHeader("网格")) {
    auto &mesh = registry.get<MeshComponent>(entity);

    // 显示网格资产信息
    if (ImGui::Button("更换网格")) {
      auto newMesh = AssetManager::PickMeshAsset();
      if (newMesh)
        mesh.meshId = newMesh->id;
    }

    // 材质列表编辑
    for (auto &material : mesh.materials) {
      ImGui::Text("材质槽 %d", &material - mesh.materials.data());
      ImGui::SameLine();
      if (ImGui::Button("编辑")) {
        MaterialSystem::OpenEditor(material);
      }
    }
  }
}

void InspectorPanel::OnEntitySelected(entt::entity entity)
{
  m_currentEntity = entity;
}
};
