#ifndef MITE_UI_PANEL_H
#define MITE_UI_PANEL_H

#include "ui_widget.h"

namespace mite {

/**
 * @brief UI面板基类，用于管理一组控件
 * 
 * Panel应当继承自Widget，因为Panel是一个特殊的Widget：
 * 1. Panel具有Widget的所有基本特性（位置、尺寸、可见性、渲染能力等）
 * 2. Panel在Widget的基础上扩展了容器功能（管理子元素、布局等）
 */
class UIPanel : public UIWidget {
 public:
  explicit UIPanel(const std::string &name = "");
  virtual ~UIPanel();

  /**
   * @brief 添加子控件
   */
  virtual void AddWidget(std::shared_ptr<UIWidget> widget);

  /**
   * @brief 移除子控件
   */
  virtual void RemoveWidget(UUID widgetId);

  /**
   * @brief 获取子控件
   */
  std::shared_ptr<UIWidget> GetWidget(UUID widgetId) const;

  /**
   * @brief 获取所有子控件
   */
  const std::vector<std::shared_ptr<UIWidget>> &GetWidgets() const;

  /**
   * @brief 清空所有子控件
   */
  virtual void ClearWidgets();

  /**
   * @brief 更新面板状态
   */
  virtual void Update(float deltaTime) override;

  /**
   * @brief 渲染面板及其子控件
   */
  virtual void Render() = 0;

  /**
   * @brief 处理布局计算
   */
  virtual void CalculateLayout();

  /**
   * @brief 获取面板标题
   */
  const std::string &GetTitle() const;

  /**
   * @brief 设置面板标题
   */
  void SetTitle(const std::string &title);

  /**
   * @brief 获取面板是否可拖动
   */
  bool IsDraggable() const;

  /**
   * @brief 设置面板是否可拖动
   */
  void SetDraggable(bool draggable);

  /**
   * @brief 获取面板是否可调整大小
   */
  bool IsResizable() const;

  /**
   * @brief 设置面板是否可调整大小
   */
  void SetResizable(bool resizable);

 protected:
  std::string m_Title;
  bool m_Draggable;
  bool m_Resizable;
  std::vector<std::shared_ptr<UIWidget>> m_Widgets;
  std::unordered_map<UUID, std::shared_ptr<UIWidget>> m_WidgetMap;
};

}  // namespace mite

#endif  // MITE_UI_PANEL_H
