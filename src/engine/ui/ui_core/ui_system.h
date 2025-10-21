#ifndef MITE_UI_SYSTEM_H
#define MITE_UI_SYSTEM_H

#include "input/input_manager.h"
#include "ui_backend.h"
#include "ui_event/ui_event.h"
#include "ui_event/ui_events_lifecycle.h"
#include "ui_localization.h"
#include "ui_style_manager.h"
#include "ui_panel/ui_panel.h"

namespace mite {
/**
 * @brief UI系统核心管理类
 * 负责管理UI系统的生命周期、事件处理、渲染集成等
 */
class UISystem {
 public:
  /**
   * @brief UI系统构造函数
   * @param renderer 渲染器依赖注入
   * @param window 窗口依赖注入
   */
  UISystem();
  ~UISystem() = default;

  // 禁用拷贝和移动
  UISystem(const UISystem &) = delete;
  UISystem(UISystem &&) = delete;
  UISystem &operator=(const UISystem &) = delete;
  UISystem &operator=(UISystem &&) = delete;

  // 使用窗口句柄初始化（如GLFWwindow）
  void Initialize(void *nativeWindow);
  void Shutdown();

  /**
   * @brief 更新UI系统
   * @param deltaTime 帧时间差
   */
  void Update(float deltaTime);

  /**
   * @brief 开始UI帧
   */
  void BeginFrame();

  /**
   * @brief 渲染UI
   */
  void Render();

  /**
   * @brief 结束UI帧
   */
  void EndFrame();

  /**
   * @brief 创建面板
   * @param name 面板名称
   * @return 面板指针
   */
  void RegisterPanel(std::shared_ptr<UIPanel>);

  /**
   * @brief 销毁面板
   * @param panelId 面板ID
   */
  void DestroyPanel(std::shared_ptr<UIPanel> panel);

 private:
  // 使用窗口句柄初始化后端
  bool InitializeBackend(void *nativeWindow);

  // 核心依赖
  std::unique_ptr<UIBackend> m_Backend;
  std::unique_ptr<UIStyleManager> m_StyleManager;

  // 管理对象
  std::unordered_set<std::shared_ptr<UIPanel>> m_Panels;

  // 事件订阅
  SubscriptionGroup m_EventSubscriptions;

  // 日志系统
  Logger m_Logger;
};
}  // namespace mite

#endif  // MITE_UI_SYSTEM_H
