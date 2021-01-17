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

#ifndef __SCENARIOCHOICE_HPP_INCLUDED__
#define __SCENARIOCHOICE_HPP_INCLUDED__

#include "irrlicht.h"
#include "../Lang.hpp"
#include <string>
#include <vector>

class ScenarioChoice
{
public:
    ScenarioChoice(irr::IrrlichtDevice* device, Lang* language);
    void chooseScenario(std::string& scenarioName, std::string& hostname, std::string scenarioPath);

private:
    void getScenarioList(std::vector<std::string>&scenarioList, std::string scenarioPath);
    irr::IrrlichtDevice* device;
    irr::gui::IGUIEnvironment* gui;
    Lang* language;

    enum GUI_ELEMENTS// Define some values that we'll use to identify individual GUI controls.
    {
        GUI_ID_SCENARIO_LISTBOX = 101,
        GUI_ID_OK_BUTTON
    };

};

#endif


