/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2016 James Packer

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

//Event receiver to allow diversion of log messages before we start main receiver

#include "DefaultEventReceiver.hpp"

#include <iostream>

//using namespace irr;

    DefaultEventReceiver::DefaultEventReceiver(std::vector<std::string>* logMessages, irr::IrrlichtDevice* dev) //Constructor
	{
        this->logMessages = logMessages;
        device = dev;
	}

	bool DefaultEventReceiver::OnEvent(const irr::SEvent& event)
	{

        //From log
        if (event.EventType == irr::EET_LOG_TEXT_EVENT) {
            //Store these in a global log.
            std::string eventText(event.LogEvent.Text);
            logMessages->push_back(eventText);
            return true;
        }

        if (event.EventType == irr::EET_KEY_INPUT_EVENT) {
            if (event.KeyInput.Key == irr::KEY_ESCAPE || event.KeyInput.Key ==  irr::KEY_F4) { //Esc or F4 (for alt f4)
                device->closeDevice(); //Shutdown.
            }


        }

        return false;
    }


