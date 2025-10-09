#ifndef MITE_RENDER_DEVICE
#define MITE_RENDER_DEVICE

#include "basic_data/mesh.h"
#include "basic_data/model.h"
#include "basic_data/runtime_texture.h"
#include "basic_event/asset_event.h"
#include "basic_shader/framebuffer.h"
#include "basic_type/asset_type.h"

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
 * 使用单例模式的目的是方便Texture和Mesh每次创建时可以不通过对RenderDevice的
 * 依赖注入，且可独立实现Draw方法，确保代码的简洁性。
 *
 * 存在风险：
 * 1. 如果渲染指令需在多个线程提交（如渲染线程 vs. 资源加载线程），
 *	  单例的全局锁可能成为性能瓶颈。此时需设计无锁队列或线程局部存储（TLS）。
 * 2. 即使当前无需多渲染器，未来可能支持多视口、多GPU或离线渲染。单例会限制架构灵活性。
 */
class RenderDevice {
 public:
  RenderDevice();
  virtual ~RenderDevice() = default;

  // ---- 纹理操作 ----
  virtual TextureGPUHandle CreateTexture(std::shared_ptr<TextureSourceData> data) = 0;
  virtual TextureGPUHandle CreateRuntimeTexture(std::shared_ptr<TextureCreateInfo> createInfo) = 0;
  virtual void DestroyTexture(TextureGPUHandle handle) = 0;
  virtual void BindRuntimeTexture(RuntimeTextureType type,
                                  TextureGPUHandle textureHandle,
                                  TextureTarget target = TextureTarget::TEXTURE_2D) const = 0;
  virtual void BindExternalTexture(ExternalTextureType type,
                                   TextureGPUHandle textureHandle,
                                   TextureTarget target = TextureTarget::TEXTURE_2D) const = 0;

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
  virtual void BindMesh(Mesh mesh) const = 0;
  /**
   * @brief SelectMeshLODLevel 根据输入LOD偏差，选择单个Mesh的LOD层级
   * @param mesh 网格体对象
   * @param cameraPosition 相机距离
   * @param worldTransform 局部空间到世界空间的旋转矩阵
   * @param lodBias LOD层级偏差值(偏差值越高，越倾向于高精度。默认1不偏差)
   * @return LOD层级
   *
   * 针对超大Model（如地形）可以逐Mesh划分LOD，降低渲染压力
   */
  virtual uint32_t SelectMeshLODLevel(Mesh mesh,
                                      const glm::vec3 &cameraPosition,
                                      const glm::mat4 &worldTransform,
                                      const glm::mat4 &viewProjectionMatrix,
                                      float screenWidth,
                                      float lodBias = 1) const = 0;
  /**
   * @brief DrawMeshLOD 根据LOD绘制Mesh
   * @param mesh
   * @param lodLevel
   */
  virtual void DrawMeshLOD(Mesh mesh, uint32_t lodLevel) const = 0;
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
                           GLenum indexType = GL_UNSIGNED_INT) const = 0;

  // ---- FrameBuffer 操作 ----
  virtual std::shared_ptr<FrameBuffer> CreateFrameBuffer(const FrameBufferSpec &spec) = 0;
  virtual void DestroyFrameBuffer(std::shared_ptr<FrameBuffer> framebuffer) = 0;

 protected:
  // ---- 事件处理 ----
  virtual void OnModelLoaded(ModelLoadEvent &e) = 0;
  virtual void OnTextureLoaded(TextureLoadEvent &e) = 0;
  virtual void OnRuntimeTextureCreate(RuntimeTextureCreateEvent &e) = 0;
  virtual void OnRuntimeTextureDestroyRequest(RuntimeTextureDestroyRequestEvent &e) = 0;

  SubscriptionGroup m_EventSubscriptions;  // 事件订阅
};
};  // namespace mite

#endif
