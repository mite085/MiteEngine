// ui_menu_item.cpp
#include "ui_menu_item.h"
#include <algorithm>

namespace mite {
// ==================== UIMenuItem 实现 ====================

UIMenuItem::UIMenuItem(const std::string &label, MenuItemType type) : m_Label(label), m_Type(type)
{
}

void UIMenuItem::AddChild(std::shared_ptr<UIMenuItem> child)
{
  if (child) {
    m_Children.push_back(child);
  }
}

void UIMenuItem::RemoveChild(const std::string &label)
{
  auto it = std::remove_if(
      m_Children.begin(), m_Children.end(), [&label](const std::shared_ptr<UIMenuItem> &child) {
        return child->GetLabel() == label;
      });
  m_Children.erase(it, m_Children.end());
}

std::shared_ptr<UIMenuItem> UIMenuItem::FindChild(const std::string &label) const
{
  auto it = std::find_if(
      m_Children.begin(), m_Children.end(), [&label](const std::shared_ptr<UIMenuItem> &child) {
        return child->GetLabel() == label;
      });

  return (it != m_Children.end()) ? *it : nullptr;
}

void UIMenuItem::SetChecked(bool checked)
{
  if (m_Type == MenuItemType::CHECKBOX) {
    m_Checked = checked;

    // 触发状态改变回调
    if (m_CheckCallback) {
      m_CheckCallback(checked);
    }
  }
}

void UIMenuItem::Render(UIRender &renderer)
{
  // 如果不可见，直接返回
  if (!m_Visible) {
    return;
  }

  // 创建渲染属性
  MenuItemProps props = CreateRenderProps();

  // 设置子菜单渲染回调（仅SUBMENU类型需要）
  if (m_Type == MenuItemType::SUBMENU) {
    props.submenuRenderCallback = [this, &renderer]() {
      // 渲染所有子项
      for (auto &child : m_Children) {
        child->Render(renderer);
      }
    };
  }

  // 渲染菜单项
  renderer.RenderMenuItem(props);
}

MenuItemProps UIMenuItem::CreateRenderProps() const
{
  MenuItemProps props;
  props.visible = m_Visible;
  props.enabled = m_Enabled;
  props.translationKey = m_Label;
  props.shortcut = m_Shortcut;
  props.hasSubmenu = (m_Type == MenuItemType::SUBMENU);
  props.isChecked = m_Checked;
  props.isSeparator = (m_Type == MenuItemType::SEPARATOR);
  props.callback = m_Callback;

  return props;
}

// ==================== UIMenuItemNormal 实现 ====================

UIMenuItemNormal::UIMenuItemNormal(const std::string &label, std::function<void()> callback)
    : UIMenuItem(label, MenuItemType::NORMAL)
{
  SetCallback(callback);
}

void UIMenuItemNormal::Render(UIRender &renderer)
{
  // 调用基类渲染
  UIMenuItem::Render(renderer);
}

// ==================== UIMenuItemCheckbox 实现 ====================

UIMenuItemCheckbox::UIMenuItemCheckbox(const std::string &label,
                                       bool checked,
                                       std::function<void(bool)> callback)
    : UIMenuItem(label, MenuItemType::CHECKBOX)
{
  SetChecked(checked);
  SetCheckCallback(callback);
}

void UIMenuItemCheckbox::Render(UIRender &renderer)
{
  // 调用基类渲染
  UIMenuItem::Render(renderer);
}

MenuItemProps UIMenuItemCheckbox::CreateRenderProps() const
{
  MenuItemProps props = UIMenuItem::CreateRenderProps();
  props.isChecked = IsChecked();
  return props;
}

// ==================== UIMenuItemSubmenu 实现 ====================

UIMenuItemSubmenu::UIMenuItemSubmenu(const std::string &label)
    : UIMenuItem(label, MenuItemType::SUBMENU)
{
}

void UIMenuItemSubmenu::Render(UIRender &renderer)
{
  // 调用基类渲染
  UIMenuItem::Render(renderer);
}

MenuItemProps UIMenuItemSubmenu::CreateRenderProps() const
{
  MenuItemProps props = UIMenuItem::CreateRenderProps();
  props.hasSubmenu = true;
  return props;
}

UIMenuItemSubmenu *UIMenuItemSubmenu::AddSubmenu(const std::string &label)
{
  auto submenu = std::make_shared<UIMenuItemSubmenu>(label);
  AddChild(submenu);
  return static_cast<UIMenuItemSubmenu *>(submenu.get());
}

UIMenuItemNormal *UIMenuItemSubmenu::AddItem(const std::string &label,
                                             std::function<void()> callback)
{
  auto item = std::make_shared<UIMenuItemNormal>(label, callback);
  AddChild(item);
  return static_cast<UIMenuItemNormal *>(item.get());
}

UIMenuItemCheckbox *UIMenuItemSubmenu::AddCheckbox(const std::string &label,
                                                   bool checked,
                                                   std::function<void(bool)> callback)
{
  auto checkbox = std::make_shared<UIMenuItemCheckbox>(label, checked, callback);
  AddChild(checkbox);
  return static_cast<UIMenuItemCheckbox *>(checkbox.get());
}

void UIMenuItemSubmenu::AddSeparator()
{
  auto separator = std::make_shared<UIMenuItemSeparator>();
  AddChild(separator);
}

// ==================== UIMenuItemSeparator 实现 ====================

UIMenuItemSeparator::UIMenuItemSeparator() : UIMenuItem("", MenuItemType::SEPARATOR)
{
  // 分隔符不需要标签
}

void UIMenuItemSeparator::Render(UIRender &renderer)
{
  // 调用基类渲染
  UIMenuItem::Render(renderer);
}

MenuItemProps UIMenuItemSeparator::CreateRenderProps() const
{
  MenuItemProps props = UIMenuItem::CreateRenderProps();
  props.isSeparator = true;
  return props;
}
}  // namespace mite