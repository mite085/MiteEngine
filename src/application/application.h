#ifndef MITE_APPLICATION
#define MITE_APPLICATION

#include "asset_manager.h"
#include "material_system.h"
#include "input/input.h"
#include "input/modular_input_context.h"
#include "opengl_renderer/opengl_renderer.h"
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
  void NewScene();
  void LoadScene(const std::string &filepath);
  void SaveScene(const std::string &filepath);
  void LoadDefaultScene();

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
  void InitializeSceneCore();
  void CleanUpSceneCore();
  void InitializeSceneGraph();
  void CleanUpSceneGraph();
  void InitializeSceneView();
  void CleanUpSceneView();

  // 帧循环相关
  void BeginFrame();
  void Update();
  void Render();
  void EndFrame();
  void LimitFrameRate();
  void UpdateFrameStats();

  // 渲染相关
  void RenderUI();
  void RenderSceneHierarchy();
  void RenderPropertiesPanel();
  void RenderViewport();
  void RenderPreviewWindow();
  void RenderMainMenu();

  // 更新相关
  void UpdateEditorState();
  void UpdateAnimations();
  void HandlePendingOperations();

  // 事件处理
  void OnWindowResize(uint32_t width, uint32_t height);
  void OnWindowClose(WindowCloseEvent& e);

 private:
  // 子系统
  std::shared_ptr<InputContextStack> m_InputContextStack;
  std::unique_ptr<AssetManager> m_AssetManager;

  std::unique_ptr<Window> m_Window;
  std::unique_ptr<Renderer> m_Renderer;
  std::unique_ptr<SceneCore> m_SceneCore;
  std::unique_ptr<SceneGraph> m_SceneGraph;
  std::unique_ptr<SceneView> m_SceneView;
  std::unique_ptr<UISystem> m_UISystem;
  std::unique_ptr<MaterialSystem> m_MaterialSystem;

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
