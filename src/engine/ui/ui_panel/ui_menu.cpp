#include "ui_menu.h"
#include "ui_core/ui_render.h"
#include <algorithm>
#include <sstream>

namespace mite {

UIMenuItemSubmenu *UIMenu::AddMenu(const std::string &label)
{
  auto menu = std::make_shared<UIMenuItemSubmenu>(label);
  m_TopLevelMenus.push_back(menu);
  return menu.get();
}

UIMenuItemSubmenu *UIMenu::FindMenu(const std::string &label)
{
  auto it = std::find_if(m_TopLevelMenus.begin(),
                         m_TopLevelMenus.end(),
                         [&label](const std::shared_ptr<UIMenuItemSubmenu> &menu) {
                           return menu->GetLabel() == label;
                         });

  return (it != m_TopLevelMenus.end()) ? it->get() : nullptr;
}

void UIMenu::RemoveMenu(const std::string &label)
{
  auto it = std::remove_if(m_TopLevelMenus.begin(),
                           m_TopLevelMenus.end(),
                           [&label](const std::shared_ptr<UIMenuItemSubmenu> &menu) {
                             return menu->GetLabel() == label;
                           });
  m_TopLevelMenus.erase(it, m_TopLevelMenus.end());
}

std::shared_ptr<UIMenuItem> UIMenu::FindMenuItem(const std::string &path)
{
  // 解析路径
  std::vector<std::string> pathComponents = SplitPath(path);
  if (pathComponents.empty()) {
    return nullptr;
  }

  // 首先查找顶级菜单
  std::shared_ptr<UIMenuItem> currentItem = nullptr;
  for (auto &menu : m_TopLevelMenus) {
    if (menu->GetLabel() == pathComponents[0]) {
      currentItem = menu;
      break;
    }
  }

  if (!currentItem) {
    return nullptr;
  }

  // 递归查找子项
  return FindMenuItemRecursive(currentItem, pathComponents, 1);
}

void UIMenu::SetMenuEnabled(const std::string &menuLabel, bool enabled)
{
  auto menu = FindMenu(menuLabel);
  if (menu) {
    menu->SetEnabled(enabled);
  }
}

void UIMenu::SetMenuItemEnabled(const std::string &path, bool enabled)
{
  auto item = FindMenuItem(path);
  if (item) {
    item->SetEnabled(enabled);
  }
}

void UIMenu::SetMenuItemVisible(const std::string &path, bool visible)
{
  auto item = FindMenuItem(path);
  if (item) {
    item->SetVisible(visible);
  }
}

void UIMenu::SetMenuItemChecked(const std::string &path, bool checked)
{
  auto item = FindMenuItem(path);
  if (item && item->GetType() == MenuItemType::CHECKBOX) {
    item->SetChecked(checked);
  }
}

void UIMenu::Render()
{
  // 获取渲染器
  UIRender &renderer = UIRender::Get();

  // 渲染所有顶级菜单
  for (auto &menu : m_TopLevelMenus) {
    if (menu->IsVisible()) {
      menu->Render(renderer);
    }
  }
}
// 添加新的RenderMenuBar方法
void UIMenu::RenderMenuBar()
{
  // 获取渲染器
  UIRender &renderer = UIRender::Get();

  // 创建菜单栏属性
  MenuBarProps props;
  props.visible = true;

  // 开始菜单栏渲染
  if (renderer.BeginMenuBar(props)) {
    // 渲染菜单内容
    Render();

    // 结束菜单栏渲染
    renderer.EndMenuBar();
  }
}

std::vector<std::string> UIMenu::SplitPath(const std::string &path)
{
  std::vector<std::string> components;
  std::stringstream ss(path);
  std::string component;

  while (std::getline(ss, component, '/')) {
    if (!component.empty()) {
      components.push_back(component);
    }
  }

  return components;
}

std::shared_ptr<UIMenuItem> UIMenu::FindMenuItemRecursive(
    std::shared_ptr<UIMenuItem> current,
    const std::vector<std::string> &pathComponents,
    size_t currentIndex)
{
  // 如果已经到达路径末尾，返回当前项
  if (currentIndex >= pathComponents.size()) {
    return current;
  }

  // 查找下一级子项
  std::string targetLabel = pathComponents[currentIndex];
  auto child = current->FindChild(targetLabel);

  if (!child) {
    return nullptr;
  }

  // 递归查找
  return FindMenuItemRecursive(child, pathComponents, currentIndex + 1);
}

}  // namespace mite
