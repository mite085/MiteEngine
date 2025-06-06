#include "application.h"

namespace mite {
MiteApplication::MiteApplication()
{
  // ��ʼ��LOGGER
  m_logger = mite::LoggerSystem::CreateModuleLogger("Mite Application");
  m_logger->info("Create logger for application");
}

MiteApplication::~MiteApplication() {}

void MiteApplication::run()
{
  Initialize();

  while (!m_ShouldClose) {
    // Timeϵͳ����ʱ��
    Time::Update();

    // 1���������¼�
    m_Window->PollEvents();

    // 2����������ϵͳ
    //m_InputSystem->Update();

    // 3����ʼ�µ�һ֡
    BeginFrame();

    // 4�����³���
    Update();

    // 5����Ⱦ����
    Render();

    // 6��������ǰ֡
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
  // TODO: Ŀǰ��ʵ��OpenGLģʽ��Ԥ�������ģʽ�ӿ�
  InitializeWithOpenGL();
}

void MiteApplication::InitializeWithOpenGL()
{ 
  m_logger->info("Initialize applicaton with OpenGL mode");

  // ��ʼ������
  m_Config = WindowConfig();
  m_Window = Window::Create();
  m_Window->Initialize(m_Config);
  m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

  // ��ʼ����Ⱦ��
  m_Renderer = std::make_unique<OpenGLRenderer>();
  m_Renderer->Init(GetWindow());

  // ��ʼ������ϵͳ
  m_Scene = std::make_unique<Scene>();
  m_SceneView = std::make_unique<SceneView>(*m_Scene.get());

  // TODO����ʼ��UI

  // TODO����ʼ���ʲ��Ͳ���ϵͳ

  // ����Ĭ�ϳ���
  LoadDefaultScene();
}

void MiteApplication::Cleanup() {
  m_logger->info("Cleaning up application");
  m_Window->Shutdown();
}

void MiteApplication::LoadDefaultScene()
{
  m_logger->info("Loading default scene");

  // TODO��Э����ģ�飬���س�ʼ����
  // m_AssetManager->LoadDefaultAssets();
  m_Scene->LoadDefaultScene();
  // m_MaterialSystem->CreateDefaultMaterials();

  // ���³�����ͼ
  m_SceneView->SyncFromSceneCore();
}

void MiteApplication::InitializeSubsystems() {}

void MiteApplication::BeginFrame()
{
  // �������
  m_Renderer->Clear();

  // TODO����ʼUI֡
  // m_UIManager->BeginFrame()

  // TODO�����³�����ͼ(��ECS����ת��Ϊ��Ⱦ�Ѻø�ʽ)
  m_SceneView->UpDate();

  // ����֡ͳ����Ϣ
  UpdateFrameStats();
}

void MiteApplication::Update()
{
  // ���³���״̬(ECSϵͳ����)
  m_Scene->UpDate();

  // TODO��������
  UpdateAnimations();

  // TODO������༭��״̬����
  UpdateEditorState();

  // TODO��������Դ���ض���
  // m_AssetManager->ProcessLoadingQueue();
}

void MiteApplication::Render()
{
  // ��������Ⱦ
  if (m_ShowMainViewport) {
    // ��Ⱦ����
    m_Renderer->RenderScene(m_SceneView->GetRenderData());

    // TODO����Ⱦ������Ϣ
    // if (m_ShowDebug) {
    //  m_Renderer->RenderDebug(m_SceneView->GetRenderData());
    //}
  }

  // UI��Ⱦ
  RenderUI();

  // TODO: Ԥ��������Ⱦ
  // if (m_ShowPreviewWindow) {
  //  RenderPreview();
  //}
}

void MiteApplication::EndFrame()
{
  // ��������
  m_Renderer->SwapBuffers();

  // TODO: �����ӳ��ͷŵ���Դ
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
  // �¼����ߣ��ɷַ��������¼��ַ�
  EventDispatcher dispatcher(event);

  // ��WindowCloseEvent�¼��ַ���Application::OnWindowClose����
  dispatcher.Dispatch<WindowCloseEvent>(BIND_DISPATCH_FN(OnWindowClose));


}

void MiteApplication::OnWindowResize(uint32_t width, uint32_t height) {}

bool MiteApplication::OnWindowClose(WindowCloseEvent& e) {
    m_logger->info("Window close event triggered.");
    m_ShouldClose = true;
    return true;
}
}  // namespace mite