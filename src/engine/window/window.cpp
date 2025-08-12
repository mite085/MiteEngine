#include "window.h"
#include "glfw_window/glfw_window.h"

namespace mite {
const uint32_t Window::WindowCount(WindowType& type)
{
  if (type == WindowType::GLFWWINDOW)
    return GLFWWindow::GLFWWindowCount();
  else
    return 0;
}
std::unique_ptr<Window> Window::Create(const WindowConfig &config)
{
  if (config.type == WindowType::GLFWWINDOW)
    return std::make_unique<GLFWWindow>();
  else
    throw std::runtime_error("No window backend configured!");
}
uint32_t WindowResizeEvent::GetWidth() const
{
  return m_Width;
}
uint32_t WindowResizeEvent::GetHeight() const
{
  return m_Height;
}

}  // namespace mite
