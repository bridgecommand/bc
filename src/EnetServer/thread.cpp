#include "thread.h"
#include "com.h"
#include <enet/enet.h>
#include <thread>

static void TaskLaunchServer(Fsm* aFsm)
{
  std::cout << "OK 1" << std::endl;
  aFsm->Run();
}

int CreateThread(Fsm* aFsm)
{
  int ret = 0;
  
  std::cout << "Creating thread :: Launch Enet Server" << std::endl;
  std::thread taskServer(TaskLaunchServer, aFsm);

  taskServer.detach();

  return ret;
}
