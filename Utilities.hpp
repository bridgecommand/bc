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

namespace Utilities
{
    void to_lower(std::string& toConvert);
    void to_lower(std::wstring& toConvert);
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
