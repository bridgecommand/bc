#include "irrlicht.h"
#include <string>

namespace IniFile
{
    std::string iniFileToString(std::string fileName, std::string command);

    //Load unsigned integer from an ini file
    irr::u32 iniFileToul(std::string fileName, std::string command);
}

int main();
