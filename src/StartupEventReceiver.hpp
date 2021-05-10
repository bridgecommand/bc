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

#ifndef __STARTUPEVENTRECEIVER_HPP_INCLUDED__
#define __STARTUPEVENTRECEIVER_HPP_INCLUDED__

#include "irrlicht.h"

class StartupEventReceiver : public irr::IEventReceiver
{
public:

    StartupEventReceiver(irr::gui::IGUIListBox* scenarioListBox, irr::gui::IGUIStaticText* scenarioText, irr::gui::IGUIStaticText* hostnameText, irr::gui::IGUIEditBox* hostnameBox, irr::gui::IGUICheckBox* secondaryBox, irr::gui::IGUICheckBox* multiplayerBox, irr::gui::IGUIStaticText* portText, irr::gui::IGUIEditBox* portBox, irr::gui::IGUIStaticText* description, irr::s32 listBoxID, irr::s32 okButtonID, irr::s32 secondaryBoxID, irr::s32 multiplayerBoxID, irr::IrrlichtDevice* dev);
    bool OnEvent(const irr::SEvent& event);

    irr::s32 getScenarioSelected() const;

private:

    irr::IrrlichtDevice* device;
    irr::gui::IGUIStaticText* description;
    irr::gui::IGUIListBox* scenarioListBox;
    irr::gui::IGUIStaticText* hostnameText;
    irr::gui::IGUIStaticText* portText;
    irr::gui::IGUIStaticText* scenarioText;
    irr::gui::IGUIEditBox* hostnameBox;
    irr::gui::IGUIEditBox* portBox;
    irr::gui::IGUICheckBox* secondaryBox;
    irr::gui::IGUICheckBox* multiplayerBox;
    irr::s32 listBoxID;
    irr::s32 okButtonID;
    irr::s32 secondaryBoxID;
    irr::s32 multiplayerBoxID;
    irr::s32 scenarioSelected;

};

#endif

