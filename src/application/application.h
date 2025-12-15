#ifndef MITE_APPLICATION
#define MITE_APPLICATION

#include "asset_manager.h"
#include "filesystem/filesystem.h"
#include "glfw_window/glfw_window.h"
#include "input/input_manager.h"
#include "material_factory.h"
#include "render_core/render_pipeline.h"
#include "scene_core/scene_core.h"
#include "scene_graph.h"
#include "scene_view.h"
#include "ui_core/ui_system.h"

namespace mite {
// 前向声明
class SceneReloadCalling; 
class ModelLoadCalling;
    /**
 * @brief MiteApplication为应用程序统筹管理者
 * @note 负责引擎启动/关闭，模块初始化，DEMO场景构建，主循环管理
 * 
 * （暂时负责统筹，后续考虑拆分Editor和Runtime以分担Application职责）
 */
class MiteApplication {
 public:
  MiteApplication();
  ~MiteApplication();

  // 主循环控制
  void run();

  // 快照测试用函数（后续由Command系统接管）
  void SnapShotTest();

  // 场景管理
  void LoadScene(const std::string &filepath);
  void SaveScene(const std::string &filepath);
  void LoadDemoScene(int index);

  // 创建灯光、模型
  void CreatePointLight();
  void CreateDirectionalLight();
  void LoadModelToScene(const std::string &filepath);

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

  // 界面相关
  void CreateMenuBar();
  void RenderUI();

  // 事件处理
  void OnWindowResize(uint32_t width, uint32_t height);
  void OnWindowClose(WindowCloseEvent &e);
  void OnSceneReloadCalling(SceneReloadCalling &e);

 private:
  // 子系统
  // std::unique_ptr<InputManager> m_InputManager; // 单例
  // std::unique_ptr<AssetManager> m_AssetManager; // 单例

  std::unique_ptr<Window> m_Window;
  std::unique_ptr<RenderPipeline> m_Renderer;
  std::unique_ptr<SceneCore> m_SceneCore;
  std::unique_ptr<SceneGraph> m_SceneGraph;
  std::unique_ptr<SceneView> m_SceneView;
  std::unique_ptr<UISystem> m_UISystem;

  // 状态信息
  WindowConfig m_Config;
  bool m_ShouldClose = false;  // 主窗口应当关闭

  // 重加载场景的标志位，用于控制主循环
  std::atomic<bool> m_ShouldReloadScene{false};
  std::atomic<bool> m_IsReloading{false};
  int m_ReloadSceneIndex = 0;

  // 日志系统
  Logger m_Logger;
  // 订阅事件集合
  SubscriptionGroup m_EventSubscriptions;
};

/**
 * @brief Demo场景加载事件（呼叫型事件）
 */
class SceneReloadCalling : public Event {
 public:
  explicit SceneReloadCalling(int index) : m_Index(index) {}

  int GetSceneIndex() const { return m_Index; }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override { return new SceneReloadCalling(m_Index); }

 private:
  int m_Index;
};

/**
 * @brief 模型加载事件（呼叫型事件）
 */
class ModelLoadCalling : public Event {
 public:
  explicit ModelLoadCalling(){}
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override { return new ModelLoadCalling(); }
};

}  // namespace mite

#endif
