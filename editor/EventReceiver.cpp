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
#include "GeneralDataStruct.hpp"
//#include "Network.hpp"
#include "../Utilities.hpp"

using namespace irr;

    EventReceiver::EventReceiver(irr::IrrlichtDevice* device, ControllerModel* model, GUIMain* gui/*, Network* network*/) //Constructor
	{
		this->device = device; //Link to the irrlicht device
		this->model = model; //Link to the model
		this->gui = gui; //Link to GUI
		//this->network = network; //Link to the network
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

                    //Todo: use model method to apply change
                    model->changeLeg(ship,leg,legCourse,legSpeed,legDistance);

                }
                if (id == GUIMain::GUI_ID_DELETELEG_BUTTON) {
                    int ship = gui->getSelectedShip();
                    int leg = gui->getSelectedLeg();

                    //Todo: use model method to delete selected leg

                }

                if (id == GUIMain::GUI_ID_ADDLEG_BUTTON) {

                    irr::f32 legCourse = gui->getEditBoxCourse();
                    irr::f32 legSpeed = gui->getEditBoxSpeed();
                    irr::f32 legDistance = gui->getEditBoxDistance();

                    int ship = gui->getSelectedShip();
                    int leg = gui->getSelectedLeg();

                    //Todo: use model method to add leg

                }
                if (id == GUIMain::GUI_ID_MOVESHIP_BUTTON) {

                    int ship = gui->getSelectedShip();
                    irr::core::vector2df screenCentrePos = gui->getScreenCentrePosition(); //Check screen centre

                    //Todo: use model method to apply change in ownship position
                    model->setShipPosition(ship, screenCentrePos);

                    //If moving own ship, reset offset, so the map doesn't jump
                    if (ship==0) {
                        model->resetOffset();
                    }

                }
                if (id == GUIMain::GUI_ID_APPLY_BUTTON) {
                    //Get changes to general data from gui

                    GeneralData tempData;
                    tempData.startTime = gui->getStartTime();
                    tempData.startDay = gui->getStartDay();
                    tempData.startMonth = gui->getStartMonth();
                    tempData.startYear = gui->getStartYear();
                    tempData.sunRiseTime = gui->getSunRise();
                    tempData.sunSetTime = gui->getSunSet();
                    tempData.weather = gui->getWeather();
                    tempData.rain = gui->getRain();

                    model->setScenarioData(tempData);

                 }

            }

            if (event.GUIEvent.EventType==gui::EGET_COMBO_BOX_CHANGED || event.GUIEvent.EventType==gui::EGET_LISTBOX_CHANGED) {
                if (id==GUIMain::GUI_ID_SHIP_COMBOBOX) {
                    model->updateSelectedShip( ((gui::IGUIComboBox*)event.GUIEvent.Caller)->getSelected());
                    gui->updateEditBoxes();
                }

                if (id==GUIMain::GUI_ID_LEG_LISTBOX) {
                    model->updateSelectedLeg( ((gui::IGUIListBox*)event.GUIEvent.Caller)->getSelected());
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

		//From mouse
		if (event.EventType == EET_MOUSE_INPUT_EVENT) {

            if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN ) {

                //Check if we're over a gui element, and if so ignore the click
                irr::gui::IGUIElement* overElement = device->getGUIEnvironment()->getRootGUIElement()->getElementFromPoint(device->getCursorControl()->getPosition());
                if ( (overElement == 0 || overElement == device->getGUIEnvironment()->getRootGUIElement()) ) {
                    model->setMouseDown(true);
                }
            }

            if (event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP ) {
                model->setMouseDown(false);
            }

		} //end of mouse event


        return false;

    }
