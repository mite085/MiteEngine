#ifndef MITE_CAMERA_INPUT_PROCESSOR_H
#define MITE_CAMERA_INPUT_PROCESSOR_H

#include "basic_data/camera.h"
#include "input/input_processor.h"
#include "input/input_event.h"

namespace mite {

/**
 * @brief 相机控制输入处理器
 *
 * 功能：
 * - 鼠标右键拖拽旋转视角
 * - 鼠标中键拖拽平移视角
 * - 滚轮缩放
 * - WASD键盘移动
 * 
 * 注意：
 * Moving（移动）行为要求持续输入（按住键时每帧触发），
 * handle函数仅仅维护InputState，
 * 真正负责修改Camera数据的，
 * 是每帧调用的UpdateCameraTransform函数
 * 
 * Rotating（旋转）和Panning（平移）行为
 * 则要求瞬时相对位移（每帧获取鼠标偏移量）
 * handle函数可以直接执行Rotate和Pan
 */
class CameraInputProcessor : public InputProcessor {
 public:
  explicit CameraInputProcessor(std::shared_ptr<Camera> camera);

  // InputProcessor接口实现
  int GetPriority() const override
  {
    return InputPriority::CAMERA;
  }
  const std::string &GetID() const override
  {
    static const std::string id = "CameraProcessor";
    return id;
  }

  // 配置方法
  void SetMoveSpeed(float speed)
  {
    m_MoveSpeed = speed;
  }
  void SetRotationSpeed(float speed)
  {
    m_RotationSpeed = speed;
  }
  void SetZoomSpeed(float speed)
  {
    m_ZoomSpeed = speed;
  }

  // 相机控制方法
  void UpdateCameraTransform(float deltaTime);

 protected:
  // 事件处理辅助方法
  virtual void handleMouseMove(MouseMoveEvent &e);
  virtual void handleMouseButton(MouseButtonReleasedEvent &e);
  virtual void handleMouseScroll(MouseScrollEvent &e);
  virtual void handleKeyEvent(KeyReleasedEvent &e);

  std::shared_ptr<Camera> m_Camera;
  glm::vec2 m_LastMousePos{0.0f, 0.0f};

  // 控制参数
  float m_MoveSpeed = 5.0f;
  float m_RotationSpeed = 0.5f;
  float m_ZoomSpeed = 2.0f;

  // 输入状态
  struct {
    bool rotating = false;
    bool panning = false;
    glm::vec3 moveDirection{0.0f};
  } m_InputState;
};

}  // namespace mite

#endif