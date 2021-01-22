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

    StartupEventReceiver::StartupEventReceiver(irr::gui::IGUIListBox* scenarioListBox, irr::s32 listBoxID, irr::s32 okButtonID) //Constructor
	{
		this->scenarioListBox = scenarioListBox;
		this->listBoxID = listBoxID;
		this->okButtonID = okButtonID;
		scenarioSelected = -1; //Set as initially invalid
	}

    bool StartupEventReceiver::OnEvent(const irr::SEvent& event)
	{
        if (event.EventType == irr::EET_GUI_EVENT)
		{
			irr::s32 id = event.GUIEvent.Caller->getID();
			//If OK button, or double click on list
            if ( (event.GUIEvent.EventType==irr::gui::EGET_BUTTON_CLICKED && id == okButtonID ) || event.GUIEvent.EventType==irr::gui::EGET_LISTBOX_SELECTED_AGAIN  )
            {
                if (scenarioListBox->getSelected() > -1 ) {
                    scenarioSelected = scenarioListBox->getSelected();
                }
            }

		}

		if (event.EventType == irr::EET_KEY_INPUT_EVENT)
		{
		    if (event.KeyInput.Key==irr::KEY_RETURN) {
                if (scenarioListBox->getSelected() > -1 ) {
                    scenarioSelected = scenarioListBox->getSelected();
                }
		    }
		}
        return false;
    }

    irr::s32 StartupEventReceiver::getScenarioSelected() const
    {
        return scenarioSelected;
    }
