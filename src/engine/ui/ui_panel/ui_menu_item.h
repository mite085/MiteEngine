#ifndef MITE_UI_MENU_ITEM_H
#define MITE_UI_MENU_ITEM_H
#include "ui_core/ui_render.h"
#include "ui_core/ui_render_props.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>
namespace mite {
/**
 * @brief 菜单项类型枚举
 */
enum class MenuItemType {
  NORMAL,     // 普通菜单项（叶子节点）
  CHECKBOX,   // 复选框菜单项（叶子节点）
  SEPARATOR,  // 分隔符
  SUBMENU     // 子菜单（枝干节点）
};
/**
 * @brief 菜单项基类
 * 支持树状结构的菜单项，可以包含任意深度的子项
 */
class UIMenuItem {
 public:
  /**
   * @brief 构造函数
   * @param label 菜单项标签（也是翻译键）
   * @param type 菜单项类型
   */
  UIMenuItem(const std::string &label, MenuItemType type = MenuItemType::NORMAL);
  virtual ~UIMenuItem() = default;

  // ==================== 基础属性访问 ====================

  /**
   * @brief 获取菜单项标签
   */
  const std::string &GetLabel() const { return m_Label; }

  /**
   * @brief 设置菜单项标签
   */
  void SetLabel(const std::string &label) { m_Label = label; }

  /**
   * @brief 获取菜单项类型
   */
  MenuItemType GetType() const { return m_Type; }

  /**
   * @brief 是否启用
   */
  bool IsEnabled() const { return m_Enabled; }

  /**
   * @brief 设置启用状态
   */
  void SetEnabled(bool enabled) { m_Enabled = enabled; }

  /**
   * @brief 是否可见
   */
  bool IsVisible() const { return m_Visible; }

  /**
   * @brief 设置可见状态
   */
  void SetVisible(bool visible) { m_Visible = visible; }

  /**
   * @brief 是否有快捷键
   */
  bool HasShortcut() const { return !m_Shortcut.empty(); }

  /**
   * @brief 获取快捷键文本
   */
  const std::string &GetShortcut() const { return m_Shortcut; }

  /**
   * @brief 设置快捷键文本
   */
  void SetShortcut(const std::string &shortcut) { m_Shortcut = shortcut; }

  // ==================== 子项管理 ====================

  /**
   * @brief 添加子菜单项
   * @param child 子菜单项
   */
  void AddChild(std::shared_ptr<UIMenuItem> child);

  /**
   * @brief 移除指定标签的子菜单项
   * @param label 要移除的子项标签
   */
  void RemoveChild(const std::string &label);

  /**
   * @brief 获取所有子项
   */
  const std::vector<std::shared_ptr<UIMenuItem>> &GetChildren() const { return m_Children; }

  /**
   * @brief 是否有子项
   */
  bool HasChildren() const { return !m_Children.empty(); }

  /**
   * @brief 查找子项
   * @param label 要查找的子项标签
   * @return 找到的子项指针，未找到返回nullptr
   */
  std::shared_ptr<UIMenuItem> FindChild(const std::string &label) const;

  // ==================== 回调管理 ====================

  /**
   * @brief 设置点击回调（仅叶子节点有效）
   * @param callback 回调函数
   */
  void SetCallback(std::function<void()> callback) { m_Callback = callback; }

  /**
   * @brief 获取点击回调
   */
  std::function<void()> GetCallback() const { return m_Callback; }

  // ==================== 复选框状态管理 ====================

  /**
   * @brief 是否被选中（仅CHECKBOX类型有效）
   */
  bool IsChecked() const { return m_Checked; }

  /**
   * @brief 设置选中状态（仅CHECKBOX类型有效）
   */
  void SetChecked(bool checked);

  /**
   * @brief 设置复选框状态改变回调
   */
  void SetCheckCallback(std::function<void(bool)> callback) { m_CheckCallback = callback; }

  // ==================== 渲染接口 ====================

  /**
   * @brief 渲染菜单项及其所有子项
   * @param renderer UI渲染器
   */
  virtual void Render(UIRender &renderer);

  /**
   * @brief 创建渲染属性
   * @return 菜单项渲染属性
   */
  virtual MenuItemProps CreateRenderProps() const;

 protected:
  std::string m_Label;     // 菜单项标签（也是翻译键）
  MenuItemType m_Type;     // 菜单项类型
  bool m_Enabled = true;   // 是否启用
  bool m_Visible = true;   // 是否可见
  std::string m_Shortcut;  // 快捷键文本

  // 子菜单相关
  std::vector<std::shared_ptr<UIMenuItem>> m_Children;  // 子菜单项

  // 事件回调
  std::function<void()> m_Callback;  // 点击回调（仅叶子节点有效）

  // 复选框相关（仅CHECKBOX类型有效）
  bool m_Checked = false;                     // 是否被选中
  std::function<void(bool)> m_CheckCallback;  // 复选框状态改变回调
};
/**
 * @brief 普通菜单项（叶子节点）
 */
class UIMenuItemNormal : public UIMenuItem {
 public:
  UIMenuItemNormal(const std::string &label, std::function<void()> callback = nullptr);

  void Render(UIRender &renderer) override;
};
/**
 * @brief 复选框菜单项（叶子节点）
 */
class UIMenuItemCheckbox : public UIMenuItem {
 public:
  UIMenuItemCheckbox(const std::string &label,
                     bool checked = false,
                     std::function<void(bool)> callback = nullptr);

  void Render(UIRender &renderer) override;
  MenuItemProps CreateRenderProps() const override;
};
/**
 * @brief 子菜单项（枝干节点）
 */
class UIMenuItemSubmenu : public UIMenuItem {
 public:
  UIMenuItemSubmenu(const std::string &label);

  void Render(UIRender &renderer) override;
  MenuItemProps CreateRenderProps() const override;

  // ==================== 便捷构建方法 ====================

  /**
   * @brief 添加子菜单（链式调用）
   * @param label 子菜单标签
   * @return 新创建的子菜单项指针
   */
  UIMenuItemSubmenu *AddSubmenu(const std::string &label);

  /**
   * @brief 添加普通菜单项（链式调用）
   * @param label 菜单项标签
   * @param callback 点击回调
   * @return 新创建的菜单项指针
   */
  UIMenuItemNormal *AddItem(const std::string &label, std::function<void()> callback = nullptr);

  /**
   * @brief 添加复选框菜单项（链式调用）
   * @param label 菜单项标签
   * @param checked 初始选中状态
   * @param callback 状态改变回调
   * @return 新创建的复选框菜单项指针
   */
  UIMenuItemCheckbox *AddCheckbox(const std::string &label,
                                  bool checked = false,
                                  std::function<void(bool)> callback = nullptr);

  /**
   * @brief 添加分隔符
   */
  void AddSeparator();
};
/**
 * @brief 菜单分隔符
 */
class UIMenuItemSeparator : public UIMenuItem {
 public:
  UIMenuItemSeparator();

  void Render(UIRender &renderer) override;
  MenuItemProps CreateRenderProps() const override;
};
}  // namespace mite
#endif  // MITE_UI_MENU_ITEM_H