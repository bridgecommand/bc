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

#include <fstream> //for ini loading
#include <string> //for ini loading
#include <iostream>
#include "Utilities.hpp" //for ini loading
#ifndef _WIN32
#include <codecvt> //For UTF-8 reading
#endif // _WIN32

// Irrlicht Namespaces
using namespace irr;

//Utility functions
namespace IniFile
{
    std::string enumerate1(std::string commandName, irr::s32 number)
    //Build up a command in the format 'commandName(#)'
    {
        std::string ans = commandName;
        ans.append("(");
        ans.append(Utilities::lexical_cast<std::string>(number));
        ans.append(")");

        return ans;
    }

    std::string enumerate2(std::string commandName, irr::s32 number1, irr::s32 number2)
    //Build up a command in the format 'commandName(#,#)'
    {
        std::string ans = commandName;
        ans.append("(");
        ans.append(Utilities::lexical_cast<std::string>(number1));
        ans.append(",");
        ans.append(Utilities::lexical_cast<std::string>(number2));
        ans.append(")");

        return ans;
    }

    std::string trim(const std::string& str, const std::string& trimChr=" \n\r\t")
    {
        const std::size_t strBegin = str.find_first_not_of(trimChr);
        if (strBegin == std::string::npos)
            return ""; // no content
        const std::size_t strEnd = str.find_last_not_of(trimChr);
        const std::size_t strRange = strEnd - strBegin + 1;

        return str.substr(strBegin, strRange);
    }

    std::wstring trim(const std::wstring& str, const std::wstring& trimChr=L" \n\r\t")
    {
        const std::size_t strBegin = str.find_first_not_of(trimChr);
        if (strBegin == std::wstring::npos)
            return L""; // no content
        const std::size_t strEnd = str.find_last_not_of(trimChr);
        const std::size_t strRange = strEnd - strBegin + 1;

        return str.substr(strBegin, strRange);
    }

    std::string iniFileToString(std::string fileName, std::string command)
    {
        std::ifstream file (fileName.c_str());

        //Set UTF-8 on Linux/OSX etc
        #ifndef _WIN32
        try {
            #ifdef __APPLE__
            char* thisLocale = setlocale(LC_ALL, "");
            if (thisLocale) {
                file.imbue(std::locale(thisLocale));
            }
            #else
            file.imbue(std::locale("en_US.UTF8"));
            #endif
        } catch (const std::runtime_error& runtimeError) {
            file.imbue(std::locale(""));
        }
        #endif

        std::string valuePart = "";
        if (file.is_open())
        {
            std::string line;
            while ( std::getline (file,line) )
            {
                std::string lowerLine(line); //Duplicate the line so we can make it lowercase, without affecting the result
                Utilities::to_lower(lowerLine);
                Utilities::to_lower(command); //Make the command lowercase, so we can check this without case sensitivity

                //Look for the command and the '=' sign in the line
                std::size_t commandFound = lowerLine.find(command);
                std::size_t equalsFound = lowerLine.find("=");

                //If both found, with equals after the command
                if (commandFound!=std::string::npos && equalsFound!=std::string::npos && equalsFound>commandFound)
                {
                    //Check that the part before the '=' exactly matches the command
                    if (trim(lowerLine.substr(0,equalsFound))==trim(command))
                    {
                        //get the value
                        //try {
                            valuePart = line.substr(equalsFound+1,std::string::npos);//from equals to end, not including the equals
                        //}
                        //catch (const std::out_of_range& oor) {
                        //    std::cerr << "Could not get value for: " << command << " " << oor.what() << '\n';
                        //}
                    }
                }

            }
            file.close();
        } else {
            if (irrlichtLogger) {
                std::string logMessage = "Unable to open file: ";
                logMessage.append(fileName);
                irrlichtLogger->log(logMessage.c_str());
            } else {
                std::cerr << "Unable to open file " << fileName << std::endl;
            }
        }//

        //trim whitespace and " characters if present
        valuePart = trim(valuePart);
        valuePart = trim(valuePart,"\"");
        return valuePart;
    }

    std::wstring iniFileToWString(std::string fileName, std::wstring command)
    {
        std::wifstream file (fileName.c_str());

        //Set UTF-8 on Linux/OSX etc
        #ifndef _WIN32
        try {
            #ifdef __APPLE__
            char* thisLocale = setlocale(LC_ALL, "");
            if (thisLocale) {
                file.imbue(std::locale(thisLocale));
            }
            #else
            file.imbue(std::locale("en_US.UTF8"));
            #endif
        } catch (const std::runtime_error& runtimeError) {
            file.imbue(std::locale(""));
        }
        #endif

        std::wstring valuePart = L"";
        if (file.is_open())
        {
            std::wstring line;
            while ( std::getline (file,line) )
            {
                std::wstring lowerLine(line); //Duplicate the line so we can make it lowercase, without affecting the result
                Utilities::to_lower(lowerLine);
                Utilities::to_lower(command); //Make the command lowercase, so we can check this without case sensitivity

                //Look for the command and the '=' sign in the line
                std::size_t commandFound = lowerLine.find(command);
                std::size_t equalsFound = lowerLine.find(L"=");

                //If both found, with equals after the command
                if (commandFound!=std::string::npos && equalsFound!=std::string::npos && equalsFound>commandFound)
                {
                    //Check that the part before the '=' exactly matches the command
                    if (trim(lowerLine.substr(0,equalsFound))==trim(command))
                    {
                        //get the value
                        //try {
                            valuePart = line.substr(equalsFound+1,std::string::npos);//from equals to end, not including the equals
                        //}
                        //catch (const std::out_of_range& oor) {
                        //    std::cerr << "Could not get value for: " << command << " " << oor.what() << '\n';
                        //}
                    }
                }

            }
            file.close();
        } else {
            if (irrlichtLogger) {
                std::string logMessage = "Unable to open file: ";
                logMessage.append(fileName);
                irrlichtLogger->log(logMessage.c_str());
            } else {
                std::cerr << "Unable to open file " << fileName << std::endl;
            }
        }


        //trim whitespace and " characters if present
        valuePart = trim(valuePart);
        valuePart = trim(valuePart,L"\"");
        return valuePart;
    }

    //Load unsigned integer from an ini file
    u32 iniFileTou32(std::string fileName, std::string command)
    {
        u32 result = 0;
        std::string valueString = iniFileToString(fileName, command); //Get the value as a string
        result = core::strtoul10(valueString.c_str()); //Convert this into an unsigned int
        return result;
    }

    //Load float from an ini file
    f32 iniFileTof32(std::string fileName, std::string command)
    {
        f32 result = 0;
        std::string valueString = iniFileToString(fileName, command); //Get the value as a string
        result = core::fast_atof(valueString.c_str()); //Convert this into a float
        return result;
    }

}


