#ifndef MITE_GBUFFER_STAGE_H
#define MITE_GBUFFER_STAGE_H

#include "basic_shader/gbuffer.h"
#include "basic_shader/shader.h"
#include "render_stage.h"

namespace mite {
/**
 * @brief G-Buffer渲染阶段 - 负责填充几何缓冲区
 *
 * 职责：
 * 1. 管理G-Buffer渲染目标
 * 2. 执行几何Pass，将场景几何信息写入G-Buffer
 * 3. 处理材质参数到G-Buffer的编码
 * 4. 不进行任何光照计算
 */
class GBufferStage : public RenderStage {
 public:
  GBufferStage();
  ~GBufferStage() override;

  // ---- 生命周期管理 ----
  void Initialize() override;
  void Execute(RenderContext &context) override;
  void Shutdown() override;

  // ---- G-Buffer访问 ----
  GBufferPtr GetGBuffer() const
  {
    return m_GBuffer;
  }

  // ---- 配置接口 ----
  void SetGBufferShader(std::shared_ptr<OpenGLShader> shader)
  {
    m_GBufferShader = shader;
  }

 private:
  // ---- 私有渲染方法 ----
  /**
   * @brief 渲染不透明物体到G-Buffer
   */
  void RenderOpaqueQueue(RenderContext &context);

  /**
   * @brief 渲染Alpha测试物体到G-Buffer
   */
  void RenderAlphaTestQueue(RenderContext &context);

  /**
   * @brief 设置G-Buffer渲染状态
   */
  void SetupGBufferRenderState();

  /**
   * @brief 验证渲染项是否适合G-Buffer渲染
   */
  bool ValidateGBufferRenderableItem(const RenderableItem &item) const;

  /**
   * @brief 编码材质参数到G-Buffer
   */
  void EncodeMaterialToGBuffer(const RenderableItem &item);

  /**
   * @brief 获取G-Buffer着色器
   */
  std::shared_ptr<OpenGLShader> GetGBufferShaderForMaterial(const RenderableItem &item);

  // ---- 成员变量 ----
  GBufferPtr m_GBuffer;                           // G-Buffer数据容器
  std::shared_ptr<OpenGLShader> m_GBufferShader;  // 默认G-Buffer着色器

  // 渲染状态配置
  RenderState m_OpaqueState;
  RenderState m_AlphaTestState;

  // 配置参数
  bool m_Initialized = false;
};
}  // namespace mite

#endif
