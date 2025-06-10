#ifndef MITE_SCENE_VIEW
#define MITE_SCENE_VIEW

#include "scene_core.h"
#include "scene_data.h"

namespace mite {

class SceneView {
 public:
  // 构造函数接受场景引用
  explicit SceneView(const Scene &scene);

  void UpDate();
  void SyncFromSceneCore();
  RenderData& GetRenderData();

  private:
  RenderData m_RenderData;
  std::shared_ptr<Scene> m_Scene;
};
}  // namespace mite

#endif