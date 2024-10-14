#include <iostream>
#include "com.h"
#include "fsm.h"


int main(int argc, char *argv[])
{
  Com hComBC("localhost", 18304);
  Fsm hBC(hComBC);

  hBC.Run();

  return 0;
}
