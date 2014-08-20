#ifndef __SCENARIOCHOICE_HPP_INCLUDED__
#define __SCENARIOCHOICE_HPP_INCLUDED__

#include "irrlicht.h"
#include <string>
#include <vector>

class ScenarioChoice
{
public:
    ScenarioChoice(irr::IrrlichtDevice* device);
    std::string chooseScenario();

private:
    void getScenarioList(std::vector<std::string>&scenarioList, std::string scenarioPath);
    irr::IrrlichtDevice* device;

};

#endif


