#ifndef __TIDE_HPP_INCLUDED__
#define __TIDE_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>
#include <string>

class Tide {

public:
    Tide();
    virtual ~Tide();
    void load(const std::string& worldName);
    void update(irr::f32 scenarioTime);
    irr::f32 getTideHeight();

private:
    irr::f32 tideHeight;


};

#endif // __TIDE_HPP_INCLUDED__

