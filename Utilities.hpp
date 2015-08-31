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

namespace Utilities
{
    void to_lower(std::string& toConvert);
    void to_lower(std::wstring& toConvert);
    signed int round(float numberIn);
    time_t dmyToTimestamp(int day, int month, int year);
    std::string timestampToString(time_t timestamp, std::string format);
    std::string timestampToString(time_t timestamp);
    std::string trim(std::string inString);
    std::vector<std::string> split(const std::string &inputString, char delim);
    std::string getUserDir(); //Returns the directory path (absolute, with trailing slash) for a user read/writable directory
    bool pathExists(std::string filePath);

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
            var = std::numeric_limits<float>::infinity();
        } else if (
            in.compare("-inf")==0 ||
            in.compare("-INF")==0 ||
            in.compare("-infinity")==0 ||
            in.compare("-INFINITY")==0
        ) {
           //-inf
           var = -std::numeric_limits<float>::infinity();;
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
