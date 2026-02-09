/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2014 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#ifndef __INIFILE_HPP_INCLUDED__
#define __INIFILE_HPP_INCLUDED__

#include <cstdint>
#include <string>

// Forward declarations
namespace irr { class ILogger; }

namespace IniFile
{
    extern irr::ILogger* irrlichtLogger;

    std::string enumerate1(std::string commandName, int32_t number);
    std::string enumerate2(std::string commandName, int32_t number1, int32_t number2);

    std::string iniFileToString(const std::string &fileName, const std::string &key, const std::string &defValue = "");
    std::wstring iniFileToWString(const std::string &fileName, const std::string &key, const std::wstring &defValue = L"");

    uint32_t iniFileTou32(const std::string &fileName, const std::string &key, uint32_t defValue = 0);
    int32_t iniFileTos32(const std::string &fileName, const std::string &key, int32_t defValue = 0);
    float iniFileTof32(const std::string &fileName, const std::string &key, float defValue = 0.f);
}

#endif
