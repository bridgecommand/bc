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

#ifndef __UTILITIES_HPP_INCLUDED__
#define __UTILITIES_HPP_INCLUDED__

//General utility methods

#include <string>
#include <sstream>
#include <ctime>
#include <vector>
#include <limits>
#include <type_traits>

//#include "ScenarioDataStructure.hpp"
//Forward declaration
class ScenarioData;

namespace Utilities
{
    void to_lower(std::string& toConvert);
    void to_lower(std::wstring& toConvert);
    signed int round(float numberIn);
    time_t dmyToTimestamp(int day, int month, int year);

    std::string timestampToString(time_t timestamp, std::string format);
    std::string timestampToString(time_t timestamp);
    std::string ttos(time_t timestamp);

    std::string trim(std::string inString, std::string trimChrs = " \f\n\r\t\v");
    std::wstring trim(std::wstring inString, std::wstring trimChrs = L" \f\n\r\t\v");

    std::vector<std::string> split(const std::string &inputString, char delim);
    std::string getUserDirBase(); //Returns the directory path (absolute, with trailing slash) for a user read/writable directory, the first level folder in the user's filesystem (eg %appdata%/Bridge Command/ on windows)
    std::string getUserDir(); //Returns the directory path (absolute, with trailing slash) for a user read/writable directory (eg %appdata%/Bridge Command/VERSIONUMBER/ on windows)
    bool pathExists(std::string filePath);
    ScenarioData getScenarioDataFromFile(std::string scenarioPath, std::string scenarioName); //Read a scenario from ini files
    bool hasEnding(std::string const &fullString, std::string const &ending);

    template <typename T>
    T lexical_cast(std::string in) //Special case for string so we can check for inf
    {
        T var;
        if (in.compare("inf")==0 ||
            in.compare("INF")==0 ||
            in.compare("infinity")==0 ||
            in.compare("INFINITY")==0 ||
            in.compare("+inf")==0 ||
            in.compare("+INF")==0 ||
            in.compare("+infinity")==0 ||
            in.compare("+INFINITY")==0
        ) {
            //+inf
            if (std::is_floating_point<T>::value) {
				var = (T)std::numeric_limits<float>::infinity();
            } else {
				//var = 1000000000; //FIXME: Temporary fix
				var = (std::numeric_limits<T>::max)();
            }
        } else if (
            in.compare("-inf")==0 ||
            in.compare("-INF")==0 ||
            in.compare("-infinity")==0 ||
            in.compare("-INFINITY")==0
        ) {
           //-inf
			//var = -1000000000; //FIXME: Temporary fix
			if (std::is_floating_point<T>::value) {
                var = (T)std::numeric_limits<float>::infinity();
				var = -1.0 * var;
            } else {
                var = std::numeric_limits<T>::lowest();
            }
        } else {
            std::stringstream iss;
            iss << in;
            iss >> var;
            // FIXME: deal with any error bits that may have been set on the stream
        }
        return var;
    }

    template <typename T, typename U>
    T lexical_cast(U in)
    {
        T var;
        std::stringstream iss;
        iss << in;
        iss >> var;
        // FIXME: deal with any error bits that may have been set on the stream
        return var;
    }

}

#endif
