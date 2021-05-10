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

//using namespace irr;

    EventReceiver::EventReceiver(irr::IrrlichtDevice* device, ControllerModel* model, GUIMain* gui/*, Network* network*/) //Constructor
	{
		this->device = device; //Link to the irrlicht device
		this->model = model; //Link to the model
		this->gui = gui; //Link to GUI
		//this->network = network; //Link to the network
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

                if (id == GUIMain::GUI_ID_SETMMSI_BUTTON) {
                    irr::u32 mmsi = gui->getEditBoxMMSI();
                    int ship = gui->getSelectedShip();
                    model->setMMSI(ship,mmsi);
                }

                if (id == GUIMain::GUI_ID_CHANGE_BUTTON) {
                    //Get data from gui

                    irr::f32 legCourse = gui->getEditBoxCourse();
                    irr::f32 legSpeed = gui->getEditBoxSpeed();
                    irr::f32 legDistance = gui->getEditBoxDistance();

                    int ship = gui->getSelectedShip();
                    int leg = gui->getSelectedLeg();

                    //Use model method to apply change
                    model->changeLeg(ship,leg,legCourse,legSpeed,legDistance);

                }
                if (id == GUIMain::GUI_ID_DELETELEG_BUTTON) {
                    int ship = gui->getSelectedShip();
                    int leg = gui->getSelectedLeg();

                    //Use model method to delete selected leg
                    model->deleteLeg(ship,leg);

                }

                if (id == GUIMain::GUI_ID_ADDLEG_BUTTON) {

                    irr::f32 legCourse = gui->getEditBoxCourse();
                    irr::f32 legSpeed = gui->getEditBoxSpeed();
                    irr::f32 legDistance = gui->getEditBoxDistance();

                    int ship = gui->getSelectedShip();
                    int leg = gui->getSelectedLeg();

                    //Use model method to add leg
                    model->addLeg(ship,leg,legCourse,legSpeed,legDistance);

                }
                if (id == GUIMain::GUI_ID_MOVESHIP_BUTTON) {

                    int ship = gui->getSelectedShip();
                    irr::core::vector2df screenCentrePos = gui->getScreenCentrePosition(); //Check screen centre

                    //Use model method to apply change in ownship position
                    model->setShipPosition(ship, screenCentrePos);

                    //If moving own ship, reset offset, so the map doesn't jump
                    if (ship==0) {
                        model->resetOffset();
                    }

                }
                if (id==GUIMain::GUI_ID_ADDSHIP_BUTTON) {
                    //Add a new ship, at the current screen centre
                    irr::core::vector2df screenCentrePos = gui->getScreenCentrePosition();
                    std::string newShipName = gui->getOtherShipTypeSelected();

                    model->addShip(newShipName,screenCentrePos);
                }
				if (id == GUIMain::GUI_ID_DELETESHIP_BUTTON) {
					int ship = gui->getSelectedShip();
					model->deleteShip(ship);
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

                if (id==GUIMain::GUI_ID_OWNSHIPSELECT_COMBOBOX)  {
                    //Change type of selected own ship
                    model->changeOwnShipName(gui->getOwnShipTypeSelected());
                }

                if (id==GUIMain::GUI_ID_OTHERSHIPSELECT_COMBOBOX)  {
                    //Change type of selected other ship
                    int ship = gui->getSelectedShip(); //-1 if nothing selected, 0 for own ship, 1 - numberOfOtherShips for otherShips
                    model->changeOtherShipName(ship, gui->getOtherShipTypeSelected());
                }


            }

            //Check scenario name change, or user clicks on 'save or apply'. If so, apply general data changes to model.
            if ((event.GUIEvent.EventType==irr::gui::EGET_BUTTON_CLICKED && (id == GUIMain::GUI_ID_APPLY_BUTTON || id == GUIMain::GUI_ID_SAVE_BUTTON)) || (event.GUIEvent.EventType==irr::gui::EGET_EDITBOX_CHANGED && id == GUIMain::GUI_ID_SCENARIONAME_EDITBOX)) {
                GeneralData tempData;
                tempData.startTime = gui->getStartTime();
                tempData.startDay = gui->getStartDay();
                tempData.startMonth = gui->getStartMonth();
                tempData.startYear = gui->getStartYear();
                tempData.sunRiseTime = gui->getSunRise();
                tempData.sunSetTime = gui->getSunSet();
                tempData.weather = gui->getWeather();
                tempData.rain = gui->getRain();
                tempData.visibility = gui->getVisibility();
                tempData.scenarioName = gui->getScenarioName();
                tempData.description = gui->getDescription();

                model->setScenarioData(tempData);
            }

            //Check for Save button here (ensure 'Apply' gets run first!)
            if (event.GUIEvent.EventType==irr::gui::EGET_BUTTON_CLICKED && id == GUIMain::GUI_ID_SAVE_BUTTON) {
                model->save();
            }

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
