#include "input_processor.h"

namespace mite {
InputProcessor::InputProcessor() {
  // 创建日志系统
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Input Processor: {" + GetID() + "}");
  m_Logger->trace("Created Input Processor : {}", GetID());
}
};  // namespace mite
