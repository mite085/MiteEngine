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

    // 1、处理事件
    m_Window->PollEvents();
    EventBus::Get().ProcessQueue();

    // 2、更新输入系统
    Input::Update();

    // 3、开始新的一帧
    BeginFrame();

    // 4、更新场景
    Update();

    // 5、渲染场景
    Render();

    // 6、结束当前帧
    EndFrame();
  }

  CleanUp();
}

void MiteApplication::NewScene() {}

void MiteApplication::LoadScene(const std::string &filepath) {}

void MiteApplication::SaveScene(const std::string &filepath) {}

void MiteApplication::Initialize()
{
  m_logger->info("Initialize application");

  // 订阅事件，并管理订阅句柄
  m_EventSubscriptions.Subscribe<WindowCloseEvent>(BIND_DISPATCH_FN(OnWindowClose));

  // 先初始化shared模块
  InitializeInputSystem();
  InitializeAssertManager();

  // 再初始化unique模块
  InitializeWindowWithOpenGL();
  InitializeRenderWithOpenGL();
  InitializeUI();
  InitializeMaterialSystem();
  InitializeScene();

  // 加载默认场景
  LoadDefaultScene();
}

void MiteApplication::InitializeWindowWithOpenGL()
{
  m_logger->info("Initializing window with OpenGL mode");

  // 初始化OpenGL窗口
  m_Config = WindowConfig();
  m_Window = Window::Create();
  m_Window->Initialize(m_Config);
}

void MiteApplication::InitializeRenderWithOpenGL()
{
  m_logger->info("Initializing renderer with OpenGL mode");

  // 初始化 OpenGL 设备
  IRenderDevice::SetCurrent(std::make_unique<OpenGLDevice>());

  // 初始化OpenGL渲染器
  m_Renderer = std::make_unique<OpenGLRenderer>();
  m_Renderer->Initialize();
}

void MiteApplication::InitializeUI()
{
  m_logger->info("Initializing user interface");

  // TODO：初始化UI
}

void MiteApplication::InitializeAssertManager()
{
  m_logger->info("Initializing asset manager");

  // 目前AssetManager并不包含需要Init的内容
  AssetManager::Get();
}

void MiteApplication::InitializeScene()
{
  m_logger->info("Initializing scene");

  // 初始化场景系统
  m_Scene = std::make_unique<Scene>();
  m_SceneView = std::make_unique<SceneView>(m_Scene->GetRegistry());
}

void MiteApplication::InitializeMaterialSystem()
{
  m_logger->info("Initializing material system");

  MaterialSystem::Initialize();
}

void MiteApplication::InitializeInputSystem()
{
  m_logger->info("Initializing input system");

  // 创建输入上下文栈ContextStack
  m_InputContextStack = std::make_shared<InputContextStack>();

  // 初始化输入系统,将输入上下文栈ContextStack注入到Manager
  Input::Init(m_InputContextStack);

  // 创建编辑器上下文
  auto editorContext = std::make_shared<ModularInputContext>("Editor");

  //// TODO: 为编辑器上下文装配处理器，以PropertyPanelProcessor为例
  // std::shared_ptr<PropertyPanel> panel = std::make_shared<PropertyPanel>();
  // editorContext->AddProcessor(std::make_shared<PropertyPanelProcessor>(panel));

  // 将编辑器上下文推入全局栈
  Input::PushContext(editorContext);
}

void MiteApplication::LoadDefaultScene()
{
  m_logger->info("Loading default scene");

  // 协调各模块，加载初始场景

  // 0. 创建模型Entity
  Entity plane = m_Scene->CreateEntity("plane");

  // 1. 加载模型
  AssetID plane_model_asset_id = AssetManager::Get().LoadModel(
      FileSystem::GetAssetPath("models/plane.obj").string());
  Model plane_model = Model(AssetManager::Get().GetModel(plane_model_asset_id)->handle);

  for (size_t i = 0; i < plane_model.GetSubMeshCount(); ++i) {
    // 2. 创建网格实体，挂载组件
    Entity plane_submesh = m_Scene->CreateEntity("plane_submesh");
    MeshComponent &plane_mesh_component = m_Scene->GetRegistry().AddComponent<MeshComponent>(
        plane_submesh);
    plane_mesh_component.SetMesh(plane_model.GetMeshes(i));

    // 3. 创建材质实例
    std::shared_ptr<MaterialInstance> plane_material = MaterialSystem::Get().CreateInstance(
        "BasicMaterial");
    // 4. 创建材质组件
    MaterialComponent &plane_material_component =
        m_Scene->GetRegistry().AddComponent<MaterialComponent>(plane_submesh);
    plane_material_component.SetMaterial(plane_material.get());

    auto material_ins =
        m_Scene->GetRegistry().GetComponent<MaterialComponent>(plane_submesh).GetMaterial();

    // 5. 创建变换组件
    TransformComponent &plane_transform_component =
        m_Scene->GetRegistry().AddComponent<TransformComponent>(plane_submesh);

    // 6. 推入渲染队列
    m_SceneView->AddToRenderQueue(plane_submesh);

  }

  
  // 更新场景视图
  // m_SceneView->SyncFromSceneCore();
}

void MiteApplication::CleanUp()
{
  m_logger->info("Cleaning up application");

  // 取消事件订阅
  m_EventSubscriptions.UnsubscribeAll();

  CleanUpInputSystem();
  CleanUpWindow();
  CleanUpRenderWithOpenGL();
  CleanUpUI();
  CleanUpAssertManager();
  CleanUpMaterialSystem();
  CleanUpScene();
}

void MiteApplication::CleanUpInputSystem()
{
  Input::Shutdown();
}

void MiteApplication::CleanUpWindow()
{
  m_Window->Shutdown();
}

void MiteApplication::CleanUpRenderWithOpenGL() {}

void MiteApplication::CleanUpUI() {}

void MiteApplication::CleanUpAssertManager() {}

void MiteApplication::CleanUpMaterialSystem() {}

void MiteApplication::CleanUpScene()
{
  m_Scene->Clear();
}

void MiteApplication::BeginFrame()
{
  // 清除缓冲
  m_Renderer->BeginFrame();

  // TODO：开始UI帧
  // m_UIManager->BeginFrame()

  // 更新帧统计信息
  UpdateFrameStats();
}

void MiteApplication::Update()
{
  // 更新场景状态(ECS系统更新)
  m_Scene->OnUpdate(Time::DeltaTime());

  // TODO：处理动画
  UpdateAnimations();

  // TODO：处理编辑器状态更新
  UpdateEditorState();

  // TODO：处理资源加载队列
  // m_AssetManager->ProcessLoadingQueue();

  // TODO：更新场景视图(将ECS数据转换为渲染友好格式)
  m_SceneView->Update();
}

void MiteApplication::Render()
{
  // 主场景渲染
  if (m_ShowMainViewport) {
    // 渲染场景
    m_Renderer->RenderScene(m_SceneView->GetRenderQueue());

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
  m_Renderer->EndFrame();

  // TODO: 窗口负责交换缓冲
  m_Window->SwapBuffers();

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

void MiteApplication::OnWindowResize(uint32_t width, uint32_t height) {}

void MiteApplication::OnWindowClose(WindowCloseEvent &e)
{
  m_logger->info("Window close event triggered.");
  m_ShouldClose = true;

  // 标记事件已处理，阻断传播
  e.Handled();
}
}  // namespace mite