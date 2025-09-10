#ifndef MITE_UI_STYLE_MANAGER_H
#define MITE_UI_STYLE_MANAGER_H

#include "ui_style.h"

namespace mite {
/**
 * @brief UI样式管理器
 * 负责管理所有UI样式，提供样式的注册、获取、切换等功能
 * 采用单例模式确保全局唯一性
 * 
 * 使用示例：
 * // 初始化
 * manager.Initialize();
 * 
 * // 获取当前样式
 * auto currentStyle = manager.GetCurrentStyle();
 * 
 * // 切换主题
 * manager.SetCurrentStyle("dark");
 * 
 * // 监听样式变更事件
 * SubscriptionGroup m_EventSubscriptions;
 * m_EventSubscriptions.Subscribe<StyleChangedEvent>((BIND_DISPATCH_FN(onStyleChanged));
 */
class UIStyleManager {
 public:
  /**
   * @brief 私有构造函数
   * 确保只能通过Get()方法获取实例
   */
  UIStyleManager();

  // 禁用拷贝和移动构造函数
  UIStyleManager(const UIStyleManager &) = delete;
  UIStyleManager(UIStyleManager &&) = delete;
  UIStyleManager &operator=(const UIStyleManager &) = delete;
  UIStyleManager &operator=(UIStyleManager &&) = delete;

  /**
   * @brief 初始化样式管理器
   * 创建默认样式并设置为当前样式
   */
  void Initialize();

  /**
   * @brief 注册样式
   * @param name 样式名称
   * @param style 样式对象共享指针
   * @return bool 注册是否成功
   */
  bool RegisterStyle(const std::string &name, std::shared_ptr<UIStyle> style);

  /**
   * @brief 获取指定名称的样式
   * @param name 样式名称
   * @return std::shared_ptr<UIStyle> 样式对象共享指针，如果不存在返回nullptr
   */
  std::shared_ptr<UIStyle> GetStyle(const std::string &name) const;

  /**
   * @brief 获取当前使用的样式
   * @return std::shared_ptr<UIStyle> 当前样式对象共享指针
   */
  std::shared_ptr<UIStyle> GetCurrentStyle() const;

  /**
   * @brief 获取当前样式名称
   * @return std::string 当前样式名称
   */
  std::string GetCurrentStyleName() const;

  /**
   * @brief 设置当前样式
   * @param name 样式名称
   * @return bool 设置是否成功
   */
  bool SetCurrentStyle(const std::string &name);

  /**
   * @brief 检查样式是否存在
   * @param name 样式名称
   * @return bool 样式是否存在
   */
  bool HasStyle(const std::string &name) const;

  /**
   * @brief 获取所有已注册的样式名称
   * @return std::vector<std::string> 样式名称列表
   */
  std::vector<std::string> GetAllStyleNames() const;

  /**
   * @brief 获取样式数量
   * @return size_t 样式数量
   */
  size_t GetStyleCount() const;

  /**
   * @brief 创建暗色主题样式
   * @return std::shared_ptr<UIStyle> 暗色主题样式对象
   */
  static std::shared_ptr<UIStyle> CreateDarkTheme();

  /**
   * @brief 创建亮色主题样式
   * @return std::shared_ptr<UIStyle> 亮色主题样式对象
   */
  static std::shared_ptr<UIStyle> CreateLightTheme();

 private:

  /**
   * @brief 创建并注册内置样式
   */
  void CreateBuiltinStyles();

  Logger m_Logger;

  std::unordered_map<std::string, std::shared_ptr<UIStyle>> m_Styles;  // 样式存储映射表
  std::string m_CurrentStyleName;                                      // 当前样式名称
};



}  // namespace mite

#endif  // MITE_UI_STYLE_MANAGER_H
