#ifndef MITE_RENDERER_COMMAND
#define MITE_RENDERER_COMMAND

#include "basic_data/shader.h"
#include "basic_data/texture.h"
#include "basic_data/mesh.h"
#include "basic_data/framebuffer.h"
#include "renderable_item.h"

namespace mite {
/**
 * @brief 渲染命令系统（单例模式，线程安全）
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

  // 渲染状态结构体
  struct RenderState {
    bool depthTest = true;
    GLenum depthFunc = GL_LESS;
    bool blend = true;
    GLenum blendSrc = GL_SRC_ALPHA;
    GLenum blendDst = GL_ONE_MINUS_SRC_ALPHA;
    bool cullFace = true;
    GLenum cullFaceMode = GL_BACK;
  };

  // 命令数据结构
  struct Command {
    CommandType type;
    std::function<void()> execute;
    std::string debugName;  // 调试用名称
  };

  // 获取单例实例
  static RenderCommand &Get();

  // ---- 核心接口 ----
  static void Init();  // 初始化默认渲染状态

  // 清屏命令
  static void Clear(uint32_t clearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                    const glm::vec4 &clearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f),
                    float depthClear = 1.0f,
                    int stencilClear = 0);

  // 帧缓冲操作（新增）
  static void BindFrameBuffer(const FrameBuffer::Ptr &framebuffer);
  static void UnbindFrameBuffer();

  // 绘制命令
  // TODO: 这一部分还可以继续优化，原则上仅需传递以下几个参数
  // const std::shared_ptr<OpenGLShader>& shader,
  // const std::shared_ptr<Mesh> &mesh,
  // const glm::mat4 &transform  对应"u_Model"矩阵
  static void Submit(std::shared_ptr<RenderableItem> item,
                     glm::mat4 viewMatrix,
                     glm::mat4 projectionMatrix);

  // 视口设置
  static void SetViewport(int x, int y, int width, int height);

  // 渲染状态设置（扩展）
  static void SetRenderState(const RenderState &state);

  // 自定义命令
  template<typename Func>
  static void PushCustomCommand(Func &&func, const std::string &debugName = "Custom");

  // 执行控制
  static void Flush();       // 执行所有命令
  static void ClearQueue();  // 清空命令队列（新增）

 private:
  RenderCommand() = default;

  std::queue<Command> m_CommandQueue;
  std::mutex m_QueueMutex;
  glm::vec4 m_ClearColor{0.1f, 0.1f, 0.1f, 1.0f};
  float m_DepthClearValue = 1.0f;
  int m_StencilClearValue = 0;
  uint32_t m_ClearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
  RenderState m_CurrentState;

  Logger m_Logger;  // 日志系统
};

// 模板实现
template<typename Func>
void RenderCommand::PushCustomCommand(Func &&func, const std::string &debugName)
{
  auto &instance = Get();
  std::lock_guard<std::mutex> lock(instance.m_QueueMutex);
  instance.m_CommandQueue.push({CommandType::Custom, std::forward<Func>(func), debugName});
}

}  // namespace mite

#endif
