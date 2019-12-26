/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2015 James Packer

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

#ifndef __GUIMAIN_HPP_INCLUDED__
#define __GUIMAIN_HPP_INCLUDED__

#include <string>
#include <vector>

#include "irrlicht.h"

#include "../Lang.hpp"
#include "../HeadingIndicator.h"
#include "PositionDataStruct.hpp"

class GUIMain //Create, build and update GUI
{
public:
    GUIMain(irr::IrrlichtDevice* device, Lang* language, irr::core::stringw message);
    ~GUIMain();

    enum GUI_ELEMENTS// Define some values that we'll use to identify individual GUI controls.
    {
        GUI_ID_HEADING_CHOICE = 101,
        GUI_ID_REPEATER_CHOICE
    };

    void updateGuiData(irr::f32 time, irr::f32 ownShipHeading, irr::f32 rudderAngle, irr::f32 wheelAngle, irr::s32 portEngineRPM, irr::s32 stbdEngineRPM);
    void setMode(bool headingMode);

private:

    Lang* language;

    irr::IrrlichtDevice* device;
    irr::gui::IGUIEnvironment* guienv;

    irr::gui::IGUIStaticText* messageText;
    irr::gui::IGUIButton* headingButton;
    irr::gui::IGUIButton* repeaterButton;

    bool modeChosen;
    bool showHeadingIndicator;

    irr::gui::HeadingIndicator* heading;

    std::wstring f32To3dp(irr::f32 value);
    std::wstring f32To1dp(irr::f32 value);

};

#endif


