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

#include "MyEventReceiver.hpp"

#include <string>

#include "GUIMain.hpp"
#include "SimulationModel.hpp"
#include "Utilities.hpp"
#include "AzimuthDial.h"

//using namespace irr;

    MyEventReceiver::MyEventReceiver(irr::IrrlichtDevice* dev, SimulationModel* model, GUIMain* gui, JoystickSetup joystickSetup, std::vector<std::string>* logMessages) //Constructor
	{
		this->model = model; //Link to the model
		this->gui = gui; //Link to GUI
		scrollBarPosSpeed = 0;
		scrollBarPosHeading = 0;

		//store device
		device = dev;

		lastShownJoystickStatus = device->getTimer()->getRealTime()-5000; // Show joystick raw data every 5s in log
		lastTimeAzimuth1MasterChanged = device->getTimer()->getRealTime()-500; // Allow azimuth master to change every 500ms (debounce)
		lastTimeAzimuth2MasterChanged = device->getTimer()->getRealTime()-500; // Allow azimuth master to change every 500ms (debounce)

		//set up joystick if present, and inform user what's available
		dev->activateJoysticks(joystickInfo);

		//Tell user about joysticks via the log
		dev->getLogger()->log(""); //add a blank line
		std::string joystickInfoMessage = "Number of joysticks detected: ";
		joystickInfoMessage.append(std::string(irr::core::stringc(joystickInfo.size()).c_str()));
		dev->getLogger()->log(joystickInfoMessage.c_str());
        for(unsigned int i = 0; i<joystickInfo.size(); i++) {
            //Print out name and number of each joystick
            joystickInfoMessage = "Joystick number: ";
            joystickInfoMessage.append(irr::core::stringc(i).c_str());
            joystickInfoMessage.append(", Name: ");
            joystickInfoMessage.append(std::string(joystickInfo[i].Name.c_str()));
            dev->getLogger()->log(joystickInfoMessage.c_str());
        }
        dev->getLogger()->log(""); //add a blank line

		this->joystickSetup = joystickSetup;

		//Indicate that previous joystick information hasn't been initialised
		previousJoystickPort = INFINITY; 		// DEE 10JAN26 note ... port thrust lever in azimuth drive
		previousJoystickStbd = INFINITY;		// DEE 10JAN26 note ... stbd thrust lever in azimuth drive
		previousJoystickRudder = INFINITY;
		previousJoystickBowThruster = INFINITY;
		previousJoystickSternThruster = INFINITY;
// DEE 10JAN23 vvvv
//        previousJoystickAzimuthAngPort = INFINITY;
//        previousJoystickAzimuthAngStbd = INFINITY;
	previousJoystickSchottelPort = INFINITY;
	previousJoystickSchottelStbd = INFINITY;
	previousJoystickThrustLeverPort = INFINITY;
	previousJoystickThrustLeverStbd = INFINITY;

// DEE 10JAN23 ^^^^

        previousJoystickPOVInitialised = false;

		this->logMessages = logMessages;

        //assume mouse buttons not pressed initially
        leftMouseDown = false;
        rightMouseDown = false;

        shutdownDialogActive = false;
	}

    bool MyEventReceiver::OnEvent(const irr::SEvent& event)
	{

        //std::cout << "Any event in receiver" << std::endl;
        //From log
        if (event.EventType == irr::EET_LOG_TEXT_EVENT) {
            //Store these in a global log.
            std::string eventText(event.LogEvent.Text);
            logMessages->push_back(eventText);
            return true;
        }

        //From mouse - keep track of button press state
        if (event.EventType == irr::EET_MOUSE_INPUT_EVENT) {
            if (event.MouseInput.Event == irr::EMIE_LMOUSE_PRESSED_DOWN ) {
                    leftMouseDown=true;
                    //Log position of mouse click, so we can track relative movement
                    mouseClickX = event.MouseInput.X;
                    mouseClickY = event.MouseInput.Y;
            }
            if (event.MouseInput.Event == irr::EMIE_LMOUSE_LEFT_UP ) {
                    leftMouseDown=false;
            }
            if (event.MouseInput.Event == irr::EMIE_RMOUSE_PRESSED_DOWN ) {
                rightMouseDown=true;
                //Force focus on right click
                irr::gui::IGUIElement* overElement;
                overElement = device->getGUIEnvironment()->getRootGUIElement()->getElementFromPoint(irr::core::position2d<irr::s32>(event.MouseInput.X,event.MouseInput.Y));
                if (overElement) {
                    device->getGUIEnvironment()->setFocus(overElement);
                }
            }
            if (event.MouseInput.Event == irr::EMIE_RMOUSE_LEFT_UP ) {
                    rightMouseDown=false;
            }
            model->setMouseDown(leftMouseDown || rightMouseDown); //Set if either mouse is down

            //Check mouse movement
            if (event.MouseInput.Event == irr::EMIE_MOUSE_MOVED && leftMouseDown) {
                //Check if focus in on a gui element
                irr::gui::IGUIElement* focussedElement;
                focussedElement = device->getGUIEnvironment()->getFocus();
                if (!focussedElement) {
                    irr::s32 deltaX = event.MouseInput.X - mouseClickX;
                    irr::s32 deltaY = event.MouseInput.Y - mouseClickY;
                    model->changeLookPx(deltaX,deltaY);
                }
                //Store for next time
                mouseClickX = event.MouseInput.X;
                mouseClickY = event.MouseInput.Y;
            }

            if (event.MouseInput.Event == irr::EMIE_MOUSE_WHEEL) {
                if (event.MouseInput.Wheel < 0) {
                    model->setWheel(model->getWheel()+1.0);
                } if (event.MouseInput.Wheel > 0) {
                    model->setWheel(model->getWheel()-1.0);
                }
                return true;
            }
        }

        if (event.EventType == irr::EET_GUI_EVENT)
		{
			irr::s32 id = event.GUIEvent.Caller->getID();

            if (event.GUIEvent.EventType==irr::gui::EGET_CHECKBOX_CHANGED)
            {
                if (id == GUIMain::GUI_ID_AZIMUTH_1_MASTER_CHECKBOX) {
                    model->setAzimuth1Master(((irr::gui::IGUICheckBox*)event.GUIEvent.Caller)->isChecked());
                }

                if (id == GUIMain::GUI_ID_AZIMUTH_2_MASTER_CHECKBOX) {
                    model->setAzimuth2Master(((irr::gui::IGUICheckBox*)event.GUIEvent.Caller)->isChecked());
                }

                if ((id==GUIMain::GUI_ID_ARPA_ON_BOX || id==GUIMain::GUI_ID_BIG_ARPA_ON_BOX)) {
                    //ARPA on/off checkbox
                    bool boxState = ((irr::gui::IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                    model->setArpaOn(boxState);

                    //Set the linked checkbox (big/small radar window)
                    gui->setARPACheckboxes(boxState);
                }
            }

            if (event.GUIEvent.EventType==irr::gui::EGET_SCROLL_BAR_CHANGED)
            {

               if (id == GUIMain::GUI_ID_HEADING_SCROLL_BAR)
                  {
                      scrollBarPosHeading = ((irr::gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                      model->setHeading(scrollBarPosHeading);
                  }

              if (id == GUIMain::GUI_ID_SPEED_SCROLL_BAR)
                  {
                        scrollBarPosSpeed = ((irr::gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                        model->setSpeed(scrollBarPosSpeed);
                  }

              if (id == GUIMain::GUI_ID_STBD_SCROLL_BAR)
                  {
                        irr::f32 value = ((irr::gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/-100.0;  //Convert to from +-100 to +-1, and invert up/down
                        model->setStbdEngine(value);
                        //If right mouse button, set the other engine as well
                        if (rightMouseDown) {
                            model->setPortEngine(value);
                        }
                  }
              if (id == GUIMain::GUI_ID_PORT_SCROLL_BAR)
                  {
                        irr::f32 value = ((irr::gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/-100.0;  //Convert to from +-100 to +-1, and invert up/down
                        model->setPortEngine(value);
                        //If right mouse button, set the other engine as well
                        if (rightMouseDown) {
                            model->setStbdEngine(value);
                        }
                  }

// DEE debug this must be disabled becuase only the wheel is directly controlled
// disbaling mouse controlling the rudder directly change it to change wheel

              if (id == GUIMain::GUI_ID_RUDDER_SCROLL_BAR)
                  {
//                        model->setRudder(((irr::gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos());
                  }




// DEE capture the wheel
              if (id == GUIMain::GUI_ID_WHEEL_SCROLL_BAR)
                  {
                        //Check if either NFU button is down, in which case force the change (even if the follow up rudder isn't working)
                        bool nfuActive = gui->isNFUActive();
                        model->setWheel(((irr::gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos(),nfuActive);
                  }
// DEE capture the wheel


// DEE_NOV22 this deals with capturing input from the Azimuth gui
// JP: Removed GUI inputs from GUI_ID_AZIMUTH_1 and GUI_ID_AZIMUTH_2, as I think these are intended to be display only, not for input?
// DEE_NOV22 ^^^^ end of the inputs from the azimth GUI


// DEE_NOV22 vvvv controls for mouse inputs to the engine rpm indicators and the schottels

	    if ( id == GUIMain::GUI_ID_SCHOTTEL_PORT )
	    {
		// DEE_NOV22 only really want this to respond to left mouse click , no master mode
		// as in practice if you want to steer with only one schottel, you just
		// leave the other dead ahead, for small steering corrections then that
		// is adequate
		irr::f32 angle = (((irr::gui::AzimuthDial*)event.GUIEvent.Caller)->getPos()); // Range 0-360
		// not intersted in magnitude
		model->setPortSchottel(angle);
	    } // end if schottel port



	    if ( id == GUIMain::GUI_ID_SCHOTTEL_STBD )
	    {
		irr::f32 angle = (((irr::gui::AzimuthDial*)event.GUIEvent.Caller)->getPos()); // Range 0-360
		// not intersted in magnitude
		model->setStbdSchottel(angle);
	    } // end if schottel stbd



	    if ( id == GUIMain::GUI_ID_ENGINE_PORT )
	    {
		irr::f32 angle = (((irr::gui::AzimuthDial*)event.GUIEvent.Caller)->getPos()); // Range 0-360
		// we arent interested in the getMag
		// DEE_Boxing_Day_2022 vvvv
                //TODO: change gui control mapping so we don't need scaling here
                irr::f32 tempEngLevel; // temporary variable 0..1 to represent attempted engine setting

                if ((angle >= 0) && (angle <135)) {
                	tempEngLevel = (0.5 + angle/270);
		}
                if ((angle >= 135)  && (angle < 180)) {
                	tempEngLevel = 1;
                }
                if ((angle >= 180)  && (angle < 180)) {
                	tempEngLevel = 0;
                }
                if ((angle >= 225) && (angle < 360)) {
                	tempEngLevel = ((angle-225)/270);
                }
// DEE_Boxing_Day_2022 I am sure there is a far more elegant solution than the above

    		// limit the output to 0..1 only leaving this in for future elegant solution
                if (tempEngLevel < 0) {tempEngLevel=0;}
                if (tempEngLevel > 1) {tempEngLevel=1;}

                model->setPortThrustLever(tempEngLevel);

                // DEBUG
                //device->getLogger()->log(f32To3dp(angle).c_str());
		//device->getLogger()->log(f32To3dp(tempEngLevel).c_str());
                // DEBUG END
                		// DEE_Boxing_Day_2022 ^^^^
            } // end if engine port



	    if ( id == GUIMain::GUI_ID_ENGINE_STBD )
	    {
// DEE_Boxing_Day_2022 vvvv
		irr::f32 angle = (((irr::gui::AzimuthDial*)event.GUIEvent.Caller)->getPos()); // Range 0-360
		// we arent interested in the getMag
		// DEE_Boxing_Day_2022 vvvv
                //TODO: change gui control mapping so we don't need scaling here
                irr::f32 tempEngLevel; // temporary variable 0..1 to represent attempted engine setting

                if ((angle >= 0) && (angle <135)) {
                	tempEngLevel = (0.5 + angle/270);
		}
                if ((angle >= 135)  && (angle < 180)) {
                	tempEngLevel = 1;
                }
                if ((angle >= 180)  && (angle < 180)) {
                	tempEngLevel = 0;
                }
                if ((angle >= 225) && (angle < 360)) {
                	tempEngLevel = ((angle-225)/270);
                }
// DEE_Boxing_Day_2022 I am sure there is a far more elegant solution than the above

    		// limit the output to 0..1 only leaving this in for future elegant solution
                if (tempEngLevel < 0) {tempEngLevel=0;}
                if (tempEngLevel > 1) {tempEngLevel=1;}

                model->setStbdThrustLever(tempEngLevel);


//		model->setStbdEngine((angle/360));
// DEE_Boxing_Day_2022 ^^^^

            } // end if engine port





            //DEAL WITH THRUSTER SCROLL BARS HERE - ALSO WITH JOYSTICK

              if (id == GUIMain::GUI_ID_BOWTHRUSTER_SCROLL_BAR) {
                  irr::f32 value = ((irr::gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/100.0;  //Convert to from +-100 to +-1
                  model->setBowThruster(value);
              }
              if (id == GUIMain::GUI_ID_STERNTHRUSTER_SCROLL_BAR) {
                  irr::f32 value = ((irr::gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/100.0;  //Convert to from +-100 to +-1
                  model->setSternThruster(value);
              }

              if (id == GUIMain::GUI_ID_RADAR_GAIN_SCROLL_BAR)
                  {
                        model->setRadarGain(((irr::gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos());
                  }
              if (id == GUIMain::GUI_ID_RADAR_CLUTTER_SCROLL_BAR)
                  {
                        model->setRadarClutter(((irr::gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos());
                  }
              if (id == GUIMain::GUI_ID_RADAR_RAIN_SCROLL_BAR)
                  {
                        model->setRadarRain(((irr::gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos());
                  }
              if (id == GUIMain::GUI_ID_WEATHER_SCROLL_BAR)
                  {
                        model->setWeather(((irr::gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/10.0); //Scroll bar 0-120, weather 0-12
                  }
              if (id == GUIMain::GUI_ID_RAIN_SCROLL_BAR)
                  {
                        model->setRain(((irr::gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/10.0); //Scroll bar 0-100, rain 0-10
                  }
              if (id == GUIMain::GUI_ID_VISIBILITY_SCROLL_BAR)
                  {
                        model->setVisibility(((irr::gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/10.0); //Scroll bar 1-101, vis 0.1-10.1
                  }
            }

            if (event.GUIEvent.EventType==irr::gui::EGET_MESSAGEBOX_OK) {
                if (id == GUIMain::GUI_ID_CLOSE_BOX) {
                    device->closeDevice(); //Confirm shutdown.
                }
            }

            if (event.GUIEvent.EventType==irr::gui::EGET_MESSAGEBOX_CANCEL) {
                if (id == GUIMain::GUI_ID_CLOSE_BOX) {
                    shutdownDialogActive = false;
                }
            }


            if (event.GUIEvent.EventType==irr::gui::EGET_BUTTON_CLICKED) {

                if (id == GUIMain::GUI_ID_EXIT_BUTTON)
                {
                    startShutdown();
                }

                if (id == GUIMain::GUI_ID_START_BUTTON)
                {
                    model->setAccelerator(1.0);
                }

				if (id == GUIMain::GUI_ID_RADAR_ONOFF_BUTTON)
				{
					model->toggleRadarOn();
				}

                if (id == GUIMain::GUI_ID_RADAR_INCREASE_BUTTON)
                {
                    model->increaseRadarRange();
                }

                if (id == GUIMain::GUI_ID_RADAR_DECREASE_BUTTON)
                {
                    model->decreaseRadarRange();
                }

                if (id == GUIMain::GUI_ID_BIG_RADAR_BUTTON)
                {
                    gui->setLargeRadar(true);
                    model->setRadarDisplayRadius(gui->getRadarPixelRadius());
                    gui->hide2dInterface();
                }

                if (id == GUIMain::GUI_ID_SMALL_RADAR_BUTTON)
                {
                    gui->setLargeRadar(false);
                    model->setRadarDisplayRadius(gui->getRadarPixelRadius());
                    gui->show2dInterface();
                }

                if (id == GUIMain::GUI_ID_SHOW_INTERFACE_BUTTON)
                {
                    gui->show2dInterface();
                }

                if (id == GUIMain::GUI_ID_HIDE_INTERFACE_BUTTON)
                {
                    gui->hide2dInterface();
                }

                if (id == GUIMain::GUI_ID_BINOS_INTERFACE_BUTTON)
                {
                    model->setZoom(((irr::gui::IGUIButton*)event.GUIEvent.Caller)->isPressed());
                }

                if (id == GUIMain::GUI_ID_RADAR_EBL_LEFT_BUTTON)
                {
                    model->decreaseRadarEBLBrg();
                }

                if (id == GUIMain::GUI_ID_RADAR_EBL_RIGHT_BUTTON)
                {
                    model->increaseRadarEBLBrg();
                }

                if (id == GUIMain::GUI_ID_RADAR_EBL_UP_BUTTON)
                {
                    model->increaseRadarEBLRange();
                }

                if (id == GUIMain::GUI_ID_RADAR_EBL_DOWN_BUTTON)
                {
                    model->decreaseRadarEBLRange();
                }

                if (id == GUIMain::GUI_ID_RADAR_COLOUR_BUTTON)
                {
                    model->changeRadarColourChoice();
                }

                //Radar mode buttons
                if (id == GUIMain::GUI_ID_RADAR_NORTH_BUTTON)
                {
                    model->setRadarNorthUp();
                }
                if (id == GUIMain::GUI_ID_RADAR_COURSE_BUTTON)
                {
                    model->setRadarCourseUp();
                }
                if (id == GUIMain::GUI_ID_RADAR_HEAD_BUTTON)
                {
                    model->setRadarHeadUp();
                }

                if (id == GUIMain::GUI_ID_SHOW_LOG_BUTTON)
                {
                    gui->showLogWindow();
                }

                if (id == GUIMain::GUI_ID_HIDE_EXTRA_CONTROLS_BUTTON)
                {
                    gui->setExtraControlsWindowVisible(false);
                }

                if (id == GUIMain::GUI_ID_SHOW_EXTRA_CONTROLS_BUTTON)
                {
                    gui->setExtraControlsWindowVisible(true);
                }

                if (id == GUIMain::GUI_ID_RUDDERPUMP_1_WORKING_BUTTON)
                {
                    model->setRudderPumpState(1,true);
                    if (model->getRudderPumpState(2)) {
                        model->setAlarm(false); //Only turn off alarm if other pump is working
                    }
                }

                if (id == GUIMain::GUI_ID_RUDDERPUMP_1_FAILED_BUTTON)
                {
                    model->setRudderPumpState(1,false);
                    model->setAlarm(true);
                }

                if (id == GUIMain::GUI_ID_RUDDERPUMP_2_WORKING_BUTTON)
                {
                    model->setRudderPumpState(2,true);
                    if (model->getRudderPumpState(1)) {
                        model->setAlarm(false); //Only turn off alarm if other pump is working
                    }
                }

                if (id == GUIMain::GUI_ID_RUDDERPUMP_2_FAILED_BUTTON)
                {
                    model->setRudderPumpState(2,false);
                    model->setAlarm(true);
                }

                if (id == GUIMain::GUI_ID_FOLLOWUP_WORKING_BUTTON)
                {
                    model->setFollowUpRudderWorking(true);
                }

                if (id == GUIMain::GUI_ID_FOLLOWUP_FAILED_BUTTON)
                {
                    model->setFollowUpRudderWorking(false);
                }

                if (id ==GUIMain::GUI_ID_ACK_ALARMS_BUTTON)
                {
                    model->setAlarm(false);
                }

            } //Button clicked

            if (event.GUIEvent.EventType == irr::gui::EGET_COMBO_BOX_CHANGED) {

                if (id == GUIMain::GUI_ID_ARPA_TRUE_REL_BOX || id == GUIMain::GUI_ID_BIG_ARPA_TRUE_REL_BOX)
                {
                    irr::s32 selected = ((irr::gui::IGUIComboBox*)event.GUIEvent.Caller)->getSelected();
                    if(selected == 0) {
                        model->setRadarARPATrue();
                    } else if (selected == 1) {
                        model->setRadarARPARel();
                    }

                    //Set both linked inputs - brute force
                    irr::gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_ARPA_TRUE_REL_BOX,true);
                    if(other!=0) {
                        ((irr::gui::IGUIComboBox*)other)->setSelected(((irr::gui::IGUIComboBox*)event.GUIEvent.Caller)->getSelected());
                    }
                    other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_ARPA_TRUE_REL_BOX,true);
                    if(other!=0) {
                        ((irr::gui::IGUIComboBox*)other)->setSelected(((irr::gui::IGUIComboBox*)event.GUIEvent.Caller)->getSelected());
                    }

                }

            }//Combo box

            if ( (id==GUIMain::GUI_ID_ARPA_VECTOR_TIME_BOX || id==GUIMain::GUI_ID_BIG_ARPA_VECTOR_TIME_BOX) && (event.GUIEvent.EventType == irr::gui::EGET_EDITBOX_ENTER || event.GUIEvent.EventType == irr::gui::EGET_ELEMENT_FOCUS_LOST ) ) {
                std::wstring boxWString = std::wstring(((irr::gui::IGUIEditBox*)event.GUIEvent.Caller)->getText());
                std::string boxString(boxWString.begin(), boxWString.end());
                irr::f32 value = Utilities::lexical_cast<irr::f32>(boxString);

                if (value > 0 && value <= 60) {
                    model->setRadarARPAVectors(value);
                } else {
                    event.GUIEvent.Caller->setText(L"Invalid");
                }

                //Set both linked inputs - brute force
                irr::gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_ARPA_VECTOR_TIME_BOX,true);
                if(other!=0) {
                    other->setText(event.GUIEvent.Caller->getText());
                }
                other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_ARPA_VECTOR_TIME_BOX,true);
                if(other!=0) {
                    other->setText(event.GUIEvent.Caller->getText());
                }
            }

            //Radar PI controls
            //PI selected
            if (event.GUIEvent.EventType == irr::gui::EGET_COMBO_BOX_CHANGED) {
                if (id == GUIMain::GUI_ID_PI_SELECT_BOX || id == GUIMain::GUI_ID_BIG_PI_SELECT_BOX) {

                    //Set to match
                    if (id == GUIMain::GUI_ID_PI_SELECT_BOX) { //Selected on small screen
                        irr::gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_PI_SELECT_BOX,true);
                        if(other!=0) {
                            ((irr::gui::IGUIComboBox*)other)->setSelected(((irr::gui::IGUIComboBox*)event.GUIEvent.Caller)->getSelected());
                        }
                    } else { //Selected on big screen
                        irr::gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_SELECT_BOX,true);
                        if(other!=0) {
                            ((irr::gui::IGUIComboBox*)other)->setSelected(((irr::gui::IGUIComboBox*)event.GUIEvent.Caller)->getSelected());
                        }
                    }

                    //Get PI data for the newly selected PI
                    irr::s32 selectedPI = ((irr::gui::IGUIComboBox*)event.GUIEvent.Caller)->getSelected(); //(-1 or 0-9)
                    //TODO: Use this to get data from model, and set fields
                    irr::gui::IGUIElement* piBrg = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_BEARING_BOX,true);
                    irr::gui::IGUIElement* piRng = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_RANGE_BOX,true);
                    irr::gui::IGUIElement* piBrgBig = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_PI_BEARING_BOX,true);
                    irr::gui::IGUIElement* piRngBig = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_PI_RANGE_BOX,true);
                    if (piBrg && piRng && piBrgBig && piRngBig) {
                        piBrg->setText(f32To3dp(model->getPIbearing(selectedPI),true).c_str());
                        piBrgBig->setText(f32To3dp(model->getPIbearing(selectedPI),true).c_str());
                        piRng->setText(f32To3dp(model->getPIrange(selectedPI),true).c_str());
                        piRngBig->setText(f32To3dp(model->getPIrange(selectedPI),true).c_str());
                    }
                }

            }

            if (event.GUIEvent.EventType == irr::gui::EGET_EDITBOX_CHANGED || event.GUIEvent.EventType == irr::gui::EGET_EDITBOX_ENTER || event.GUIEvent.EventType == irr::gui::EGET_ELEMENT_FOCUS_LOST ) {

                //Bearing/range boxes:
                if (id == GUIMain::GUI_ID_PI_BEARING_BOX || id == GUIMain::GUI_ID_BIG_PI_BEARING_BOX || id == GUIMain::GUI_ID_PI_RANGE_BOX || id == GUIMain::GUI_ID_BIG_PI_RANGE_BOX ) {
                    //Make the controls match
                    if (id == GUIMain::GUI_ID_PI_BEARING_BOX) {
                        irr::gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_PI_BEARING_BOX,true);
                        if (other!=0) {other->setText(event.GUIEvent.Caller->getText());}
                    }
                    if (id == GUIMain::GUI_ID_BIG_PI_BEARING_BOX) {
                        irr::gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_BEARING_BOX,true);
                        if (other!=0) {other->setText(event.GUIEvent.Caller->getText());}
                    }
                    if (id == GUIMain::GUI_ID_PI_RANGE_BOX) {
                        irr::gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_PI_RANGE_BOX,true);
                        if (other!=0) {other->setText(event.GUIEvent.Caller->getText());}
                    }
                    if (id == GUIMain::GUI_ID_BIG_PI_RANGE_BOX) {
                        irr::gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_RANGE_BOX,true);
                        if (other!=0) {other->setText(event.GUIEvent.Caller->getText());}
                    }

                    if (event.GUIEvent.EventType == irr::gui::EGET_EDITBOX_ENTER || event.GUIEvent.EventType == irr::gui::EGET_ELEMENT_FOCUS_LOST ) {
                        //Use the result
                        irr::gui::IGUIElement* piCombo = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_SELECT_BOX,true);
                        irr::gui::IGUIElement* piBrg = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_BEARING_BOX,true);
                        irr::gui::IGUIElement* piRng = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_RANGE_BOX,true);
                        if (piCombo && piBrg && piRng) {
                            irr::s32 selectedPI = ((irr::gui::IGUIComboBox*)piCombo)->getSelected(); //(0-9)

                            std::wstring brgWString = std::wstring(piBrg->getText());
                            std::string brgString(brgWString.begin(), brgWString.end());
                            irr::f32 bearingChosen = Utilities::lexical_cast<irr::f32>(brgString);

                            std::wstring rngWString = std::wstring(piRng->getText());
                            std::string rngString(rngWString.begin(), rngWString.end());
                            irr::f32 rangeChosen = Utilities::lexical_cast<irr::f32>(rngString);

                            //Apply to model
                            model->setPIData(selectedPI,bearingChosen,rangeChosen);

                        }
                    }

                }

            }


        } //GUI Event


        //From keyboard
        if (event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown) {
            //Check here that there isn't focus on a GUI edit box. If we are, don't process key inputs here.
            irr::gui::IGUIElement* focussedElement = device->getGUIEnvironment()->getFocus();
            if ( !(focussedElement && focussedElement->getType()==irr::gui::EGUIET_EDIT_BOX)) {

                if (event.KeyInput.Shift && event.KeyInput.Control) {

                    switch(event.KeyInput.Key)
                    {
                        //Move camera
                        case irr::KEY_UP:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->moveCameraForwards();
                            break;
                        case irr::KEY_DOWN:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->moveCameraBackwards();
                            break;
                        default:
                            //don't do anything
                            break;
                    }

                } else if (event.KeyInput.Shift) {
                    //Shift down

                    switch(event.KeyInput.Key)
                    {
                        //Camera look
                        case irr::KEY_LEFT:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookStepLeft();
                            break;
                        case irr::KEY_RIGHT:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookStepRight();
                            break;
                        case irr::KEY_SPACE:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->changeView();
                            model->setMoveViewWithPrimary(false); //Don't allow the view to change automatically after this
                            break;
                        default:
                            //don't do anything
                            break;
                    }


                } else if (event.KeyInput.Control) {
                    //Ctrl down

                    switch(event.KeyInput.Key)
                    {
                        //Camera look
                        case irr::KEY_UP:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookAhead();
                            break;
                        case irr::KEY_DOWN:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookAstern();
                            break;
                        case irr::KEY_LEFT:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookPort(); // DEENOV22 TODO make all screens lookPort
                            break;
                        case irr::KEY_RIGHT:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookStbd();
                            break;

                        case irr::KEY_KEY_M:
                            model->retrieveManOverboard();
                            break;

                        default:
                            //don't do anything
                            break;
                    }

                } else {
                    //Shift and Ctrl not down

                    switch(event.KeyInput.Key)
                    {
                        //Accelerator
                        case irr::KEY_KEY_0:
                            model->setAccelerator(0.0);
                            break;
                        case irr::KEY_RETURN:
                            model->setAccelerator(1.0);
                            break;
                        case irr::KEY_KEY_1:
                            model->setAccelerator(1.0);
                            break;
                        case irr::KEY_KEY_2:
                            model->setAccelerator(2.0);
                            break;
                        case irr::KEY_KEY_3:
                            model->setAccelerator(5.0);
                            break;
                        case irr::KEY_KEY_4:
                            model->setAccelerator(15.0);
                            break;
                        case irr::KEY_KEY_5:
                            model->setAccelerator(30.0);
                            break;
                        case irr::KEY_KEY_6:
                            model->setAccelerator(60.0);
                            break;
                        case irr::KEY_KEY_7:
                            model->setAccelerator(3600.0);
                            break;

			case irr::KEY_KEY_H:
			    model->startHorn();
			    break;

// DEE_NOV22 vvvvv

// only assign these key bindings if this is an Azimuth Drive

// Purpose of this code is to allow keyboard control of the azipods
// they should be ineffective if the ship does not have azipods however
// I shall put this in the model object

// ultimately there should be a choice of Non followup mode and Follow up mode
// so they keys control the movement of the schottels in follow up mode
// the pod's azimuth then chases the commanded azimuth
// and the engine chases each thrust lever and clutches in and out automatically when thresholds are reached
// there can be no direct engine in reverse

// todo also need to model the shetland trader type case of when going forward they act as a steering wheel
// rather than a tiller, which could be the normal case


// key map for this currently is as follows
// Port Azipod : A pod anticlockwise , D pod clockwise, W thrust lever forward, S thrust lever backwards
// Starboard Azipod : J pod anticlockwise, L pod clockwise, I thrust lever forward, K thrust lever backwards


// the if ASDs are left in the below controls so that the keys can be assigned to something else on a
// non azipod ship in the future


			case irr::KEY_KEY_W:
			    if (model->isAzimuthDrive()) {
				// Port Azipod Thrust lever increase
			        model->btnIncrementPortThrustLever();
			    }
			    break;

// KEY_KEY_S ... decrement port thrust is further down the code as it has a duplicate use

			case irr::KEY_KEY_J:
			    if (model->isAzimuthDrive()) {
				// Starboard Schottel anticlockwise decrement
			        model->btnDecrementStbdSchottel();
			    }
			    break;

			case irr::KEY_KEY_L:
			    if (model->isAzimuthDrive()) {
				// Starboard Azipod Schottel clockwise increment
			        model->btnIncrementStbdSchottel();
			    }
			    break;

			case irr::KEY_KEY_I:
			    if (model->isAzimuthDrive()) {
				// Starboard Azipod Thurst lever increase
				model->btnIncrementStbdThrustLever();
			    }
			    break;

			case irr::KEY_KEY_K:
			    if (model->isAzimuthDrive()) {
				// Starboard Azipod Thrust lever decrease
			        model->btnDecrementStbdThrustLever();
			    }
			    break;


// DEE_NOV22 ^^^^^

                        //Camera look
                        case irr::KEY_UP:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookUp();
                            break;
                        case irr::KEY_DOWN:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookDown();
                            break;
                        case irr::KEY_LEFT:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookLeft();
                            break;
                        case irr::KEY_RIGHT:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookRight();
                            break;
                        case irr::KEY_SPACE:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->changeView();
                            model->setMoveViewWithPrimary(true); //Allow the view to change automatically after this
                            break;

                        //toggle full screen 3d
                        case irr::KEY_KEY_F:
                            gui->toggleShow2dInterface();
                            break;

                        //Quit with esc or F4 (for alt-F4)
                        case irr::KEY_ESCAPE:
                        case irr::KEY_F4:
                            startShutdown();
                            return true; //Return true here, so second 'esc' button pushes don't close the message box
                            break;

                        case irr::KEY_KEY_M:
                            model->releaseManOverboard();
                            break;

                        //Keyboard control of engines
                        case irr::KEY_KEY_A:
			    // DEE_NOV22 vvvv
			    if(model->isAzimuthDrive()) {
				// if vessel is Azimuth drive then turn Port Schottel anticlockwise decrement
				model->btnDecrementPortSchottel();
			    } else {
			    // DEE_NOV_22 ^^^^
                            // (if not Azimuth Drive) Increase port engine revs:
                                model->setPortEngine(model->getPortEngine()+0.1); //setPortEngine clamps the setting to the allowable range
			    // DEE_NOV22 vvvv
			    }
			    // DEE_NOV22 ^^^^
                            break;

                        case irr::KEY_KEY_Z:
                            //Decrease port engine revs:
			    // DEE_NOV22 vvvv disable this function if azimuth drive
			    if(!(model->isAzimuthDrive())) {
                                model->setPortEngine(model->getPortEngine()-0.1); //setPortEngine clamps the setting to the allowable range
			    }
			    // DEE_NOV22 ^^^^
                            break;

                        case irr::KEY_KEY_S:
			    // DEE_NOV22 vvvv
			    if(model->isAzimuthDrive()) {
				// as Azimuth drive then Port thrust lever decrease
				model->btnDecrementPortThrustLever();
			    } else {
				// this is an non azimuth drive vessel to interpret S as Increase stpd engine revs
			    // DEE_NOV22 ^^^^
                                //Increase stbd engine revs:
                                model->setStbdEngine(model->getStbdEngine()+0.1); //setPortEngine clamps the setting to the allowable range
				} // DEE_NOV22 end if isAzimuthDrive and indentations
                            break;

                        case irr::KEY_KEY_X:

			    // DEE_NOV22 vvvv only enable this response if it is not an azimuth drive
			    if(!(model->isAzimuthDrive()))
				{
                                //Decrease stbd engine revs:
                                model->setStbdEngine(model->getStbdEngine()-0.1); //setPortEngine clamps the setting to the allowable range
				}
			    // DEE_NOV22 ^^^^
                            break;

                        case irr::KEY_KEY_D:
			    // DEE_NOV22 vvvv in the case of the ship being Azimuth Drive
			    if(model->isAzimuthDrive()) {
				// as Azimuth drive then Port Azipod Schottel clockwise
				model->btnIncrementPortSchottel();
			    } else {
			    // DEE_NOV22 ^^^^
			    // else the ship is normally propelled indents made to code below DEE_NOV22 ^^^^
                                //Increase stbd and port engine revs:
                                model->setStbdEngine(model->getStbdEngine()+0.1); //setPortEngine clamps the setting to the allowable range
                                model->setPortEngine(model->getPortEngine()+0.1); //setPortEngine clamps the setting to the allowable range
			    } // end if DEE_NOV22
                            break;
                        case irr::KEY_KEY_C:
			    // DEE_NOV22 vvvv only if not azimuth drive
			    if (!(model->isAzimuthDrive()))
			    {
			    // DEE_NOV22 ^^^^ indentations added below
                                //Decrease stbd engine revs:
                                model->setStbdEngine(model->getStbdEngine()-0.1); //setPortEngine clamps the setting to the allowable range
                                model->setPortEngine(model->getPortEngine()-0.1); //setPortEngine clamps the setting to the allowable range
			    // DEE_NOV22 vvvv
			    }
			    // DEE_NOV22 ^^^^
                            break;

// DEE vvvv key rudder to port changed to rudder wheel to port
                        case irr::KEY_KEY_V:

			// DEE_NOV22 vvvv the 'wheel' should be enabled only when not an azimuth drive
			    if (!(model->isAzimuthDrive()))
			    {
//                               model->setRudder(model->getRudder()-5);
			        model->setWheel(model->getWheel()-1);
			    }
			// DEE_NOV22 ^^^^ indentations added to original code
                            break;
// DEE ^^^^

// DEE vvvv key rudder to starboard changed to key wheel to starboard
                        case irr::KEY_KEY_B:
			    // DEE_NOV22 vvvv
			    if (!(model->isAzimuthDrive()))
			    {
			    // DEE_NOV22 ^^^^

//                                model->setRudder(model->getRudder()+5);
                                model->setWheel(model->getWheel()+1);
			    // DEE_NOV22 vvvv
			    }
			    // DEE_NOV22 ^^^^
                            break;
// DEE ^^^^

                        default:
                            //don't do anything
                            break;
                    }
                }
            }
		}

		if (event.EventType == irr::EET_KEY_INPUT_EVENT && !event.KeyInput.PressedDown) {
			if (event.KeyInput.Key == irr::KEY_KEY_H) {
				model->endHorn();
			}
		}







// DEE 10JAN23 comment  joystickCode starts here

		//From joystick (actually polled, once per run():
        if (event.EventType == irr::EET_JOYSTICK_INPUT_EVENT) {

        	irr::u8 thisJoystick = event.JoystickEvent.Joystick;

            //Initialise the joystick POV
            if (!previousJoystickPOVInitialised) {
                if (thisJoystick == joystickSetup.joystickNoPOV) {
                    previousJoystickPOVInitialised = true;
                    previousJoystickPOV = event.JoystickEvent.POV;
                }
            }

        	//Show joystick raw status in log window
        	if (device->getTimer()->getRealTime() - lastShownJoystickStatus > 5000) {

        		std::string joystickInfoMessage = "Joystick status (";
        		joystickInfoMessage.append(irr::core::stringc(event.JoystickEvent.Joystick).c_str());
        		joystickInfoMessage.append(")\n");
        		device->getLogger()->log(joystickInfoMessage.c_str());

        		std::string thisJoystickStatus = "";
        		for (irr::u8 thisAxis = 0; thisAxis < event.JoystickEvent.NUMBER_OF_AXES; thisAxis++) {
        			irr::s16 axisSetting = event.JoystickEvent.Axis[thisAxis];
        			thisJoystickStatus.append(irr::core::stringc(axisSetting).c_str());
        			thisJoystickStatus.append(" ");
        		}
                thisJoystickStatus.append("POV: ");
                thisJoystickStatus.append(irr::core::stringc(event.JoystickEvent.POV).c_str());
        		device->getLogger()->log(thisJoystickStatus.c_str());
        		device->getLogger()->log("");

        		//If we've shown for all joysticks, don't show again.
        		if (event.JoystickEvent.Joystick+1 == joystickInfo.size()) {
        			lastShownJoystickStatus = device->getTimer()->getRealTime();
        		}

        	}

            // Keep joystick values the same unless they are being changed by user input
            irr::f32 newJoystickPort = previousJoystickPort;
            irr::f32 newJoystickStbd = previousJoystickStbd;
            irr::f32 newJoystickRudder = previousJoystickRudder;

            // DEE 10JAN23 vvvv Azimuth drive physical controls
	    // for disambuigity then define a separate variable for thrust levers, as they control the engine but are not the engine
	    irr::f32 newJoystickThrustLeverPort = previousJoystickThrustLeverPort;
	    irr::f32 newJoystickThrustLeverStbd = previousJoystickThrustLeverStbd;
	    // DEE 10JAN23 ^^^^


	    // DEE 10JAN23 vvvv
//            irr::f32 newJoystickAzimuthAngPort = previousJoystickAzimuthAngPort;
//            irr::f32 newJoystickAzimuthAngStbd = previousJoystickAzimuthAngStbd;
            irr::f32 newJoystickSchottelPort = previousJoystickSchottelPort;
            irr::f32 newJoystickSchottelStbd = previousJoystickSchottelStbd;
	    // DEE 10JAN23 ^^^^
            irr::f32 newJoystickBowThruster = previousJoystickBowThruster;
            irr::f32 newJoystickSternThruster = previousJoystickSternThruster;


            for (irr::u8 thisAxis = 0; thisAxis < event.JoystickEvent.NUMBER_OF_AXES; thisAxis++) {

                //Check which type we correspond to
                if (thisJoystick == joystickSetup.portJoystickNo && thisAxis == joystickSetup.portJoystickAxis) {
                    newJoystickPort = event.JoystickEvent.Axis[joystickSetup.portJoystickAxis]/32768.0;
                    //If previous value is Inf, store current value in previous and current, otherwise only in current
                    if (previousJoystickPort==INFINITY) {
                        previousJoystickPort = newJoystickPort;
                    }
                }
                if (thisJoystick == joystickSetup.stbdJoystickNo && thisAxis == joystickSetup.stbdJoystickAxis) {
                    newJoystickStbd = event.JoystickEvent.Axis[joystickSetup.stbdJoystickAxis]/32768.0;
                    //If previous value is Inf, store current value in previous and current, otherwise only in current
                    if (previousJoystickStbd==INFINITY) {
                        previousJoystickStbd = newJoystickStbd;
                    }
                }
                if (thisJoystick == joystickSetup.rudderJoystickNo && thisAxis == joystickSetup.rudderJoystickAxis) {
                    newJoystickRudder = 30*event.JoystickEvent.Axis[joystickSetup.rudderJoystickAxis]/32768.0;
                    //If previous value is Inf, store current value in previous and current, otherwise only in current
                    if (previousJoystickRudder==INFINITY) {
                        previousJoystickRudder = newJoystickRudder;
                    }
                }



// DEE 10 Jan 23 vvvv TODO change this to control port schottel not the azimuth, let ownship.cpp take care of the follow up

//		if (thisJoystick == joystickSetup.azimuth1JoystickNo && thisAxis == joystickSetup.azimuth1JoystickAxis) {
//                    newJoystickAzimuthAngPort = 180*event.JoystickEvent.Axis[joystickSetup.azimuth1JoystickAxis]/32768.0;
                    //If previous value is Inf, store current value in previous and current, otherwise only in current
//                    if (previousJoystickAzimuthAngPort==INFINITY) {
//                        previousJoystickAzimuthAngPort = newJoystickAzimuthAngPort;
//                    }
//                }
//                if (thisJoystick == joystickSetup.azimuth2JoystickNo && thisAxis == joystickSetup.azimuth2JoystickAxis) {
//                    newJoystickAzimuthAngStbd = 180*event.JoystickEvent.Axis[joystickSetup.azimuth2JoystickAxis]/32768.0;
                    //If previous value is Inf, store current value in previous and current, otherwise only in current
//                    if (previousJoystickAzimuthAngStbd==INFINITY) {
//                        previousJoystickAzimuthAngStbd = newJoystickAzimuthAngStbd;
//                    }
//                }


// TODO 10JAN23 apply scaling and offset to these

		// DEE 10JAN23 Port Thrust Lever for Azimuth Drive
                if (thisJoystick == joystickSetup.portThrustLever_joystickNo && thisAxis == joystickSetup.portThrustLever_channel) {
                    newJoystickThrustLeverPort = joystickSetup.thrustLeverPortDirection*(joystickSetup.thrustLeverPortOffset+(joystickSetup.thrustLeverPortScaling*event.JoystickEvent.Axis[joystickSetup.portThrustLever_channel]/32768.0));
                    //If previous value is Inf, store current value in previous and current, otherwise only in current
                    if (previousJoystickThrustLeverPort==INFINITY) {
                        previousJoystickThrustLeverPort = newJoystickThrustLeverPort;
                    }
                }

		// DEE 10JAN23 Stbd Thrust Lever for Azimuth Drive
                if (thisJoystick == joystickSetup.stbdThrustLever_joystickNo && thisAxis == joystickSetup.stbdThrustLever_channel) {
                    newJoystickThrustLeverStbd = joystickSetup.thrustLeverStbdDirection*(joystickSetup.thrustLeverStbdOffset+(joystickSetup.thrustLeverStbdScaling*event.JoystickEvent.Axis[joystickSetup.stbdThrustLever_channel]/32768.0));
                    //If previous value is Inf, store current value in previous and current, otherwise only in current
                    if (previousJoystickThrustLeverStbd==INFINITY) {
                        previousJoystickThrustLeverStbd = newJoystickThrustLeverStbd;
                    }
                }

		// DEE 10JAN23 Port Schottel for Azimuth Drive NB changed 180 to 360
                if (thisJoystick == joystickSetup.portSchottel_joystickNo && thisAxis == joystickSetup.portSchottel_channel) {
                    newJoystickSchottelPort = joystickSetup.schottelPortDirection*(joystickSetup.schottelPortOffset+180.0*joystickSetup.schottelPortScaling*(event.JoystickEvent.Axis[joystickSetup.portSchottel_channel]/32768.0));
                    //If previous value is Inf, store current value in previous and current, otherwise only in current
                    if (previousJoystickSchottelPort==INFINITY) {
                        previousJoystickSchottelPort = newJoystickSchottelPort;
                    }
                }

		// DEE 10JAN23 Stbd Schottel for Azimuth Drive
                if (thisJoystick == joystickSetup.stbdSchottel_joystickNo && thisAxis == joystickSetup.stbdSchottel_channel) {
                    newJoystickSchottelStbd = joystickSetup.schottelStbdDirection*(joystickSetup.schottelStbdOffset+180.0*joystickSetup.schottelStbdScaling*(event.JoystickEvent.Axis[joystickSetup.stbdSchottel_channel]/32768.0));
                    //If previous value is Inf, store current value in previous and current, otherwise only in current
                    if (previousJoystickSchottelStbd==INFINITY) {
                        previousJoystickSchottelStbd = newJoystickSchottelStbd;
                    }
                }
// TODO 10JAN23 To JAMES check apply scaling and offsets to these as I wrote it on holiday in tenerife and I dont have my rudimentary physical controls with me

		// DEE 10Jan23 ^^^^


                if (thisJoystick == joystickSetup.bowThrusterJoystickNo && thisAxis == joystickSetup.bowThrusterJoystickAxis) {
                    newJoystickBowThruster = event.JoystickEvent.Axis[joystickSetup.bowThrusterJoystickAxis]/32768.0;
                    //If previous value is Inf, store current value in previous and current, otherwise only in current
                    if (previousJoystickBowThruster==INFINITY) {
                        previousJoystickBowThruster = newJoystickBowThruster;
                    }
                }
                if (thisJoystick == joystickSetup.sternThrusterJoystickNo && thisAxis == joystickSetup.sternThrusterJoystickAxis) {
                    newJoystickSternThruster = event.JoystickEvent.Axis[joystickSetup.sternThrusterJoystickAxis]/32768.0;
                    //If previous value is Inf, store current value in previous and current, otherwise only in current
                    if (previousJoystickSternThruster==INFINITY) {
                        previousJoystickSternThruster = newJoystickSternThruster;
                    }
                }

            }

            //Do joystick stuff here

            //check if any have changed
            bool joystickChanged = false;
            irr::f32 portChange = fabs(newJoystickPort - previousJoystickPort);
            irr::f32 stbdChange = fabs(newJoystickStbd - previousJoystickStbd);
            irr::f32 wheelChange = fabs(newJoystickRudder - previousJoystickRudder);
// DEE 10JAN23 vvvv
//	    irr::f32 azimuth1AngChange = fabs(newJoystickAzimuthAngPort - previousJoystickAzimuthAngPort);
//          irr::f32 azimuth2AngChange = fabs(newJoystickAzimuthAngStbd - previousJoystickAzimuthAngStbd);


	    irr::f32 thrustLeverPortChange = fabs(newJoystickThrustLeverPort - previousJoystickThrustLeverPort);
	    irr::f32 thrustLeverStbdChange = fabs(newJoystickThrustLeverStbd - previousJoystickThrustLeverStbd);
	    irr::f32 schottelPortChange = fabs(newJoystickSchottelPort - previousJoystickSchottelPort);
            irr::f32 schottelStbdChange = fabs(newJoystickSchottelStbd - previousJoystickSchottelStbd);
// DEE 10JAN23 ^^^^




// DEE
//            irr::f32 rudderChange = fabs(newJoystickRudder - previousJoystickRudder);
            irr::f32 bowThrusterChange = fabs(newJoystickBowThruster - previousJoystickBowThruster);
            irr::f32 sternThrusterChange = fabs(newJoystickSternThruster - previousJoystickSternThruster);
// DEE
//            if (portChange > 0.01 || stbdChange > 0.01 || rudderChange > 0.01 || bowThrusterChange > 0.01 || sternThrusterChange > 0.01 )
            if (portChange > 0.01 || stbdChange > 0.01 || wheelChange > 0.01 ||
                bowThrusterChange > 0.01 || sternThrusterChange > 0.01 ||
                schottelPortChange > 0.01 || schottelStbdChange > 0.01 || thrustLeverPortChange > 0.01 || thrustLeverStbdChange > 0.01)
            {
                joystickChanged = true;
            }

            //If any have changed, use all (iff non-infinite)
            if (joystickChanged) {



		if (newJoystickPort<INFINITY) { // refers to the port engine control
                    irr::f32 mappedValue = lookup1D(newJoystickPort,joystickSetup.inputPoints, joystickSetup.outputPoints);
		    // DEE 10JAN23 vvvv
		    // if this an azidrive then change thrust lever.  In fact in the future I suggest that all engines are controlled via thrust lever
		    // as the bigger the engine, then the longer the spool up time is.


		    if (!(model->isAzimuthDrive())) {
		 	    model->setPortEngine(mappedValue);
	    	    } // fi
                    previousJoystickPort=newJoystickPort;
                }

                if (newJoystickStbd<INFINITY) { // refers to the starboard engine control
                    irr::f32 mappedValue = lookup1D(newJoystickStbd,joystickSetup.inputPoints, joystickSetup.outputPoints);
		    if (!(model->isAzimuthDrive())) {
			    model->setStbdEngine(mappedValue);
		    }
                    previousJoystickStbd=newJoystickStbd;
                }



		// Azimuth drive specific
		// prefer to separate off the azimuth drive code from the conventional code for clarity and ease of future modification
		//
		if (model->isAzimuthDrive()) {

    		    // Port Thrust Lever
                    if (newJoystickThrustLeverPort<INFINITY) {

                        irr::f32 mappedValue = lookup1D(newJoystickThrustLeverPort,joystickSetup.inputPoints, joystickSetup.outputPoints);
			// the above does range -1 to 1 as output, however we want a range 0..1, dont want to change the mappings so
			// we can ammend this to
			//mappedValue = (mappedValue*0.5)+0.5;
		    	//model->setPortThrustLever(mappedValue);
		    	model->setPortThrustLever(0.5+newJoystickThrustLeverPort*0.5);
                        previousJoystickThrustLeverPort=newJoystickThrustLeverPort;
                    }

		    // Stbd Thrust Lever
                    if (newJoystickThrustLeverStbd<INFINITY) {
                        irr::f32 mappedValue = lookup1D(newJoystickThrustLeverStbd,joystickSetup.inputPoints, joystickSetup.outputPoints);
			model->setStbdThrustLever(0.5+newJoystickThrustLeverStbd*0.5);
			//		    	model->setStbdThrustLever(mappedValue);
                        previousJoystickThrustLeverStbd=newJoystickThrustLeverStbd;
                    }

		    // Port Schottel
		    // mapping is completely inappropriate for schottel controls
                    if (newJoystickSchottelPort<INFINITY) {
                        //irr::f32 mappedValue = lookup1D(newJoystickSchottelPort,joystickSetup.inputPoints, joystickSetup.outputPoints);
		    	//model->setPortSchottel(mappedValue);
		    	model->setPortSchottel(newJoystickSchottelPort);
                        previousJoystickSchottelPort=newJoystickSchottelPort;
                    }

		    // Stbd Schottel
		    // mapping is completely inappropriate for schottel controls
                    if (newJoystickSchottelStbd<INFINITY) {
                        // irr::f32 mappedValue = lookup1D(newJoystickSchottelStbd,joystickSetup.inputPoints, joystickSetup.outputPoints);
		    	// model->setStbdSchottel(mappedValue);
		    	model->setStbdSchottel(newJoystickSchottelStbd);
                        previousJoystickSchottelStbd=newJoystickSchottelStbd;
                    }

		    // DEE note perhaps the "master" should be implemented in here



		} // end if is azimuth drive



// DEE 10JAN23 ^^^^ end of joystick engine controls




		if (newJoystickRudder<INFINITY) {
// DEE if the joystick rudder control is used then make it change the wheel not the rudder
                    model->setWheel(newJoystickRudder*joystickSetup.rudderDirection);
//                    model->setRudder(newJoystickRudder);
                    previousJoystickRudder=newJoystickRudder;
                }







                if (newJoystickBowThruster<INFINITY) {
                    model->setBowThruster(newJoystickBowThruster);
                    previousJoystickBowThruster=newJoystickBowThruster;
                }

                if (newJoystickSternThruster<INFINITY) {
                    model->setSternThruster(newJoystickSternThruster);
                    previousJoystickSternThruster=newJoystickSternThruster;
                }

            } // DEE 10JAN23 end if JoysickChanged

            //Check joystick buttons here
            //Make sure the joystickPreviousButtonStates has an entry for this joystick
            while (joystickPreviousButtonStates.size() <= thisJoystick) {
                joystickPreviousButtonStates.push_back(0); //All zeros equivalent to no buttons pressed
            }

            irr::u32 thisButtonState = event.JoystickEvent.ButtonStates;
            irr::u32 previousButtonState = joystickPreviousButtonStates.at(thisJoystick);

            //Horn
            if (thisJoystick == joystickSetup.joystickNoHorn) {
                if (IsButtonPressed(joystickSetup.joystickButtonHorn,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonHorn,previousButtonState)) {
                    model->startHorn();
                }
                if (!IsButtonPressed(joystickSetup.joystickButtonHorn,thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonHorn,previousButtonState)) {
                    model->endHorn();
                }
            }
            //Change view
            if (thisJoystick == joystickSetup.joystickNoChangeView) {
                if (IsButtonPressed(joystickSetup.joystickButtonChangeView,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonChangeView,previousButtonState)) {
                    model->changeView();
                    model->setMoveViewWithPrimary(true); //Allow the view to change automatically after this
                }
            }
            if (thisJoystick == joystickSetup.joystickNoChangeAndLockView) {
                if (IsButtonPressed(joystickSetup.joystickButtonChangeAndLockView,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonChangeAndLockView,previousButtonState)) {
                    model->changeView();
                    model->setMoveViewWithPrimary(false); //Don't allow the view to change automatically after this
                }
            }
            //Look step left
            if (thisJoystick == joystickSetup.joystickNoLookStepLeft) {
                if (IsButtonPressed(joystickSetup.joystickButtonLookStepLeft,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonLookStepLeft,previousButtonState)) {
                    model->lookStepLeft();
                }
            }
            //Look step right
            if (thisJoystick == joystickSetup.joystickNoLookStepRight) {
                if (IsButtonPressed(joystickSetup.joystickButtonLookStepRight,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonLookStepRight,previousButtonState)) {
                    model->lookStepRight();
                }
            }
            //Decrease bow thrust
            if (thisJoystick == joystickSetup.joystickNoDecreaseBowThrust) {
                if (IsButtonPressed(joystickSetup.joystickButtonDecreaseBowThrust,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonDecreaseBowThrust,previousButtonState)) {
                    model->setBowThrusterRate(-0.5);
                }
                if (!IsButtonPressed(joystickSetup.joystickButtonDecreaseBowThrust,thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonDecreaseBowThrust,previousButtonState)) {
                    model->setBowThrusterRate(0);
                }
            }
            //Increase bow thrust
            if (thisJoystick == joystickSetup.joystickNoIncreaseBowThrust) {
                if (IsButtonPressed(joystickSetup.joystickButtonIncreaseBowThrust,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonIncreaseBowThrust,previousButtonState)) {
                    model->setBowThrusterRate(0.5);
                }
                if (!IsButtonPressed(joystickSetup.joystickButtonIncreaseBowThrust,thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonIncreaseBowThrust,previousButtonState)) {
                    model->setBowThrusterRate(0);
                }
            }
            //Decrease stern thrust
            if (thisJoystick == joystickSetup.joystickNoDecreaseSternThrust) {
                if (IsButtonPressed(joystickSetup.joystickButtonDecreaseSternThrust,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonDecreaseSternThrust,previousButtonState)) {
                    model->setSternThrusterRate(-0.5);
                }
                if (!IsButtonPressed(joystickSetup.joystickButtonDecreaseSternThrust,thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonDecreaseSternThrust,previousButtonState)) {
                    model->setSternThrusterRate(0);
                }
            }
            //Increase stern thrust
            if (thisJoystick == joystickSetup.joystickNoIncreaseSternThrust) {
                if (IsButtonPressed(joystickSetup.joystickButtonIncreaseSternThrust,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonIncreaseSternThrust,previousButtonState)) {
                    model->setSternThrusterRate(0.5);
                }
                if (!IsButtonPressed(joystickSetup.joystickButtonIncreaseSternThrust,thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonIncreaseSternThrust,previousButtonState)) {
                    model->setSternThrusterRate(0);
                }
            }
            //Bearings on
            if (thisJoystick == joystickSetup.joystickNoBearingOn) {
                if (IsButtonPressed(joystickSetup.joystickButtonBearingOn,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonBearingOn,previousButtonState)) {
                    gui->showBearings();
                }
            }
            //Bearings off
            if (thisJoystick == joystickSetup.joystickNoBearingOff) {
                if (IsButtonPressed(joystickSetup.joystickButtonBearingOff,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonBearingOff,previousButtonState)) {
                    gui->hideBearings();
                }
            }
            //Zoom on
            if (thisJoystick == joystickSetup.joystickNoZoomOn) {
                if (IsButtonPressed(joystickSetup.joystickButtonZoomOn,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonZoomOn,previousButtonState)) {
                    gui->zoomOn();
                    model->setZoom(true);
                }
            }
            //Zoom off
            if (thisJoystick == joystickSetup.joystickNoZoomOff) {
                if (IsButtonPressed(joystickSetup.joystickButtonZoomOff,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonZoomOff,previousButtonState)) {
                    gui->zoomOff();
                    model->setZoom(false);
                }
            }
            //Look around
            //Left
            if (thisJoystick == joystickSetup.joystickNoLookLeft) {
                if (IsButtonPressed(joystickSetup.joystickButtonLookLeft,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonLookLeft,previousButtonState)) {
                    model->setPanSpeed(-5);
                }
                if (!IsButtonPressed(joystickSetup.joystickButtonLookLeft,thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonLookLeft,previousButtonState)) {
                    model->setPanSpeed(0);
                }
            }

            //Right
            if (thisJoystick == joystickSetup.joystickNoLookRight) {
                if (IsButtonPressed(joystickSetup.joystickButtonLookRight,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonLookRight,previousButtonState)) {
                    model->setPanSpeed(5);
                }
                if (!IsButtonPressed(joystickSetup.joystickButtonLookRight,thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonLookRight,previousButtonState)) {
                    model->setPanSpeed(0);
                }
            }

            //Up
            if (thisJoystick == joystickSetup.joystickNoLookUp) {
                if (IsButtonPressed(joystickSetup.joystickButtonLookUp,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonLookUp,previousButtonState)) {
                    model->setVerticalPanSpeed(5);
                }
                if (!IsButtonPressed(joystickSetup.joystickButtonLookUp,thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonLookUp,previousButtonState)) {
                    model->setVerticalPanSpeed(0);
                }
            }

            //Down
            if (thisJoystick == joystickSetup.joystickNoLookDown) {
                if (IsButtonPressed(joystickSetup.joystickButtonLookDown,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonLookDown,previousButtonState)) {
                    model->setVerticalPanSpeed(-5);
                }
                if (!IsButtonPressed(joystickSetup.joystickButtonLookDown,thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonLookDown,previousButtonState)) {
                    model->setVerticalPanSpeed(0);
                }
            }

            //Rudder pump 1 on
            if (thisJoystick == joystickSetup.joystickNoPump1On) {
                if (IsButtonPressed(joystickSetup.joystickButtonPump1On,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonPump1On,previousButtonState)) {
                    model->setRudderPumpState(1,true);
                    if (model->getRudderPumpState(2)) {
                        model->setAlarm(false); //Only turn off alarm if other pump is working
                    }
                }
            }

            //Rudder pump 1 off
            if (thisJoystick == joystickSetup.joystickNoPump1Off) {
                if (IsButtonPressed(joystickSetup.joystickButtonPump1Off,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonPump1Off,previousButtonState)) {
                    model->setRudderPumpState(1,false);
                    model->setAlarm(true);
                }
            }

            //Rudder pump 2 on
            if (thisJoystick == joystickSetup.joystickNoPump2On) {
                if (IsButtonPressed(joystickSetup.joystickButtonPump2On,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonPump2On,previousButtonState)) {
                    model->setRudderPumpState(2,true);
                    if (model->getRudderPumpState(1)) {
                        model->setAlarm(false); //Only turn off alarm if other pump is working
                    }
                }
            }

            //Rudder pump 2 off
            if (thisJoystick == joystickSetup.joystickNoPump2Off) {
                if (IsButtonPressed(joystickSetup.joystickButtonPump2Off,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonPump2Off,previousButtonState)) {
                    model->setRudderPumpState(2,false);
                    model->setAlarm(true);
                }
            }

            //Follow up rudder on
            if (thisJoystick == joystickSetup.joystickNoFollowUpOn) {
                if (IsButtonPressed(joystickSetup.joystickButtonFollowUpOn,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonFollowUpOn,previousButtonState)) {
                    model->setFollowUpRudderWorking(true);
                }
            }

            //Follow up rudder off
            if (thisJoystick == joystickSetup.joystickNoFollowUpOff) {
                if (IsButtonPressed(joystickSetup.joystickButtonFollowUpOff,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonFollowUpOff,previousButtonState)) {
                    model->setFollowUpRudderWorking(false);
                }
            }

            if (thisJoystick == joystickSetup.joystickNoNFUPort) {
                if (IsButtonPressed(joystickSetup.joystickButtonNFUPort,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonNFUPort,previousButtonState)) {
                    model->setWheel(-30,true);
                }
                if (!IsButtonPressed(joystickSetup.joystickButtonNFUPort,thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonNFUPort,previousButtonState)) {
                    model->setWheel(model->getRudder(),true);
                }
            }

            if (thisJoystick == joystickSetup.joystickNoNFUStbd) {
                if (IsButtonPressed(joystickSetup.joystickButtonNFUStbd,thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonNFUStbd,previousButtonState)) {
                    model->setWheel(30,true);
                }
                if (!IsButtonPressed(joystickSetup.joystickButtonNFUStbd,thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonNFUStbd,previousButtonState)) {
                    model->setWheel(model->getRudder(),true);
                }
            }

            if (thisJoystick == joystickSetup.joystickNoAckAlarm) {
                if (IsButtonPressed(joystickSetup.joystickButtonAckAlarm,thisButtonState)) {
                    model->setAlarm(false);
                }
            }


// DEE 10JAN23 .... Ive never seen the master concept implemented on azimuth drives in real life as in practice you can steer
// 			perfectly well with just one drive on passage.  Whilst Maneouvering or steaming in confined waters then
// 			both drives are needed to operate independently.
// 			When under autopilot, then the autopilot can be set to control port stbd or both azidrives.
// 		    Is it worth the effort of implementing master for drives ?
            if (thisJoystick == joystickSetup.joystickNoAzimuth1Master) {
                if (IsButtonPressed(joystickSetup.joystickButtonAzimuth1Master,thisButtonState)) {
                    // debounce:
                    if (device->getTimer()->getRealTime()-lastTimeAzimuth1MasterChanged > 500) {
                        // Allow azimuth master to change every 500ms (debounce)
                        model->setAzimuth1Master(!model->getAzimuth1Master());
                        lastTimeAzimuth1MasterChanged=device->getTimer()->getRealTime();
                    }
                }
            }

            if (thisJoystick == joystickSetup.joystickNoAzimuth2Master) {
                if (IsButtonPressed(joystickSetup.joystickButtonAzimuth2Master,thisButtonState)) {
                    // debounce:
                    if (device->getTimer()->getRealTime()-lastTimeAzimuth2MasterChanged > 500) {
                        // Allow azimuth master to change every 500ms (debounce)
                        model->setAzimuth2Master(!model->getAzimuth2Master());
                        lastTimeAzimuth2MasterChanged=device->getTimer()->getRealTime();
                    }
                }
            }

            //Store previous settings
            joystickPreviousButtonStates.at(thisJoystick) = event.JoystickEvent.ButtonStates;

            //POV hat
            if (previousJoystickPOVInitialised && thisJoystick == joystickSetup.joystickNoPOV) {
                if (event.JoystickEvent.POV == joystickSetup.joystickPOVLookLeft && previousJoystickPOV != joystickSetup.joystickPOVLookLeft) {
                    model->setPanSpeed(-5);
                }
                if (event.JoystickEvent.POV != joystickSetup.joystickPOVLookLeft && previousJoystickPOV == joystickSetup.joystickPOVLookLeft) {
                    model->setPanSpeed(0);
                }

                if (event.JoystickEvent.POV == joystickSetup.joystickPOVLookRight && previousJoystickPOV != joystickSetup.joystickPOVLookRight) {
                    model->setPanSpeed(5);
                }
                if (event.JoystickEvent.POV != joystickSetup.joystickPOVLookRight && previousJoystickPOV == joystickSetup.joystickPOVLookRight) {
                    model->setPanSpeed(0);
                }

                if (event.JoystickEvent.POV == joystickSetup.joystickPOVLookUp && previousJoystickPOV != joystickSetup.joystickPOVLookUp) {
                    model->setVerticalPanSpeed(5);
                }
                if (event.JoystickEvent.POV != joystickSetup.joystickPOVLookUp && previousJoystickPOV == joystickSetup.joystickPOVLookUp) {
                    model->setVerticalPanSpeed(0);
                }

                if (event.JoystickEvent.POV == joystickSetup.joystickPOVLookDown && previousJoystickPOV != joystickSetup.joystickPOVLookDown) {
                    model->setVerticalPanSpeed(-5);
                }
                if (event.JoystickEvent.POV != joystickSetup.joystickPOVLookDown && previousJoystickPOV == joystickSetup.joystickPOVLookDown) {
                    model->setVerticalPanSpeed(0);
                }
                previousJoystickPOV = event.JoystickEvent.POV; //Store for next time
            }
        }

        return false;

    }

    irr::f32 MyEventReceiver::lookup1D(irr::f32 lookupValue, std::vector<irr::f32> inputPoints, std::vector<irr::f32> outputPoints)
    {
        //Check that the input and output points list are the same length
        if (inputPoints.size() != outputPoints.size() || inputPoints.size() < 2) {
            std::cerr << "Error: lookup1D needs inputPoints and outputPoints list size to be the same, and needs at least two points." << std::endl;
            return 0;
        }

        std::vector<irr::f32>::size_type numberOfPoints = inputPoints.size();

        //Check that inputPoints does not have decreasing values (must be increasing or equal)
        for (unsigned int i=0; i+1<numberOfPoints; i++) {
            if (inputPoints.at(i+1) < inputPoints.at(i)) {
                std::cerr << "Error: inputPoints to lookup1D must not be in a decreasing order." << std::endl;
                return 0;
            }
        }

        //Return first output if at or below lowest input
        if (lookupValue <= inputPoints.at(0)) {
            return outputPoints.at(0);
        }

        //Return last output if at or above highest input
        if (lookupValue >= inputPoints.at(numberOfPoints-1)) {
            return outputPoints.at(numberOfPoints-1);
        }

        //Main interpolation
        //Find the first point above the one we're interested in
        unsigned int nextPoint=1;
        while (nextPoint < numberOfPoints && inputPoints.at(nextPoint)<=lookupValue) {
            nextPoint++;
        }

        //check for div by zero - shouldn't happen, but protect against
        if (inputPoints.at(nextPoint)-inputPoints.at(nextPoint-1) == 0)
            return 0.0;

        //do interpolation
        return outputPoints.at(nextPoint-1) + (outputPoints.at(nextPoint)-outputPoints.at(nextPoint-1))*(lookupValue-inputPoints.at(nextPoint-1))/(inputPoints.at(nextPoint)-inputPoints.at(nextPoint-1));


    }

    std::wstring MyEventReceiver::f32To3dp(irr::f32 value, bool stripZeros)
    {
        //Convert a floating point value to a wstring, with 3dp
        char tempStr[100];
        snprintf(tempStr,100,"%.3f",value);
        std::wstring outputWstring = std::wstring(tempStr, tempStr+strlen(tempStr));
        //Strip trailing zeros and decimal point
        if (stripZeros) {
            while (outputWstring.back() == '0') {
                outputWstring.pop_back();
            }
            if (outputWstring.back() == '.') {
                outputWstring.pop_back();
            }
        }
        return outputWstring;
    }

    bool MyEventReceiver::IsButtonPressed(irr::u32 button, irr::u32 buttonBitmap) const
    {
        if(button >= 32)
            return false;

        return (buttonBitmap & (1 << button)) ? true : false;
    }

    void MyEventReceiver::startShutdown() {
        model->setAccelerator(0.0);
        device->sleep(500);
        if (!shutdownDialogActive) {
            device->getGUIEnvironment()->addMessageBox(L"Quit?",L"Quit?",true,irr::gui::EMBF_OK|irr::gui::EMBF_CANCEL,0,GUIMain::GUI_ID_CLOSE_BOX);//I18n
            shutdownDialogActive = true;
        }
    }

/*
	irr::s32 MyEventReceiver::GetScrollBarPosSpeed() const
	{
		return scrollBarPosSpeed;
	}

	irr::s32 MyEventReceiver::GetScrollBarPosHeading() const
	{
		return scrollBarPosHeading;
	}
*/
