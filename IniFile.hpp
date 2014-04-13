#ifndef __INIFILE_HPP_INCLUDED__
#define __INIFILE_HPP_INCLUDED__

#include "irrlicht.h"
#include <string>

namespace IniFile
{
    std::string enumerate1(std::string commandName, irr::u32 number);
    std::string enumerate2(std::string commandName, irr::u32 number1, irr::u32 number2);
    std::string iniFileToString(std::string fileName, std::string command);
    irr::u32 iniFileTou32(std::string fileName, std::string command);
    irr::f32 iniFileTof32(std::string fileName, std::string command);
}

#endif
