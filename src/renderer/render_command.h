#ifndef MITE_RENDERER_COMMAND
#define MITE_RENDERER_COMMAND

#include "shader.h"
#include "texture.h"
#include "mesh.h"

namespace mite {
/**
 * @brief 渲染命令队列（封装OpenGL调用，支持多线程提交）
 * @note 职责：
 * 1. 提供线程安全的渲染命令提交接口
 * 2. 管理渲染状态（深度测试/混合等）
 * 3. 执行实际渲染操作（在渲染线程调用）
 */
class RenderCommand {
 public:
  // ---- 命令类型 ----
  enum class CommandType {
    Clear,           // 清屏
    SetClearColor,   // 设置清屏颜色
    DrawIndexed,     // 绘制索引几何体
    SetViewport,     // 设置视口
    SetRenderState,  // 设置渲染状态
    Custom           // 自定义命令
  };

  // ---- 命令数据结构 ----
  struct Command {
    CommandType type;
    std::function<void()> execute;  // 执行lambda
  };

  // ---- 单例访问 ----
  static RenderCommand &Get();

  // ---- 核心接口 ----
  /**
   * @brief 提交一个清屏命令
   * @param clearColor 清屏颜色（可选）
   */
  static void Clear(const glm::vec4 &clearColor = {0.1f, 0.1f, 0.1f, 1.0f});

  /**
   * @brief 提交一个绘制命令
   * @param shader      使用的Shader程序
   * @param vertexArray 顶点数组对象
   * @param transform   模型变换矩阵
   */
  static void Submit(const std::shared_ptr<Shader> &shader,
                     const std::shared_ptr<Mesh> &mesh,
                     const glm::mat4 &transform = glm::mat4(1.0f));

  /**
   * @brief 设置视口大小
   * @param x,y     左下角坐标
   * @param width,height 尺寸
   */
  static void SetViewport(int x, int y, int width, int height);

  /**
   * @brief 提交自定义渲染命令
   * @param func 可调用对象（lambda/函数指针）
   */
  template<typename Func> static void PushCustomCommand(Func &&func);

  // ---- 执行控制 ----
  /**
   * @brief 执行所有已提交的命令（必须在渲染线程调用）
   */
  static void Flush();

  /**
   * @brief 初始化渲染状态（程序启动时调用）
   */
  static void Init();

 private:
  RenderCommand();

  std::queue<Command> m_CommandQueue;
  std::mutex m_QueueMutex;
  glm::vec4 m_ClearColor{0.1f, 0.1f, 0.1f, 1.0f};
};

// 模板实现必须放在头文件
template<typename Func> void RenderCommand::PushCustomCommand(Func &&func)
{
  auto &instance = Get();
  std::lock_guard<std::mutex> lock(instance.m_QueueMutex);
  instance.m_CommandQueue.push({CommandType::Custom, std::forward<Func>(func)});
}

}  // namespace mite

#endif
