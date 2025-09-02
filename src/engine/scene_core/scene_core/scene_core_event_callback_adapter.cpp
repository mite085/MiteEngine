#include "scene_core_event_callback_adapter.h"
namespace mite {
SceneCoreEventCallbackAdapter::SceneCoreEventCallbackAdapter()
{
  RegisterCallbacks();
}

SceneCoreEventCallbackAdapter::~SceneCoreEventCallbackAdapter()
{
  UnregisterCallbacks();
}

void SceneCoreEventCallbackAdapter::RegisterCallbacks()
{
	// 由SceneRegistry负责触发
}

void SceneCoreEventCallbackAdapter::UnregisterCallbacks()
{
  std::unique_lock lock(m_Mutex);
  m_ConstructCallbacks.clear();
  //m_UpdateCallbacks.clear();
  m_DestroyCallbacks.clear();
}

};  // namespace mite
