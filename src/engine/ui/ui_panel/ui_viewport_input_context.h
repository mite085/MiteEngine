#ifndef MITE_VIEWPORT_INPUT_CONTEXT_H
#define MITE_VIEWPORT_INPUT_CONTEXT_H

#include "basic_data/transform.h"
#include "input/input_context.h"
#include "input/input_state_tracker.h"

namespace mite {
/**
 * @brief 视口输入上下文 - 处理ViewportPanel的鼠标交互
 *
 * 职责：
 * 1. 管理视口焦点状态
 * 2. 处理鼠标在视口内的交互
 * 3. 支持Node选择和拖动操作
 * 4. 与GizmoOverlay协同工作
 */
class ViewportInputContext : public InputContext {
 public:
  explicit ViewportInputContext(const std::string &name);
  virtual ~ViewportInputContext() = default;

  // Update每帧更新
  void Update(float deltatime, bool gizmoUsing);
  // 应用变换 (获取到ViewManipulate操作后新的相机ViewMatrix)
  void Apply(Transform cameraTransform);

  // 视口状态管理
  void SetViewportFocus(bool focused) { m_ViewportFocused = focused; }
  void SetViewportHovered(bool hovered) { m_ViewportHovered = hovered; }
  void SetViewportRect(const glm::vec2 &pos, const glm::vec2 &size);

  // 重写输入事件处理逻辑
  void ProcessEvent(Event &e) override;

 protected:
  // 输入事件处理
  void ProcessMouseMoveEvent(MouseMoveEvent &e) override;
  void ProcessMouseButtonPressedEvent(MouseButtonPressedEvent &e) override;
  void ProcessMouseButtonReleasedEvent(MouseButtonReleasedEvent &e) override;
  void ProcessMouseScrollEvent(MouseScrollEvent &e) override;
  void ProcessKeyPressdEvent(KeyPressedEvent &e) override;
  void ProcessKeyReleasedEvent(KeyReleasedEvent &e) override;
  void ProcessKeyTypedEvent(KeyTypedEvent &e) override;

  // 更新相机变换逻辑
  void UpdateCameraMove(float deltatime);
  void ClearCameraCache();

 private:
  // 输入状态机（仅记录Button和Key的按下/释放状态）
  std::unique_ptr<InputStateTracker> m_InputStateTracker;
  glm::vec2 m_LastMousePos{0.0f, 0.0f};  // 鼠标位置缓存

  // Viewport状态
  bool m_ViewportFocused = false;  // Viewport窗口是否聚焦
  bool m_ViewportHovered = false;  // 鼠标是否悬停于Viewport上
  bool m_ViewportGizmoUsing =
      false;  // Gizmo是否占用中(若占用则不接收鼠标/键盘事件)
  glm::vec2 m_ViewportPos = {0, 0};   // Viewport位置记录
  glm::vec2 m_ViewportSize = {0, 0};  // Viewport尺寸记录

  // Camera控制参数与操作累积
  float m_CameraMoveSpeed = 5.0f;
  float m_CameraRotationSpeed = 0.1f;  // 0.1 deg/pix
  float m_CameraZoomSpeed = 2.0f;
  glm::vec3 m_CameraMoveCache = {0.0f, 0.0f, 0.0f};
  glm::vec2 m_CameraRotateCache = {0.0f, 0.0f};  // 仅支持俯仰/偏航旋转
  glm::vec2 m_CameraPanCache = {0.0f, 0.0f};     // 仅支持上下/左右平移
  float m_CameraZoomCache = 0.0f;  // 仅支持放大/缩小视场角

  /**
   * @brief 将屏幕坐标转换为视口UV坐标
   *
   * UV坐标范围：[0, 1]，左下角为(0,0)，右上角为(1,1)
   * 屏幕坐标：左上角为原点，Y轴向下
   *
   * @param screenPos 屏幕坐标（像素单位，左上角为原点）
   * @return glm::vec2 UV坐标（左下角为原点，范围[0,1]）
   */
  glm::vec2 ScreenToUV(const glm::vec2 &screenPos);
  /**
   * @brief UV坐标转回屏幕坐标（反向转换）
   *
   * @param uv UV坐标（左下角为原点，范围[0,1]）
   * @return glm::vec2 屏幕坐标（左上角为原点，像素单位）
   */
  glm::vec2 UVToScreen(const glm::vec2 &uv);
};
}  // namespace mite

#endif  // MITE_VIEWPORT_INPUT_CONTEXT_H
