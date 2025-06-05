#include "application.h"

#ifdef _WIN32
#include <crtdbg.h>
#endif

int main(int argc, char **argv)
{
#ifdef _DEBUG
  // ��Windowsƽ̨�������ڴ�й©���
  // ����ģʽ�¼���ڴ�й©
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  // ��ʼ����־ϵͳ
  mite::LoggerSystem::Init();
  LOG_INFO("Starting Mite Engine");

  // TODO: ���������в���


  // ���г���
  try {
    auto app = std::make_unique<mite::MiteApplication>();
    app->run();
  }
  catch (const std::exception &e) {
    LOG_CRITICAL("Application crashed: {}", e.what());
    return EXIT_FAILURE;
  }

  // ������־ϵͳ
  LOG_INFO("Application exited successfully");
  mite::LoggerSystem::Shutdown();

  return 0;
}
