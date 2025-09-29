#ifndef MITE_RENDERER_COMMAND
#define MITE_RENDERER_COMMAND

#include "basic_data/framebuffer.h"
#include "render_device.h"
#include "renderable_item.h"

namespace mite {
// 渲染状态结构体
struct RenderState {
  // 基本属性
  bool depthTest = true;   // 深度测试
  bool depthWrite = true;  // 深度写出
  bool blend = false;      // 混合模式
  bool cullFace = true;    // 面剔除

  // 模板测试
  bool stencilTest = false;

  // 颜色写出
  bool colorWriteR = true;
  bool colorWriteG = true;
  bool colorWriteB = true;
  bool colorWriteA = true;

  // 线框模式
  bool wireframe = false;
};

/**
 * @brief 渲染命令系统
 *
 * 主要功能：
 * 1. 支持FrameBuffer操作
 * 2. 完善的渲染状态管理
 * 3. 可扩展性设计
 * 4. 详细的错误检查
 */
class RenderCommand {
 public:
  // 命令类型枚举
  enum class CommandType {
    Clear,              // 清屏命令
    SetClearColor,      // 设置清屏颜色
    BindFrameBuffer,    // 绑定帧缓冲（新增）
    UnbindFrameBuffer,  // 解绑帧缓冲（新增）
    DrawIndexed,        // 绘制索引几何体
    SetViewport,        // 设置视口
    SetRenderState,     // 设置渲染状态
    Custom              // 自定义命令
  };

  struct Command {
    CommandType type;
    std::function<void()> execute;
    std::string debugName;
  };
  virtual ~RenderCommand() = default;
  virtual void Init() = 0;
  virtual void Clear(uint32_t clearFlags,
                     const glm::vec4 &clearColor,
                     float depthClear = 1.0f,
                     int stencilClear = 0) = 0;
  // 帧缓冲操作
  virtual void BindFrameBuffer(const FrameBuffer::Ptr &framebuffer) = 0;
  virtual void UnbindFrameBuffer() = 0;
  // 绘制命令
  virtual void Submit(RenderableItem item, glm::mat4 viewMatrix, glm::mat4 projectionMatrix) = 0;
  // 视口设置
  virtual void SetViewport(int x, int y, int width, int height) = 0;
  // 渲染状态设置
  virtual void SetRenderState(const RenderState &state) = 0;
  // 自定义命令
  template<typename Func>
  void PushCustomCommand(Func &&func, const std::string &debugName = "Custom");

  // 执行控制
  virtual void Flush() = 0;       // 执行所有命令
  virtual void ClearQueue() = 0;  // 清空命令队列
  static RenderCommand &Get();

 protected:
  RenderCommand() = default;

  std::unique_ptr<RenderDevice> m_Device;

  std::queue<Command> m_CommandQueue;
  std::mutex m_QueueMutex;
  glm::vec4 m_ClearColor{0.1f, 0.1f, 0.1f, 1.0f};
  float m_DepthClearValue = 1.0f;
  int m_StencilClearValue = 0;
  uint32_t m_ClearFlags = 0;
};
// 辅助函数
template<typename Func>
void RenderCommand::PushCustomCommand(Func &&func, const std::string &debugName)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);
  m_CommandQueue.push({CommandType::Custom, std::forward<Func>(func), debugName});
}
}  // namespace mite

#endif
