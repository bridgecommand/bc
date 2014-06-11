#ifndef __LEXICAL_CAST_HPP_INCLUDED__
#define __LEXICAL_CAST_HPP_INCLUDED__

//Simple implementation similar to boost::lexical_casts

#include <string>
#include <sstream>

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

#endif
