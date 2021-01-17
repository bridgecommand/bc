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

#include "Lang.hpp"

#include "IniFile.hpp"

//using namespace irr;

Lang::Lang(std::string language)
{
    languageFileName = language;
}

irr::core::stringw Lang::translate(std::string phraseName)
{
    //Look up
    std::wstring translatedPhrase = IniFile::iniFileToWString(languageFileName, phraseName);
    //Fall back
    if (translatedPhrase==L"") {
        translatedPhrase = std::wstring(phraseName.begin(), phraseName.end());
        //FIXME: Temp fix for the degree symbol, while utf-8 isn't properly sorted on all platforms
        if (phraseName == "deg") {
            translatedPhrase = L"°";
        }
    }

    //Convert '\n' characters within string to a newline - based on http://stackoverflow.com/a/24315631
    size_t start_pos = 0;
    std::wstring from = L"\\n";
    std::wstring to = L"\n";
    while((start_pos = translatedPhrase.find(from, start_pos)) != std::wstring::npos) {
        translatedPhrase.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }

    //convert to stringw
    irr::core::stringw returnPhrase(translatedPhrase.c_str());

    return returnPhrase;
}
