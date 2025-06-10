#ifndef MITE_INPUT_PROCESSOR
#define MITE_INPUT_PROCESSOR

#include "headers.h"

namespace mite {
// 处理器优先级常量
namespace InputPriority {
const int SYSTEM = 1000;   // 系统级快捷键
const int UI_MODAL = 800;  // 模态对话框
const int UI_FORM = 500;   // 常规UI控件
const int GAMEPLAY = 300;  // 游戏逻辑
const int CAMERA = 200;    // 摄像机控制
}  // namespace InputPriority

class InputProcessor {
 public:
  virtual ~InputProcessor() = default;

  // 核心处理接口
  virtual bool HandleEvent(Event &e) = 0;

  // 配置接口
  virtual int GetPriority() const = 0;
  virtual const std::string &GetID() const = 0;
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
};

#endif
