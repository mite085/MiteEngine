#ifndef MITE_RENDER_EVENT
#define MITE_RENDER_EVENT

#include "basic_data/transform.h"
#include "basic_shader/framebuffer.h"
#include "subscription_group.h"

namespace mite {
/**
 * @brief Viewport尺寸改变事件
 * @note SceneView订阅事件修改相机宽高比，Pipeline订阅事件修改帧缓冲尺寸
 */
class ViewportResizeEvent : public Event {
 public:
  explicit ViewportResizeEvent(const glm::vec2 &size) : m_Size(size) {}
  const glm::vec2 &GetSize() const
  {
    return m_Size;
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_RENDER)
  Event *Clone() const override
  {
    return new ViewportResizeEvent(m_Size);
  }

 private:
  glm::vec2 m_Size;
};
/**
 * @brief Viewport拾取物体事件（仅记录UV）
 * @note SceneView订阅事件执行RayCast
 */
class ViewportPickedEvent : public Event {
 public:
  explicit ViewportPickedEvent(const glm::vec2 &uv) : m_UV(uv) {}
  const glm::vec2 &GetUV() const { return m_UV; }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_RENDER)
  Event *Clone() const override { return new ViewportPickedEvent(m_UV); }

 private:
  glm::vec2 m_UV;
};
/**
 * @brief Viewport相机操控事件
 * @note SceneView订阅事件执行相机变换
 */
class ViewportCameraUpdateEvent : public Event {
 public:
  explicit ViewportCameraUpdateEvent(const Transform cameraTransform, const float cameraZoom)
      : m_CameraTransform(cameraTransform), m_CameraZoom(cameraZoom)
  {
  }
  const Transform &GetCameraTransform() const { return m_CameraTransform; }
  float GetCameraZoom() const { return m_CameraZoom; }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_RENDER)
  Event *Clone() const override
  {
    return new ViewportCameraUpdateEvent(m_CameraTransform, m_CameraZoom);
  }

 private:
  Transform m_CameraTransform;
  float m_CameraZoom;
};

/**
 * @brief ViewportPicked物体操控事件
 * @note SceneView订阅事件执行物体变换
 */
class ViewportPickedUpdateEvent : public Event {
 public:
  explicit ViewportPickedUpdateEvent(const Transform transform) : m_Transform(transform) {
  }
  const Transform &GetTransform() const { return m_Transform; }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_RENDER)
  Event *Clone() const override { return new ViewportPickedUpdateEvent(m_Transform); }

 private:
  Transform m_Transform;
};

/**
 * 渲染完成事件，每帧结束触发，可通过订阅该事件获取FBO
 */
class RuntimeTextureFinishedEvent : public Event {
 public:
  explicit RuntimeTextureFinishedEvent(RuntimeTexturePtr texture, std::string identify)
      : m_Texture(texture), m_Identify(identify)
  {
  }
  RuntimeTexturePtr GetTexture() {
    return m_Texture;
  }
  RuntimeTextureType GetTextureType()
  {
    if (m_Texture)
      return m_Texture->GetType();
    else
      return RuntimeTextureType::None;
  }
  std::string GetIdentify() {
    return m_Identify;
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_RENDER)
  Event *Clone() const override
  {
    return new RuntimeTextureFinishedEvent(m_Texture, m_Identify);
  }

 private:
  RuntimeTexturePtr m_Texture;
  std::string m_Identify;
};
}  // namespace mite

#endif  // MITE_RENDER_EVENT
