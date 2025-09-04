#ifndef MITE_UI_SYSTEM_H
#define MITE_UI_SYSTEM_H

#include "ui_core/ui_backend.h"
#include "ui_core/ui_localization.h"
#include "ui_core/ui_style.h"
#include "headers/headers.h"

namespace mite {

// 前向声明
class UIWidget;
class UIPanel;

/**
 * @brief UI系统单例，管理全局UI状态和事件总线
 */
class UISystem : public std::enable_shared_from_this<UISystem> {
 public:
  /**
   * @brief 获取UI系统单例实例
   */
  static std::shared_ptr<UISystem> GetInstance();

  /**
   * @brief 初始化UI系统
   * @param backend UI后端实现
   * @return 是否初始化成功
   */
  bool Initialize(std::unique_ptr<UIBackend> backend);

  /**
   * @brief 关闭UI系统
   */
  void Shutdown();

  /**
   * @brief 开始UI帧
   */
  void BeginFrame();

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
   * @brief 渲染UI
   */
  void Render();

  /**
   * @brief 获取事件总线
   */
  EventBus &GetEventBus()
  {
    return m_EventBus;
  }

  /**
   * @brief 获取UI后端
   */
  UIBackend *GetBackend()
  {
    return m_Backend.get();
  }

  /**
   * @brief 获取样式管理器
   */
  UIStyle &GetStyle()
  {
    return m_Style;
  }

  /**
   * @brief 获取本地化管理器
   */
  UILocalization &GetLocalization()
  {
    return m_Localization;
  }

  /**
   * @brief 注册控件
   * @param widget 控件指针
   */
  void RegisterWidget(std::shared_ptr<UIWidget> widget);

  /**
   * @brief 注销控件
   * @param widgetId 控件ID
   */
  void UnregisterWidget(uint64_t widgetId);

  /**
   * @brief 注册面板
   * @param panel 面板指针
   */
  void RegisterPanel(std::shared_ptr<UIPanel> panel);

  /**
   * @brief 注销面板
   * @param panelId 面板ID
   */
  void UnregisterPanel(uint64_t panelId);

  /**
   * @brief 获取控件
   * @param widgetId 控件ID
   * @return 控件指针，如果不存在返回nullptr
   */
  std::shared_ptr<UIWidget> GetWidget(uint64_t widgetId) const;

  /**
   * @brief 获取面板
   * @param panelId 面板ID
   * @return 面板指针，如果不存在返回nullptr
   */
  std::shared_ptr<UIPanel> GetPanel(uint64_t panelId) const;

  /**
   * @brief 设置是否启用UI输入
   * @param enabled 是否启用
   */
  void SetInputEnabled(bool enabled)
  {
    m_InputEnabled = enabled;
  }

  /**
   * @brief 获取是否启用UI输入
   */
  bool IsInputEnabled() const
  {
    return m_InputEnabled;
  }

  /**
   * @brief 设置是否显示UI
   * @param visible 是否显示
   */
  void SetUIVisible(bool visible)
  {
    m_UIVisible = visible;
  }

  /**
   * @brief 获取是否显示UI
   */
  bool IsUIVisible() const
  {
    return m_UIVisible;
  }

  /**
   * @brief 获取UI系统是否已初始化
   */
  bool IsInitialized() const
  {
    return m_Initialized;
  }

  /**
   * @brief 获取帧统计信息
   */
  struct FrameStats {
    uint32_t widgetCount = 0;
    uint32_t panelCount = 0;
    uint32_t drawCalls = 0;
    float frameTime = 0.0f;
  };

  const FrameStats &GetFrameStats() const
  {
    return m_FrameStats;
  }

 private:
  UISystem();
  ~UISystem();

  // 禁止拷贝和移动
  UISystem(const UISystem &) = delete;
  UISystem &operator=(const UISystem &) = delete;
  UISystem(UISystem &&) = delete;
  UISystem &operator=(UISystem &&) = delete;

  // 事件处理函数
  void OnWidgetCreated(const WidgetCreatedEvent &event);
  void OnWidgetDestroyed(const WidgetDestroyedEvent &event);
  void OnPanelOpened(const PanelOpenedEvent &event);
  void OnPanelClosed(const PanelClosedEvent &event);

 private:
  static std::shared_ptr<UISystem> s_Instance;

  bool m_Initialized = false;
  bool m_InputEnabled = true;
  bool m_UIVisible = true;

  EventBus m_EventBus;
  std::unique_ptr<UIBackend> m_Backend;
  UIStyle m_Style;
  UILocalization m_Localization;

  std::unordered_map<uint64_t, std::shared_ptr<UIWidget>> m_Widgets;
  std::unordered_map<uint64_t, std::shared_ptr<UIPanel>> m_Panels;

  FrameStats m_FrameStats;

  // 事件订阅句柄
  EventBus::SubscriptionHandle m_WidgetCreatedHandle;
  EventBus::SubscriptionHandle m_WidgetDestroyedHandle;
  EventBus::SubscriptionHandle m_PanelOpenedHandle;
  EventBus::SubscriptionHandle m_PanelClosedHandle;
};

}  // namespace mite

#endif  // MITE_UI_SYSTEM_H
