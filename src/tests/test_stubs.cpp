// Stubs for external symbols needed by BC source files in test builds
#include "IniFile.hpp"
#include "irrlicht.h"

// IniFile.cpp uses this extern for logging; set to null for tests
namespace IniFile {
    irr::ILogger* irrlichtLogger = nullptr;
}

// fast_atof.h references this symbol from Irrlicht.cpp; provide it for tests
namespace irr { namespace core {
    irr::core::stringc LOCALE_DECIMAL_POINTS(".");
}}
