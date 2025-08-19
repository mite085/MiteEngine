#include "input_processor.h"

namespace mite {
bool InputProcessor::IsEnabled() const
{
  return m_Enabled;
}

void InputProcessor::SetEnabled(bool enabled)
{
  m_Enabled = enabled;
}
};  // namespace mite