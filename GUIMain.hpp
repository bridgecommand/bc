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

#ifndef __GUIMAIN_HPP_INCLUDED__
#define __GUIMAIN_HPP_INCLUDED__

#include "irrlicht.h"

#include <string>

class GUIMain //Create, build and update GUI
{
public:
    GUIMain(irr::IrrlichtDevice* device);

    enum GUI_ELEMENTS// Define some values that we'll use to identify individual GUI controls.
    {
        GUI_ID_HEADING_SCROLL_BAR = 101,
        GUI_ID_SPEED_SCROLL_BAR,
        GUI_ID_PORT_SCROLL_BAR,
        GUI_ID_STBD_SCROLL_BAR,
        GUI_ID_RUDDER_SCROLL_BAR,
        GUI_ID_START_BUTTON,
        GUI_ID_RADAR_INCREASE_BUTTON,
        GUI_ID_RADAR_DECREASE_BUTTON
    };

    void updateGuiData(irr::f32 hdg, irr::f32 spd, irr::f32 portEng, irr::f32 stbdEng, irr::f32 depth, std::string currentTime, bool paused);

    void drawGUI();


private:

    irr::IrrlichtDevice* device;
    irr::gui::IGUIEnvironment* guienv;


    irr::gui::IGUIScrollBar* hdgScrollbar;
    irr::gui::IGUIScrollBar* spdScrollbar;
    irr::gui::IGUIScrollBar* portScrollbar;
    irr::gui::IGUIScrollBar* stbdScrollbar;
    irr::gui::IGUIScrollBar* rudderScrollbar;
    irr::gui::IGUIStaticText* dataDisplay;
    irr::gui::IGUIButton* pausedButton;
    irr::gui::IGUIButton* increaseRangeButton;
    irr::gui::IGUIButton* decreaseRangeButton;

    irr::f32 guiHeading;
    irr::f32 guiSpeed;
    irr::f32 guiDepth;
    std::string guiTime;
    bool guiPaused;

};

#endif

