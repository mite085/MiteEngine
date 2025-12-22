#include "application.h"

#ifdef _WIN32
#include <crtdbg.h>
#endif

int main()
{
#ifdef _DEBUG
  // 在Windows平台上启用内存泄漏检测
  // 调试模式下检查内存泄漏
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  // 调适界面出现内存泄露时，使用该宏，按照泄漏分配号填写参数
  // 例如出现 {694} normal block at 0x00……,则填写694
  // 这会在分配这块内存时触发断点
  // 
  // _CrtSetBreakAlloc(694);
#endif

  // 初始化日志系统
  mite::LoggerSystem::Initialize();
  // 初始化文件系统（需要在日志系统之后）
  mite::FileSystem::Initialize();
  // 初始化线程池
  mite::ThreadPoolManager::Initialize();
  LOG_INFO("Starting Mite Engine");

  // 运行程序
  try {
    auto app = std::make_unique<mite::MiteApplication>();
    app->run();
  }
  catch (const std::exception &e) {
    LOG_CRITICAL("Application crashed: {}", e.what());
    return EXIT_FAILURE;
  }

  // 关闭线程池
  mite::ThreadPoolManager::Shutdown();
  // 结束日志系统
  LOG_INFO("Application exited successfully");
  mite::LoggerSystem::Shutdown();
  return 0;
}
