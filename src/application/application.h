#ifndef MITE_APPLICATION
#define MITE_APPLICATION

#include "filesystem/filesystem.h"
#include "asset_manager.h"
#include "material_factory.h"
#include "input/input_manager.h"
#include "render_core/render_pipeline.h"
#include "scene_core/scene_core.h"
#include "scene_graph.h"
#include "scene_view.h"
#include "glfw_window/glfw_window.h"
#include "ui_core/ui_system.h"

namespace mite {

class MiteApplication {
 public:
  MiteApplication();
  ~MiteApplication();

  // 主循环控制
  void run();

  // 场景管理
  void LoadScene(const std::string &filepath);
  void SaveScene(const std::string &filepath);
  void LoadDefaultScene();
  void LoadModelToScene(const std::string &filepath);

  // 获取子系统
  //Window* GetWindow() const { return m_Window.get(); }
  //Renderer* GetRenderer() const { return m_Renderer.get(); }
  //SceneCore* GetScene() const { return m_SceneCore.get(); }
  //MaterialSystem* GetMaterialSystem() const { return m_MaterialSystem.get(); }


 private:
  // 初始化与清理
  void Initialize();
  void CleanUp();

  void InitializeInputSystem();
  void CleanUpInputSystem();
  void InitializeWindowWithOpenGL();
  void CleanUpWindow();
  void InitializeRenderWithOpenGL();
  void CleanUpRenderWithOpenGL();
  void InitializeUI();
  void CleanUpUI();
  void InitializeAssertManager();
  void CleanUpAssertManager();
  void InitializeMaterialSystem();
  void CleanUpMaterialSystem();
  void InitializeLightSystem();
  void CleanUpLightSystem();
  void InitializeSceneCore();
  void CleanUpSceneCore();
  void InitializeSceneGraph();
  void CleanUpSceneGraph();
  void InitializeSceneView();
  void CleanUpSceneView();

  // 帧循环相关
  void Update();
  void Render();

  // 渲染相关
  void RenderUI();

  // 事件处理
  void OnWindowResize(uint32_t width, uint32_t height);
  void OnWindowClose(WindowCloseEvent& e);

 private:
  // 子系统
  //std::unique_ptr<InputManager> m_InputManager; // 单例
  //std::unique_ptr<AssetManager> m_AssetManager; // 单例

  std::unique_ptr<Window> m_Window;
  std::unique_ptr<RenderPipeline> m_Renderer;
  std::unique_ptr<SceneCore> m_SceneCore;
  std::unique_ptr<SceneGraph> m_SceneGraph;
  std::unique_ptr<SceneView> m_SceneView;
  std::unique_ptr<UISystem> m_UISystem;

  // 状态信息
  WindowConfig m_Config;
  bool m_ShouldClose = false;

  // 日志系统
  Logger m_Logger;
  // 订阅事件集合
  SubscriptionGroup m_EventSubscriptions;
};



}  // namespace mite

#endif
