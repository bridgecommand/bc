#include "Utilities.hpp"

#include <algorithm>
#include <locale>

#include <iostream> //Fixme: Debug only

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
        return timestampToString(timestamp, "%d %b %Y %I:%M:%S"); //Default date/time format
    }

}
