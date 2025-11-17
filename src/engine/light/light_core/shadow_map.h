#ifndef MITE_SHADOW_MAP_H
#define MITE_SHADOW_MAP_H

#include "shadow_map_type.h"
#include "basic_data/transform.h"

namespace mite {
/**
 * @brief 阴影贴图抽象基类
 */
class ShadowMap {
 public:
  explicit ShadowMap(const ShadowMapData &data);
  virtual ~ShadowMap() = default;
  // ---- 核心接口 ----
  virtual ShadowMapData PrepareShadowData(const uint32_t lightIndex,
                                          const Transform &lightWorldTransform,
                                          const Transform &cameraView,
                                          const glm::mat4 &cameraProj = glm::mat4(1.0f)) = 0;
  virtual size_t GetShadowMatrixCount() const = 0;
  virtual glm::mat4 GetShadowMatrix(size_t index) const = 0;

  /**
   * @brief 更新阴影数据
   */
  virtual void UpdateData(const ShadowMapData &data);

  /**
   * @brief 获取阴影数据
   */
  const ShadowMapData &GetData() const
  {
    return m_Data;
  }
  virtual bool NeedsUpdate() const = 0;
  virtual void MarkUpdated() = 0;
  virtual std::string GetShadowTypeName() const = 0;
  virtual bool Validate() const;

 protected:
  ShadowMapData m_Data;
  bool m_NeedsUpdate = true;
  bool ValidateBaseParameters() const;
};
using ShadowMapPtr = std::shared_ptr<ShadowMap>;
}  // namespace mite

#endif  // MITE_SHADOW_MAP_H
