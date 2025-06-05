#ifndef MITE_LOGGER
#define MITE_LOGGER

// VS����spdlogʱ��
// fmt��ĳЩ�ڲ�ʵ�ֻᱨwarning
// ��Ӱ�칦�ܣ����԰�ȫ����
#pragma warning(push)
#pragma warning(disable : 26495)
#pragma warning(disable : 26498)
#include <spdlog/fmt/ostr.h>  // ֧���Զ����������
#include <spdlog/spdlog.h>
#pragma warning(pop)

namespace mite {

typedef std::shared_ptr<spdlog::logger> Logger;

class LoggerSystem {
 public:
  static void Init();
  static void Shutdown();

  // ������־�ӿ�
  template<typename... Args> static void Trace(const std::string &fmt, Args &&...args)
  {
    GetCoreLogger()->trace(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args> static void Debug(const std::string &fmt, Args &&...args)
  {
    GetCoreLogger()->debug(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args> static void Info(const std::string &fmt, Args &&...args)
  {
    GetCoreLogger()->info(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args> static void Warn(const std::string &fmt, Args &&...args)
  {
    GetCoreLogger()->warn(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args> static void Error(const std::string &fmt, Args &&...args)
  {
    GetCoreLogger()->error(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args> static void Critical(const std::string &fmt, Args &&...args)
  {
    GetCoreLogger()->critical(fmt, std::forward<Args>(args)...);
  }

  // ����ǩ����־�ӿ�
  template<typename... Args>
  static void TraceTag(const std::string &tag, const std::string &fmt, Args &&...args)
  {
    GetCoreLogger()->trace("[{}] {}", tag, fmt::format(fmt, std::forward<Args>(args)...));
  }

  template<typename... Args>
  static void InfoTag(const std::string &tag, const std::string &fmt, Args &&...args)
  {
    GetCoreLogger()->info("[{}] {}", tag, fmt::format(fmt, std::forward<Args>(args)...));
  }

  template<typename... Args>
  static void WarnTag(const std::string &tag, const std::string &fmt, Args &&...args)
  {
    GetCoreLogger()->warn("[{}] {}", tag, fmt::format(fmt, std::forward<Args>(args)...));
  }

  template<typename... Args>
  static void ErrorTag(const std::string &tag, const std::string &fmt, Args &&...args)
  {
    GetCoreLogger()->error("[{}] {}", tag, fmt::format(fmt, std::forward<Args>(args)...));
  }

  template<typename... Args>
  static void CriticalTag(const std::string &tag, const std::string &fmt, Args &&...args)
  {
    GetCoreLogger()->critical("[{}] {}", tag, fmt::format(fmt, std::forward<Args>(args)...));
  }

  // ����ȫ����־����
  static void SetLevel(spdlog::level::level_enum level);

  // ��ȡ����logger
  static std::shared_ptr<spdlog::logger> &GetCoreLogger();

  // ����ģ��ר��logger
  static std::shared_ptr<spdlog::logger> CreateModuleLogger(const std::string &name);

 private:
  static std::shared_ptr<spdlog::logger> s_CoreLogger;
  static std::vector<std::shared_ptr<spdlog::logger>> s_ModuleLoggers;
};

// ��ݺ궨��
#define LOG_TRACE(...) ::mite::LoggerSystem::Trace(__VA_ARGS__)
#define LOG_DEBUG(...) ::mite::LoggerSystem::Debug(__VA_ARGS__)
#define LOG_INFO(...) ::mite::LoggerSystem::Info(__VA_ARGS__)
#define LOG_WARN(...) ::mite::LoggerSystem::Warn(__VA_ARGS__)
#define LOG_ERROR(...) ::mite::LoggerSystem::Error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::mite::LoggerSystem::Critical(__VA_ARGS__)

#define LOG_TRACE_TAG(tag, ...) ::mite::LoggerSystem::TraceTag(tag, __VA_ARGS__)
#define LOG_INFO_TAG(tag, ...) ::mite::LoggerSystem::InfoTag(tag, __VA_ARGS__)
#define LOG_WARN_TAG(tag, ...) ::mite::LoggerSystem::WarnTag(tag, __VA_ARGS__)
#define LOG_ERROR_TAG(tag, ...) ::mite::LoggerSystem::ErrorTag(tag, __VA_ARGS__)
#define LOG_CRITICAL_TAG(tag, ...) ::mite::LoggerSystem::CriticalTag(tag, __VA_ARGS__)

}  // namespace mite
#endif
