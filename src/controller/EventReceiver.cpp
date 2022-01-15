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

////using namespace irr;

    EventReceiver::EventReceiver(irr::IrrlichtDevice* device, ControllerModel* model, GUIMain* gui, Network* network) //Constructor
	{
		this->device = device; //Link to the irrlicht device
		this->model = model; //Link to the model
		this->gui = gui; //Link to GUI
		this->network = network; //Link to the network
    }

    bool EventReceiver::OnEvent(const irr::SEvent& event)
	{

        if (event.EventType == irr::EET_GUI_EVENT)
		{
			irr::s32 id = event.GUIEvent.Caller->getID();


            if (event.GUIEvent.EventType==irr::gui::EGET_BUTTON_CLICKED) {

                if (id == GUIMain::GUI_ID_ZOOMIN_BUTTON) {
                    model->increaseZoom();
                }

                if (id == GUIMain::GUI_ID_ZOOMOUT_BUTTON) {
                    model->decreaseZoom();
                }

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
                if (id == GUIMain::GUI_ID_DELETELEG_BUTTON) {
                    int ship = gui->getSelectedShip();
                    int leg = gui->getSelectedLeg();

                    std::string messageToSend = "MCDL,";
                    messageToSend.append(Utilities::lexical_cast<std::string>(ship));
                    messageToSend.append(",");
                    messageToSend.append(Utilities::lexical_cast<std::string>(leg));
                    messageToSend.append("#");

                    network->setStringToSend(messageToSend);

                }

                if (id == GUIMain::GUI_ID_ADDLEG_BUTTON) {

                    irr::f32 legCourse = gui->getEditBoxCourse();
                    irr::f32 legSpeed = gui->getEditBoxSpeed();
                    irr::f32 legDistance = gui->getEditBoxDistance();

                    int ship = gui->getSelectedShip();
                    int leg = gui->getSelectedLeg();

                    std::string messageToSend = "MCAL,";
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

                    //std::cout << messageToSend << std::endl;

                    network->setStringToSend(messageToSend);

                }
                if (id == GUIMain::GUI_ID_MOVESHIP_BUTTON) {

                    int ship = gui->getSelectedShip();
                    irr::core::vector2df screenCentrePos = gui->getScreenCentrePosition(); //Check screen centre

                    std::string messageToSend = "MCRS,";
                    messageToSend.append(Utilities::lexical_cast<std::string>(ship));
                    messageToSend.append(",");
                    messageToSend.append(Utilities::lexical_cast<std::string>(screenCentrePos.X));
                    messageToSend.append(",");
                    messageToSend.append(Utilities::lexical_cast<std::string>(screenCentrePos.Y));
                    messageToSend.append("#");
                    network->setStringToSend(messageToSend);

                    //If moving own ship, reset offset, so the map doesn't jump
                    if (ship==0) {
                        model->resetOffset();
                    }
                    //std::cout << messageToSend << std::endl;
                }

                if (id == GUIMain::GUI_ID_SETMMSI_BUTTON) {

                    int ship = gui->getSelectedShip();
                    int mmsi = gui->getEditBoxMMSI();
                    
                    std::string messageToSend = "MCMM,";
                    messageToSend.append(Utilities::lexical_cast<std::string>(ship));
                    messageToSend.append(",");
                    messageToSend.append(Utilities::lexical_cast<std::string>(mmsi));
                    messageToSend.append("#");
                    network->setStringToSend(messageToSend);
                    //std::cout << messageToSend << std::endl;
                }

                if (id == GUIMain::GUI_ID_RELEASEMOB_BUTTON) {
                    std::string messageToSend = "MCMO,1#";
                    network->setStringToSend(messageToSend);
                }

                if (id == GUIMain::GUI_ID_RETRIEVEMOB_BUTTON) {
                    std::string messageToSend = "MCMO,-1#";
                    network->setStringToSend(messageToSend);
                }

                if (id == GUIMain::GUI_ID_RUDDERPUMP_1_WORKING_BUTTON) {
                    std::string messageToSend = "MCRW,1,1#";
                    network->setStringToSend(messageToSend);
                }

                if (id == GUIMain::GUI_ID_RUDDERPUMP_1_FAILED_BUTTON) {
                    std::string messageToSend = "MCRW,1,0#";
                    network->setStringToSend(messageToSend);
                }

                if (id == GUIMain::GUI_ID_RUDDERPUMP_2_WORKING_BUTTON) {
                    std::string messageToSend = "MCRW,2,1#";
                    network->setStringToSend(messageToSend);
                }

                if (id == GUIMain::GUI_ID_RUDDERPUMP_2_FAILED_BUTTON) {
                    std::string messageToSend = "MCRW,2,0#";
                    network->setStringToSend(messageToSend);
                }

                if (id == GUIMain::GUI_ID_FOLLOWUP_WORKING_BUTTON) {
                    std::string messageToSend = "MCRF,1#";
                    network->setStringToSend(messageToSend);
                }

                if (id == GUIMain::GUI_ID_FOLLOWUP_FAILED_BUTTON) {
                    std::string messageToSend = "MCRF,0#";
                    network->setStringToSend(messageToSend);
                }

            }

            if (event.GUIEvent.EventType==irr::gui::EGET_COMBO_BOX_CHANGED || event.GUIEvent.EventType==irr::gui::EGET_LISTBOX_CHANGED) {
                if (id==GUIMain::GUI_ID_SHIP_COMBOBOX) {
                    model->updateSelectedShip( ((irr::gui::IGUIComboBox*)event.GUIEvent.Caller)->getSelected());
                    gui->updateEditBoxes();
                }

                if (id==GUIMain::GUI_ID_LEG_LISTBOX) {
                    model->updateSelectedLeg( ((irr::gui::IGUIListBox*)event.GUIEvent.Caller)->getSelected());
                    gui->updateEditBoxes();
                }
            }

            if (event.GUIEvent.EventType==irr::gui::EGET_SCROLL_BAR_CHANGED) {
                if (id == GUIMain::GUI_ID_WEATHER_SCROLLBAR || id == GUIMain::GUI_ID_RAIN_SCROLLBAR || id == GUIMain::GUI_ID_VISIBILITY_SCROLLBAR) {
                    //Weather
                    //4 elements in 'Set weather' command: MCSW,weather,rain,vis
                    irr::f32 weather=gui->getWeather();
                    irr::f32 rain=gui->getRain();
                    irr::f32 visibility=gui->getVisibility();

                    std::string messageToSend = "MCSW,";
                    messageToSend.append(Utilities::lexical_cast<std::string>(weather));
                    messageToSend.append(",");
                    messageToSend.append(Utilities::lexical_cast<std::string>(rain));
                    messageToSend.append(",");
                    messageToSend.append(Utilities::lexical_cast<std::string>(visibility));
                    messageToSend.append("#");
                    network->setStringToSend(messageToSend);
                }
            }
            /*
            if (event.GUIEvent.EventType==irr::gui::EGDT_WINDOW_CLOSE) {
                if (id==GUIMain::GUI_ID_WINDOW) {
                    return true; //Absorb event : TODO: Should this trigger program close?
                }

            }
            */


        }

        //From keyboard
        if (event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown)
		{

            if (event.KeyInput.Shift) {
                //Shift down

            } else if (event.KeyInput.Control) {
                //Ctrl down


            } else {
                //Shift and Ctrl not down

            }
		} //end of key down event

		//From mouse
		if (event.EventType == irr::EET_MOUSE_INPUT_EVENT) {

            if (event.MouseInput.Event == irr::EMIE_LMOUSE_PRESSED_DOWN ) {

                //Check if we're over a gui element, and if so ignore the click
                irr::gui::IGUIElement* overElement = device->getGUIEnvironment()->getRootGUIElement()->getElementFromPoint(device->getCursorControl()->getPosition());
                if ( (overElement == 0 || overElement == device->getGUIEnvironment()->getRootGUIElement()) ) {
                    model->setMouseDown(true);
                }
            }

            if (event.MouseInput.Event == irr::EMIE_LMOUSE_LEFT_UP ) {
                model->setMouseDown(false);
            }

		} //end of mouse event


        return false;

    }
