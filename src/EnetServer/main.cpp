#include <iostream>
#include "com.h"
#include "fsm.h"
#include "../IniFile.hpp"
#include "../Utilities.hpp"

#ifdef _WIN32
#include <windows.h> 
#include <direct.h> 
#endif

namespace IniFile {
    irr::ILogger* irrlichtLogger = 0;
}

int main(int argc, char *argv[])
{
    std::string userFolder = Utilities::getUserDir();
    std::string iniFilename = "bc5.ini";

    if (Utilities::pathExists(userFolder + "bc5.ini"))
    {
        iniFilename = userFolder + "bc5.ini";
    }

    unsigned int enetSrvPort = IniFile::iniFileTou32(iniFilename, "udp_server_port");
    if (enetSrvPort == 0) {
        enetSrvPort = 18304;
    }

    std::string enetSrvAddr = IniFile::iniFileToString(iniFilename, "udp_server_address");
    if (enetSrvAddr.empty()) {
        enetSrvAddr = "localhost";
    }


  Com hComBC(enetSrvAddr, enetSrvPort);
  Fsm hBC(hComBC);

  hBC.Run();

  return 0;
}
