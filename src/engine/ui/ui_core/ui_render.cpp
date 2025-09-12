#include "ui_render.h"
#include "ui_imgui_backend/ui_imgui_render.h"

namespace mite {

UIRender &UIRender::Get()
{
  // 当前唯一后端为Json翻译系统
  static ImGuiUIRender instance;

  return instance;
}
}  // namespace mite