#ifndef __UTILITIES_HPP_INCLUDED__
#define __UTILITIES_HPP_INCLUDED__

//General utility methods

#include <string>
#include <sstream>

namespace Utilities
{
    void to_lower(std::string& toConvert);

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
