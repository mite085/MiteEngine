#include "ui_system.h"
#include "ui_imgui_backend/ui_imgui_backend.h"
#include "window.h"
#include "renderer.h"

namespace mite {

UISystem &UISystem::Get()
{
  static UISystem instance;
  return instance;
}

UISystem::UISystem()
    : m_Logger("UISystem"),
      m_Initialized(false),
      m_Visible(true),
      m_Renderer(nullptr),
      m_Window(nullptr),
      m_StyleManager(&UIStyleManager::Get()),
      m_Localization(&UILocalization::Get())
{
}

UISystem::~UISystem()
{
  if (m_Initialized) {
    Shutdown();
  }
}

bool UISystem::Initialize(Renderer *renderer, Window *window)
{
  if (m_Initialized) {
    m_Logger.Warn("UI系统已经初始化");
    return true;
  }

  m_Renderer = renderer;
  m_Window = window;

  // 初始化事件总线
  m_EventBus = std::make_unique<EventBus>();

  // 初始化后端
  if (!InitializeBackend()) {
    m_Logger.Error("UI后端初始化失败");
    return false;
  }

  // 订阅事件
  SubscribeEvents();

  m_Initialized = true;
  m_Logger.Info("UI系统初始化成功");

  // 发布初始化完成事件
  m_EventBus->Publish<UIInitializedEvent>(UIInitializedEvent());

  return true;
}

void UISystem::Shutdown()
{
  if (!m_Initialized) {
    return;
  }

  // 发布关闭事件
  m_EventBus->Publish<UIShutdownEvent>(UIShutdownEvent());

  // 清理面板
  m_Panels.clear();

  // 取消事件订阅
  m_EventSubscriptions.UnsubscribeAll();

  // 关闭后端
  if (m_Backend) {
    m_Backend->Shutdown();
    m_Backend.reset();
  }

  m_Initialized = false;
  m_Logger.Info("UI系统已关闭");
}

void UISystem::Update(float deltaTime)
{
  if (!m_Initialized || !m_Visible) {
    return;
  }

  // 更新所有面板
  for (auto &[id, panel] : m_Panels) {
    if (panel->IsVisible()) {
      panel->Update(deltaTime);
    }
  }
}

void UISystem::BeginFrame()
{
  if (!m_Initialized || !m_Visible) {
    return;
  }

  if (m_Backend) {
    m_Backend->BeginFrame();
  }
}

void UISystem::Render()
{
  if (!m_Initialized || !m_Visible) {
    return;
  }

  if (m_Backend) {
    m_Backend->Render();
  }

  // 渲染所有可见面板
  for (auto &[id, panel] : m_Panels) {
    if (panel->IsVisible()) {
      panel->Render();
    }
  }
}

void UISystem::EndFrame()
{
  if (!m_Initialized || !m_Visible) {
    return;
  }

  if (m_Backend) {
    m_Backend->EndFrame();
  }
}

void UISystem::ProcessInputEvent(const Event &event)
{
  if (!m_Initialized || !m_Visible) {
    return;
  }

  if (m_Backend) {
    // 转换为非const引用供后端处理
    Event &nonConstEvent = const_cast<Event &>(event);
    m_Backend->ProcessInputEvent(nonConstEvent);
  }
}

std::shared_ptr<UIPanel> UISystem::CreatePanel(const std::string &name)
{
  auto panel = std::make_shared<UIPanel>(name);
  m_Panels[panel->GetID()] = panel;

  // 发布面板创建事件
  m_EventBus->Publish<PanelOpenedEvent>(PanelOpenedEvent(panel->GetID(), name));

  return panel;
}

void UISystem::DestroyPanel(UUID panelId)
{
  auto it = m_Panels.find(panelId);
  if (it != m_Panels.end()) {
    // 发布面板关闭事件
    m_EventBus->Publish<PanelClosedEvent>(PanelClosedEvent(panelId, it->second->GetName()));
    m_Panels.erase(it);
  }
}

std::shared_ptr<UIPanel> UISystem::GetPanel(UUID panelId) const
{
  auto it = m_Panels.find(panelId);
  return it != m_Panels.end() ? it->second : nullptr;
}

void UISystem::SetPanelVisible(UUID panelId, bool visible)
{
  if (auto panel = GetPanel(panelId)) {
    panel->SetVisible(visible);
    // 发布可见性变更事件
    m_EventBus->Publish<UIVisibilityChangedEvent>(
        UIVisibilityChangedEvent(panelId, "Panel", visible));
  }
}

UIStyleManager &UISystem::GetStyleManager() const
{
  return *m_StyleManager;
}

UILocalization &UISystem::GetLocalization() const
{
  return *m_Localization;
}

bool UISystem::IsVisible() const
{
  return m_Visible;
}

void UISystem::SetVisible(bool visible)
{
  m_Visible = visible;
}

EventBus &UISystem::GetEventBus() const
{
  return *m_EventBus;
}

bool UISystem::InitializeBackend()
{
  // 目前只实现ImGui后端
  m_Backend = std::make_unique<UIImGuiBackend>();

  if (!m_Backend->Initialize()) {
    m_Logger.Error("ImGui后端初始化失败");
    return false;
  }

  // 设置显示尺寸
  if (m_Window) {
    m_Backend->SetDisplaySize(m_Window->GetWidth(), m_Window->GetHeight());
  }

  m_Logger.Info("UI后端初始化成功: {}", m_Backend->GetBackendName());
  return true;
}

void UISystem::SubscribeEvents()
{
  // 订阅语言变更事件
  m_EventSubscriptions.Subscribe<LocalizationChangedEvent>(
      [this](const LocalizationChangedEvent &event) { OnLanguageChanged(event); });

  // 订阅样式变更事件
  m_EventSubscriptions.Subscribe<UIStyleChangedEvent>(
      [this](const UIStyleChangedEvent &event) { OnStyleChanged(event); });
}

void UISystem::OnLanguageChanged(const LocalizationChangedEvent &event)
{
  m_Logger.Info("语言已切换至: {}", event.GetLanguageName());
  // 这里可以添加语言切换后的处理逻辑
}

void UISystem::OnStyleChanged(const UIStyleChangedEvent &event)
{
  m_Logger.Info("样式已切换至: {}", event.GetStyleName());
  // 这里可以添加样式切换后的处理逻辑
}

}  // namespace mite
