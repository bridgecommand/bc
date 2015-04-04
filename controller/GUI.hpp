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
#include "PositionDataStruct.hpp"
#include "OtherShipDataStruct.hpp"

class GUIMain //Create, build and update GUI
{
public:
    GUIMain(irr::IrrlichtDevice* device, Lang* language);

    enum GUI_ELEMENTS// Define some values that we'll use to identify individual GUI controls.
    {
        GUI_ID_HEADING_SCROLL_BAR = 101,
        GUI_ID_SPEED_SCROLL_BAR
    };

    void updateGuiData(irr::f32 ownShipPosX, irr::f32 ownShipPosZ, const std::vector<PositionData>& buoys, const std::vector<OtherShipData>& otherShips, irr::video::ITexture* displayMapTexture);

private:

    Lang* language;

    irr::IrrlichtDevice* device;
    irr::gui::IGUIEnvironment* guienv;

    irr::gui::IGUIStaticText* dataDisplay;
//  irr::gui::IGUITreeView* dataTree;

};

#endif


