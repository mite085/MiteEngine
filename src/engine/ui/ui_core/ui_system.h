#ifndef MITE_UI_SYSTEM_H
#define MITE_UI_SYSTEM_H

#include "input/input_manager.h"
#include "ui_backend.h"
#include "ui_event/ui_event.h"
#include "ui_event/ui_events_lifecycle.h"
#include "ui_localization.h"
#include "ui_widget/ui_panel.h"
#include "ui_style_manager.h"
#include "ui_widget/ui_widget.h"

namespace mite {

// 前向声明
class Renderer;
class Window;

/**
 * @brief UI系统核心管理类
 * 负责管理UI系统的生命周期、事件处理、渲染集成等
 */
class UISystem {
 public:
  // 禁用拷贝和移动
  UISystem(const UISystem &) = delete;
  UISystem(UISystem &&) = delete;
  UISystem &operator=(const UISystem &) = delete;
  UISystem &operator=(UISystem &&) = delete;

  /**
   * @brief 获取UI系统单例实例
   */
  static UISystem &Get();

  /**
   * @brief 初始化UI系统
   * @param renderer 渲染器实例
   * @param window 窗口实例
   * @return 是否初始化成功
   */
  bool Initialize(Renderer *renderer, Window *window);

  /**
   * @brief 关闭UI系统
   */
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
   * @brief 处理输入事件
   * @param event 输入事件
   */
  void ProcessInputEvent(const Event &event);

  /**
   * @brief 创建面板
   * @param name 面板名称
   * @return 面板指针
   */
  std::shared_ptr<UIPanel> CreatePanel(const std::string &name);

  /**
   * @brief 销毁面板
   * @param panelId 面板ID
   */
  void DestroyPanel(UUID panelId);

  /**
   * @brief 获取面板
   * @param panelId 面板ID
   * @return 面板指针
   */
  std::shared_ptr<UIPanel> GetPanel(UUID panelId) const;

  /**
   * @brief 显示/隐藏面板
   * @param panelId 面板ID
   * @param visible 是否可见
   */
  void SetPanelVisible(UUID panelId, bool visible);

  /**
   * @brief 获取样式管理器
   */
  UIStyleManager &GetStyleManager() const;

  /**
   * @brief 获取本地化管理器
   */
  UILocalization &GetLocalization() const;

  /**
   * @brief 获取UI是否可见
   */
  bool IsVisible() const;

  /**
   * @brief 设置UI可见性
   * @param visible 是否可见
   */
  void SetVisible(bool visible);

  /**
   * @brief 获取事件总线
   */
  EventBus &GetEventBus() const;

 private:
  UISystem();
  ~UISystem();

  // 初始化后端
  bool InitializeBackend();

  // 订阅事件
  void SubscribeEvents();

  // 处理语言变更事件
  void OnLanguageChanged(const LocalizationChangedEvent &event);

  // 处理样式变更事件
  void OnStyleChanged(const UIStyleChangedEvent &event);

 private:
  Logger m_Logger;
  bool m_Initialized;
  bool m_Visible;

  // 核心依赖
  Renderer *m_Renderer;
  Window *m_Window;
  std::unique_ptr<UIBackend> m_Backend;

  // 管理对象
  std::unordered_map<UUID, std::shared_ptr<UIPanel>> m_Panels;
  std::unique_ptr<EventBus> m_EventBus;
  SubscriptionGroup m_EventSubscriptions;

  // 管理器实例
  UIStyleManager *m_StyleManager;
  UILocalization *m_Localization;
};

}  // namespace mite

#endif  // MITE_UI_SYSTEM_H
