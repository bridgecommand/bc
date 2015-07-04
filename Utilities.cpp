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

#include "Utilities.hpp"

#include <algorithm>
#include <locale>
#include <sstream>

namespace Utilities
{
    void to_lower(std::string& toConvert) {
        //A simple version of boost::to_lower.
        //ToDo: Test effect on internationalisation

        //convert to lower case
        //std::transform(toConvert.begin(), toConvert.end(), toConvert.begin(),
        //               std::bind2nd(std::ptr_fun(&std::tolower<char>), std::locale("")));

        std::transform(toConvert.begin(), toConvert.end(), toConvert.begin(), ::tolower);

    }

    void to_lower(std::wstring& toConvert) {
        //A simple version of boost::to_lower.
        //ToDo: Test effect on internationalisation

        //convert to lower case
        //std::transform(toConvert.begin(), toConvert.end(), toConvert.begin(),
        //               std::bind2nd(std::ptr_fun(&std::tolower<char>), std::locale("")));

        std::transform(toConvert.begin(), toConvert.end(), toConvert.begin(), ::tolower);

    }

    std::string trim(std::string inString) {
        //Based on http://codereview.stackexchange.com/questions/40124/trim-white-space-from-string, Loki Astari answer
        if(inString.empty()) {
            return inString;
        }

        std::size_t firstScan = inString.find_first_not_of(" \f\n\r\t\v");
        std::size_t first     = firstScan == std::string::npos ? inString.length() : firstScan;
        std::size_t last      = inString.find_last_not_of(" \f\n\r\t\v");
        return inString.substr(first, last-first+1);
    }

    signed int round(float numberIn) {
        //Implements round away from zero

        signed int result;
        if (numberIn > 0) {
            result = numberIn + 0.5;
        } else {
            result = numberIn - 0.5;
        }
        return result;
    }

    time_t dmyToTimestamp(int day, int month, int year) {
        tm timeInfo;
        timeInfo.tm_year = year-1900;
        timeInfo.tm_mon = month-1;
        timeInfo.tm_mday = day;
        timeInfo.tm_hour=0; timeInfo.tm_min=0; timeInfo.tm_sec=0; timeInfo.tm_isdst=0; //Set all others to zero
        return(mktime(&timeInfo));
    }

    std::string timestampToString(time_t timestamp, std::string format) {
        char buffer[80];
        tm * timeinfo;

        timeinfo = gmtime(&timestamp);
        strftime(buffer,80,format.c_str(),timeinfo);
        std::string returnString(buffer);
        return(returnString);
    }

    std::string timestampToString(time_t timestamp) {
        return timestampToString(timestamp, "%d %b %Y %H:%M:%S"); //Default date/time format
    }

    std::vector<std::string> split(const std::string &inputString, char delim) {
        //from http://stackoverflow.com/questions/236129/split-a-string-in-c, Evan Teran answer
        std::vector<std::string> splitStrings;

        std::stringstream ss(inputString);
        std::string item;
        while (std::getline(ss, item, delim)) {
            splitStrings.push_back(item);
        }
        return splitStrings;
    }

}
