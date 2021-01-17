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

#include "StartupEventReceiver.hpp"

#include <iostream>

//using namespace irr;

    StartupEventReceiver::StartupEventReceiver(irr::gui::IGUIListBox* scenarioListBox, irr::gui::IGUIListBox* worldListBox, irr::s32 scenarioListBoxID, irr::s32 worldListBoxID, irr::s32 okScenarioButtonID, irr::s32 okWorldButtonID) //Constructor
	{
		this->scenarioListBox = scenarioListBox;
		this->worldListBox = worldListBox;
		this->scenarioListBoxID = scenarioListBoxID;
		this->worldListBoxID = worldListBoxID;
		this->okScenarioButtonID = okScenarioButtonID;
		this->okWorldButtonID = okWorldButtonID;
		scenarioSelected = -1; //Set as initially invalid
		worldSelected = -1; //Set as initially invalid
	}

    bool StartupEventReceiver::OnEvent(const irr::SEvent& event)
	{
        if (event.EventType == irr::EET_GUI_EVENT)
		{
			irr::s32 id = event.GUIEvent.Caller->getID();
			//If OK button, or double click on list, for scenario
            if ( (event.GUIEvent.EventType==irr::gui::EGET_BUTTON_CLICKED && id == okScenarioButtonID ) || (event.GUIEvent.EventType==irr::gui::EGET_LISTBOX_SELECTED_AGAIN  && id == scenarioListBoxID ) )
            {
                if (scenarioListBox->getSelected() > -1 ) {
                    scenarioSelected = scenarioListBox->getSelected();
                }
            }

            //If OK button, or double click on list, for world
            if ( (event.GUIEvent.EventType==irr::gui::EGET_BUTTON_CLICKED && id == okWorldButtonID ) || (event.GUIEvent.EventType==irr::gui::EGET_LISTBOX_SELECTED_AGAIN  && id == worldListBoxID ) )
            {
                if (worldListBox->getSelected() > -1 ) {
                    worldSelected = worldListBox->getSelected();
                }
            }


		}

        return false;
    }

    irr::s32 StartupEventReceiver::getScenarioSelected() const
    {
        return scenarioSelected;
    }

    irr::s32 StartupEventReceiver::getWorldSelected() const
    {
        return worldSelected;
    }
