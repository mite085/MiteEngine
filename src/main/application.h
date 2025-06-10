#ifndef MITE_APPLICATION
#define MITE_APPLICATION

#include "assert_manager.h"
#include "input.h"
#include "modular_input_context.h"
#include "material_system.h"
#include "opengl_renderer/opengl_renderer.h"
#include "scene_core.h"
#include "scene_system.h"
#include "scene_view.h"
#include "glfw_window/glfw_window.h"

// TODO: 确保编译通过，仅include panel组件，后续重新整理include层级
#include "property_panel_processor.h"

namespace mite {

// 当前帧统计信息
struct FrameStatistics {
  float fps = 0.0f;
  float frameTime = 0.0f;
  uint32_t drawCalls = 0;
  uint32_t entityCount = 0;
  size_t gpuMemoryUsage = 0;
};

// Editor状态
struct EditorState {
  bool showDemoWindow = false;
  bool showMetricsWindow = false;
  bool showSceneHierarchy = true;
  bool showPropertiesPanel = true;
  bool showMaterialEditor = false;
  bool showPreviewWindow = false;
  bool showDebugOverlay = true;
};

class MiteApplication {
 public:
  MiteApplication();
  ~MiteApplication();

  // 主循环控制
  void run();
  void close();

  // 场景管理
  void NewScene();
  void LoadScene(const std::string &filepath);
  void SaveScene(const std::string &filepath);

  // 获取子系统
  Window* GetWindow() const { return m_Window.get(); }
  Renderer* GetRenderer() const { return m_Renderer.get(); }
  Scene* GetScene() const { return m_Scene.get(); }
  //AssetManager* GetAssetManager() const { return m_AssetManager.get(); }
  //MaterialSystem* GetMaterialSystem() const { return m_MaterialSystem.get(); }


 private:
  // 初始化与清理
  void Initialize();
  void Cleanup();
  void InitializeInputSystem();
  void InitializeWindowWithOpenGL();
  void InitializeRenderWithOpenGL();
  void InitializeUI();
  void InitializeAssertManager();
  void InitializeMaterialSystem();
  void InitializeScene();
  void LoadDefaultScene();
  //void InitializeSubsystems();

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
  void OnEvent(Event &event);
  void OnWindowResize(uint32_t width, uint32_t height);
  bool OnWindowClose(WindowCloseEvent& e);

 private:
  // 子系统
  std::unique_ptr<Window> m_Window;
  std::unique_ptr<Renderer> m_Renderer;
  std::unique_ptr<Scene> m_Scene;
  std::unique_ptr<SceneView> m_SceneView;
  //std::unique_ptr<UIManager> m_UIManager;
  //std::unique_ptr<AssetManager> m_AssetManager;
  //std::unique_ptr<MaterialSystem> m_MaterialSystem;
  std::shared_ptr<InputContextStack> m_InputContextStack;

  // 状态信息
  WindowConfig m_Config;
  bool m_ShouldClose = false;
  // TODO: 以下状态信息暂未启用
  FrameStatistics m_FrameStats;
  EditorState m_EditorState;
  bool m_ShowMainViewport = true;
  bool m_ShowDebug = true;
  bool m_IsInitialized = false;
  float m_TargetFrameRate = 60.0f;

  // 待处理操作队列
  std::vector<std::function<void()>> m_PendingOperations;
  std::mutex m_PendingOperationsMutex;

  // 日志系统
  Logger m_logger;
};



}  // namespace mite

#endif
