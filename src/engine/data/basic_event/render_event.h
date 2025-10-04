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
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new ViewPortResizeEvent(m_Size);
  }

 private:
  glm::uvec2 m_Size;
};
/**
 * 渲染完成事件，每帧结束触发，ViewPortPanel订阅此事件，以获取FBO
 * （是否应当由事件的形式触发？存疑）
 */
class RenderFinishedEvent : public Event {
 public:
  explicit RenderFinishedEvent(std::shared_ptr<FrameBuffer> fbo) : m_FBO(fbo) {}
  std::shared_ptr<FrameBuffer> GetFBO() const
  {
    return m_FBO;
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new RenderFinishedEvent(m_FBO);
  }

 private:
  std::shared_ptr<FrameBuffer> m_FBO;
};
}  // namespace mite

#endif  // MITE_RENDER_EVENT
