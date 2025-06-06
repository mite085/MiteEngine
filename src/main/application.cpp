#include "application.h"

namespace mite {
MiteApplication::MiteApplication()
{
  // 初始化LOGGER
  m_logger = mite::LoggerSystem::CreateModuleLogger("Mite Application");
  m_logger->info("Create logger for application");
}

MiteApplication::~MiteApplication() {}

void MiteApplication::run()
{
  Initialize();

  while (!m_ShouldClose) {
    // Time系统更新时间
    Time::Update();

    // 1、处理窗口事件
    m_Window->PollEvents();

    // 2、更新输入系统
    //m_InputSystem->Update();

    // 3、开始新的一帧
    BeginFrame();

    // 4、更新场景
    Update();

    // 5、渲染场景
    Render();

    // 6、结束当前帧
    EndFrame();
  }

  Cleanup();
}

void MiteApplication::close() {}

void MiteApplication::NewScene() {}

void MiteApplication::LoadScene(const std::string &filepath) {}

void MiteApplication::SaveScene(const std::string &filepath) {}

void MiteApplication::Initialize()
{
  // TODO: 目前仅实现OpenGL模式，预留添加新模式接口
  InitializeWithOpenGL();
}

void MiteApplication::InitializeWithOpenGL()
{ 
  m_logger->info("Initialize applicaton with OpenGL mode");

  // 初始化窗口
  m_Config = WindowConfig();
  m_Window = Window::Create();
  m_Window->Initialize(m_Config);
  m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

  // 初始化渲染器
  m_Renderer = std::make_unique<OpenGLRenderer>();
  m_Renderer->Init(GetWindow());

  // 初始化场景系统
  m_Scene = std::make_unique<Scene>();
  m_SceneView = std::make_unique<SceneView>(*m_Scene.get());

  // TODO：初始化UI

  // TODO：初始化资产和材质系统

  // 加载默认场景
  LoadDefaultScene();
}

void MiteApplication::Cleanup() {
  m_logger->info("Cleaning up application");
  m_Window->Shutdown();
}

void MiteApplication::LoadDefaultScene()
{
  m_logger->info("Loading default scene");

  // TODO：协调各模块，加载初始场景
  // m_AssetManager->LoadDefaultAssets();
  m_Scene->LoadDefaultScene();
  // m_MaterialSystem->CreateDefaultMaterials();

  // 更新场景视图
  m_SceneView->SyncFromSceneCore();
}

void MiteApplication::InitializeSubsystems() {}

void MiteApplication::BeginFrame()
{
  // 清除缓冲
  m_Renderer->Clear();

  // TODO：开始UI帧
  // m_UIManager->BeginFrame()

  // TODO：更新场景视图(将ECS数据转换为渲染友好格式)
  m_SceneView->UpDate();

  // 更新帧统计信息
  UpdateFrameStats();
}

void MiteApplication::Update()
{
  // 更新场景状态(ECS系统更新)
  m_Scene->UpDate();

  // TODO：处理动画
  UpdateAnimations();

  // TODO：处理编辑器状态更新
  UpdateEditorState();

  // TODO：处理资源加载队列
  // m_AssetManager->ProcessLoadingQueue();
}

void MiteApplication::Render()
{
  // 主场景渲染
  if (m_ShowMainViewport) {
    // 渲染场景
    m_Renderer->RenderScene(m_SceneView->GetRenderData());

    // TODO：渲染调试信息
    // if (m_ShowDebug) {
    //  m_Renderer->RenderDebug(m_SceneView->GetRenderData());
    //}
  }

  // UI渲染
  RenderUI();

  // TODO: 预览窗口渲染
  // if (m_ShowPreviewWindow) {
  //  RenderPreview();
  //}
}

void MiteApplication::EndFrame()
{
  // 交换缓冲
  m_Renderer->SwapBuffers();

  // TODO: 处理延迟释放的资源
  // m_AssetManager->ProcessDeletionQueue();
}

void MiteApplication::LimitFrameRate() {}

void MiteApplication::UpdateFrameStats() {}

void MiteApplication::RenderUI() {}

void MiteApplication::RenderSceneHierarchy() {}

void MiteApplication::RenderPropertiesPanel() {}

void MiteApplication::RenderViewport() {}

void MiteApplication::RenderPreviewWindow() {}

void MiteApplication::RenderMainMenu() {}

void MiteApplication::UpdateEditorState() {}

void MiteApplication::UpdateAnimations() {}

void MiteApplication::HandlePendingOperations() {}

void MiteApplication::OnEvent(Event &event)
{
  // 事件总线，由分发器进行事件分发
  EventDispatcher dispatcher(event);

  // 将WindowCloseEvent事件分发给Application::OnWindowClose函数
  dispatcher.Dispatch<WindowCloseEvent>(BIND_DISPATCH_FN(OnWindowClose));


}

void MiteApplication::OnWindowResize(uint32_t width, uint32_t height) {}

bool MiteApplication::OnWindowClose(WindowCloseEvent& e) {
    m_logger->info("Window close event triggered.");
    m_ShouldClose = true;
    return true;
}
}  // namespace mite