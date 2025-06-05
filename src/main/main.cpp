#include "application.h"

#ifdef _WIN32
#include <crtdbg.h>
#endif

int main(int argc, char **argv)
{
#ifdef _DEBUG
  // 在Windows平台上启用内存泄漏检测
  // 调试模式下检查内存泄漏
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  // 初始化日志系统
  mite::LoggerSystem::Init();
  LOG_INFO("Starting Mite Engine");

  // TODO: 处理命令行参数


  // 运行程序
  try {
    auto app = std::make_unique<mite::MiteApplication>();
    app->run();
  }
  catch (const std::exception &e) {
    LOG_CRITICAL("Application crashed: {}", e.what());
    return EXIT_FAILURE;
  }

  // 结束日志系统
  LOG_INFO("Application exited successfully");
  mite::LoggerSystem::Shutdown();

  return 0;
}
