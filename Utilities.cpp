#include "Utilities.hpp"

#include <algorithm>
#include <locale>

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
}
