#include "logger.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace mite {

std::shared_ptr<spdlog::logger> LoggerSystem::s_CoreLogger;
std::vector<std::shared_ptr<spdlog::logger>> LoggerSystem::s_ModuleLoggers;

void LoggerSystem::Init()
{
  // ������sink��logger
  std::vector<spdlog::sink_ptr> sinks;

  // ����̨sink������ɫ��
  sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

  // �����ļ�sink��ÿ�ļ�5MB�����3���ļ���
  sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      "logs/editor.log", 5 * 1024 * 1024, 3));

  // ��������logger
  s_CoreLogger = std::make_shared<spdlog::logger>("Mite Engine", begin(sinks), end(sinks));
  spdlog::register_logger(s_CoreLogger);

  // ����Ĭ�ϸ�ʽ: [ʱ��] [����] [logger��] ��Ϣ
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");

  // Ĭ�ϼ���
#ifdef _DEBUG
  s_CoreLogger->set_level(spdlog::level::trace);
#else
  s_CoreLogger->set_level(spdlog::level::info);
#endif

  // ˢ�²���
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