#ifndef MITE_RENDER_EVENT
#define MITE_RENDER_EVENT

#include "basic_shader/framebuffer.h"
#include "subscription_group.h"

namespace mite {
/**
 * 视口尺寸改变事件
 */
class ViewPortResizeEvent : public Event {
 public:
  explicit ViewPortResizeEvent(const glm::uvec2 &size) : m_Size(size) {}
  const glm::uvec2 &GetSize() const
  {
    return m_Size;
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_RENDER)
  Event *Clone() const override
  {
    return new ViewPortResizeEvent(m_Size);
  }

 private:
  glm::uvec2 m_Size;
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
      return m_Texture->getType();
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
