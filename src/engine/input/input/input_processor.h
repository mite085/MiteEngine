#ifndef MITE_INPUT_PROCESSOR
#define MITE_INPUT_PROCESSOR

#include "headers/headers.h"

namespace mite {
/**
 * @brief 输入处理器优先级定义
 *
 * 数值越大优先级越高，相同优先级按注册顺序处理
 *
 */
namespace InputPriority {
const int SYSTEM = 1000;   // 系统级输入（如调试控制台）
const int UI_MODAL = 800;  // 模态UI（如弹窗）
const int GIZMO = 600;    // Transform控件
const int UI_FORM = 500;  // 普通编辑器UI（如ViewportInput）
const int CAMERA = 200;   // 相机控制
}  // namespace InputPriority

/**
 * @brief 模块化输入处理器抽象基类
 *
 * 派生类需实现具体输入事件处理逻辑
 */
class InputProcessor {
 public:
  virtual ~InputProcessor() = default;

  /**
   * @brief 处理输入事件
   * @param e 输入事件引用
   * @return 是否已处理（若返回true，事件将不再传递）
   */
  virtual bool HandleEvent(Event &e) = 0;

  // === 必须实现的接口 ===
  virtual int GetPriority() const = 0;
  virtual const std::string &GetID() const = 0;

  // === 可选重写的接口 ===
  virtual bool IsEnabled() const
  {
    return m_Enabled;
  }
  virtual void SetEnabled(bool enabled)
  {
    m_Enabled = enabled;
  }

 protected:
  bool m_Enabled = true;
};
};  // namespace mite

#endif
