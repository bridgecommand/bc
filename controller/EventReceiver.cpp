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

    EventReceiver::EventReceiver(irr::IrrlichtDevice* dev, ControllerModel* model, GUIMain* gui, Network* network) //Constructor
	{
		this->model = model; //Link to the model
		this->gui = gui; //Link to GUI
		this->network = network; //Link to the network
    }

    bool EventReceiver::OnEvent(const SEvent& event)
	{

        if (event.EventType == EET_GUI_EVENT)
		{
			s32 id = event.GUIEvent.Caller->getID();


            if (event.GUIEvent.EventType==gui::EGET_BUTTON_CLICKED) {
                if (id == GUIMain::GUI_ID_CHANGE_BUTTON || id == GUIMain::GUI_ID_CHANGE_COURSESPEED_BUTTON) {
                    //Get data from gui

                    irr::f32 legCourse = gui->getEditBoxCourse();
                    irr::f32 legSpeed = gui->getEditBoxSpeed();
                    irr::f32 legDistance = gui->getEditBoxDistance();

                    if (id == GUIMain::GUI_ID_CHANGE_COURSESPEED_BUTTON) {
                        legDistance = -1; //Flag to change course and speed, but not distance
                    }

                    int ship = gui->getSelectedShip();
                    int leg = gui->getSelectedLeg();

                    std::string messageToSend = "MCCL,";
                    messageToSend.append(Utilities::lexical_cast<std::string>(ship));
                    messageToSend.append(",");
                    messageToSend.append(Utilities::lexical_cast<std::string>(leg));
                    messageToSend.append(",");
                    messageToSend.append(Utilities::lexical_cast<std::string>(legCourse));
                    messageToSend.append(",");
                    messageToSend.append(Utilities::lexical_cast<std::string>(legSpeed));
                    messageToSend.append(",");
                    messageToSend.append(Utilities::lexical_cast<std::string>(legDistance));
                    messageToSend.append("#");

                    network->setStringToSend(messageToSend);

                }
            }

            if (event.GUIEvent.EventType==gui::EGET_COMBO_BOX_CHANGED) {
                if (id==GUIMain::GUI_ID_SHIP_COMBOBOX) {
                    model->updateSelectedShip( ((gui::IGUIComboBox*)event.GUIEvent.Caller)->getSelected());
                    gui->updateEditBoxes();
                }

                if (id==GUIMain::GUI_ID_LEG_COMBOBOX) {
                    model->updateSelectedLeg( ((gui::IGUIComboBox*)event.GUIEvent.Caller)->getSelected());
                    gui->updateEditBoxes();
                }
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
