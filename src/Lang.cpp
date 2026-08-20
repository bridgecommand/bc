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

#include <iostream>
#include "Lang.hpp"
#include "IniFile.hpp"

#ifdef _WIN32
// For UTF8 to ANSI conversion
#include <codecvt>
#include <locale>
#include <vector>
#endif // _WIN32

//using namespace irr;

Lang::Lang(std::string language)
{
    languageFileName = language;
}

irr::core::stringw Lang::translate(std::string phraseName)
{
    //Look up
    std::string translatedPhrase = IniFile::iniFileToString(languageFileName, phraseName);
    //Fall back
    if (translatedPhrase=="") {
        if (phraseName == "deg") {
            //FIXME: Temp fix for the degree symbol, while utf-8 isn't properly sorted on all platforms
            wchar_t degSymbolChar = 176;
            irr::core::stringw degSymbolString = L"";
            degSymbolString.append(degSymbolChar);
            return degSymbolString;
        } else {
            std::cout << "Translation for " << phraseName << " not found in " << languageFileName << std::endl;
            translatedPhrase = phraseName;
        }
    }

    //Convert '\n' characters within string to a newline - based on http://stackoverflow.com/a/24315631
    size_t start_pos = 0;
    std::string from = "\\n";
    std::string to = "\n";
    while((start_pos = translatedPhrase.find(from, start_pos)) != std::string::npos) {
        translatedPhrase.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }

#ifdef _WIN32
    // If Windows, convert the UTF8 string to ANSI:
    std::wstring_convert<std::codecvt_utf8<wchar_t>> wconv;
    std::wstring wstr = wconv.from_bytes(translatedPhrase);
    // wstring to string
    std::vector<char> buf(wstr.size());
    std::use_facet<std::ctype<wchar_t>>(std::locale(".1252")).narrow(wstr.data(), wstr.data() + wstr.size(), '?', buf.data());
    translatedPhrase = std::string(buf.data(), buf.size());
#endif

    //convert to stringw
    irr::core::stringw returnPhrase;
    irr::core::multibyteToWString(returnPhrase, translatedPhrase.c_str());

    return returnPhrase;
}
