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

using namespace irr;

    EventReceiver::EventReceiver(irr::IrrlichtDevice* dev, ControllerModel* model, GUIMain* gui) //Constructor
	{
		this->model = model; //Link to the model
		this->gui = gui; //Link to GUI (Not currently used!)
    }

    bool EventReceiver::OnEvent(const SEvent& event)
	{

        if (event.EventType == EET_GUI_EVENT)
		{
			s32 id = event.GUIEvent.Caller->getID();


            if (event.GUIEvent.EventType==gui::EGET_BUTTON_CLICKED) {

            }


        }

        //From keyboard
        if (event.EventType == EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown)
		{

            if (event.KeyInput.Shift) {
                //Shift down

            } else if (event.KeyInput.Control) {
                //Ctrl down


            } else {
                //Shift and Ctrl not down

            }
		} //end of key down event

        return false;

    }
