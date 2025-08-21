#ifndef MITE_UI_VIEWPORT_PANEL
#define MITE_UI_VIEWPORT_PANEL

#include "basic_data/framebuffer.h"
#include "input/input.h"
#include "renderer.h"
#include "scene_core/entity.h"
#include "ui_core/ui_panel.h"
#include "ui_input_processors/gizmo_input_processor.h"
#include "ui_input_processors/viewport_input_processor.h"

namespace mite {
/**
 * @brief 3D视口面板，负责场景渲染和交互
 *
 * 核心功能：
 * 1. 显示3D场景渲染结果（通过ImGui::Image绘制Framebuffer）
 * 2. 提供相机导航控制（通过ViewportInputProcessor控制Camera）
 * 3. 支持Gizmo物体操作（通过GizmoInputProcessor控制Entity）
 * 4. 显示右上角视图控件（通过ImGuizmo::ViewManipulate绘制）
 */
class ViewportPanel : public UIPanel {
 public:
  explicit ViewportPanel(const std::string &title = "ViewPort");
  virtual ~ViewportPanel() override;

  // ========== 基础面板接口 ==========
  void onAttach() override;
  void onDetach() override;
  void onUpdate(float deltaTime) override;
  void onRender() override;
  bool onEvent(Event &event) override;

  // ========== 视口配置方法 ==========

  /**
   * @brief 设置视口使用的相机
   * @param camera 共享指针管理的相机对象
   */
  void setCamera(std::shared_ptr<Camera> camera);

  /**
   * @brief 设置视口的帧缓冲对象
   * @param framebuffer 包含场景渲染结果的帧缓冲
   */
  void setFramebuffer(std::shared_ptr<FrameBuffer> framebuffer);

  /**
   * @brief 设置当前选中的变换矩阵（依赖注入，直接修改其值）
   * @param transform 用于Gizmo操作的变换矩阵引用
   */
  void setCurrentTransform(glm::mat4 &transform);

 private:
  // ========== 内部方法 ==========

  // 更新视口尺寸和边界信息
  void updateViewportSize();

  // 处理视口输入事件
  bool handleViewportEvent(Event &event);

  // 在ViewPort界面右上角绘制ViewManipulate
  void DrawViewManipulate();

  // ========== 成员变量 ==========

  // 视口状态
  glm::vec2 m_viewportSize = {100.0f, 100.0f};  // 视口当前尺寸
  glm::vec2 m_viewportBounds[2] = {{100.0f, 100.0f}, {100.0f, 100.0f}};  // 视口屏幕边界坐标
  bool m_viewportFocused = false;               // 视口是否有输入焦点
  bool m_viewportHovered = false;               // 鼠标是否悬停在视口上

  // 渲染资源
  std::shared_ptr<Camera> m_camera = nullptr;  // 视口相机
  std::shared_ptr<FrameBuffer> m_framebuffer = nullptr;  // 场景帧缓冲

  // 输入处理
  std::shared_ptr<ViewportInputProcessor> m_viewportInput;  // 视口导航处理器
  std::shared_ptr<GizmoInputProcessor> m_gizmoInput;        // Gizmo操作处理器
  std::shared_ptr<ModularInputContext> m_inputContext;      // 输入上下文

  // Entity的Gizmo变换状态
  bool m_isEntitySelected = false;			// 当前是否处于Entity选中状态（用于控制Gizmo显示）
  Entity m_entitySelected;					// 当前选中的Entity对象
  glm::mat4 *m_currentTransform = nullptr;  // 当前操作的Entity的World变换矩阵(外部管理)

  // 当前操作的Camera的View状态，Gizmo的绘制和反向修改相机需要该参数
  glm::mat4 m_currentCameraViewTransform;
};
};  // namespace mite

#endif
