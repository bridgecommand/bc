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

#include "IniFile.hpp"

#include <cassert>
#include <fstream> //for ini loading
#include <string> //for ini loading
#include <iostream>
#include <map>

#include "Utilities.hpp" //for ini loading
#ifndef _WIN32
#include <codecvt> //For UTF-8 reading
#endif // _WIN32

// Irrlicht Namespaces
//using namespace irr;

class IniCache
{
public:
    IniCache() = default;

    std::string getStringValue(const std::string &fileName, const std::string &key, const std::string &defValue = "");
    std::wstring getWStringValue(const std::string &fileName, const std::string &key, const std::wstring &defValue = L"");

    irr::u32 getUIntValue(const std::string &fileName, const std::string &key, irr::u32 defValue = 0);
    irr::s32 getSIntValue(const std::string &fileName, const std::string &key, irr::s32 defValue = 0);
    irr::f32 getFloatValue(const std::string &fileName, const std::string &key, irr::f32 defValue = 0.f);

private:
    bool readFile(const std::string &fileName);

    bool readWFile(const std::string &fileName);

private:
    std::map<std::string, std::map<std::string,  std::string>>  m_stringData;
    std::map<std::string, std::map<std::wstring, std::wstring>> m_wstringData;
};


static IniCache g_iniCache;


bool IniCache::readFile(const std::string& fileName)
{
    if (m_stringData.find(fileName) != m_stringData.end()) {
        return true; // file already read
    }

    std::ifstream file (fileName.c_str());

    //Set UTF-8 on Linux/OSX etc
#ifndef _WIN32
    try {
#  ifdef __APPLE__
        char* thisLocale = setlocale(LC_ALL, "");
        if (thisLocale) {
            file.imbue(std::locale(thisLocale));
        }
#  else
        file.imbue(std::locale("en_US.UTF8"));
#  endif
    } catch (const std::runtime_error& runtimeError) {
        file.imbue(std::locale(""));
    }
#endif

    if (!file.is_open()) {
        if (IniFile::irrlichtLogger) {
            std::string logMessage = "Unable to open file: ";
            logMessage.append(fileName);
            IniFile::irrlichtLogger->log(logMessage.c_str());
        }
        else {
            std::cerr << "Unable to open file " << fileName << std::endl;
        }
        return false;
    }

    std::string line;
    while ( std::getline (file,line) )
    {
        const std::size_t equalsPos = line.find_first_of("=");
        if (equalsPos != std::string::npos) {
            std::string key   = Utilities::trim(line.substr(0, equalsPos));
            std::string value = Utilities::trim(line.substr(equalsPos+1, std::string::npos));

            Utilities::to_lower(key);
            value = Utilities::trim(value, "\"");

            m_stringData[fileName][key] = value;
        }
    }

    file.close();
    return true;
}


bool IniCache::readWFile(const std::string& fileName)
{
    if (m_stringData.find(fileName) != m_stringData.end()) {
        return true; // file already read
    }

    std::wifstream file(fileName.c_str());

    //Set UTF-8 on Linux/OSX etc
    #ifndef _WIN32
    try {
        #  ifdef __APPLE__
        char* thisLocale = setlocale(LC_ALL, "");
        if (thisLocale) {
            file.imbue(std::locale(thisLocale));
        }
        #  else
        file.imbue(std::locale("en_US.UTF8"));
        #  endif
    } catch (const std::runtime_error& runtimeError) {
        file.imbue(std::locale(""));
    }
    #endif

    if (!file.is_open()) {
        if (IniFile::irrlichtLogger) {
            std::string logMessage = "Unable to open file: ";
            logMessage.append(fileName);
            IniFile::irrlichtLogger->log(logMessage.c_str());
        }
        else {
            std::cerr << "Unable to open file " << fileName << std::endl;
        }
        return false;
    }

    std::wstring line;
    while ( std::getline (file,line) )
    {
        const std::size_t equalsPos = line.find_first_of(L"=");
        if (equalsPos != std::wstring::npos) {
            std::wstring key   = Utilities::trim(line.substr(0, equalsPos));
            std::wstring value = Utilities::trim(line.substr(equalsPos+1, std::wstring::npos));

            Utilities::to_lower(key);
            value = Utilities::trim(value, L"\"");

            m_wstringData[fileName][key] = value;
        }
    }

    file.close();
    return true;
}


std::string IniCache::getStringValue(const std::string& fileName, const std::string& key, const std::string& defValue)
{
    std::string keyLow = key;
    Utilities::to_lower(keyLow);

    auto fileIt = m_stringData.find(fileName);
    if (fileIt == m_stringData.end()) {
        if (!readFile(fileName)) {
            //file not found
            return defValue;
        }
        fileIt = m_stringData.find(fileName);
        assert(fileIt != m_stringData.end());
    }

    const auto keyIt = fileIt->second.find(keyLow);
    if (keyIt == (fileIt->second).end()) {
        return defValue;
    }

    return keyIt->second;
}

std::wstring IniCache::getWStringValue(const std::string& fileName, const std::string& key, const std::wstring& defValue)
{
    std::wstring keyLow = std::wstring(key.begin(), key.end());
    Utilities::to_lower(keyLow);

    auto fileIt = m_wstringData.find(fileName);
    if (fileIt == m_wstringData.end()) {
        if (!readWFile(fileName)) {
            // file not found
            return defValue;
        }
        fileIt = m_wstringData.find(fileName);
        assert(fileIt != m_wstringData.end());
    }

    const auto keyIt = fileIt->second.find(keyLow);
    if (keyIt == (fileIt->second).end()) {
        return defValue;
    }

    return keyIt->second;

}

irr::u32 IniCache::getUIntValue(const std::string& fileName, const std::string& key, irr::u32 defValue)
{
    const std::string value = getStringValue(fileName, key, "");
    if (value == "") {
        return defValue; // not found
    }

    const char *val = value.c_str();
    const char *end = nullptr;
    irr::u32 result = irr::core::strtoul10(val, &end);
    if (end - val == value.length()) {
        return result;
    }
    else {
        return defValue; // failed to parse value
    }
}

irr::s32 IniCache::getSIntValue(const std::string& fileName, const std::string& key, irr::s32 defValue)
{
    const std::string value = getStringValue(fileName, key, "");
    if (value == "") {
        return defValue; // not found
    }

    const char *val = value.c_str();
    const char *end = nullptr;
    irr::s32 result = irr::core::strtol10(val, &end);
    if (end - val == value.length()) {
        return result;
    }
    else {
        return defValue; // failed to parse value
    }
}

irr::f32 IniCache::getFloatValue(const std::string& fileName, const std::string& key, irr::f32 defValue)
{
    const std::string value = getStringValue(fileName, key, "");
    if (value == "") {
        return defValue; // not found
    }

    const char *val = value.c_str();
    const char *end = nullptr;
    irr::f32 result = irr::core::fast_atof(val, &end);
    if (end - val == value.length()) {
        return result;
    }
    else {
        return defValue; // failed to parse value
    }
}


//Utility functions
namespace IniFile
{
    //Build up a command in the format 'key(#)'
    std::string enumerate1(std::string key, irr::s32 number)
    {
        return key + "(" + std::to_string(number) + ")";
    }

    //Build up a command in the format 'key(#,#)'
    std::string enumerate2(std::string key, irr::s32 number1, irr::s32 number2)
    {
        return key + "(" + std::to_string(number1) + "," + std::to_string(number2) + ")";
    }

    std::string iniFileToString(const std::string &fileName, const std::string &key, const std::string &defValue)
    {
        return g_iniCache.getStringValue(fileName, key, defValue);
    }

    std::wstring iniFileToWString(const std::string &fileName, const std::string &key, const std::wstring &defValue)
    {
        return g_iniCache.getWStringValue(fileName, key, defValue);
    }

    //Load unsigned integer from an ini file
    irr::u32 iniFileTou32(const std::string &fileName, const std::string &key, irr::u32 defValue)
    {
        return g_iniCache.getUIntValue(fileName, key, defValue);
    }

    //Load signed integer from an ini file
    irr::s32 iniFileTos32(const std::string &fileName, const std::string &key, irr::s32 defValue)
    {
        return g_iniCache.getSIntValue(fileName, key, defValue);
    }

    //Load float from an ini file
    irr::f32 iniFileTof32(const std::string &fileName, const std::string &key, irr::f32 defValue)
    {
        return g_iniCache.getFloatValue(fileName, key, defValue);
    }

}


