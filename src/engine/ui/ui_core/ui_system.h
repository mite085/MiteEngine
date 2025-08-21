#ifndef MITE_UI_SYSTEM
#define MITE_UI_SYSTEM

#include "headers/headers.h"
#include "ui_panel.h"

// 前向声明
struct ImGuiContext;
struct GLFWwindow;

namespace mite {
/**
 * @brief UI系统核心管理类（简化版，使用传统虚函数）
 *
 * 管理所有UI面板的生命周期、渲染顺序和输入事件分发
 *
 * 使用示例：
 * 
 * // 初始化
 * m_UISystem.Init(window);

 * // 注册面板
 * auto viewportPanel = std::make_shared<ViewportPanel>();
 * m_UISystem.RegisterPanel("Viewport", viewportPanel);
 *
 * // 主循环中
 * while (running) {
 *     m_UISystem.BeginFrame();
 *
 *    // 更新逻辑...
 *    m_UISystem.Update(deltaTime);
 *
 *    // 渲染逻辑...
 *    m_UISystem.EndFrame();
 * }
 *
 * // 关闭时
 * m_UISystem.Shutdown();
 * 
 */
class UISystem {
 public:
  UISystem();
  ~UISystem();

  // 禁止拷贝和移动
  UISystem(const UISystem &) = delete;
  UISystem &operator=(const UISystem &) = delete;

  // 系统初始化/关闭
  void Init(GLFWwindow *window);
  void Shutdown();

  // 面板管理
  void RegisterPanel(const std::string &name, std::shared_ptr<UIPanel> panel);
  void UnregisterPanel(const std::string &name);
  std::shared_ptr<UIPanel> GetPanel(const std::string &name);

  // 主循环接口
  void BeginFrame();
  void Update(float deltaTime);
  void EndFrame();

  //// 事件处理
  //bool ProcessEvent(Event &event);

  // 面板可见性控制
  void SetPanelVisible(const std::string &name, bool visible);
  void TogglePanelVisible(const std::string &name);

 private:

  // 面板存储
  std::unordered_map<std::string, std::shared_ptr<UIPanel>> m_panels;
  std::vector<std::string> m_panelOrder;  // 维护渲染顺序

  // ImGui上下文
  ImGuiContext *m_imguiContext = nullptr;
  bool m_frameStarted = false;

  // 事件订阅ID，用于取消订阅EventBus
  //EventBus::HandlerID m_EventHandlerID;  

  // 日志系统
  Logger m_logger;
};
};  // namespace mite

#endif
