#ifndef MITE_UI_VIEWPORT_PANEL
#define MITE_UI_VIEWPORT_PANEL

#include "basic_data/framebuffer.h"
#include "ui_element/ui_panel.h"

namespace mite {
/**
 * @brief 3D视口面板，负责场景渲染和交互
 * （功能内聚，待后续功能扩展时解耦，将控制器分离到独立文件）
 *
 * 核心显示功能：
 * 帧缓冲区渲染：显示FrameBuffer内容
 * 宽高比保持：支持各种宽高比设置
 * 自适应布局：根据面板尺寸自动调整
 * 
 * 交互功能：
 * 鼠标悬停检测：判断鼠标是否在视口内
 * 摄像机控制：右键拖拽旋转摄像机
 * 对象选择：左键点击选择场景对象
 * 焦点管理：视口焦点状态跟踪
 * 
 * UI控件功能：
 * 工具栏：包含常用操作按钮
 * 覆盖层：显示统计信息和安全框
 * 分辨率显示：实时显示当前视口分辨率
 * 设置切换：覆盖层显示/隐藏切换
 * 
 * 扩展功能槽位：
 * Gizmo集成：对象变换Gizmo
 * 网格显示：显示/隐藏网格
 * 后期处理：实时后期效果切换
 * 多视口支持：分屏显示
 */

class ViewportPanel : public UIPanel {
 public:
  ViewportPanel(const std::string &title);
  ~ViewportPanel() override;
  void Render() override;
  void Update(float deltaTime) override;
  // 帧缓冲区管理
  void SetFrameBuffer(std::shared_ptr<FrameBuffer> frameBuffer);
  std::shared_ptr<FrameBuffer> GetFrameBuffer() const;

  // 视口控制
  void SetAspectRatio(float aspectRatio);
  float GetAspectRatio() const;
  glm::vec2 GetViewportSize() const;

  // 交互控制
  void SetInteractive(bool interactive);
  bool IsInteractive() const;

  // 覆盖层控制
  void ShowOverlay(bool show);
  bool IsOverlayVisible() const;

 private:
  void RenderViewportContent();
  void RenderOverlay();
  void RenderToolbar();
  void HandleInput();
  void UpdateLayout();
  std::shared_ptr<FrameBuffer> m_FrameBuffer;
  float m_AspectRatio = 16.0f / 9.0f;
  bool m_Interactive = true;
  bool m_ShowOverlay = true;

  // 布局相关
  glm::vec2 m_ViewportPosition;
  glm::vec2 m_ViewportSize;

  // 状态管理
  bool m_IsHovered = false;
  bool m_IsFocused = false;
};

}  // namespace mite

#endif
