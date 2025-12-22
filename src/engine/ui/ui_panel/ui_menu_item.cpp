#include "ui_menu_item.h"

#include <algorithm>

namespace mite {
// ==================== UIMenuItem 实现 ====================

UIMenuItem::UIMenuItem(const std::string &label, MenuItemType type,
                       bool selected, std::function<void(bool)> callback)
    : m_Type(type) {
  // 初始化渲染属性
  InitRenderProps();

  // 设置基础属性
  m_Props.translationKey = label;
  m_Props.selected = selected;
  m_Props.callback = callback;

  // 根据类型设置是否可勾选
  m_Props.isCheckable = (type == MenuItemType::CHECKBOX);
}

void UIMenuItem::InitRenderProps() {
  // 设置基础渲染属性
  m_Props.visible = true;
  m_Props.enabled = true;
  m_Props.hasSubmenu = (m_Type == MenuItemType::SUBMENU);
  m_Props.isSeparator = (m_Type == MenuItemType::SEPARATOR);
  m_Props.isCheckable = (m_Type == MenuItemType::CHECKBOX);
}

void UIMenuItem::SetSelected(bool selected) {
  // 只有可勾选的菜单项才能设置选中状态
  if (m_Props.isCheckable) {
    m_Props.selected = selected;
  }
}

void UIMenuItem::SetupSubmenuRenderCallback() {
  if (m_Type == MenuItemType::SUBMENU) {
    m_Props.submenuRenderCallback = [this]() {
      // 渲染所有子项
      for (auto &child : m_Children) {
        child->Render(UIRender::Get());
      }
    };
  }
}

void UIMenuItem::AddChild(std::shared_ptr<UIMenuItem> child) {
  if (child) {
    m_Children.push_back(child);
    // 添加子项后重新设置子菜单回调
    SetupSubmenuRenderCallback();
  }
}

void UIMenuItem::RemoveChild(const std::string &label) {
  auto it = std::remove_if(m_Children.begin(), m_Children.end(),
                           [&label](const std::shared_ptr<UIMenuItem> &child) {
                             return child->GetLabel() == label;
                           });
  m_Children.erase(it, m_Children.end());

  // 移除子项后重新设置子菜单回调
  SetupSubmenuRenderCallback();
}

std::shared_ptr<UIMenuItem> UIMenuItem::FindChild(
    const std::string &label) const {
  auto it = std::find_if(m_Children.begin(), m_Children.end(),
                         [&label](const std::shared_ptr<UIMenuItem> &child) {
                           return child->GetLabel() == label;
                         });

  return (it != m_Children.end()) ? *it : nullptr;
}

void UIMenuItem::Render(UIRender &renderer) {
  // 如果不可见，直接返回
  if (!m_Props.visible) {
    return;
  }

  // 确保子菜单回调已设置
  if (m_Type == MenuItemType::SUBMENU && !m_Props.submenuRenderCallback) {
    SetupSubmenuRenderCallback();
  }

  // 渲染菜单项
  renderer.RenderMenuItem(m_Props);
}

// ==================== UIMenuItemSubmenu 实现 ====================

UIMenuItemSubmenu::UIMenuItemSubmenu(const std::string &label)
    : UIMenuItem(label, MenuItemType::SUBMENU, false, nullptr) {}

void UIMenuItemSubmenu::InitRenderProps() {
  UIMenuItem::InitRenderProps();
  m_Props.hasSubmenu = true;
}

UIMenuItemSubmenu *UIMenuItemSubmenu::AddSubmenu(const std::string &label) {
  auto submenu = std::make_shared<UIMenuItemSubmenu>(label);
  AddChild(submenu);
  return static_cast<UIMenuItemSubmenu *>(submenu.get());
}

UIMenuItem *UIMenuItemSubmenu::AddItem(const std::string &label,
                                       std::function<void()> callback) {
  // 包装回调函数，将void()转换为void(bool)
  std::function<void(bool)> wrappedCallback = nullptr;
  if (callback) {
    wrappedCallback = [callback](bool) { callback(); };
  }

  auto item = std::make_shared<UIMenuItem>(label, MenuItemType::ITEM, false,
                                           wrappedCallback);
  AddChild(item);
  return item.get();
}

UIMenuItem *UIMenuItemSubmenu::AddCheckbox(const std::string &label,
                                           bool checked,
                                           std::function<void(bool)> callback) {
  auto item = std::make_shared<UIMenuItem>(label, MenuItemType::CHECKBOX,
                                           checked, callback);
  AddChild(item);
  return item.get();
}

void UIMenuItemSubmenu::AddSeparator() {
  auto separator =
      std::make_shared<UIMenuItem>("", MenuItemType::SEPARATOR, false, nullptr);
  AddChild(separator);
}
}  // namespace mite