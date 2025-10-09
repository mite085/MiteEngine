#ifndef MITE_INSTANCE_EVENT
#define MITE_INSTANCE_EVENT

#include "basic_instance/camera_instance.h"
#include "basic_instance/material_instance.h"
#include "basic_instance/mesh_instance.h"
#include "subscription_group.h"

namespace mite {
/**
 * 相机实例创建事件（RenderContext订阅事件，负责执行实例与Shader的UBO绑定）
 */
class CameraInstanceCreateEvent : public Event {
 public:
  explicit CameraInstanceCreateEvent(const std::shared_ptr<CameraInstance> instance)
      : m_Instance(instance)
  {
  }
  std::shared_ptr<CameraInstance> GetInstance() const { return m_Instance; }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_RENDER)
  Event *Clone() const override { return new CameraInstanceCreateEvent(m_Instance); }

 private:
  std::shared_ptr<CameraInstance> m_Instance;
};
/**
 * 材质实例创建事件
 */
class MaterialInstanceCreateEvent : public Event {
 public:
  explicit MaterialInstanceCreateEvent(const std::shared_ptr<MaterialInstance> instance)
      : m_Instance(instance)
  {
  }
  std::shared_ptr<MaterialInstance> GetInstance() const { return m_Instance; }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_RENDER)
  Event *Clone() const override { return new MaterialInstanceCreateEvent(m_Instance); }

 private:
  std::shared_ptr<MaterialInstance> m_Instance;
};
/**
 * 网格体实例创建事件
 */
class MeshInstanceCreateEvent : public Event {
 public:
  explicit MeshInstanceCreateEvent(const std::shared_ptr<MeshInstance> instance)
      : m_Instance(instance)
  {
  }
  std::shared_ptr<MeshInstance> GetInstance() const { return m_Instance; }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_RENDER)
  Event *Clone() const override { return new MeshInstanceCreateEvent(m_Instance); }

 private:
  std::shared_ptr<MeshInstance> m_Instance;
};

/**
 * 光源SSBO创建事件
 */
class LightSSBOCreateEvent : public Event {
 public:
  explicit LightSSBOCreateEvent(const std::shared_ptr<LightShaderStorgeBuffer> ssbo) : m_SSBO(ssbo)
  {
  }
  std::shared_ptr<LightShaderStorgeBuffer> GetSSBO() const { return m_SSBO; }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_RENDER)
  Event *Clone() const override { return new LightSSBOCreateEvent(m_SSBO); }

 private:
  std::shared_ptr<LightShaderStorgeBuffer> m_SSBO;
};
}  // namespace mite

#endif  // MITE_RENDER_EVENT
