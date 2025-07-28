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

void InspectorPanel::DrawAddComponentMenu()
{
  auto &registry = SceneCore::GetRegistry();

  // 1. 不可重复添加的组件（已存在时禁用）
  auto drawUniqueComponent = [&](const char *name, auto componentType) {
    if (registry.all_of<decltype(componentType)>(m_currentEntity)) {
      ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    }

    if (ImGui::MenuItem(name)) {
      registry.emplace<decltype(componentType)>(m_currentEntity);
      EventBus::Publish(ComponentAddedEvent{m_currentEntity, typeid(componentType)});
    }

    if (registry.all_of<decltype(componentType)>(m_currentEntity)) {
      ImGui::PopItemFlag();
      ImGui::PopStyleVar();
    }
  };

  // 2. 可重复添加的组件
  auto drawMultiComponent = [&](const char *name, auto componentType) {
    if (ImGui::MenuItem(name)) {
      registry.emplace<decltype(componentType)>(m_currentEntity);
      EventBus::Publish(ComponentAddedEvent{m_currentEntity, typeid(componentType)});
    }
  };

  // 3. 带参数的组件
  auto drawAssetComponent = [&](const char *name) {
    if (ImGui::MenuItem(name)) {
      if (auto mesh = AssetManager::PickMeshAsset()) {
        registry.emplace<MeshComponent>(m_currentEntity, mesh->id);
        EventBus::Publish(ComponentAddedEvent{m_currentEntity, typeid(MeshComponent)});
      }
    }
  };

  //=== 组件菜单分类 ===//
  if (ImGui::BeginMenu("渲染")) {
    drawUniqueComponent("网格过滤器", MeshFilterComponent{});
    drawAssetComponent("网格渲染器");
    drawUniqueComponent("粒子系统", ParticleComponent{});
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("物理")) {
    drawUniqueComponent("刚体", RigidbodyComponent{});
    drawUniqueComponent("碰撞体", ColliderComponent{});
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("脚本")) {
    drawMultiComponent("C++脚本", NativeScriptComponent{});
    drawMultiComponent("Lua脚本", LuaScriptComponent{});
    ImGui::EndMenu();
  }

  // 4. 自定义组件扩展点
  if (ImGui::BeginMenu("自定义")) {
    if (ImGui::MenuItem("从模板添加...")) {
      ImGui::OpenPopup("ComponentTemplates");
    }

    if (ImGui::BeginPopup("ComponentTemplates")) {
      for (auto &[name, factory] : ComponentFactory::GetTemplates()) {
        if (ImGui::MenuItem(name.c_str())) {
          factory(registry, m_currentEntity);
        }
      }
      ImGui::EndPopup();
    }
    ImGui::EndMenu();
  }
}

};
