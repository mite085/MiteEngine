#ifndef MITE_UI_PANEL_H
#define MITE_UI_PANEL_H

#include "ui_widget.h"
#include "ui_layout/ui_layout.h"

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
  explicit UIPanel(const std::string &title,
                   UILayout::LayoutType layoutType = UILayout::LayoutType::Horizontal,
                   UILayout::Alignment layoutAlignment = UILayout::Alignment::TopLeft);
  virtual ~UIPanel();

  virtual void Update(float deltaTime) override;
  virtual void Render() = 0;

  
  // ==================== 子控件管理 ====================
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


  // ==================== 布局管理 ====================
  /**
   * @brief 设置布局算法
   */
  void SetLayout(std::shared_ptr<UILayout> layout);
  /**
   * @brief 获取当前布局
   */
  std::shared_ptr<UILayout> GetLayout() const;
  /**
   * @brief 应用布局
   */
  virtual void ApplyLayout();
  /**
   * @brief 标记布局dirty，Update时统一处理
   */
  void MarkLayoutDirty();

  // ==================== 状态管理 ====================
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
  // 状态管理
  bool m_Draggable = true;
  bool m_Resizable = true;

  // 子控件管理
  std::vector<std::shared_ptr<UIWidget>> m_Widgets;
  std::unordered_map<UUID, std::shared_ptr<UIWidget>> m_WidgetMap;

  // 布局管理
  bool m_LayoutDirty = false;
  std::shared_ptr<UILayout> m_Layout;
};

}  // namespace mite

#endif  // MITE_UI_PANEL_H
