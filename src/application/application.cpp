#include "application.h"
#include "basic_shader/shader_binding_point_manager.h"
#include "light_data/point_light.h"
#include "material_templates/material_template_pure_color.h"
#include "render_opengl/opengl_pipeline.h"
#include "scene_core_components/component_headers.h"
#include "time/time.h"
#include "ui_panel/ui_property_panel.h"
#include "ui_panel/ui_scenetree_panel.h"
#include "ui_panel/ui_viewport_panel.h"

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

    // 1.1. 检查重加载标志
    if (m_ShouldReloadScene.load()) {
      // 重置标志位
      m_ShouldReloadScene.store(false);
      m_IsReloading.store(true);

      // 重置场景系统
      m_SceneCore->Clear();
      m_SceneGraph->Clear();
      LightManager::Get().ClearAllLights();
      m_SceneView->Initialize();

      // 加载新场景
      LoadDemoScene(m_ReloadSceneIndex);
      m_IsReloading.store(false);
    }

    // 1.2. 如果正在重加载，跳过渲染
    // （目前主循环负责场景重加载，所以此标志位无效）
    // （待后续将场景加载移到子线程，不阻塞主线程的话，可以考虑启用）
    if (m_IsReloading.load()) {
      continue;
    }

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

void MiteApplication::SnapShotTest()
{
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

void MiteApplication::LoadScene(const std::string &filepath) {}

void MiteApplication::SaveScene(const std::string &filepath) {}

void MiteApplication::LoadDemoScene(int index)
{
  if (index == 0) {
    m_Logger->info("Loading empty scene");

    CreatePointLight();
    CreateDirectionalLight();
  }
  else if (index == 1) {
    m_Logger->info("Loading tree demo scene");

    CreatePointLight();
    CreateDirectionalLight();

    // 加载Ground-Tree模型
    LoadModelToScene("models/ground.glb");
    LoadModelToScene("models/oak.glb");
  }
  else if (index == 2) {
    m_Logger->info("Loading car demo scene");

    CreatePointLight();
    CreateDirectionalLight();

    // 加载Car-Room模型
    LoadModelToScene("models/room.glb");
    LoadModelToScene("models/car.glb");
  }
}

void MiteApplication::CreatePointLight()
{
  // 创建点光源、实体与对应组件
  std::shared_ptr<Light> pointLight = LightManager::Get().CreateLight(LightType::POINT);
  Entity lightEntity = m_SceneCore->CreateEntity("Point Light");
  TransformComponent &lightTransformComponent =
      m_SceneCore->GetRegistry().AddComponent<TransformComponent>(lightEntity);
  LightComponent &lightComponent = m_SceneCore->GetRegistry().AddComponent<LightComponent>(
      lightEntity);
  lightComponent.SetLight(pointLight);
  BoundingVolumeComponent &lightBoundingVolumeComponent =
      m_SceneCore->GetRegistry().AddComponent<BoundingVolumeComponent>(lightEntity);

  // 摆放位置，设定强度
  lightTransformComponent.SetLocalTransform(
      [=](Transform &localtrans) { localtrans.Translate(glm::vec3(3.0f, 5.0f, 1.0f)); });
  lightComponent.SetIntensity(300);
}

void MiteApplication::CreateDirectionalLight()
{
  // 创建方向光、实体与对应组件
  std::shared_ptr<Light> directionalLight = LightManager::Get().CreateLight(
      LightType::DIRECTIONAL);
  Entity directionalLightEntity = m_SceneCore->CreateEntity("Directional Light");
  TransformComponent &directionalLightTransformComponent =
      m_SceneCore->GetRegistry().AddComponent<TransformComponent>(directionalLightEntity);
  LightComponent &directionalLightComponent =
      m_SceneCore->GetRegistry().AddComponent<LightComponent>(directionalLightEntity);
  directionalLightComponent.SetLight(directionalLight);
  BoundingVolumeComponent &directionalLightBoundingVolumeComponent =
      m_SceneCore->GetRegistry().AddComponent<BoundingVolumeComponent>(directionalLightEntity);

  // 摆放位置，设定强度
  directionalLightTransformComponent.SetLocalTransform([=](Transform &localtrans) {
    localtrans.Translate(glm::vec3(0.0f, 0.0f, 0.0f));
    localtrans.LookAt(glm::vec3(-1.0f, -1.0f, -1.0f));
  });
  directionalLightComponent.SetIntensity(20);
}

void MiteApplication::LoadModelToScene(const std::string &modelName)
{
  // 使用资产管理器加载模型（启用LOD，按照默认4层LOD参数生成）
  ModelAssetID plane_model_asset_id = AssetManager::Get().LoadGLTFModel(
      FileSystem::GetAssetPath(modelName).string(), true, true);
  std::shared_ptr<ModelAsset> modelAsset = AssetManager::Get().GetModel(plane_model_asset_id);

  // 获取材质列表
  std::vector<MaterialAssetID> modelMaterialIDs = modelAsset->materialRefs;
  std::vector<std::shared_ptr<MaterialInstance>> modelMaterials;
  for (const MaterialAssetID &id : modelMaterialIDs) {
    std::shared_ptr<MaterialInstance> instance = AssetManager::Get().GetMaterial(id)->instance;
    modelMaterials.push_back(instance);
  }
  Model model(modelAsset->handle, modelAsset->subMeshSection, modelMaterials);

  // 模型组件与实体创建
  Entity modelEntity = m_SceneCore->CreateEntity(modelName);
  // 创建变换组件
  TransformComponent &planeTransformComponent =
      m_SceneCore->GetRegistry().AddComponent<TransformComponent>(modelEntity);
  // 创建包围盒组件
  BoundingVolumeComponent &modelBoundingVolumeComponent =
      m_SceneCore->GetRegistry().AddComponent<BoundingVolumeComponent>(modelEntity);
  const std::pair<glm::vec3, glm::vec3> modelBoundingbox = model.GetBoundingBox();
  BoundingVolume modelBoundingVolume = BoundingVolume::CreateFromPoints(
      BoundingVolumeType::AABB, {modelBoundingbox.first, modelBoundingbox.second});
  modelBoundingVolumeComponent.SetVolume(modelBoundingVolume);

  // 网格体组件与实体创建
  for (size_t i = 0; i < model.GetSubMeshCount(); ++i) {
    // 1. 创建网格实体，以模型节点作为父节点
    Entity submeshEntity = m_SceneCore->CreateEntity(model.GetSubMesh(i).GetName(), modelEntity);

    // 2. 创建网格组件，设定组件数据
    MeshComponent &submeshMeshComponent = m_SceneCore->GetRegistry().AddComponent<MeshComponent>(
        submeshEntity);
    submeshMeshComponent.SetMesh(std::make_shared<Mesh>(model.GetSubMesh(i)));

    // 3. 获取材质
    std::shared_ptr<MaterialInstance> submeshMaterial = model.GetSubMaterial(
        model.GetSubMesh(i).GetMaterialIndex());

    // 4. 创建材质组件
    MaterialComponent &submeshMaterialComponent =
        m_SceneCore->GetRegistry().AddComponent<MaterialComponent>(submeshEntity);
    submeshMaterialComponent.SetMaterialInstance(submeshMaterial);

    // 5. 创建变换组件
    TransformComponent &submeshTransformComponent =
        m_SceneCore->GetRegistry().AddComponent<TransformComponent>(
            submeshEntity, model.GetSubMesh(i).GetTransform());

    // 6. 创建包围盒组件，使用Mesh的包围盒填充AABB包围盒数据
    BoundingVolumeComponent &submeshBoundingVolumeComponent =
        m_SceneCore->GetRegistry().AddComponent<BoundingVolumeComponent>(submeshEntity);
    const std::pair<glm::vec3, glm::vec3> submeshBoundingbox =
        model.GetSubMesh(i).GetBoundingBox();
    BoundingVolume submeshBoundingVolume = BoundingVolume::CreateFromPoints(
        BoundingVolumeType::AABB, {submeshBoundingbox.first, submeshBoundingbox.second});
    submeshBoundingVolumeComponent.SetVolume(submeshBoundingVolume);

    // 7. 创建可见性组件
    VisibilityComponent &submeshVisibilityComponent =
        m_SceneCore->GetRegistry().AddComponent<VisibilityComponent>(submeshEntity);
  }
}

void MiteApplication::Initialize()
{
  m_Logger->info("Initialize application");

  // 订阅窗口关闭事件
  m_EventSubscriptions.SubscribeImmediate<WindowCloseEvent>(
      BIND_DISPATCH_FN(OnWindowClose),
      EventPriority::Highest  // 最高优先级确保及时处理
  );

  // 订阅场景加载事件（延迟订阅确保当前帧正常绘制结束）
  m_EventSubscriptions.SubscribeDeferred<SceneReloadCalling>(
      BIND_DISPATCH_FN(OnSceneReloadCalling),
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
  InitializeLightSystem();       // Light模块初始化时同步创建LightSSBO，依赖Render绑定
  InitializeUI();                // 必须在Window创建GL上下文后执行

  // 加载默认场景
  LoadDemoScene(1);
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

  // OpenGL/GLFW/GLAD初始化之后，预分配UBO和SSBO绑定点资源
  BindingPointManager::Get().PreallocateCommonResources();
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
  std::shared_ptr<ViewportPanel> viewportPanel = std::make_shared<ViewportPanel>(*m_SceneView,
                                                                                 "viewport");
  std::shared_ptr<SceneTreePanel> scenetreePanel = std::make_shared<SceneTreePanel>(*m_SceneGraph,
                                                                                    "scenetree");
  std::shared_ptr<PropertyPanel> propertyPanel = std::make_shared<PropertyPanel>(
      m_SceneCore->GetRegistry(), "properties");

  // 注册面板到UI系统
  m_UISystem->RegisterPanel(viewportPanel);
  m_UISystem->RegisterPanel(scenetreePanel);
  m_UISystem->RegisterPanel(propertyPanel);

  // 创建菜单栏
  CreateMenuBar();
}

void MiteApplication::InitializeAssertManager()
{
  m_Logger->info("Initializing asset manager");

  // 初始化资产管理器
  // m_AssetManager = std::make_unique<AssetManager>();
}

void MiteApplication::InitializeSceneCore()
{
  m_Logger->info("Initializing scene core");

  // 初始化ECS场景核心
  m_SceneCore = std::make_unique<SceneCore>();
  m_SceneCore->InitializeComponentSystems();
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
  // m_InputManager = std::make_unique<InputManager>();
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
  m_SceneGraph->Clear();
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

// 测试用静态局部变量
static bool Sub0003Checked = true;

void MiteApplication::CreateMenuBar()
{
  // 获取菜单栏
  UIMenu &menu = m_UISystem->GetMenu();

  // 添加Scene菜单，控制场景
  UIMenuItemSubmenu *sceneMenu = menu.AddMenu("common.Scene");
  sceneMenu->AddItem("common.Empty_Scene", [&]() {
    SceneReloadCalling e(0);
    EventBus::Publish<SceneReloadCalling>(e);
  });
  sceneMenu->AddItem("common.Tree_Demo_Scene", [&]() {
    SceneReloadCalling e(1);
    EventBus::Publish<SceneReloadCalling>(e);
  });
  sceneMenu->AddItem("common.Car_Demo_Scene", [&]() {
    SceneReloadCalling e(2);
    EventBus::Publish<SceneReloadCalling>(e);
  });

  // 添加Layer菜单，控制显示内容
  UIMenuItemSubmenu *layerMenu = menu.AddMenu("common.Layer_Select");
  // 显示GBuffer的世界坐标和深度（深度作为Alpha通道）
  layerMenu->AddItem("common.GBuffer_WorldPosDepth", []() {
    DisplayTextureTypeChangedEvent e(RuntimeTextureType::GBuffer_WorldPosDepth);
    EventBus::Publish<DisplayTextureTypeChangedEvent>(e);
  });
  layerMenu->AddItem("common.GBuffer_NormalScale", []() {
    DisplayTextureTypeChangedEvent e(RuntimeTextureType::GBuffer_NormalScale);
    EventBus::Publish<DisplayTextureTypeChangedEvent>(e);
  });
  layerMenu->AddItem("common.Deferred_Lighting", []() {
    DisplayTextureTypeChangedEvent e(RuntimeTextureType::Lighting_Combined);
    EventBus::Publish<DisplayTextureTypeChangedEvent>(e);
  });
  layerMenu->AddItem("common.Forward_Rendering", []() {
    DisplayTextureTypeChangedEvent e(RuntimeTextureType::Forward_Transparent);
    EventBus::Publish<DisplayTextureTypeChangedEvent>(e);
  });
  layerMenu->AddItem("common.Blend_Result", []() {
    DisplayTextureTypeChangedEvent e(RuntimeTextureType::Forward_Blend);
    EventBus::Publish<DisplayTextureTypeChangedEvent>(e);
  });

  // 添加Test测试菜单，测试功能
  UIMenuItemSubmenu *testMenu = menu.AddMenu("Test");

  // Test -> Sub01 -> Sub001 (三级菜单)
  UIMenuItemSubmenu *newSubmenu = testMenu->AddSubmenu("Sub01");
  newSubmenu->AddItem("Sub001", []() {
    // 点击Sub001的响应
    LOG_INFO("Sub001 Menu Item Triggered");
  });

  // Test -> Sub02 (二级菜单)
  testMenu->AddItem("Sub02", []() {
    // 点击Sub02的响应
    LOG_INFO("Sub02 Menu Item Triggered");
  });

  // Test -> 分隔符
  testMenu->AddSeparator();

  // Test -> Sub03 -> Sub003 -> Sub0003 (四级菜单，复选框)
  UIMenuItemSubmenu *sub03Menu = testMenu->AddSubmenu("Sub03");
  UIMenuItemSubmenu *sub003Menu = sub03Menu->AddSubmenu("Sub003");
  sub003Menu->AddCheckbox("Sub0003", Sub0003Checked, [this](bool enabled) {
    // 点击Sub0003的响应
    LOG_INFO("Sub0003 Menu Item Triggered");
    // 反选Sub0003
    Sub0003Checked = enabled;
  });
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
void MiteApplication::OnSceneReloadCalling(SceneReloadCalling &e) {
  m_Logger->info("Scene Reload Calling.");

  m_ShouldReloadScene.store(true);
  m_ReloadSceneIndex = e.GetSceneIndex();

   // 标记事件已处理，阻断传播
  e.SetResult(EventResult::Consumed);
}
}  // namespace mite