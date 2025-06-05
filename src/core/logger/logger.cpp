#include "logger.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace mite {

std::shared_ptr<spdlog::logger> LoggerSystem::s_CoreLogger;
std::vector<std::shared_ptr<spdlog::logger>> LoggerSystem::s_ModuleLoggers;

void LoggerSystem::Init()
{
  // 创建多sink的logger
  std::vector<spdlog::sink_ptr> sinks;

  // 控制台sink（带颜色）
  sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

  // 滚动文件sink（每文件5MB，最多3个文件）
  sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      "logs/editor.log", 5 * 1024 * 1024, 3));

  // 创建核心logger
  s_CoreLogger = std::make_shared<spdlog::logger>("Mite Engine", begin(sinks), end(sinks));
  spdlog::register_logger(s_CoreLogger);

  // 设置默认格式: [时间] [级别] [logger名] 消息
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");

  // 默认级别
#ifdef _DEBUG
  s_CoreLogger->set_level(spdlog::level::trace);
#else
  s_CoreLogger->set_level(spdlog::level::info);
#endif

  // 刷新策略
  s_CoreLogger->flush_on(spdlog::level::warn);

  LOG_INFO("Logger initialized");
}

void LoggerSystem::Shutdown()
{
  LOG_INFO("Shutting down logger");
  spdlog::shutdown();
}

void LoggerSystem::SetLevel(spdlog::level::level_enum level)
{
  s_CoreLogger->set_level(level);
}

std::shared_ptr<spdlog::logger> &LoggerSystem::GetCoreLogger()
{
  return s_CoreLogger;
}

std::shared_ptr<spdlog::logger> LoggerSystem::CreateModuleLogger(const std::string &name)
{
  auto logger = std::make_shared<spdlog::logger>(
      name, s_CoreLogger->sinks().begin(), s_CoreLogger->sinks().end());
  logger->set_level(s_CoreLogger->level());
  spdlog::register_logger(logger);
  s_ModuleLoggers.push_back(logger);
  return logger;
}

}  // namespace mite