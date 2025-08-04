#include "inspector_panel.h"
#include <imgui.h>
#include "scene_core_components/transform_component.h"
#include "scene_core_components/mesh_component.h"

namespace mite {
InspectorPanel::InspectorPanel(SceneRegistry &registry)
    : m_registry(registry), UIPanel("Inspector")
{
  // 订阅实体选择事件
  //EventBus::Subscribe(this, &InspectorPanel::OnEntitySelected);
}

void InspectorPanel::DrawContent()
{
  if (!m_currentEntity.IsValid()) {
    ImGui::Text("No selected entity");
    return;
  }

  if (!m_registry.IsValid(m_currentEntity)) {
    m_currentEntity = Entity();
    return;
  }

  // 1. 显示实体基本信息
  ImGui::Text("Entity ID: %d", static_cast<int>(m_currentEntity.GetHandle()));
  ImGui::SameLine();
  if (ImGui::Button("Destroy Entity")) {
    m_registry.DestroyEntity(m_currentEntity);
    m_currentEntity = Entity();
    return;
  }

  // 2. 绘制所有组件
  DrawTransformComponent();
  if (m_registry.HasComponent<MeshComponent>(m_currentEntity)) {
    DrawMeshComponent();
  }

  // 3. 添加组件按钮
  ImGui::Separator();
  if (ImGui::Button("Add Component")) {
    ImGui::OpenPopup("AddComponentPopup");
  }
  if (ImGui::BeginPopup("AddComponentPopup")) {
    DrawAddComponentMenu();
    ImGui::EndPopup();
  }
}

void InspectorPanel::DrawTransformComponent()
{
  if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
    auto &transform = m_registry.GetComponent<TransformComponent>(m_currentEntity);

    //ImGui::DragFloat3("位置", transform.GetLocalPosition().x, 0.1f);
    //ImGui::DragFloat3("旋转", transform.GetLocalPosition().x, 1.0f);
    //ImGui::DragFloat3("缩放", transform.GetLocalPosition().x, 0.1f, 0.01f);

    if (ImGui::Button("Reset")) {
      //transform = TransformComponent();
    }
  }
}

void InspectorPanel::DrawMeshComponent()
{
  if (ImGui::CollapsingHeader("Mesh")) {
    auto &mesh = m_registry.GetComponent<MeshComponent>(m_currentEntity);

    // TODO:显示网格资产信息
    //if (ImGui::Button("更换网格")) {
    //  auto newMesh = AssetManager::PickMeshAsset();
    //  if (newMesh)
    //    mesh.meshId = newMesh->id;
    //}

    // 材质列表编辑
  //  for (auto &material : mesh.materials) {
  //    ImGui::Text("材质槽 %d", &material - mesh.materials.data());
  //    ImGui::SameLine();
  //    if (ImGui::Button("编辑")) {
  //      MaterialSystem::OpenEditor(material);
  //    }
  //  }
  }
}

void InspectorPanel::OnEntitySelected(Entity entity)
{
  m_currentEntity = entity;
}

void InspectorPanel::DrawAddComponentMenu()
{

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
  //auto drawAssetComponent = [&](const char *name) {
  //  if (ImGui::MenuItem(name)) {
  //    if (auto mesh = AssetManager::PickMeshAsset()) {
  //      registry.emplace<MeshComponent>(m_currentEntity, mesh->id);
  //      EventBus::Publish(ComponentAddedEvent{m_currentEntity, typeid(MeshComponent)});
  //    }
  //  }
  //};

  ////=== 组件菜单分类 ===//
  //if (ImGui::BeginMenu("渲染")) {
  //  drawUniqueComponent("网格过滤器", MeshFilterComponent{});
  //  drawAssetComponent("网格渲染器");
  //  drawUniqueComponent("粒子系统", ParticleComponent{});
  //  ImGui::EndMenu();
  //}

  //if (ImGui::BeginMenu("物理")) {
  //  drawUniqueComponent("刚体", RigidbodyComponent{});
  //  drawUniqueComponent("碰撞体", ColliderComponent{});
  //  ImGui::EndMenu();
  //}

  //if (ImGui::BeginMenu("脚本")) {
  //  drawMultiComponent("C++脚本", NativeScriptComponent{});
  //  drawMultiComponent("Lua脚本", LuaScriptComponent{});
  //  ImGui::EndMenu();
  //}

  //// 4. 自定义组件扩展点
  //if (ImGui::BeginMenu("自定义")) {
  //  if (ImGui::MenuItem("从模板添加...")) {
  //    ImGui::OpenPopup("ComponentTemplates");
  //  }

  //  if (ImGui::BeginPopup("ComponentTemplates")) {
  //    for (auto &[name, factory] : ComponentFactory::GetTemplates()) {
  //      if (ImGui::MenuItem(name.c_str())) {
  //        factory(registry, m_currentEntity);
  //      }
  //    }
  //    ImGui::EndPopup();
  //  }
  //  ImGui::EndMenu();
  //}
}

};
