#ifndef MITE_GIZMO_H
#define MITE_GIZMO_H

#include "basic_data/camera.h"
#include "imgui.h"
#include "imguizmo.h"

namespace mite {
/**
 * @brief Gizmo工具类，封装ImGuizmo的核心功能
 */
class Gizmo {
 public:
  Gizmo();

  // 设置操作模式
  void SetOperation(ImGuizmo::OPERATION operation);
  void SetMode(ImGuizmo::MODE mode);

  // 显示/隐藏控制
  void Show();                    // 显示Gizmo
  void Hide();                    // 隐藏Gizmo
  void Toggle();                  // 切换显示/隐藏状态
  bool IsVisible() const;         // 检查是否可见
  void SetVisible(bool visible);  // 设置可见性

  /**
   * @brief 执行Gizmo操作
   * @param transformMatrix 输入输出的变换矩阵
   * @param camera 使用的相机
   * @param viewportPos 视口位置(左上角)
   * @param viewportSize 视口尺寸
   * @return 是否进行了操作
   */
  bool Manipulate(glm::mat4 &transformMatrix,
                  const Camera &camera,
                  const glm::vec2 &viewportPos,
                  const glm::vec2 &viewportSize);

  // 状态获取
  bool IsUsing() const;
  bool IsOver() const;
  ImGuizmo::OPERATION GetOperation() const;
  ImGuizmo::MODE GetMode() const;

  // 矩阵分解工具
  static void DecomposeTransform(const glm::mat4 &transform,
                                 glm::vec3 &translation,
                                 glm::vec3 &rotation,
                                 glm::vec3 &scale);

 private:
  ImGuizmo::OPERATION m_Operation = ImGuizmo::OPERATION::TRANSLATE;
  ImGuizmo::MODE m_Mode = ImGuizmo::MODE::LOCAL;
  bool m_IsUsing = false;
  bool m_IsOver = false;
  bool m_IsVisible = true;  // 默认可见
};
}  // namespace mite

#endif  // MITE_GIZMO_H