#ifndef __UTILITIES_HPP_INCLUDED__
#define __UTILITIES_HPP_INCLUDED__

//General utility methods

#include <string>
#include <sstream>
#include <ctime>

namespace Utilities
{
    void to_lower(std::string& toConvert);
    signed int round(float numberIn);
    time_t dmyToTimestamp(int day, int month, int year);
    std::string timestampToString(time_t timestamp, std::string format);
    std::string timestampToString(time_t timestamp);

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
