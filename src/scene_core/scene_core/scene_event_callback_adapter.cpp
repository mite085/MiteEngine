#include "scene_event_callback_adapter.h"
namespace mite {
SceneEventCallbackAdapter::SceneEventCallbackAdapter()
{
  RegisterCallbacks();
}

SceneEventCallbackAdapter::~SceneEventCallbackAdapter()
{
  UnregisterCallbacks();
}

void SceneEventCallbackAdapter::RegisterCallbacks()
{
	// 由SceneRegistry负责触发
}

void SceneEventCallbackAdapter::UnregisterCallbacks()
{
  std::unique_lock lock(m_CallbackMutex);
  m_ConstructCallbacks.clear();
  //m_UpdateCallbacks.clear();
  m_DestroyCallbacks.clear();
}

};  // namespace mite
