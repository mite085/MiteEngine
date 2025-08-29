#ifndef MITE_RENDER_DEVICE
#define MITE_RENDER_DEVICE

#include "basic_data/framebuffer.h"
#include "basic_data/mesh.h"
#include "basic_data/model.h"
#include "basic_data/texture.h"
#include "basic_event/asset_event.h"

namespace mite {
/**
 * 渲染设备抽象接口
 *
 * 职责：
 * 1. 提供跨渲染API的资源管理接口
 * 2. 处理资源加载事件
 * 3. 管理GPU资源生命周期
 *
 * 设计原则：
 * 1. 接口与具体API解耦
 * 2. 线程安全设计
 * 3. 支持未来多后端扩展
 *
 * 单例模式：
 * 使用单例模式的目的是方便Texture和Mesh每次创建时可以不通过对IRenderDevice的
 * 依赖注入，且可独立实现Draw方法，确保代码的简洁性。
 *
 * 存在风险：
 * 1. 如果渲染指令需在多个线程提交（如渲染线程 vs. 资源加载线程），
 *	  单例的全局锁可能成为性能瓶颈。此时需设计无锁队列或线程局部存储（TLS）。
 * 2. 单例的 IRenderDevice 会阻碍单元测试中对渲染接口的模拟（Mocking）。
 *    依赖注入更利于隔离测试。
 * 3. 即使当前无需多渲染器，未来可能支持多视口、多GPU或离线渲染。单例会限制架构灵活性。
 */
class IRenderDevice {
 public:
  virtual ~IRenderDevice() = default;

  // ---- 纹理操作 ----
  virtual TextureGPUHandle CreateTexture(std::shared_ptr<TextureSourceData> data) = 0;
  virtual void DestroyTexture(TextureGPUHandle handle) = 0;
  virtual void BindTexture(TextureGPUHandle handle, uint32_t slot) const = 0;
  virtual void SetTextureWrapMode(TextureGPUHandle handle, TextureWrapMode mode) = 0;
  virtual void SetTextureFilterMode(TextureGPUHandle handle, TextureFilterMode mode) = 0;
  virtual void GenerateMipmaps(TextureGPUHandle handle) = 0;

  // ---- 模型/网格操作 ----
  virtual ModelGPUHandle CreateModel(std::shared_ptr<ModelSourceData> data) = 0;
  virtual void DestroyModel(ModelGPUHandle handle) = 0;
  /**
   * @brief 绑定Mesh
   * @param mesh 网格体对象
   *
   * 注意：
   * 由于Asset仅维护Model，由Model维护Mesh，
   * 所以Create和Destroy接收的是Model数据。
   * 但Bind和Draw的操作是和Mesh强相关，
   * 所以这里实现Bind Mesh而非Bind Model
   */
  virtual void BindMesh(std::shared_ptr<Mesh> mesh) const = 0;
  /**
   * @brief SelectLODLevel 根据输入LOD偏差，选择LOD层级
   * @param model 模型GPU句柄
   * @param cameraPosition 相机距离
   * @param lodBias LOD层级偏差值
   * @return
   */
  virtual uint32_t SelectLODLevel(const ModelGPUHandle &model,
                                  const glm::vec3 &cameraPosition,
                                  float lodBias) const = 0;
  /**
   * @brief DrawMeshLOD 根据LOD绘制Mesh
   * @param mesh
   * @param lodLevel
   */
  virtual void DrawMeshLOD(std::shared_ptr<Mesh> mesh, uint32_t lodLevel) const = 0;
  /**
   * @brief DrawIndexed 按照顶点执行绘制任务
   * @param indexCount 绘制的顶点数量
   * @param indexOffset 绘制的顶点在Handle中的偏移量
   * @param mode 默认按照三角形模式执行绘制
   * @param indexType 顶点数据格式，默认UNSIGNED INT
   * @param enableDepthTest 允许深度测试，默认开启
   */
  virtual void DrawIndexed(uint32_t indexCount,
                           uint32_t indexOffset,
                           GLenum mode = GL_TRIANGLES,
                           GLenum indexType = GL_UNSIGNED_INT,
                           bool enableDepthTest = true) const = 0;

  // ---- FrameBuffer 操作 (新增) ----
  virtual FrameBuffer::Ptr CreateFrameBuffer(const FrameBufferSpec &spec) = 0;
  virtual void DestroyFrameBuffer(FrameBuffer::Ptr framebuffer) = 0;

  // ---- 设备管理 ----
  static IRenderDevice &Current();
  static void SetCurrent(std::unique_ptr<IRenderDevice> device);

 protected:
  // 私有构造函数
  IRenderDevice();

  // ---- 事件处理 ----
  virtual bool OnModelLoaded(ModelLoadEvent &e) = 0;
  virtual bool OnTextureLoaded(TextureLoadEvent &e) = 0;

  SubscriptionGroup m_EventSubscriptions;  // 事件订阅
};
};  // namespace mite

#endif
