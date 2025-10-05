#ifndef MITE_RENDERER_COMMAND
#define MITE_RENDERER_COMMAND

#include "basic_instance/camera_instance.h"
#include "basic_shader/framebuffer.h"
#include "basic_event/render_event.h"
#include "render_device.h"
#include "renderable_item.h"

namespace mite {
// 渲染状态结构体
struct RenderState {
  // 基本属性
  bool depthTest = true;     // 深度测试
  bool depthWrite = true;    // 深度写出
  bool blend = false;        // 混合模式
  bool cullFace = true;      // 面剔除
  bool stencilTest = false;  // 模板测试
  bool wireframe = false;    // 线框模式

  // 颜色写出控制
  bool colorWriteR = true;
  bool colorWriteG = true;
  bool colorWriteB = true;
  bool colorWriteA = true;
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

    // 原子操作命令
    BindCameraUBO,          // 绑定相机UBO
    BindShader,            // 绑定着色器程序
    UnbindShader,          // 解绑着色器程序
    UploadShaderUniforms,  // 上传着色器Uniforms
    BindTextures,          // 绑定纹理
    BindMesh,              // 绑定网格VAO
    DrawMesh,              // 绘制网格

    Custom  // 自定义命令
  };

  struct Command {
    CommandType type;
    std::function<void()> execute;
    std::string debugName;
  };
  virtual ~RenderCommand() = default;

  // ---------------- 基础命令接口 ----------------
  virtual void Init() = 0;
  virtual void Clear(uint32_t clearFlags,
                     const glm::vec4 &clearColor,
                     float depthClear = 1.0f,
                     int stencilClear = 0) = 0;
  // 帧缓冲操作
  virtual void BindFrameBuffer(const std::shared_ptr<FrameBuffer> &framebuffer) = 0;
  virtual void UnbindFrameBuffer() = 0;
  // 视口设置
  virtual void SetViewport(int x, int y, int width, int height) = 0;
  // 渲染状态设置
  virtual void SetRenderState(const RenderState &state) = 0;

  // ---------------- 原子操作命令 ----------------
  /**
   * @brief 初始化绑定相机UBO，每个Stage开始阶段调用一次即可
   * @param instance 相机实例引用
   * @param shader 当前stage使用的shader
   */
  virtual void BindCameraUBO(CameraInstance &instance) = 0;
  /**
   * @brief 绑定/解绑着色器程序
   * @param shader 着色器程序指针
   * @param uniformSetup 可选的Uniform设置回调
   *
   * 使用示例：
   * command.BindShader(gbufferShader, [&](ShaderProgram::Ptr shader) {
   *     shader->SetMat4("u_Model", item.worldTransform);
   *     shader->SetMat4("u_View", viewMatrix);
   *     shader->SetMat4("u_Projection", projectionMatrix);
   * });
   */
  virtual void BindShader(
      std::shared_ptr<OpenGLShader> shader,
      std::function<void(std::shared_ptr<OpenGLShader>)> uniformSetup = nullptr) = 0;
  virtual void UnbindShader(std::shared_ptr<OpenGLShader> shader) = 0;
  /**
   * @brief 绑定纹理到指定槽位
   * @param textureHandle 纹理句柄
   * @param slot 纹理槽位
   * @param samplerType 采样器类型（2D/Cube等）
   */
  virtual void BindTexture(TextureGPUHandle textureHandle,
                           uint32_t slot,
                           uint32_t samplerType = 0) = 0;
  /**
   * @brief 绑定网格VAO
   * @param mesh 网格数据
   */
  virtual void BindMesh(const Mesh &mesh) = 0;
  /**
   * @brief 绘制已绑定的网格
   * @param indexCount 索引数量
   * @param indexOffset 索引偏移
   * @param primitiveType 图元类型
   * @param indexType 索引类型
   */
  virtual void DrawMesh(uint32_t indexCount,
                        uint32_t indexOffset = 0,
                        uint32_t primitiveType = 0x0004,   // GL_TRIANGLES from glad
                        uint32_t indexType = 0x1405) = 0;  // GL_UNSIGNED_INT from glad

  // ---------------- 整合操作命令 ----------------
  /**
   * @brief 前向渲染的便捷提交方法（使用新的原子命令重构）
   * @deprecated 建议在新代码中使用原子命令
   */
  virtual void Submit(RenderableItem item) = 0;
  /**
   * @brief G-Buffer渲染的专用提交方法
   * @param item 可渲染项
   * @param gbufferShader 专用的G-Buffer着色器（覆盖材质自带的着色器）
   */
  virtual void SubmitToGBuffer(RenderableItem item,
                               std::shared_ptr<OpenGLShader> gbufferShader) = 0;

  // ---------------- 完成事件发布 ----------------
  /**
   * @brief 每当一个运行时纹理完成绘制时发布事件，可以通过订阅该事件获取运行时纹理用于显示
   * @param texture 运行时纹理指针
   * @param identify 可选的标识符，用于区分纹理（如GBuffer无需区分，但ShadowMap需要按照光源名称区分）
   */
  virtual void PublishEventRuntimeTextureFinished(RuntimeTexturePtr texture,
                                                  std::string identify = "") = 0;

  // ---------------- 执行控制 ----------------
  virtual void Flush() = 0;       // 执行所有命令
  virtual void ClearQueue() = 0;  // 清空命令队列
  static RenderCommand &Get();    // 单例获取

  // ---------------- 模板方法：自定义命令 ----------------
  template<typename Func>
  void PushCustomCommand(Func &&func, const std::string &debugName = "Custom");

 protected:
  RenderCommand() = default;

  std::unique_ptr<RenderDevice> m_Device;  // 设备管理
  std::queue<Command> m_CommandQueue;      // 命令存储
  std::mutex m_QueueMutex;                 // 命令锁

  // 清除状态
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
