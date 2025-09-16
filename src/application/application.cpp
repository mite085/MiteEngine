#include "application.h"
#include "scene_core_components/component_headers.h"
#include "ui_viewport/ui_viewport_panel.h"

namespace mite {
MiteApplication::MiteApplication()
{
  // 初始化LOGGER
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Application");
  m_Logger->info("Create logger for application");
}

MiteApplication::~MiteApplication() {}

void MiteApplication::run()
{
  Initialize();

  while (!m_ShouldClose) {
    // Time系统更新时间
    Time::Update();

    // 1. 处理事件
    m_Window->PollEvents();
    EventBus::Get().ProcessQueue();

    // 2. 更新输入系统
    Input::Update();

    // 3. 开始新的一帧
    BeginFrame();

    // 4. 更新场景
    Update();

    // 5. 渲染场景
    Render();

    // 6. 结束当前帧
    EndFrame();

    // 7. UI渲染
    RenderUI();

    // 8. 窗口负责交换缓冲
    m_Window->SwapBuffers();
  }

  CleanUp();
}

void MiteApplication::NewScene() {}

void MiteApplication::LoadScene(const std::string &filepath) {}

void MiteApplication::SaveScene(const std::string &filepath) {}

void MiteApplication::LoadDefaultScene()
{
  m_Logger->info("Loading default scene");

  // 协调各模块，加载初始场景

  // 0. 创建并绑定主相机（该步骤必须在m_SceneCore->InitializeComponentSystems();之后执行)
  Camera mainCamera;

  // 投影参数
  mainCamera.SetPerspective(60.0f,  // FOV: 60度（便于计算）
                             1.0f,   // 宽高比: 1:1（正方形视口，简化计算）
                             1.0f,   // 近平面: 1m
                             20.0f   // 远平面: 20m（足够包含场景）
  );
  Entity mainCameraEntity = m_SceneCore->CreateEntity("main_camera");
  CameraComponent &mainCameraComponent =
      m_SceneCore->GetRegistry().AddComponent<CameraComponent>(
          mainCameraEntity, std::make_shared<Camera>(mainCamera));
  TransformComponent &mainCameraTransform =
      m_SceneCore->GetRegistry().AddComponent<TransformComponent>(mainCameraEntity);

  // 设定方便观看模型的角度（相机没有Parent，暂时将Local坐标当成World坐标使用）
  mainCameraTransform.SetLocalPosition(glm::vec3(0.0f, 3.0f, 5.0f));
  mainCameraTransform.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));

  m_SceneCore->SetMainCamera(mainCameraEntity);

  // 0. 创建ViewportPanel并设置FrameBuffer
  auto viewportPanel = std::make_shared<ViewportPanel>("viewport");
  viewportPanel->SetFrameBuffer(m_Renderer->GetViewportFrameBuffer());
  // 注册面板到UI系统
  m_UISystem->RegisterPanel(viewportPanel);

  // 1. 加载模型（启用LOD，按照默认4层LOD参数生成）
  AssetID plane_model_asset_id = m_AssetManager->LoadModel(
      FileSystem::GetAssetPath("models/plane.obj").string(), true, true);
  Model plane_model(m_AssetManager->GetModel(plane_model_asset_id)->handle);

  for (size_t i = 0; i < plane_model.GetSubMeshCount(); ++i) {
    // 2. 创建网格实体，挂载组件
    Entity plane_submesh = m_SceneCore->CreateEntity("plane_submesh");
    MeshComponent &plane_mesh_component = m_SceneCore->GetRegistry().AddComponent<MeshComponent>(
        plane_submesh, plane_model.GetSubMesh(i));

    // 3. 创建材质实例
    std::shared_ptr<MaterialInstance> plane_material =
        m_MaterialSystem->CreateInstanceWithOverrides<PureColorMaterialTemplate>(
            {{"u_Color", glm::vec3(1.0, 0.1, 0.1)}});

    // 4. 创建材质组件
    MaterialComponent &plane_material_component =
        m_SceneCore->GetRegistry().AddComponent<MaterialComponent>(plane_submesh, plane_material);

    // 5. 创建变换组件
    TransformComponent &plane_transform_component =
        m_SceneCore->GetRegistry().AddComponent<TransformComponent>(plane_submesh);

    // 6. 创建包围盒
    BoundingVolumeComponent &plane_bounding_volume_component =
        m_SceneCore->GetRegistry().AddComponent<BoundingVolumeComponent>(plane_submesh);
  }

  // 更新场景视图
  // m_SceneView->SyncFromSceneCore();
}

void MiteApplication::Initialize()
{
  m_Logger->info("Initialize application");

  // 订阅事件，并管理订阅句柄
  m_EventSubscriptions.Subscribe<WindowCloseEvent>(BIND_DISPATCH_FN(OnWindowClose));

  // 按照依赖关系，先初始化底层模块，后初始化顶层模块
  InitializeInputSystem();
  InitializeAssertManager();
  InitializeWindowWithOpenGL();
  InitializeRenderWithOpenGL();  // 必须在Window创建GL上下文后执行
  InitializeMaterialSystem();
  InitializeSceneCore();
  InitializeSceneGraph();  // 依赖SceneCore
  InitializeSceneView();   // 依赖SceneCore和SceneGraph
  InitializeUI();          // 必须在Window创建GL上下文后执行

  // 初始化组件系统（必须在SceneGraph的组件注册到SceneCore之后执行）
  m_SceneCore->InitializeComponentSystems();

  // 加载默认场景
  LoadDefaultScene();
}

void MiteApplication::CleanUp()
{
  m_Logger->info("Cleaning up application");

  // 取消事件订阅
  m_EventSubscriptions.UnsubscribeAll();

  // 关闭组件系统
  m_SceneCore->ShutdownComponentSystems();

  // 按照初始化的倒序，依次CleanUp
  CleanUpUI();
  CleanUpSceneView();
  CleanUpSceneGraph();
  CleanUpSceneCore();
  CleanUpMaterialSystem();
  CleanUpRenderWithOpenGL();
  CleanUpWindow();
  CleanUpAssertManager();
  CleanUpInputSystem();
}

void MiteApplication::InitializeWindowWithOpenGL()
{
  m_Logger->info("Initializing window with OpenGL mode");

  // 初始化OpenGL窗口
  m_Config = WindowConfig();
  m_Window = Window::Create();
  m_Window->Initialize(m_Config);
}

void MiteApplication::InitializeRenderWithOpenGL()
{
  m_Logger->info("Initializing renderer with OpenGL mode");

  // 初始化 OpenGL 设备
  IRenderDevice::SetCurrent(std::make_unique<OpenGLDevice>());

  // 初始化OpenGL渲染器
  m_Renderer = std::make_unique<OpenGLRenderer>();
  m_Renderer->Initialize();
}

void MiteApplication::InitializeUI()
{
  m_Logger->info("Initializing user interface");

  // 初始化UI系统，依赖Window
  m_UISystem = std::make_unique<UISystem>();
  m_UISystem->Initialize(m_Window->GetNativeWindow());
}

void MiteApplication::InitializeAssertManager()
{
  m_Logger->info("Initializing asset manager");

  // 初始化资产管理器
  m_AssetManager = std::make_unique<AssetManager>();
}

void MiteApplication::InitializeSceneCore()
{
  m_Logger->info("Initializing scene core");

  // 初始化场景核心
  m_SceneCore = std::make_unique<SceneCore>();
}

void MiteApplication::InitializeSceneView()
{
  m_Logger->info("Initializing scene view");

  // 初始化场景视图
  m_SceneView = std::make_unique<SceneView>();
}

void MiteApplication::InitializeMaterialSystem()
{
  m_Logger->info("Initializing material system");

  // 初始化材质系统
  m_MaterialSystem = std::make_unique<MaterialSystem>();
  m_MaterialSystem->Initialize();
}

void MiteApplication::InitializeInputSystem()
{
  m_Logger->info("Initializing input system");

  // 创建输入上下文栈ContextStack
  m_InputContextStack = std::make_shared<InputContextStack>();

  // 初始化输入系统,将输入上下文栈ContextStack注入到Manager
  Input::Init(m_InputContextStack);

  // 创建编辑器上下文
  // auto editorContext = std::make_shared<ModularInputContext>("Editor");

  //// TODO: 为编辑器上下文装配处理器，以PropertyPanelProcessor为例
  // std::shared_ptr<PropertyPanel> panel = std::make_shared<PropertyPanel>();
  // editorContext->AddProcessor(std::make_shared<PropertyPanelProcessor>(panel));

  // 将编辑器上下文推入全局栈
  // Input::PushContext(editorContext);
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

void MiteApplication::CleanUpUI()
{
  m_Logger->info("Cleaning up UI");

  m_UISystem->Shutdown();
}

void MiteApplication::CleanUpAssertManager() {}

void MiteApplication::CleanUpMaterialSystem() {}

void MiteApplication::CleanUpSceneCore()
{
  m_SceneCore->Clear();
}

void MiteApplication::InitializeSceneGraph()
{
  m_Logger->info("Initializing scene graph");

  // 初始化场景图
  m_SceneGraph = std::make_unique<SceneGraph>();

  // 在SceneCore内注册SceneGraphSystem
  m_SceneGraph->Initialize();
}

void MiteApplication::CleanUpSceneGraph()
{
  m_Logger->info("Cleaning up scene graph");
  m_SceneGraph->CleanUp();
}

void MiteApplication::CleanUpSceneView() {}

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
  // 1. 更新场景状态(ECS系统更新)
  m_SceneCore->OnUpdate(Time::DeltaTime());

  // 2. SceneGraphSystem同步ECS状态到场景图
  //    - 处理实体创建/销毁
  //    - 同步变换数据
  m_SceneGraph->Update(m_SceneCore->GetRegistry());

  // 3. 更新DirtySceneNode，由SceneGraph负责
  // 该步骤也由SceneGraphSystem负责了。

  // 4. VisibilityComponentSystem执行可见性计算
  //    - 使用SceneGraph的空间查询进行视锥体裁剪

  // TODO：处理动画
  UpdateAnimations();

  // TODO：处理编辑器状态更新
  UpdateEditorState();

  // TODO：处理资源加载队列
  // m_AssetManager->ProcessLoadingQueue();

  // TODO：更新场景视图(将ECS数据转换为渲染友好格式)
  // m_SceneView->Update();
}

void MiteApplication::Render()
{
  // 主场景渲染

  // 1. 获取主相机，构建视锥体
  Entity mainCamera = m_SceneCore->GetMainCamera();

  glm::mat4 cameraView =
      m_SceneCore->GetRegistry().GetComponent<TransformComponent>(mainCamera).CreateViewMatrix();
  glm::mat4 cameraProjection =
      m_SceneCore->GetRegistry().GetComponent<CameraComponent>(mainCamera).GetProjectionMatrix();
  uint32_t mainCameraVisibilityMask = m_SceneCore->GetRegistry().GetComponent<CameraComponent>(mainCamera).GetVisibilityMask();

  Frustum mainCameraFrustum(cameraProjection * cameraView);

  // 2. SceneGraph执行视锥体裁剪查询，获取可见节点列表
  std::vector<SceneNode *> visibleNodes = m_SceneGraph->FrustumCull( mainCameraFrustum, mainCameraVisibilityMask);

  // 3. SceneView根据可见节点列表构建RendererQueue
  m_SceneView->Update(m_SceneCore->GetRegistry(), visibleNodes);
  std::shared_ptr<RenderQueue> renderQueue = m_SceneView->GetRenderQueue();

  // 4. 渲染器渲染场景
  m_Renderer->RenderScene(renderQueue, cameraView, cameraProjection);  // 渲染场景

  // TODO：渲染调试信息
  // if (m_ShowDebug) {
  //  m_Renderer->RenderDebug(m_SceneView->GetRenderData());
  //}

  // TODO: 预览窗口渲染
  // if (m_ShowPreviewWindow) {
  //  RenderPreview();
  //}
}

void MiteApplication::EndFrame()
{
  m_Renderer->EndFrame();

  // TODO: 处理延迟释放的资源
  // m_AssetManager->ProcessDeletionQueue();
}

void MiteApplication::LimitFrameRate() {}

void MiteApplication::UpdateFrameStats() {}

void MiteApplication::RenderUI()
{
  // 开始UI帧
  m_UISystem->BeginFrame();

  // 更新UI逻辑（处理输入/动画等）
  m_UISystem->Update(Time::DeltaTime());

  // 渲染所有UI面板
  m_UISystem->Render();

  // 结束当前帧
  m_UISystem->EndFrame();
}

void MiteApplication::RenderSceneHierarchy() {}

void MiteApplication::RenderPropertiesPanel() {}

void MiteApplication::RenderViewport() {}

void MiteApplication::RenderPreviewWindow() {}

void MiteApplication::RenderMainMenu() {}

void MiteApplication::UpdateEditorState() {}

void MiteApplication::UpdateAnimations() {}

void MiteApplication::HandlePendingOperations() {}

void MiteApplication::OnWindowResize(uint32_t width, uint32_t height) {}

bool MiteApplication::OnWindowClose(WindowCloseEvent &e)
{
  m_Logger->info("Window close event triggered.");
  m_ShouldClose = true;

  // 标记事件已处理，阻断传播
  e.Handled();
  return e.handled;
}
}  // namespace mite