#ifndef MITE_UI_MENU_H
#define MITE_UI_MENU_H

#include <memory>
#include <unordered_map>
#include <vector>

#include "ui_menu_item.h"

namespace mite {
/**
 * @brief 菜单管理器 - 支持树状结构的菜单系统
 *
 * 设计原则：
 * 1. 支持任意深度的树状结构
 * 2. 枝干节点（SUBMENU）只负责展开子项，不响应点击
 * 3. 叶子节点（ITEM）可以显示为普通项或复选框
 * 4. 保持简单，避免过度设计
 */
class UIMenu {
 public:
  UIMenu() = default;
  ~UIMenu() = default;

  // ==================== 菜单构建接口 ====================

  /**
   * @brief 添加顶级菜单
   * @param label 菜单标签（也是翻译键）
   * @return 新创建的顶级菜单指针
   */
  UIMenuItemSubmenu *AddMenu(const std::string &label);

  /**
   * @brief 查找顶级菜单
   * @param label 菜单标签
   * @return 找到的菜单指针，未找到返回nullptr
   */
  UIMenuItemSubmenu *FindMenu(const std::string &label);

  /**
   * @brief 移除顶级菜单
   * @param label 要移除的菜单标签
   */
  void RemoveMenu(const std::string &label);

  /**
   * @brief 通过路径查找菜单项
   * @param path 菜单路径，格式如 "File/New/Scene"
   * @return 找到的菜单项指针，未找到返回nullptr
   */
  std::shared_ptr<UIMenuItem> FindMenuItem(const std::string &path);

  // ==================== 状态管理接口 ====================

  /**
   * @brief 设置菜单启用状态
   * @param menuLabel 菜单标签
   * @param enabled 是否启用
   */
  void SetMenuEnabled(const std::string &menuLabel, bool enabled);

  /**
   * @brief 设置菜单项启用状态
   * @param path 菜单路径，格式如 "File/New/Scene"
   * @param enabled 是否启用
   */
  void SetMenuItemEnabled(const std::string &path, bool enabled);

  /**
   * @brief 设置菜单项可见状态
   * @param path 菜单路径
   * @param visible 是否可见
   */
  void SetMenuItemVisible(const std::string &path, bool visible);

  /**
   * @brief 设置菜单项选中状态
   * @param path 菜单路径
   * @param selected 是否选中（显示勾选标记）
   */
  void SetMenuItemSelected(const std::string &path, bool selected);

  // ==================== 渲染接口 ====================

  /**
   * @brief 渲染整个菜单栏
   * 在ImGui::BeginMenuBar()内部调用
   */
  void Render();

  /**
   * @brief 渲染菜单栏（包含BeginMenuBar和EndMenuBar）
   */
  void RenderMenuBar();

 private:
  // ==================== 辅助函数 ====================

  /**
   * @brief 解析菜单路径
   * @param path 菜单路径
   * @return 路径组件列表
   */
  std::vector<std::string> SplitPath(const std::string &path);

  /**
   * @brief 递归查找菜单项
   * @param current 当前菜单项
   * @param pathComponents 路径组件列表
   * @param currentIndex 当前处理的路径组件索引
   * @return 找到的菜单项指针
   */
  std::shared_ptr<UIMenuItem> FindMenuItemRecursive(
      std::shared_ptr<UIMenuItem> current,
      const std::vector<std::string> &pathComponents, size_t currentIndex);

 private:
  std::vector<std::shared_ptr<UIMenuItemSubmenu>>
      m_TopLevelMenus;  // 顶级菜单列表

  // 快速查找表（可选优化）
  std::unordered_map<std::string, std::shared_ptr<UIMenuItem>> m_ItemMap;
};
}  // namespace mite

#endif  // MITE_UI_MENU_H
