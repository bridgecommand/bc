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
#include "Lines.hpp"
#include "Utilities.hpp"
#include "AzimuthDial.h"
#include "VRInterface.hpp"
#include "Constants.hpp"
#include "Message.hpp"

// using namespace irr;

MyEventReceiver::MyEventReceiver(irr::IrrlichtDevice *dev, SimulationModel *model, GUIMain *gui, Network *network, VRInterface* vrInterface, JoystickSetup joystickSetup, std::vector<std::string> *logMessages) // Constructor
{
    this->model = model; // Link to the model
    this->gui = gui;     // Link to GUI
    this->vrInterface = vrInterface; // Link to VR interface
    scrollBarPosSpeed = 0;
    scrollBarPosHeading = 0;

    // store device
    device = dev;

    //network
    net = network;
    
    lastShownJoystickStatus = device->getTimer()->getRealTime() - 5000;      // Show joystick raw data every 5s in log

    // set up joystick if present, and inform user what's available
    dev->activateJoysticks(joystickInfo);

    // Tell user about joysticks via the log
    dev->getLogger()->log(""); // add a blank line
    std::string joystickInfoMessage = "Number of joysticks detected: ";
    joystickInfoMessage.append(std::string(irr::core::stringc(joystickInfo.size()).c_str()));
    dev->getLogger()->log(joystickInfoMessage.c_str());
    for (unsigned int i = 0; i < joystickInfo.size(); i++)
    {
        // Print out name and number of each joystick
        joystickInfoMessage = "Joystick number: ";
        joystickInfoMessage.append(irr::core::stringc(i).c_str());
        joystickInfoMessage.append(", Name: ");
        joystickInfoMessage.append(std::string(joystickInfo[i].Name.c_str()));
        dev->getLogger()->log(joystickInfoMessage.c_str());
    }
    dev->getLogger()->log(""); // add a blank line

    this->joystickSetup = joystickSetup;

    // Indicate that previous joystick information hasn't been initialised
    previousJoystickRudder = INFINITY;
    previousJoystickBowThruster = INFINITY;
    previousJoystickSternThruster = INFINITY;
  
    previousJoystickPOVInitialised = false;

    this->logMessages = logMessages;

    // assume mouse buttons not pressed initially
    leftMouseDown = false;
    rightMouseDown = false;

    shutdownDialogActive = false;

    linesMode = 0;
}

bool MyEventReceiver::OnEvent(const irr::SEvent &event)
{

    // std::cout << "Any event in receiver" << std::endl;
    // From log
    if (event.EventType == irr::EET_LOG_TEXT_EVENT)
    {
        // Store these in a global log.
        std::string eventText(event.LogEvent.Text);
        logMessages->push_back(eventText);
        return true;
    }

    // Special event used to pass VR click event for mooring lines
    if (event.EventType == irr::EET_USER_EVENT)
    {
        if ((linesMode == 1) || (linesMode == 2))
        {
            irr::core::line3df rayForLines;
            if (vrInterface->getRayFromController(&rayForLines, 1000.0)) {
                // Ray found from VR interface
                handleMooringLines(rayForLines);
            }
        }
    }
    
    // From mouse - keep track of button press state
    if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
    {
        if (event.MouseInput.Event == irr::EMIE_LMOUSE_PRESSED_DOWN)
        {
            leftMouseDown = true;
            // Log position of mouse click, so we can track relative movement
            mouseClickX = event.MouseInput.X;
            mouseClickY = event.MouseInput.Y;
        }
        if (event.MouseInput.Event == irr::EMIE_LMOUSE_LEFT_UP)
        {
            leftMouseDown = false;
        }
        if (event.MouseInput.Event == irr::EMIE_RMOUSE_PRESSED_DOWN)
        {
            rightMouseDown = true;
            // Force focus on right click
            irr::gui::IGUIElement *overElement;
            overElement = device->getGUIEnvironment()->getRootGUIElement()->getElementFromPoint(irr::core::position2d<irr::s32>(event.MouseInput.X, event.MouseInput.Y));
            if (overElement)
            {
                device->getGUIEnvironment()->setFocus(overElement);
            }
        }
        if (event.MouseInput.Event == irr::EMIE_RMOUSE_LEFT_UP)
        {
            rightMouseDown = false;
        }
        model->setMouseDown(leftMouseDown || rightMouseDown); // Set if either mouse is down

        // Mooring lines controls
        if (event.MouseInput.Event == irr::EMIE_LMOUSE_PRESSED_DOWN)
        {
            // Add line (mooring/towing) start or end if in required mode
            if ((linesMode == 1) || (linesMode == 2))
            {
                // Ignore click if over a gui element (getElementFromPoint will return root element if not over anything else)
                irr::gui::IGUIElement* rootGUIElement = device->getGUIEnvironment()->getRootGUIElement();
                irr::gui::IGUIElement* clickElement = rootGUIElement->getElementFromPoint(irr::core::position2d<irr::s32>(event.MouseInput.X, event.MouseInput.Y));
                if (clickElement == rootGUIElement)
                {
                    // Scale if required because 3d view may be different
                    irr::s32 scaledMouseY = mouseClickY;
                    if (gui->getShowInterface())
                    {
                        scaledMouseY = mouseClickY / VIEW_PROPORTION_3D;
                    }
                    irr::core::line3df rayForLines = device->getSceneManager()->getSceneCollisionManager()->getRayFromScreenCoordinates(irr::core::position2d<irr::s32>(mouseClickX, scaledMouseY));
                    handleMooringLines(rayForLines);
                }
            }
        }

        // Check mouse movement
        if (event.MouseInput.Event == irr::EMIE_MOUSE_MOVED && leftMouseDown)
        {
            // Check if focus in on a gui element
            irr::gui::IGUIElement *focussedElement;
            focussedElement = device->getGUIEnvironment()->getFocus();
            if (!focussedElement)
            {
                irr::s32 deltaX = event.MouseInput.X - mouseClickX;
                irr::s32 deltaY = event.MouseInput.Y - mouseClickY;
                model->changeLookPx(deltaX, deltaY);
            }
            // Store for next time
            mouseClickX = event.MouseInput.X;
            mouseClickY = event.MouseInput.Y;
        }

        if (event.MouseInput.Event == irr::EMIE_MOUSE_WHEEL)
        {
            if (event.MouseInput.Wheel < 0)
            {
                model->setWheel(model->getWheel() + 1.0);
            }
            if (event.MouseInput.Wheel > 0)
            {
                model->setWheel(model->getWheel() - 1.0);
            }
            return true;
        }
    }

    if (event.EventType == irr::EET_GUI_EVENT)
    {
        irr::s32 id = event.GUIEvent.Caller->getID();

        if (event.GUIEvent.EventType == irr::gui::EGET_LISTBOX_SELECTED_AGAIN)
        {
            if (id == GUIMain::GUI_ID_LINES_LIST)
            {
                // Allow de-selection by double click
                if (!vrInterface->isVRActive()) {
                    // Workaround for now for VR mode, as 'SELECTED_AGAIN' always seems to run
                    ((irr::gui::IGUIListBox*)event.GUIEvent.Caller)->setSelected(-1);
                }
                model->getLines()->setSelectedLine(((irr::gui::IGUIListBox *)event.GUIEvent.Caller)->getSelected());
            }

            if (id == GUIMain::GUI_ID_ARPA_LIST || id == GUIMain::GUI_ID_BIG_ARPA_LIST)
            {
                // Allow de-selection
                int arpaSelected = -1;
                if (vrInterface->isVRActive()) {
                    // Workaround for now for VR mode, as 'SELECTED_AGAIN' always seems to run
                    arpaSelected = ((irr::gui::IGUIListBox*)event.GUIEvent.Caller)->getSelected();
                }
                gui->setARPAList(arpaSelected);
                // Set selected ID via model.
                model->setArpaListSelection(arpaSelected);
            }
        }

        if (event.GUIEvent.EventType == irr::gui::EGET_LISTBOX_CHANGED)
        {
            if (id == GUIMain::GUI_ID_LINES_LIST)
            {
                model->getLines()->setSelectedLine(((irr::gui::IGUIListBox *)event.GUIEvent.Caller)->getSelected());
            }

            if (id == GUIMain::GUI_ID_ARPA_LIST || id == GUIMain::GUI_ID_BIG_ARPA_LIST)
            {
                // Set coupled list
                int arpaSelected = ((irr::gui::IGUIListBox *)event.GUIEvent.Caller)->getSelected();
                gui->setARPAList(arpaSelected);

                // Set selected ID via model.
                model->setArpaListSelection(arpaSelected);
            }
        }

        if (event.GUIEvent.EventType == irr::gui::EGET_CHECKBOX_CHANGED)
        {
	  /*            if (id == GUIMain::GUI_ID_AZIMUTH_1_MASTER_CHECKBOX)
            {
                model->setAzimuth1Master(((irr::gui::IGUICheckBox *)event.GUIEvent.Caller)->isChecked());
            }

            if (id == GUIMain::GUI_ID_AZIMUTH_2_MASTER_CHECKBOX)
            {
                model->setAzimuth2Master(((irr::gui::IGUICheckBox *)event.GUIEvent.Caller)->isChecked());
		}*/

            if (id == GUIMain::GUI_ID_KEEP_SLACK_LINE_CHECKBOX)
            {
                model->getLines()->setKeepSlack(
                    model->getLines()->getSelectedLine(),
                    ((irr::gui::IGUICheckBox *)event.GUIEvent.Caller)->isChecked());
            }

            if (id == GUIMain::GUI_ID_HAUL_IN_LINE_CHECKBOX)
            {
                model->getLines()->setHeaveIn(
                    model->getLines()->getSelectedLine(),
                    ((irr::gui::IGUICheckBox *)event.GUIEvent.Caller)->isChecked());
            }

            if (id == GUIMain::GUI_ID_STREAMOVERRIDE_BOX) 
            {
                model->setStreamOverride(((irr::gui::IGUICheckBox *)event.GUIEvent.Caller)->isChecked());
            }
        }

        if (event.GUIEvent.EventType == irr::gui::EGET_SCROLL_BAR_CHANGED)
        {

            if (id == GUIMain::GUI_ID_HEADING_SCROLL_BAR)
            {
                scrollBarPosHeading = ((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos();
                model->setHeading(scrollBarPosHeading);
            }

            if (id == GUIMain::GUI_ID_SPEED_SCROLL_BAR)
            {
                scrollBarPosSpeed = ((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos();
                model->setSpeed(scrollBarPosSpeed);
            }

            if (id == GUIMain::GUI_ID_STBD_SCROLL_BAR)
            {
                irr::f32 value = ((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos() / -100.0; // Convert to from +-100 to +-1, and invert up/down
                model->setStbdEngine(value);
                // If right mouse button, set the other engine as well
                if (rightMouseDown)
                {
                    model->setPortEngine(value);
                }
            }
            if (id == GUIMain::GUI_ID_PORT_SCROLL_BAR)
            {
                irr::f32 value = ((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos() / -100.0; // Convert to from +-100 to +-1, and invert up/down
                model->setPortEngine(value);
                // If right mouse button, set the other engine as well
                if (rightMouseDown)
                {
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
                // Check if either NFU button is down, in which case force the change (even if the follow up rudder isn't working)
                bool nfuActive = gui->isNFUActive();
                model->setWheel(((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos(), nfuActive);
            }
            // DEE capture the wheel

          
            if (id == GUIMain::GUI_ID_BOWTHRUSTER_SCROLL_BAR)
            {
                irr::f32 value = ((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos() / 100.0; // Convert to from +-100 to +-1
                //model->setBowThruster(value);
            }
            if (id == GUIMain::GUI_ID_STERNTHRUSTER_SCROLL_BAR)
            {
                irr::f32 value = ((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos() / 100.0; // Convert to from +-100 to +-1
                //model->setSternThruster(value);
            }

            if (id == GUIMain::GUI_ID_RADAR_GAIN_SCROLL_BAR)
            {
                model->setRadarGain(((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos());
            }
            if (id == GUIMain::GUI_ID_RADAR_CLUTTER_SCROLL_BAR)
            {
                model->setRadarClutter(((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos());
            }
            if (id == GUIMain::GUI_ID_RADAR_RAIN_SCROLL_BAR)
            {
                model->setRadarRain(((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos());
            }
            if (id == GUIMain::GUI_ID_WEATHER_SCROLL_BAR)
            {
                model->setWeather(((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos() / 10.0); // Scroll bar 0-120, weather 0-12
            }
            if (id == GUIMain::GUI_ID_RAIN_SCROLL_BAR)
            {
                model->setRain(((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos() / 10.0); // Scroll bar 0-100, rain 0-10
            }
            if (id == GUIMain::GUI_ID_VISIBILITY_SCROLL_BAR)
            {
                model->setVisibility(((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos() / 10.0); // Scroll bar 1-101, vis 0.1-10.1
            }
            if (id == GUIMain::GUI_ID_WINDDIRECTION_SCROLL_BAR)
            {
                model->setWindDirection(((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos());
            }
            if (id == GUIMain::GUI_ID_WINDSPEED_SCROLL_BAR)
            {
                model->setWindSpeed(((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos());
            }
            if (id == GUIMain::GUI_ID_STREAMDIRECTION_SCROLL_BAR)
            {
                model->setStreamOverrideDirection(((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos());
            }
            if (id == GUIMain::GUI_ID_STREAMSPEED_SCROLL_BAR)
            {
                model->setStreamOverrideSpeed(((irr::gui::IGUIScrollBar *)event.GUIEvent.Caller)->getPos());
            }   
            if (id == GUIMain::GUI_ID_MAGNIFICATION_SCROLL_BAR)
            {   
                irr::s32 rawZoomLevel = ((irr::gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                irr::f32 zoomLevel = (irr::f32)rawZoomLevel / 10.0;
                if (rawZoomLevel > 10) {
                    gui->zoomOn();
                    model->setZoom(true, zoomLevel);
                } else {
                    gui->zoomOff();
                    model->setZoom(false, zoomLevel);
                }
            }

        }

        if (event.GUIEvent.EventType == irr::gui::EGET_MESSAGEBOX_OK)
        {
            if (id == GUIMain::GUI_ID_CLOSE_BOX)
            {
	      std::string shut = Message::ShutDown();
	      net->SendMessage(shut, true);
	      device->closeDevice(); // Confirm shutdown.
            }
        }

        if (event.GUIEvent.EventType == irr::gui::EGET_MESSAGEBOX_CANCEL)
        {
            if (id == GUIMain::GUI_ID_CLOSE_BOX)
            {
                shutdownDialogActive = false;
            }
        }

        if (event.GUIEvent.EventType == irr::gui::EGET_BUTTON_CLICKED)
        {

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
                model->setZoom(((irr::gui::IGUIButton *)event.GUIEvent.Caller)->isPressed());
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

            if (id == GUIMain::GUI_ID_RADAR_INCREASE_X_BUTTON)
            {
                model->increaseRadarXCursor();
            }

            if (id == GUIMain::GUI_ID_RADAR_DECREASE_X_BUTTON)
            {
                model->decreaseRadarXCursor();
            }

            if (id == GUIMain::GUI_ID_RADAR_INCREASE_Y_BUTTON)
            {
                model->increaseRadarYCursor();
            }

            if (id == GUIMain::GUI_ID_RADAR_DECREASE_Y_BUTTON)
            {
                model->decreaseRadarYCursor();
            }

            if (id == GUIMain::GUI_ID_RADAR_COLOUR_BUTTON)
            {
                model->changeRadarColourChoice();
            }

            // Radar mode buttons
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

            // Manual/MARPA acquire/update
            if (id == GUIMain::GUI_ID_MANUAL_SCAN_BUTTON)
            {
                if (model->getArpaMode() == 0)
                {
                    model->addManualPoint(false);
                }
                // Don't do anything in full ARPA mode (as updated automatically)
            }

            if (id == GUIMain::GUI_ID_MANUAL_NEW_BUTTON)
            {
                if (model->getArpaMode() == 0)
                {
                    model->addManualPoint(true);
                }
                else if (model->getArpaMode() == 1)
                {
                    model->trackTargetFromCursor();
                }
                // TODO: Should we allow user to trigger manual tracking in full ARPA?
            }

            if (id == GUIMain::GUI_ID_MANUAL_CLEAR_BUTTON)
            {
                if (model->getArpaMode() == 0)
                {
                    model->clearManualPoints();
                }
                else if (model->getArpaMode() == 1)
                {
                    model->clearTargetFromCursor();
                }
                // TODO: Should we allow user to manually stop tracking in full ARPA?
                // And should this allow clearing of manual target if acquired in manual mode, but switched to MARPA/ARPA
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

            if (id == GUIMain::GUI_ID_HIDE_LINES_CONTROLS_BUTTON)
            {
                gui->setLinesControlsWindowVisible(false);
            }

            if (id == GUIMain::GUI_ID_SHOW_LINES_CONTROLS_BUTTON)
            {
                gui->setLinesControlsWindowVisible(true);
            }

            if (id == GUIMain::GUI_ID_RUDDERPUMP_1_WORKING_BUTTON)
            {
	      /*model->setRudderPumpState(1, true);
                if (model->getRudderPumpState(2))
                {
                    model->setAlarm(false); // Only turn off alarm if other pump is working
		    }*/
            }

            if (id == GUIMain::GUI_ID_RUDDERPUMP_1_FAILED_BUTTON)
            {
	      //model->setRudderPumpState(1, false);
	      //model->setAlarm(true);
            }

            if (id == GUIMain::GUI_ID_RUDDERPUMP_2_WORKING_BUTTON)
            {
	      /*model->setRudderPumpState(2, true);
                if (model->getRudderPumpState(1))
                {
                    model->setAlarm(false); // Only turn off alarm if other pump is working
		    }*/
            }

            if (id == GUIMain::GUI_ID_RUDDERPUMP_2_FAILED_BUTTON)
            {
	      //model->setRudderPumpState(2, false);
	      //model->setAlarm(true);
            }

            if (id == GUIMain::GUI_ID_FOLLOWUP_WORKING_BUTTON)
            {
	      //model->setFollowUpRudderWorking(true);
            }

            if (id == GUIMain::GUI_ID_FOLLOWUP_FAILED_BUTTON)
            {
	      //model->setFollowUpRudderWorking(false);
            }

            if (id == GUIMain::GUI_ID_ACK_ALARMS_BUTTON)
            {
	      //model->setAlarm(false);
            }

            if (id == GUIMain::GUI_ID_ADD_LINE_BUTTON)
            {
                linesMode = 1;
                model->addLine();
                gui->setLinesControlsText("Click in 3d view to set start position for line (on own ship)"); // TODO: Add translation
            }

            if (id == GUIMain::GUI_ID_REMOVE_LINE_BUTTON)
            {
                model->getLines()->removeLine(model->getLines()->getSelectedLine());
            }

            if (id == GUIMain::GUI_ID_CHANGE_VIEW_BUTTON)
            {
                model->changeView();
            }

        } // Button clicked

        if (event.GUIEvent.EventType == irr::gui::EGET_COMBO_BOX_CHANGED)
        {

            if ((id == GUIMain::GUI_ID_ARPA_ON_BOX || id == GUIMain::GUI_ID_BIG_ARPA_ON_BOX))
            {
                // ARPA on/off options
                irr::s32 boxState = ((irr::gui::IGUIComboBox *)event.GUIEvent.Caller)->getSelected();
                model->setArpaMode(boxState);

                // Set the linked checkbox (big/small radar window)
                gui->setARPAComboboxes(boxState);
            }

            if (id == GUIMain::GUI_ID_ARPA_TRUE_REL_BOX || id == GUIMain::GUI_ID_BIG_ARPA_TRUE_REL_BOX)
            {
                irr::s32 selected = ((irr::gui::IGUIComboBox *)event.GUIEvent.Caller)->getSelected();
                if (selected == 0)
                {
                    model->setRadarARPATrue();
                }
                else if (selected == 1)
                {
                    model->setRadarARPARel();
                }

                // Set both linked inputs - brute force
                irr::gui::IGUIElement *other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_ARPA_TRUE_REL_BOX, true);
                if (other != 0)
                {
                    ((irr::gui::IGUIComboBox *)other)->setSelected(((irr::gui::IGUIComboBox *)event.GUIEvent.Caller)->getSelected());
                }
                other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_ARPA_TRUE_REL_BOX, true);
                if (other != 0)
                {
                    ((irr::gui::IGUIComboBox *)other)->setSelected(((irr::gui::IGUIComboBox *)event.GUIEvent.Caller)->getSelected());
                }
            }

        } // Combo box

        if ((id == GUIMain::GUI_ID_ARPA_VECTOR_TIME_BOX || id == GUIMain::GUI_ID_BIG_ARPA_VECTOR_TIME_BOX) && (event.GUIEvent.EventType == irr::gui::EGET_EDITBOX_ENTER || event.GUIEvent.EventType == irr::gui::EGET_ELEMENT_FOCUS_LOST))
        {
            std::wstring boxWString = std::wstring(((irr::gui::IGUIEditBox *)event.GUIEvent.Caller)->getText());
            std::string boxString(boxWString.begin(), boxWString.end());
            irr::f32 value = Utilities::lexical_cast<irr::f32>(boxString);

            if (value > 0 && value <= 60)
            {
                model->setRadarARPAVectors(value);
            }
            else
            {
                event.GUIEvent.Caller->setText(L"Invalid");
            }

            // Set both linked inputs - brute force
            irr::gui::IGUIElement *other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_ARPA_VECTOR_TIME_BOX, true);
            if (other != 0)
            {
                other->setText(event.GUIEvent.Caller->getText());
            }
            other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_ARPA_VECTOR_TIME_BOX, true);
            if (other != 0)
            {
                other->setText(event.GUIEvent.Caller->getText());
            }
        }

        // Radar PI controls
        // PI selected
        if (event.GUIEvent.EventType == irr::gui::EGET_COMBO_BOX_CHANGED)
        {
            if (id == GUIMain::GUI_ID_PI_SELECT_BOX || id == GUIMain::GUI_ID_BIG_PI_SELECT_BOX)
            {

                // Set to match
                if (id == GUIMain::GUI_ID_PI_SELECT_BOX)
                { // Selected on small screen
                    irr::gui::IGUIElement *other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_PI_SELECT_BOX, true);
                    if (other != 0)
                    {
                        ((irr::gui::IGUIComboBox *)other)->setSelected(((irr::gui::IGUIComboBox *)event.GUIEvent.Caller)->getSelected());
                    }
                }
                else
                { // Selected on big screen
                    irr::gui::IGUIElement *other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_SELECT_BOX, true);
                    if (other != 0)
                    {
                        ((irr::gui::IGUIComboBox *)other)->setSelected(((irr::gui::IGUIComboBox *)event.GUIEvent.Caller)->getSelected());
                    }
                }

                // Get PI data for the newly selected PI
                irr::s32 selectedPI = ((irr::gui::IGUIComboBox *)event.GUIEvent.Caller)->getSelected(); //(-1 or 0-9)
                // TODO: Use this to get data from model, and set fields
                irr::gui::IGUIElement *piBrg = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_BEARING_BOX, true);
                irr::gui::IGUIElement *piRng = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_RANGE_BOX, true);
                irr::gui::IGUIElement *piBrgBig = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_PI_BEARING_BOX, true);
                irr::gui::IGUIElement *piRngBig = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_PI_RANGE_BOX, true);
                if (piBrg && piRng && piBrgBig && piRngBig)
                {
                    piBrg->setText(f32To3dp(model->getPIbearing(selectedPI), true).c_str());
                    piBrgBig->setText(f32To3dp(model->getPIbearing(selectedPI), true).c_str());
                    piRng->setText(f32To3dp(model->getPIrange(selectedPI), true).c_str());
                    piRngBig->setText(f32To3dp(model->getPIrange(selectedPI), true).c_str());
                }
            }
        }

        if (event.GUIEvent.EventType == irr::gui::EGET_EDITBOX_CHANGED || event.GUIEvent.EventType == irr::gui::EGET_EDITBOX_ENTER || event.GUIEvent.EventType == irr::gui::EGET_ELEMENT_FOCUS_LOST)
        {

            // Bearing/range boxes:
            if (id == GUIMain::GUI_ID_PI_BEARING_BOX || id == GUIMain::GUI_ID_BIG_PI_BEARING_BOX || id == GUIMain::GUI_ID_PI_RANGE_BOX || id == GUIMain::GUI_ID_BIG_PI_RANGE_BOX)
            {
                // Make the controls match
                if (id == GUIMain::GUI_ID_PI_BEARING_BOX)
                {
                    irr::gui::IGUIElement *other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_PI_BEARING_BOX, true);
                    if (other != 0)
                    {
                        other->setText(event.GUIEvent.Caller->getText());
                    }
                }
                if (id == GUIMain::GUI_ID_BIG_PI_BEARING_BOX)
                {
                    irr::gui::IGUIElement *other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_BEARING_BOX, true);
                    if (other != 0)
                    {
                        other->setText(event.GUIEvent.Caller->getText());
                    }
                }
                if (id == GUIMain::GUI_ID_PI_RANGE_BOX)
                {
                    irr::gui::IGUIElement *other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_PI_RANGE_BOX, true);
                    if (other != 0)
                    {
                        other->setText(event.GUIEvent.Caller->getText());
                    }
                }
                if (id == GUIMain::GUI_ID_BIG_PI_RANGE_BOX)
                {
                    irr::gui::IGUIElement *other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_RANGE_BOX, true);
                    if (other != 0)
                    {
                        other->setText(event.GUIEvent.Caller->getText());
                    }
                }

                if (event.GUIEvent.EventType == irr::gui::EGET_EDITBOX_ENTER || event.GUIEvent.EventType == irr::gui::EGET_ELEMENT_FOCUS_LOST)
                {
                    // Use the result
                    irr::gui::IGUIElement *piCombo = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_SELECT_BOX, true);
                    irr::gui::IGUIElement *piBrg = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_BEARING_BOX, true);
                    irr::gui::IGUIElement *piRng = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_RANGE_BOX, true);
                    if (piCombo && piBrg && piRng)
                    {
                        irr::s32 selectedPI = ((irr::gui::IGUIComboBox *)piCombo)->getSelected(); //(0-9)

                        std::wstring brgWString = std::wstring(piBrg->getText());
                        std::string brgString(brgWString.begin(), brgWString.end());
                        irr::f32 bearingChosen = Utilities::lexical_cast<irr::f32>(brgString);

                        std::wstring rngWString = std::wstring(piRng->getText());
                        std::string rngString(rngWString.begin(), rngWString.end());
                        irr::f32 rangeChosen = Utilities::lexical_cast<irr::f32>(rngString);

                        // Apply to model
                        model->setPIData(selectedPI, bearingChosen, rangeChosen);
                    }
                }
            }
        }

    } // GUI Event

    // From keyboard
    if (event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown)
    {
        // Check here that there isn't focus on a GUI edit box. If we are, don't process key inputs here.
        irr::gui::IGUIElement *focussedElement = device->getGUIEnvironment()->getFocus();
        if (!(focussedElement && focussedElement->getType() == irr::gui::EGUIET_EDIT_BOX))
        {

            if (event.KeyInput.Shift && event.KeyInput.Control)
            {

                switch (event.KeyInput.Key)
                {
                // Move camera
                case irr::KEY_UP:
                    device->getGUIEnvironment()->setFocus(0); // Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                    model->moveCameraForwards();
                    break;
                case irr::KEY_DOWN:
                    device->getGUIEnvironment()->setFocus(0); // Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                    model->moveCameraBackwards();
                    break;
                case irr::KEY_SPACE:
                    device->getGUIEnvironment()->setFocus(0); // Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                    model->toggleFrozenCamera();
                    break;
                default:
                    // don't do anything
                    break;
                }
            }
            else if (event.KeyInput.Shift)
            {
                // Shift down

                switch (event.KeyInput.Key)
                {
                // Camera look
                case irr::KEY_LEFT:
                    device->getGUIEnvironment()->setFocus(0); // Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                    model->lookStepLeft();
                    break;
                case irr::KEY_RIGHT:
                    device->getGUIEnvironment()->setFocus(0); // Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                    model->lookStepRight();
                    break;
                case irr::KEY_SPACE:
                    device->getGUIEnvironment()->setFocus(0); // Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                    model->changeView();
                    model->setMoveViewWithPrimary(false); // Don't allow the view to change automatically after this
                    break;
                default:
                    // don't do anything
                    break;
                }
            }
            else if (event.KeyInput.Control)
            {
                // Ctrl down

                switch (event.KeyInput.Key)
                {
                // Camera look
                case irr::KEY_UP:
                    device->getGUIEnvironment()->setFocus(0); // Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                    model->lookAhead();
                    break;
                case irr::KEY_DOWN:
                    device->getGUIEnvironment()->setFocus(0); // Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                    model->lookAstern();
                    break;
                case irr::KEY_LEFT:
                    device->getGUIEnvironment()->setFocus(0); // Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                    model->lookPort();                        // DEENOV22 TODO make all screens lookPort
                    break;
                case irr::KEY_RIGHT:
                    device->getGUIEnvironment()->setFocus(0); // Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                    model->lookStbd();
                    break;
                case irr::KEY_KEY_M:
                    model->retrieveManOverboard();
                    break;
                default:
                    // don't do anything
                    break;
                }
            }
            else
            {
                // Shift and Ctrl not down

                switch (event.KeyInput.Key)
                {
                // Accelerator
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

                case irr::KEY_KEY_W:
                    break;


                case irr::KEY_KEY_J:
                    break;

                case irr::KEY_KEY_L:
                    break;

                case irr::KEY_KEY_I:
                    break;

                case irr::KEY_KEY_K:
                    break;

                // Camera look
                case irr::KEY_UP:
                    device->getGUIEnvironment()->setFocus(0); // Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                    model->lookUp();
                    break;
                case irr::KEY_DOWN:
                    device->getGUIEnvironment()->setFocus(0); // Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                    model->lookDown();
                    break;
                case irr::KEY_LEFT:
                    device->getGUIEnvironment()->setFocus(0); // Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                    model->lookLeft();
                    break;
                case irr::KEY_RIGHT:
                    device->getGUIEnvironment()->setFocus(0); // Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                    model->lookRight();
                    break;
                case irr::KEY_SPACE:
                    device->getGUIEnvironment()->setFocus(0); // Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                    model->changeView();
                    model->setMoveViewWithPrimary(true); // Allow the view to change automatically after this
                    break;

                // toggle full screen 3d
                case irr::KEY_KEY_F:
                    gui->toggleShow2dInterface();
                    break;

                // Quit with esc or F4 (for alt-F4)
                case irr::KEY_ESCAPE:
                case irr::KEY_F4:
                    startShutdown();
                    return true; // Return true here, so second 'esc' button pushes don't close the message box
                    break;

                case irr::KEY_KEY_M:
                    model->releaseManOverboard();
                    break;

                // Keyboard control of engines
                case irr::KEY_KEY_A:
		  model->setPortEngine(model->getPortEngine() + 0.1); // setPortEngine clamps the setting to the allowable
		  break;

                case irr::KEY_KEY_Z:
                    break;

                case irr::KEY_KEY_S:
                        model->setStbdEngine(model->getStbdEngine() + 0.1); // setPortEngine clamps the setting to the allowable range
                    break;

                case irr::KEY_KEY_X:
		  
                    break;

                case irr::KEY_KEY_D:
		  model->setStbdEngine(model->getStbdEngine() + 0.1); // setPortEngine clamps the setting to the allowable range
		  model->setPortEngine(model->getPortEngine() + 0.1); // setPortEngine clamps the setting to the allowable range
                    break;

		case irr::KEY_KEY_C:
                  
                    break;

                case irr::KEY_KEY_V:
             
                    break;

                case irr::KEY_KEY_B:
                    break;
                

                default:
                    // don't do anything
                    break;
                }
            }
        }
    }

    if (event.EventType == irr::EET_KEY_INPUT_EVENT && !event.KeyInput.PressedDown)
    {
        if (event.KeyInput.Key == irr::KEY_KEY_H)
        {
            model->endHorn();
        }
    }

    // DEE 10JAN23 comment  joystickCode starts here

    // From joystick (actually polled, once per run():
    if (event.EventType == irr::EET_JOYSTICK_INPUT_EVENT)
    {

        irr::u8 thisJoystick = event.JoystickEvent.Joystick;

        // Initialise the joystick POV
        if (!previousJoystickPOVInitialised)
        {
            if (thisJoystick == joystickSetup.joystickNoPOV)
            {
                previousJoystickPOVInitialised = true;
                previousJoystickPOV = event.JoystickEvent.POV;
            }
        }

        // Show joystick raw status in log window
        if (device->getTimer()->getRealTime() - lastShownJoystickStatus > 5000)
        {

            std::string joystickInfoMessage = "Joystick status (";
            joystickInfoMessage.append(irr::core::stringc(event.JoystickEvent.Joystick).c_str());
            joystickInfoMessage.append(")\n");
            device->getLogger()->log(joystickInfoMessage.c_str());

            std::string thisJoystickStatus = "";
            for (irr::u8 thisAxis = 0; thisAxis < event.JoystickEvent.NUMBER_OF_AXES; thisAxis++)
            {
                irr::s16 axisSetting = event.JoystickEvent.Axis[thisAxis];
                thisJoystickStatus.append(irr::core::stringc(axisSetting).c_str());
                thisJoystickStatus.append(" ");
            }
            thisJoystickStatus.append("POV: ");
            thisJoystickStatus.append(irr::core::stringc(event.JoystickEvent.POV).c_str());
            device->getLogger()->log(thisJoystickStatus.c_str());
            device->getLogger()->log("");

            // If we've shown for all joysticks, don't show again.
            if (event.JoystickEvent.Joystick + 1 == joystickInfo.size())
            {
                lastShownJoystickStatus = device->getTimer()->getRealTime();
            }
        }

        // Keep joystick values the same unless they are being changed by user input
        irr::f32 newJoystickPort = previousJoystickPort;
        irr::f32 newJoystickStbd = previousJoystickStbd;
        irr::f32 newJoystickRudder = previousJoystickRudder;

        irr::f32 newJoystickBowThruster = previousJoystickBowThruster;
        irr::f32 newJoystickSternThruster = previousJoystickSternThruster;

        for (irr::u8 thisAxis = 0; thisAxis < event.JoystickEvent.NUMBER_OF_AXES; thisAxis++)
        {

            // Check which type we correspond to
            if (thisJoystick == joystickSetup.portJoystickNo && thisAxis == joystickSetup.portJoystickAxis)
            {
                newJoystickPort = event.JoystickEvent.Axis[joystickSetup.portJoystickAxis] / 32768.0;
                // If previous value is Inf, store current value in previous and current, otherwise only in current
                if (previousJoystickPort == INFINITY)
                {
                    previousJoystickPort = newJoystickPort;
                }
            }
            if (thisJoystick == joystickSetup.stbdJoystickNo && thisAxis == joystickSetup.stbdJoystickAxis)
            {
                newJoystickStbd = event.JoystickEvent.Axis[joystickSetup.stbdJoystickAxis] / 32768.0;
                // If previous value is Inf, store current value in previous and current, otherwise only in current
                if (previousJoystickStbd == INFINITY)
                {
                    previousJoystickStbd = newJoystickStbd;
                }
            }
            if (thisJoystick == joystickSetup.rudderJoystickNo && thisAxis == joystickSetup.rudderJoystickAxis)
            {
                newJoystickRudder = 30 * event.JoystickEvent.Axis[joystickSetup.rudderJoystickAxis] / 32768.0;
                // If previous value is Inf, store current value in previous and current, otherwise only in current
                if (previousJoystickRudder == INFINITY)
                {
                    previousJoystickRudder = newJoystickRudder;
                }
            }


            if (thisJoystick == joystickSetup.bowThrusterJoystickNo && thisAxis == joystickSetup.bowThrusterJoystickAxis)
            {
                newJoystickBowThruster = event.JoystickEvent.Axis[joystickSetup.bowThrusterJoystickAxis] / 32768.0;
                // If previous value is Inf, store current value in previous and current, otherwise only in current
                if (previousJoystickBowThruster == INFINITY)
                {
                    previousJoystickBowThruster = newJoystickBowThruster;
                }
            }
            if (thisJoystick == joystickSetup.sternThrusterJoystickNo && thisAxis == joystickSetup.sternThrusterJoystickAxis)
            {
                newJoystickSternThruster = event.JoystickEvent.Axis[joystickSetup.sternThrusterJoystickAxis] / 32768.0;
                // If previous value is Inf, store current value in previous and current, otherwise only in current
                if (previousJoystickSternThruster == INFINITY)
                {
                    previousJoystickSternThruster = newJoystickSternThruster;
                }
            }
        }

        // Do joystick stuff here

        // check if any have changed
        bool joystickChanged = false;
        bool portChanged = fabs(newJoystickPort - previousJoystickPort) > 0.01;
        bool stbdChanged = fabs(newJoystickStbd - previousJoystickStbd) > 0.01;
        bool wheelChanged = fabs(newJoystickRudder - previousJoystickRudder) > 0.01;
        // DEE 10JAN23 vvvv
        //	    irr::f32 azimuth1AngChange = fabs(newJoystickAzimuthAngPort - previousJoystickAzimuthAngPort);
        //          irr::f32 azimuth2AngChange = fabs(newJoystickAzimuthAngStbd - previousJoystickAzimuthAngStbd);

        // DEE
        //            irr::f32 rudderChange = fabs(newJoystickRudder - previousJoystickRudder);
        bool bowThrusterChanged = fabs(newJoystickBowThruster - previousJoystickBowThruster) > 0.01;
        bool sternThrusterChanged = fabs(newJoystickSternThruster - previousJoystickSternThruster) > 0.01;
        // DEE
        //            if (portChange > 0.01 || stbdChange > 0.01 || rudderChange > 0.01 || bowThrusterChange > 0.01 || sternThrusterChange > 0.01 )
        if (portChanged || stbdChanged || wheelChanged ||
            bowThrusterChanged || sternThrusterChanged)
        {
            joystickChanged = true;
        }

        // If any have changed, use all (iff non-infinite)
        if (joystickChanged)
        {

            if (newJoystickPort < INFINITY && (joystickSetup.updateAllAxes || portChanged))
            { // refers to the port engine control
                irr::f32 mappedValue = lookup1D(newJoystickPort, joystickSetup.inputPoints, joystickSetup.outputPoints);
                previousJoystickPort = newJoystickPort;
            }

            if (newJoystickStbd < INFINITY && (joystickSetup.updateAllAxes || stbdChanged))
            { // refers to the starboard engine control
                irr::f32 mappedValue = lookup1D(newJoystickStbd, joystickSetup.inputPoints, joystickSetup.outputPoints);
                previousJoystickStbd = newJoystickStbd;
            }

            if (newJoystickRudder < INFINITY && (joystickSetup.updateAllAxes || wheelChanged))
            {
                // DEE if the joystick rudder control is used then make it change the wheel not the rudder
                model->setWheel(newJoystickRudder * joystickSetup.rudderDirection);
                //                    model->setRudder(newJoystickRudder);
                previousJoystickRudder = newJoystickRudder;
            }

            if (newJoystickBowThruster < INFINITY && (joystickSetup.updateAllAxes || bowThrusterChanged))
            {
	      //model->setBowThruster(newJoystickBowThruster);
                previousJoystickBowThruster = newJoystickBowThruster;
            }

            if (newJoystickSternThruster < INFINITY && (joystickSetup.updateAllAxes || sternThrusterChanged))
            {
	      //model->setSternThruster(newJoystickSternThruster);
                previousJoystickSternThruster = newJoystickSternThruster;
            }

        } // DEE 10JAN23 end if JoysickChanged

        // Check joystick buttons here
        // Make sure the joystickPreviousButtonStates has an entry for this joystick
        while (joystickPreviousButtonStates.size() <= thisJoystick)
        {
            joystickPreviousButtonStates.push_back(0); // All zeros equivalent to no buttons pressed
        }

        irr::u32 thisButtonState = event.JoystickEvent.ButtonStates;
        irr::u32 previousButtonState = joystickPreviousButtonStates.at(thisJoystick);

        // Horn
        if (thisJoystick == joystickSetup.joystickNoHorn)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonHorn, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonHorn, previousButtonState))
            {
                model->startHorn();
            }
            if (!IsButtonPressed(joystickSetup.joystickButtonHorn, thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonHorn, previousButtonState))
            {
                model->endHorn();
            }
        }
        // Change view
        if (thisJoystick == joystickSetup.joystickNoChangeView)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonChangeView, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonChangeView, previousButtonState))
            {
                model->changeView();
                model->setMoveViewWithPrimary(true); // Allow the view to change automatically after this
            }
        }
        if (thisJoystick == joystickSetup.joystickNoChangeAndLockView)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonChangeAndLockView, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonChangeAndLockView, previousButtonState))
            {
                model->changeView();
                model->setMoveViewWithPrimary(false); // Don't allow the view to change automatically after this
            }
        }
        // Look step left
        if (thisJoystick == joystickSetup.joystickNoLookStepLeft)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonLookStepLeft, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonLookStepLeft, previousButtonState))
            {
                model->lookStepLeft();
            }
        }
        // Look step right
        if (thisJoystick == joystickSetup.joystickNoLookStepRight)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonLookStepRight, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonLookStepRight, previousButtonState))
            {
                model->lookStepRight();
            }
        }
        // Decrease bow thrust
        if (thisJoystick == joystickSetup.joystickNoDecreaseBowThrust)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonDecreaseBowThrust, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonDecreaseBowThrust, previousButtonState))
            {
	      //model->setBowThrusterRate(-0.5);
            }
            if (!IsButtonPressed(joystickSetup.joystickButtonDecreaseBowThrust, thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonDecreaseBowThrust, previousButtonState))
            {
	      //model->setBowThrusterRate(0);
            }
        }
        // Increase bow thrust
        if (thisJoystick == joystickSetup.joystickNoIncreaseBowThrust)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonIncreaseBowThrust, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonIncreaseBowThrust, previousButtonState))
            {
	      //model->setBowThrusterRate(0.5);
            }
            if (!IsButtonPressed(joystickSetup.joystickButtonIncreaseBowThrust, thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonIncreaseBowThrust, previousButtonState))
            {
	      //model->setBowThrusterRate(0);
            }
        }
        // Decrease stern thrust
        if (thisJoystick == joystickSetup.joystickNoDecreaseSternThrust)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonDecreaseSternThrust, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonDecreaseSternThrust, previousButtonState))
            {
	      //model->setSternThrusterRate(-0.5);
            }
            if (!IsButtonPressed(joystickSetup.joystickButtonDecreaseSternThrust, thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonDecreaseSternThrust, previousButtonState))
            {
	      //model->setSternThrusterRate(0);
            }
        }
        // Increase stern thrust
        if (thisJoystick == joystickSetup.joystickNoIncreaseSternThrust)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonIncreaseSternThrust, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonIncreaseSternThrust, previousButtonState))
            {
	      //model->setSternThrusterRate(0.5);
            }
            if (!IsButtonPressed(joystickSetup.joystickButtonIncreaseSternThrust, thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonIncreaseSternThrust, previousButtonState))
            {
	      //model->setSternThrusterRate(0);
            }
        }
        // Bearings on
        if (thisJoystick == joystickSetup.joystickNoBearingOn)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonBearingOn, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonBearingOn, previousButtonState))
            {
                gui->showBearings();
            }
        }
        // Bearings off
        if (thisJoystick == joystickSetup.joystickNoBearingOff)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonBearingOff, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonBearingOff, previousButtonState))
            {
                gui->hideBearings();
            }
        }
        // Zoom on
        if (thisJoystick == joystickSetup.joystickNoZoomOn)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonZoomOn, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonZoomOn, previousButtonState))
            {
                gui->zoomOn();
                model->setZoom(true);
            }
        }
        // Zoom off
        if (thisJoystick == joystickSetup.joystickNoZoomOff)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonZoomOff, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonZoomOff, previousButtonState))
            {
                gui->zoomOff();
                model->setZoom(false);
            }
        }
        // Look around
        // Left
        if (thisJoystick == joystickSetup.joystickNoLookLeft)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonLookLeft, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonLookLeft, previousButtonState))
            {
                model->setPanSpeed(-5);
            }
            if (!IsButtonPressed(joystickSetup.joystickButtonLookLeft, thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonLookLeft, previousButtonState))
            {
                model->setPanSpeed(0);
            }
        }

        // Right
        if (thisJoystick == joystickSetup.joystickNoLookRight)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonLookRight, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonLookRight, previousButtonState))
            {
                model->setPanSpeed(5);
            }
            if (!IsButtonPressed(joystickSetup.joystickButtonLookRight, thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonLookRight, previousButtonState))
            {
                model->setPanSpeed(0);
            }
        }

        // Up
        if (thisJoystick == joystickSetup.joystickNoLookUp)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonLookUp, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonLookUp, previousButtonState))
            {
                model->setVerticalPanSpeed(5);
            }
            if (!IsButtonPressed(joystickSetup.joystickButtonLookUp, thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonLookUp, previousButtonState))
            {
                model->setVerticalPanSpeed(0);
            }
        }

        // Down
        if (thisJoystick == joystickSetup.joystickNoLookDown)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonLookDown, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonLookDown, previousButtonState))
            {
                model->setVerticalPanSpeed(-5);
            }
            if (!IsButtonPressed(joystickSetup.joystickButtonLookDown, thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonLookDown, previousButtonState))
            {
                model->setVerticalPanSpeed(0);
            }
        }

        // Rudder pump 1 on
        if (thisJoystick == joystickSetup.joystickNoPump1On)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonPump1On, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonPump1On, previousButtonState))
            {
	      /*model->setRudderPumpState(1, true);
                if (model->getRudderPumpState(2))
                {
                    model->setAlarm(false); // Only turn off alarm if other pump is working
		    }*/
            }
        }

        // Rudder pump 1 off
        if (thisJoystick == joystickSetup.joystickNoPump1Off)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonPump1Off, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonPump1Off, previousButtonState))
            {
	      //model->setRudderPumpState(1, false);
	      //model->setAlarm(true);
            }
        }

        // Rudder pump 2 on
        if (thisJoystick == joystickSetup.joystickNoPump2On)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonPump2On, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonPump2On, previousButtonState))
            {
	      //model->setRudderPumpState(2, true);
	      //if (model->getRudderPumpState(1))
                {
		  //    model->setAlarm(false); // Only turn off alarm if other pump is working
                }
            }
        }

        // Rudder pump 2 off
        if (thisJoystick == joystickSetup.joystickNoPump2Off)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonPump2Off, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonPump2Off, previousButtonState))
            {
	      //model->setRudderPumpState(2, false);
	      //model->setAlarm(true);
            }
        }

        // Follow up rudder on
        if (thisJoystick == joystickSetup.joystickNoFollowUpOn)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonFollowUpOn, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonFollowUpOn, previousButtonState))
            {
	      //model->setFollowUpRudderWorking(true);
            }
        }

        // Follow up rudder off
        if (thisJoystick == joystickSetup.joystickNoFollowUpOff)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonFollowUpOff, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonFollowUpOff, previousButtonState))
            {
	      //model->setFollowUpRudderWorking(false);
            }
        }

        if (thisJoystick == joystickSetup.joystickNoNFUPort)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonNFUPort, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonNFUPort, previousButtonState))
            {
                model->setWheel(-30, true);
            }
            if (!IsButtonPressed(joystickSetup.joystickButtonNFUPort, thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonNFUPort, previousButtonState))
            {
                model->setWheel(model->getRudder(), true);
            }
        }

        if (thisJoystick == joystickSetup.joystickNoNFUStbd)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonNFUStbd, thisButtonState) && !IsButtonPressed(joystickSetup.joystickButtonNFUStbd, previousButtonState))
            {
                model->setWheel(30, true);
            }
            if (!IsButtonPressed(joystickSetup.joystickButtonNFUStbd, thisButtonState) && IsButtonPressed(joystickSetup.joystickButtonNFUStbd, previousButtonState))
            {
                model->setWheel(model->getRudder(), true);
            }
        }

        if (thisJoystick == joystickSetup.joystickNoAckAlarm)
        {
            if (IsButtonPressed(joystickSetup.joystickButtonAckAlarm, thisButtonState))
            {
	      //model->setAlarm(false);
            }
        }


        // Store previous settings
        joystickPreviousButtonStates.at(thisJoystick) = event.JoystickEvent.ButtonStates;

        // POV hat
        if (previousJoystickPOVInitialised && thisJoystick == joystickSetup.joystickNoPOV)
        {
            if (event.JoystickEvent.POV == joystickSetup.joystickPOVLookLeft && previousJoystickPOV != joystickSetup.joystickPOVLookLeft)
            {
                model->setPanSpeed(-5);
            }
            if (event.JoystickEvent.POV != joystickSetup.joystickPOVLookLeft && previousJoystickPOV == joystickSetup.joystickPOVLookLeft)
            {
                model->setPanSpeed(0);
            }

            if (event.JoystickEvent.POV == joystickSetup.joystickPOVLookRight && previousJoystickPOV != joystickSetup.joystickPOVLookRight)
            {
                model->setPanSpeed(5);
            }
            if (event.JoystickEvent.POV != joystickSetup.joystickPOVLookRight && previousJoystickPOV == joystickSetup.joystickPOVLookRight)
            {
                model->setPanSpeed(0);
            }

            if (event.JoystickEvent.POV == joystickSetup.joystickPOVLookUp && previousJoystickPOV != joystickSetup.joystickPOVLookUp)
            {
                model->setVerticalPanSpeed(5);
            }
            if (event.JoystickEvent.POV != joystickSetup.joystickPOVLookUp && previousJoystickPOV == joystickSetup.joystickPOVLookUp)
            {
                model->setVerticalPanSpeed(0);
            }

            if (event.JoystickEvent.POV == joystickSetup.joystickPOVLookDown && previousJoystickPOV != joystickSetup.joystickPOVLookDown)
            {
                model->setVerticalPanSpeed(-5);
            }
            if (event.JoystickEvent.POV != joystickSetup.joystickPOVLookDown && previousJoystickPOV == joystickSetup.joystickPOVLookDown)
            {
                model->setVerticalPanSpeed(0);
            }
            previousJoystickPOV = event.JoystickEvent.POV; // Store for next time
        }
    }

    return false;
}

irr::f32 MyEventReceiver::lookup1D(irr::f32 lookupValue, std::vector<irr::f32> inputPoints, std::vector<irr::f32> outputPoints)
{
    // Check that the input and output points list are the same length
    if (inputPoints.size() != outputPoints.size() || inputPoints.size() < 2)
    {
        std::cerr << "Error: lookup1D needs inputPoints and outputPoints list size to be the same, and needs at least two points." << std::endl;
        return 0;
    }

    std::vector<irr::f32>::size_type numberOfPoints = inputPoints.size();

    // Check that inputPoints does not have decreasing values (must be increasing or equal)
    for (unsigned int i = 0; i + 1 < numberOfPoints; i++)
    {
        if (inputPoints.at(i + 1) < inputPoints.at(i))
        {
            std::cerr << "Error: inputPoints to lookup1D must not be in a decreasing order." << std::endl;
            return 0;
        }
    }

    // Return first output if at or below lowest input
    if (lookupValue <= inputPoints.at(0))
    {
        return outputPoints.at(0);
    }

    // Return last output if at or above highest input
    if (lookupValue >= inputPoints.at(numberOfPoints - 1))
    {
        return outputPoints.at(numberOfPoints - 1);
    }

    // Main interpolation
    // Find the first point above the one we're interested in
    unsigned int nextPoint = 1;
    while (nextPoint < numberOfPoints && inputPoints.at(nextPoint) <= lookupValue)
    {
        nextPoint++;
    }

    // check for div by zero - shouldn't happen, but protect against
    if (inputPoints.at(nextPoint) - inputPoints.at(nextPoint - 1) == 0)
        return 0.0;

    // do interpolation
    return outputPoints.at(nextPoint - 1) + (outputPoints.at(nextPoint) - outputPoints.at(nextPoint - 1)) * (lookupValue - inputPoints.at(nextPoint - 1)) / (inputPoints.at(nextPoint) - inputPoints.at(nextPoint - 1));
}

std::wstring MyEventReceiver::f32To3dp(irr::f32 value, bool stripZeros)
{
    // Convert a floating point value to a wstring, with 3dp
    char tempStr[100];
    snprintf(tempStr, 100, "%.3f", value);
    std::wstring outputWstring = std::wstring(tempStr, tempStr + strlen(tempStr));
    // Strip trailing zeros and decimal point
    if (stripZeros)
    {
        while (outputWstring.back() == '0')
        {
            outputWstring.pop_back();
        }
        if (outputWstring.back() == '.')
        {
            outputWstring.pop_back();
        }
    }
    return outputWstring;
}

bool MyEventReceiver::IsButtonPressed(irr::u32 button, irr::u32 buttonBitmap) const
{
    if (button >= 32)
        return false;

    return (buttonBitmap & (1 << button)) ? true : false;
}

void MyEventReceiver::startShutdown()
{
    model->setAccelerator(0.0);
    device->sleep(500);
    if (!shutdownDialogActive)
    {
        device->getGUIEnvironment()->getRootGUIElement()->setVisible(true);
        device->getGUIEnvironment()->addMessageBox(L"Quit?", L"Quit?", true, irr::gui::EMBF_OK | irr::gui::EMBF_CANCEL, 0, GUIMain::GUI_ID_CLOSE_BOX); // I18n
        shutdownDialogActive = true;
    }
}

void MyEventReceiver::handleMooringLines(irr::core::line3df rayForLines)
{
    if ((linesMode == 1) || (linesMode == 2)) {
        irr::scene::ISceneNode* contactNode = model->getContactFromRay(rayForLines, linesMode);

        if (contactNode)
        {
            // If returns non-null, then successful, so move onto next point or finish

            // Find the type of node (0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object, 5: Terrain)
            int nodeType = 0;
            int nodeID = 0;

            // Find node type and ID from name
            std::string nodeName = std::string(contactNode->getName());

            if (nodeName.find("OwnShip") == 0)
            {
                nodeType = 1;
                nodeID = 0;
            }
            else if (nodeName.find("OtherShip") == 0)
            {
                nodeType = 2;
                // Find other ship ID from name (should be OtherShip_#)
                std::vector<std::string> splitName = Utilities::split(nodeName, '_');
                if (splitName.size() == 2)
                {
                    nodeID = Utilities::lexical_cast<irr::s32>(splitName.at(1));
                }
            }
            else if (nodeName.find("Buoy") == 0)
            {
                nodeType = 3;
                // Find other buoy ID from name (should be Buoy_#)
                std::vector<std::string> splitName = Utilities::split(nodeName, '_');
                if (splitName.size() == 2)
                {
                    nodeID = Utilities::lexical_cast<irr::s32>(splitName.at(1));
                }
            }
            else if (nodeName.find("LandObject") == 0)
            {
                nodeType = 4;
                // Find other land object ID from name (should be LandObject_#)
                std::vector<std::string> splitName = Utilities::split(nodeName, '_');
                if (splitName.size() == 2)
                {
                    nodeID = Utilities::lexical_cast<irr::s32>(splitName.at(1));
                }
            }
            else if (nodeName.find("Terrain") == 0)
            {
                nodeType = 5;
                // Find terrain ID from name (should be Terrain_#)
                std::vector<std::string> splitName = Utilities::split(nodeName, '_');
                if (splitName.size() == 2)
                {
                    nodeID = Utilities::lexical_cast<irr::s32>(splitName.at(1));
                }
            }
            // std::cout << "Node name: " << nodeName << " nodeType: " << nodeType << " nodeID: " << nodeID << std::endl;

            if (linesMode == 2)
            {
                irr::f32 nominalMass = model->getOwnShipMassEstimate();
                if (nodeType == 2) {
                    // If connecting to another ship, find the minimum mass to use as the nominal mass for estimating default line properties
                    nominalMass = fmin(model->getOtherShipMassEstimate(nodeID), nominalMass);
                }
                model->getLines()->setLineEnd(contactNode, nominalMass, nodeType, nodeID, 1.0, false, -1);
                // Finished
                linesMode = 0;
                gui->setLinesControlsText("");
            }
            if (linesMode == 1)
            {
                model->getLines()->setLineStart(contactNode, nodeType, nodeID, false, -1); // Start should always be on 'own ship' so nodeType = 1, and ID does not matter (leave as 0)
                // Move on to end point
                linesMode = 2;
                gui->setLinesControlsText("Click in 3d view to set end position for line"); // TODO: Add translation

                // special case for 'anchoring', set end node at sea bed under the starting node
                if (gui->getAnchorLine()) {
                    
                    irr::f32 nominalMass = model->getOwnShipMassEstimate();

                    // Create a 'contact node' at the terrain height below the anchor point
                    irr::core::vector3df intersection = contactNode->getAbsolutePosition();
                    intersection.Y = model->getTerrainHeight(intersection.X, intersection.Z);
                    irr::scene::ISceneNode* terrainSceneNode = model->getTerrainSceneNode(0);

                    // Add a 'sphere' scene node, with selectedSceneNode as parent.
                    // Find local coordinates from the global one
                    irr::core::vector3df localPosition(intersection);
                    irr::core::matrix4 worldToLocal = terrainSceneNode->getAbsoluteTransformation();
                    worldToLocal.makeInverse();
                    worldToLocal.transformVect(localPosition);

                    irr::core::vector3df sphereScale = irr::core::vector3df(1.0, 1.0, 1.0);
                    irr::scene::ISceneNode* contactPointNode = device->getSceneManager()->addSphereSceneNode(0.25f, 16, terrainSceneNode, -1,
                        localPosition,
                        irr::core::vector3df(0, 0, 0),
                        sphereScale);

                    // Set name to match parent for convenience
                    contactPointNode->setName(terrainSceneNode->getName());

                    // Node ID is 0 as we always assume parent is terrain 0
                    model->getLines()->setLineEnd(contactPointNode, nominalMass, 5, 0, 1.5, false, -1);
                    
                    // Tidy up
                    linesMode = 0;
                    gui->setLinesControlsText("");
                }

            }
        }
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
