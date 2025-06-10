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

// TODO: ȷ������ͨ������include panel�����������������include�㼶
#include "property_panel_processor.h"

namespace mite {

// ��ǰ֡ͳ����Ϣ
struct FrameStatistics {
  float fps = 0.0f;
  float frameTime = 0.0f;
  uint32_t drawCalls = 0;
  uint32_t entityCount = 0;
  size_t gpuMemoryUsage = 0;
};

// Editor״̬
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

  // ��ѭ������
  void run();
  void close();

  // ��������
  void NewScene();
  void LoadScene(const std::string &filepath);
  void SaveScene(const std::string &filepath);

  // ��ȡ��ϵͳ
  Window* GetWindow() const { return m_Window.get(); }
  Renderer* GetRenderer() const { return m_Renderer.get(); }
  Scene* GetScene() const { return m_Scene.get(); }
  //AssetManager* GetAssetManager() const { return m_AssetManager.get(); }
  //MaterialSystem* GetMaterialSystem() const { return m_MaterialSystem.get(); }


 private:
  // ��ʼ��������
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

  // ֡ѭ�����
  void BeginFrame();
  void Update();
  void Render();
  void EndFrame();
  void LimitFrameRate();
  void UpdateFrameStats();

  // ��Ⱦ���
  void RenderUI();
  void RenderSceneHierarchy();
  void RenderPropertiesPanel();
  void RenderViewport();
  void RenderPreviewWindow();
  void RenderMainMenu();

  // �������
  void UpdateEditorState();
  void UpdateAnimations();
  void HandlePendingOperations();

  // �¼�����
  void OnEvent(Event &event);
  void OnWindowResize(uint32_t width, uint32_t height);
  bool OnWindowClose(WindowCloseEvent& e);

 private:
  // ��ϵͳ
  std::unique_ptr<Window> m_Window;
  std::unique_ptr<Renderer> m_Renderer;
  std::unique_ptr<Scene> m_Scene;
  std::unique_ptr<SceneView> m_SceneView;
  //std::unique_ptr<UIManager> m_UIManager;
  //std::unique_ptr<AssetManager> m_AssetManager;
  //std::unique_ptr<MaterialSystem> m_MaterialSystem;
  std::shared_ptr<InputContextStack> m_InputContextStack;

  // ״̬��Ϣ
  WindowConfig m_Config;
  bool m_ShouldClose = false;
  // TODO: ����״̬��Ϣ��δ����
  FrameStatistics m_FrameStats;
  EditorState m_EditorState;
  bool m_ShowMainViewport = true;
  bool m_ShowDebug = true;
  bool m_IsInitialized = false;
  float m_TargetFrameRate = 60.0f;

  // �������������
  std::vector<std::function<void()>> m_PendingOperations;
  std::mutex m_PendingOperationsMutex;

  // ��־ϵͳ
  Logger m_logger;
};



}  // namespace mite

#endif
