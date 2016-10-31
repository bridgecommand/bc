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

#include "EventReceiver.hpp"

#include <iostream>

#include "GUI.hpp"
#include "ControllerModel.hpp"
#include "Network.hpp"
#include "../Utilities.hpp"

using namespace irr;

    EventReceiver::EventReceiver(irr::IrrlichtDevice* device, ControllerModel* model, GUIMain* gui, Network* network) //Constructor
	{
		this->device = device; //Link to the irrlicht device
		this->model = model; //Link to the model
		this->gui = gui; //Link to GUI
		this->network = network; //Link to the network
    }

    bool EventReceiver::OnEvent(const SEvent& event)
	{

        if (event.EventType == EET_GUI_EVENT && event.GUIEvent.EventType == gui::EGET_BUTTON_CLICKED )
		{
			s32 id = event.GUIEvent.Caller->getID();
			if (id==GUIMain::GUI_ID_HEADING_CHOICE) {
                gui->setMode(true);
			}
			if (id==GUIMain::GUI_ID_REPEATER_CHOICE) {
                gui->setMode(false);
			}
        }

        return false;

    }
