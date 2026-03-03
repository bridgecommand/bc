/*   Bridge Command 5 Ship Simulator
     Copyright (C) 2024 James Packer

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

#include "EventReceiver.hpp"

    EventReceiver::EventReceiver(
        irr::s32 pauseButtonID, 
        irr::s32 runButtonID,
        irr::f32 initialAccelerator) //Constructor
	{
		accelerator = initialAccelerator;
        this->pauseButtonID = pauseButtonID;
        this->runButtonID = runButtonID;
	}

    bool EventReceiver::OnEvent(const irr::SEvent& event)
	{
        if (event.EventType == irr::EET_GUI_EVENT) {
			irr::s32 id = event.GUIEvent.Caller->getID();
            if (event.GUIEvent.EventType==irr::gui::EGET_BUTTON_CLICKED)
            {
                if (id == pauseButtonID) {
                    accelerator = 0;
                } else if (id == runButtonID) {
                    accelerator = 1;
                }
            }
		}

        if (event.EventType == irr::EET_KEY_INPUT_EVENT) {
            if (event.KeyInput.Key==irr::KEY_KEY_0)
            {
                accelerator = 0;
            } else if ((event.KeyInput.Key==irr::KEY_KEY_0) || (event.KeyInput.Key == irr::KEY_RETURN)) {
                    accelerator = 1;
            }
        }
        return false;
    }

    irr::f32 EventReceiver::getAccelerator() const
    {
        return accelerator;
    }