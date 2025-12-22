#ifndef MITE_UI_MENU_ITEM_H
#define MITE_UI_MENU_ITEM_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "ui_core/ui_render.h"
#include "ui_core/ui_render_props.h"

namespace mite {
enum class MenuItemType {
  ITEM,       // 普通菜单项
  CHECKBOX,   // 复选框菜单项
  SEPARATOR,  // 分隔符
  SUBMENU     // 子菜单（枝干节点）
};

/**
 * @brief 菜单项基类
 */
class UIMenuItem {
 public:
  UIMenuItem(const std::string &label, MenuItemType type = MenuItemType::ITEM,
             bool selected = false,
             std::function<void(bool)> callback = nullptr);

  virtual ~UIMenuItem() = default;

  // ==================== 基础属性访问 ====================

  const std::string &GetLabel() const { return m_Props.translationKey; }
  void SetLabel(const std::string &label) { m_Props.translationKey = label; }

  MenuItemType GetType() const { return m_Type; }

  bool IsEnabled() const { return m_Props.enabled; }
  void SetEnabled(bool enabled) { m_Props.enabled = enabled; }

  bool IsVisible() const { return m_Props.visible; }
  void SetVisible(bool visible) { m_Props.visible = visible; }

  bool IsSelected() const { return m_Props.selected; }
  void SetSelected(bool selected);

  bool IsCheckable() const { return m_Props.isCheckable; }

  // ==================== 子项管理 ====================

  void AddChild(std::shared_ptr<UIMenuItem> child);
  void RemoveChild(const std::string &label);
  const std::vector<std::shared_ptr<UIMenuItem>> &GetChildren() const {
    return m_Children;
  }
  bool HasChildren() const { return !m_Children.empty(); }
  std::shared_ptr<UIMenuItem> FindChild(const std::string &label) const;

  // ==================== 回调管理 ====================

  void SetCallback(std::function<void(bool)> callback) {
    m_Props.callback = callback;
  }
  std::function<void(bool)> GetCallback() const { return m_Props.callback; }

  // ==================== 渲染接口 ====================

  virtual void Render(UIRender &renderer);

  // 获取渲染属性的引用
  MenuItemProps &GetRenderProps() { return m_Props; }
  const MenuItemProps &GetRenderProps() const { return m_Props; }

 protected:
  // 初始化渲染属性
  virtual void InitRenderProps();

  // 设置子菜单渲染回调
  void SetupSubmenuRenderCallback();

 protected:
  MenuItemType m_Type;                                  // 菜单项类型
  MenuItemProps m_Props;                                // 渲染属性
  std::vector<std::shared_ptr<UIMenuItem>> m_Children;  // 子菜单项
};

/**
 * @brief 子菜单项（枝干节点）
 */
class UIMenuItemSubmenu : public UIMenuItem {
 public:
  UIMenuItemSubmenu(const std::string &label);

  // ==================== 便捷构建方法 ====================

  UIMenuItemSubmenu *AddSubmenu(const std::string &label);

  // 添加普通菜单项
  UIMenuItem *AddItem(const std::string &label,
                      std::function<void()> callback = nullptr);

  // 添加复选框菜单项
  UIMenuItem *AddCheckbox(const std::string &label, bool checked = false,
                          std::function<void(bool)> callback = nullptr);

  void AddSeparator();

 protected:
  void InitRenderProps() override;
};
}  // namespace mite

#endif  // MITE_UI_MENU_ITEM_H
