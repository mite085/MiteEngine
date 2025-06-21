#include "component.h"
#include "entity.h"

namespace mite {
// 基类方法的基础实现
// 注意：大部分功能已在头文件中实现为纯虚函数

void Component::SetDirty(bool dirty)
{
  m_Dirty = dirty;
}

bool Component::IsDirty() const
{
  return m_Dirty;
}

void Component::Update()
{
  if (m_Dirty) {
    // 执行必要的更新或重新计算
    Recalculate();

    // 清除脏标记
    m_Dirty = false;
  }
}

void Component::SetEnabled(bool enabled)
{
  m_Enabled = enabled;
}

bool Component::IsEnabled() const
{
  return m_Enabled;
}

bool Component::Serialize(std::ostream &output) const
{
  // 基础组件序列化只写入启用状态
  output.write(reinterpret_cast<const char *>(&m_Enabled), sizeof(m_Enabled));
  return !output.fail();
}

bool Component::Deserialize(std::istream &input)
{
  // 基础组件反序列化只读取启用状态
  input.read(reinterpret_cast<char *>(&m_Enabled), sizeof(m_Enabled));
  return !input.fail();
}

void Component::SetOwnerEntity(Entity entity)
{
  m_OwnerEntity = entity;
}

Entity Component::GetOwnerEntity() const
{
  return m_OwnerEntity;
}
};  // namespace mite