#include "application.h"
#include "basic_shader/shader_binding_point_manager.h"
#include "light_data/point_light.h"
#include "material_templates/material_template_pure_color.h"
#include "render_opengl/opengl_pipeline.h"
#include "scene_core_components/component_headers.h"
#include "ui_panel/ui_viewport_panel.h"
#include "ui_panel/ui_scenetree_panel.h"
#include "ui_panel/ui_property_panel.h"
#include "time/time.h"

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
    // 0. Time系统更新时间
    Time::Update();

    // 1. 处理Deferred事件
    m_Window->PollEvents();
    EventBus::Get().ProcessQueue();

    // 2. 更新场景
    Update();

    // 3. 开始新的一帧
    m_Renderer->BeginFrame();

    // 4. 渲染场景
    Render();

    // 5. 结束当前帧
    m_Renderer->EndFrame();

    // 6. UI渲染
    RenderUI();

    // 7. 窗口负责交换缓冲
    m_Window->SwapBuffers();
  }

  CleanUp();
}

void MiteApplication::LoadScene(const std::string &filepath) {}

void MiteApplication::SaveScene(const std::string &filepath) {}

void MiteApplication::LoadDefaultScene()
{
  m_Logger->info("Loading default scene");

  // 创建灯光、实体与对应组件
  std::shared_ptr<Light> pointLight = LightManager::Get().CreateLight(LightType::POINT);
  Entity lightEntity = m_SceneCore->CreateEntity("pointLight");
  TransformComponent &lightTransformComponent =
      m_SceneCore->GetRegistry().AddComponent<TransformComponent>(lightEntity);
  LightComponent &lightComponent = m_SceneCore->GetRegistry().AddComponent<LightComponent>(
      lightEntity);
  lightComponent.SetLight(pointLight);
  BoundingVolumeComponent &lightBoundingVolumeComponent =
      m_SceneCore->GetRegistry().AddComponent<BoundingVolumeComponent>(lightEntity);
  BoundingVolume lightBoundingVolume = BoundingVolume::CreateFromPoints(BoundingVolumeType::None);


  // 加载模型（启用LOD，按照默认4层LOD参数生成）
  ModelAssetID plane_model_asset_id = m_AssetManager->LoadGLTFModel(
      FileSystem::GetAssetPath("models/monkey.glb").string(), true, true);
  Model planeModel(m_AssetManager->GetModel(plane_model_asset_id)->handle,
                   m_AssetManager->GetModel(plane_model_asset_id)->subMeshSection);

  // 网格体组件与实体创建
  for (size_t i = 0; i < planeModel.GetSubMeshCount(); ++i) {
    // 1. 创建网格实体
    Entity planeSubmesh = m_SceneCore->CreateEntity("axis" + std::to_string(i));

    // 2. 创建网格组件，设定组件数据
    MeshComponent &planeMeshComponent = m_SceneCore->GetRegistry().AddComponent<MeshComponent>(
        planeSubmesh);
    planeMeshComponent.SetMesh(std::make_shared<Mesh>(planeModel.GetSubMesh(i)));

    // 3. 创建材质实例（自发光）
    std::shared_ptr<MaterialInstance> planeMaterial = MaterialFactory::Get().CreateInstance(
        MaterialType::EMISSION);

    // 4. 创建材质组件
    MaterialComponent &planeMaterialComponent =
        m_SceneCore->GetRegistry().AddComponent<MaterialComponent>(planeSubmesh);
    planeMaterialComponent.SetMaterialInstance(planeMaterial);

    // 5. 创建变换组件
    TransformComponent &planeTransformComponent =
        m_SceneCore->GetRegistry().AddComponent<TransformComponent>(planeSubmesh);

    // 6. 创建包围盒组件，使用Mesh的包围盒填充AABB包围盒数据
    BoundingVolumeComponent &planeBoundingVolumeComponent =
        m_SceneCore->GetRegistry().AddComponent<BoundingVolumeComponent>(planeSubmesh);
    const std::pair<glm::vec3, glm::vec3> boundingbox = planeModel.GetSubMesh(i).GetBoundingBox();
    BoundingVolume planeBoundingVolume = BoundingVolume::CreateFromPoints(
        BoundingVolumeType::AABB, {boundingbox.first, boundingbox.second});
    planeBoundingVolumeComponent.SetVolume(planeBoundingVolume);
  }

  // ------------- 以下为快照系统使用流程测试专用代码，可删除 -------------

  // 获取相机变换
  TransformComponent &mainCameraTransform =
      m_SceneCore->GetRegistry().GetComponent<TransformComponent>(m_SceneView->GetCameraEntity());
  // 添加快照测试
  std::unique_ptr<ComponentSnapshot<Transform>> transformSnap =
      mainCameraTransform.CreateSnapshot();
  // 相机看向远处点，不再看向原点
  mainCameraTransform.SetLocalTransform(
      [](Transform &localtrans) { localtrans.LookAt(glm::vec3(110.0f, 120.0f, 120.0f)); });
  // 恢复快照
  transformSnap->Apply();
}

void MiteApplication::Initialize()
{
  m_Logger->info("Initialize application");

  // 订阅窗口关闭事件
  m_EventSubscriptions.SubscribeImmediate<WindowCloseEvent>(
      BIND_DISPATCH_FN(OnWindowClose),
      EventPriority::Highest  // 最高优先级确保及时处理
  );

  // 按照依赖关系，先初始化底层模块，后初始化顶层模块
  InitializeInputSystem();
  InitializeAssertManager();
  InitializeWindowWithOpenGL();
  InitializeMaterialSystem();    // Material模块初始化不涉及UBO创建和绑定，无依赖
  InitializeSceneCore();         // 无依赖
  InitializeSceneGraph();        // 依赖SceneCore
  InitializeSceneView();         // 依赖SceneCore和SceneGraph
  InitializeRenderWithOpenGL();  // 必须在Window创建GL上下文后执行 & 依赖SceneView
  InitializeLightSystem();  // Light模块初始化时同步创建LightSSBO，依赖Render绑定
  InitializeUI();           // 必须在Window创建GL上下文后执行

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
  CleanUpLightSystem();
  CleanUpRenderWithOpenGL();
  CleanUpSceneView();
  CleanUpSceneGraph();
  CleanUpSceneCore();
  CleanUpMaterialSystem();
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

  // 初始化OpenGL渲染管线
  m_Renderer = std::make_unique<OpenGLPipeline>();
  m_Renderer->Initialize();
}

void MiteApplication::InitializeUI()
{
  m_Logger->info("Initializing user interface");

  // 初始化UI系统，依赖Window
  m_UISystem = std::make_unique<UISystem>();
  m_UISystem->Initialize(m_Window->GetNativeWindow());

  // 创建ViewportPanel（必须在SceneView创建之后创建）
  std::shared_ptr<ViewportPanel> viewportPanel = std::make_shared<ViewportPanel>(*m_SceneView, "viewport");
  std::shared_ptr<SceneTreePanel> scenetreePanel = std::make_shared<SceneTreePanel>(*m_SceneGraph,
                                                                                    "scenetree");
  std::shared_ptr<PropertyPanel> propertyPanel = std::make_shared<PropertyPanel>(
      m_SceneCore->GetRegistry(), "properties");

  // 注册面板到UI系统
  m_UISystem->RegisterPanel(viewportPanel);
  m_UISystem->RegisterPanel(scenetreePanel);
  m_UISystem->RegisterPanel(propertyPanel);
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

  // 初始化ECS场景核心
  m_SceneCore = std::make_unique<SceneCore>();
  m_SceneCore->InitializeComponentSystems();

  // 创建并绑定主相机（该步骤必须在SceneView初始化之前执行，后续应当改成SceneView运行时初始化，多视口渲染时创建多个SceneView）
  Camera mainCamera;
  Entity mainCameraEntity = m_SceneCore->CreateEntity("main_camera");

  // 主相机的相机组件与投影参数设定
  CameraComponent &mainCameraComponent = m_SceneCore->GetRegistry().AddComponent<CameraComponent>(
      mainCameraEntity);

  // 主相机的变换组件
  TransformComponent &mainCameraTransform =
      m_SceneCore->GetRegistry().AddComponent<TransformComponent>(mainCameraEntity);
  // 主相机的可见性组件
  VisibilityComponent &mainCameraVisibility =
      m_SceneCore->GetRegistry().AddComponent<VisibilityComponent>(mainCameraEntity);
}

void MiteApplication::InitializeSceneView()
{
  m_Logger->info("Initializing SceneView");

  if (m_SceneCore && m_SceneGraph) {
    // 初始化场景视图（具备SceneCore和SceneView的依赖注入，必须在这两个初始化之后进行）
    m_SceneView = std::make_unique<SceneView>(*m_SceneCore, *m_SceneGraph);
    m_SceneView->Initialize();
  }
  else {
    m_Logger->error("Invalid SceneCore or SceneGraph, SceneView initialize FAILED!");
  }
}

void MiteApplication::InitializeMaterialSystem()
{
  m_Logger->info("Initializing material system");

  // 初始化材质系统
  MaterialFactory::Get().Initialize();
}

void MiteApplication::InitializeLightSystem()
{
  m_Logger->info("Initializing material system");

  // 初始化材质系统
  LightManager::Get().Initialize();
}

void MiteApplication::InitializeInputSystem()
{
  m_Logger->info("Initializing input system");

  // 创建并初始化输入系统（单例）
  //m_InputManager = std::make_unique<InputManager>();
  InputManager::Get().Init();
}

void MiteApplication::CleanUpInputSystem()
{
  InputManager::Get().Shutdown();
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

void MiteApplication::CleanUpLightSystem()
{
  m_Logger->info("Cleaning up light system");
  LightManager::Get().Destroy();
}

void MiteApplication::CleanUpSceneCore()
{
  m_SceneCore->Clear();
}

void MiteApplication::InitializeSceneGraph()
{
  m_Logger->info("Initializing scene graph");

  // 初始化场景图
  m_SceneGraph = std::make_unique<SceneGraph>();
  m_SceneGraph->Initialize();
}

void MiteApplication::CleanUpSceneGraph()
{
  m_Logger->info("Cleaning up scene graph");
  m_SceneGraph->CleanUp();
}

void MiteApplication::CleanUpSceneView() {}

void MiteApplication::Update()
{
  // 1. 更新场景状态(ECS系统更新)
  m_SceneCore->OnUpdate(Time::DeltaTime());

  // 2. SceneGraphSystem同步ECS状态到场景图
  //    - 处理实体创建/销毁
  //    - 同步变换数据
  m_SceneGraph->Update(m_SceneCore->GetRegistry());

  // 3. 更新场景视图(将ECS数据转换为渲染友好格式)
  m_SceneView->Update();
}

void MiteApplication::Render()
{
  // 获取渲染队列
  std::shared_ptr<RenderQueue> renderQueue = m_SceneView->GetRenderQueue();

  // 渲染器渲染场景
  m_Renderer->RenderScene(renderQueue, m_SceneView->GetCameraInstance());  // 渲染场景
}

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

void MiteApplication::OnWindowResize(uint32_t width, uint32_t height) {}

void MiteApplication::OnWindowClose(WindowCloseEvent &e)
{
  m_Logger->info("Window close event triggered.");
  m_ShouldClose = true;

  // 标记事件已处理，阻断传播
  e.SetResult(EventResult::Consumed);
}
}  // namespace mite