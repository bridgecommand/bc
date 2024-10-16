#include "thread.h"
#include "com.h"
#include <enet/enet.h>

void *TaskLaunchServer(void *aArg)
{
  Fsm *hBC = (Fsm*)aArg;

  hBC->Run();

  pthread_exit(NULL);
}

int CreateThread(pthread_t* aTask, Fsm* aFsm)
{
  int ret = 0;

  std::cout << "Creating thread :: Lauch Enet Server" << std::endl;
  ret = pthread_create(aTask, NULL, &TaskLaunchServer, aFsm);

  return ret;
}
