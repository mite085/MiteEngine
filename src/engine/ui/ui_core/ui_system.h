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
 * 单例模式确保全局唯一访问
 *
 * 使用示例：
 * 
 * // 初始化
 * UISystem::Instance().Init(window);

 * // 注册面板
 * auto viewportPanel = std::make_shared<ViewportPanel>();
 * UISystem::Instance().RegisterPanel("Viewport", viewportPanel);
 *
 * // 主循环中
 * while (running) {
 *     UISystem::Instance().BeginFrame();
 *
 *    // 更新逻辑...
 *    UISystem::Instance().Update(deltaTime);
 *
 *    // 渲染逻辑...
 *    UISystem::Instance().EndFrame();
 * }
 *
 * // 关闭时
 * UISystem::Instance().Shutdown();
 * 
 */
class UISystem {
 public:
  // 获取单例实例
  static UISystem &Instance();

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

  // 事件处理
  bool OnEvent(Event &event);

  // 面板可见性控制
  void SetPanelVisible(const std::string &name, bool visible);
  void TogglePanelVisible(const std::string &name);

 private:
  UISystem() = default;
  ~UISystem() = default;

  // 面板存储
  std::unordered_map<std::string, std::shared_ptr<UIPanel>> m_panels;
  std::vector<std::string> m_panelOrder;  // 维护渲染顺序

  // ImGui上下文
  ImGuiContext *m_imguiContext = nullptr;
  bool m_frameStarted = false;

  // 日志系统
  Logger m_logger;
};
};  // namespace mite

#endif
