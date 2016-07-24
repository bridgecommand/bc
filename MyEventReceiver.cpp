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

#include <iostream>

#include "GUIMain.hpp"
#include "SimulationModel.hpp"

using namespace irr;

    MyEventReceiver::MyEventReceiver(irr::IrrlichtDevice* dev, SimulationModel* model, GUIMain* gui, irr::u32 portJoystickAxis, irr::u32 stbdJoystickAxis, irr::u32 rudderJoystickAxis, irr::u32 portJoystickNo, irr::u32 stbdJoystickNo, irr::u32 rudderJoystickNo, std::vector<std::string>* logMessages) //Constructor
	{
		this->model = model; //Link to the model
		this->gui = gui; //Link to GUI (Not currently used, all comms through model)
		scrollBarPosSpeed = 0;
		scrollBarPosHeading = 0;

		//store device
		device = dev;

		//set up joystick if present, and inform user what's available
		dev->activateJoysticks(joystickInfo);

		//Tell user about joysticks via the log
		dev->getLogger()->log(""); //add a blank line
		std::string joystickInfoMessage = "Number of joysticks detected: ";
		joystickInfoMessage.append(std::string(core::stringc(joystickInfo.size()).c_str()));
		dev->getLogger()->log(joystickInfoMessage.c_str());
        for(uint i = 0; i<joystickInfo.size(); i++) {
            //Print out name and number of each joystick
            joystickInfoMessage = "Joystick number: ";
            joystickInfoMessage.append(core::stringc(i).c_str());
            joystickInfoMessage.append(", Name: ");
            joystickInfoMessage.append(std::string(joystickInfo[i].Name.c_str()));
            dev->getLogger()->log(joystickInfoMessage.c_str());
        }
        dev->getLogger()->log(""); //add a blank line

		this->portJoystickAxis=portJoystickAxis;
		this->stbdJoystickAxis=stbdJoystickAxis;
		this->rudderJoystickAxis=rudderJoystickAxis;
		this->portJoystickNo=portJoystickNo;
		this->stbdJoystickNo=stbdJoystickNo;
		this->rudderJoystickNo=rudderJoystickNo;

		//Indicate that previous joystick information hasn't been initialised
		previousJoystickPort = INFINITY;
		previousJoystickStbd = INFINITY;
		previousJoystickRudder = INFINITY;

		this->logMessages = logMessages;

        //assume mouse buttons not pressed initially
        leftMouseDown = false;
        rightMouseDown = false;
	}

    bool MyEventReceiver::OnEvent(const SEvent& event)
	{

        //From log
        if (event.EventType == EET_LOG_TEXT_EVENT) {
            //Store these in a global log.
            std::string eventText(event.LogEvent.Text);
            logMessages->push_back(eventText);
            return true;
        }

        //From mouse - keep track of button press state
        if (event.EventType == EET_MOUSE_INPUT_EVENT) {
            if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN ) {leftMouseDown=true;}
            if (event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP ) {leftMouseDown=false;}
            if (event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN ) {rightMouseDown=true;}
            if (event.MouseInput.Event == EMIE_RMOUSE_LEFT_UP ) {rightMouseDown=false;}
        }

        if (event.EventType == EET_GUI_EVENT)
		{
			s32 id = event.GUIEvent.Caller->getID();
            if (event.GUIEvent.EventType==gui::EGET_SCROLL_BAR_CHANGED)
            {

               if (id == GUIMain::GUI_ID_HEADING_SCROLL_BAR)
                  {
                      scrollBarPosHeading = ((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                      model->setHeading(scrollBarPosHeading);
                  }

              if (id == GUIMain::GUI_ID_SPEED_SCROLL_BAR)
                  {
                        scrollBarPosSpeed = ((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                        model->setSpeed(scrollBarPosSpeed);
                  }

              if (id == GUIMain::GUI_ID_STBD_SCROLL_BAR)
                  {
                        model->setStbdEngine(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/-100.0); //Convert to from +-100 to +-1, and invert up/down
                        //If right mouse button, set the other engine as well
                        if (rightMouseDown) {
                            model->setPortEngine(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/-100.0);
                        }
                  }
              if (id == GUIMain::GUI_ID_PORT_SCROLL_BAR)
                  {
                        model->setPortEngine(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/-100.0); //Convert to from +-100 to +-1, and invert up/down
                        //If right mouse button, set the other engine as well
                        if (rightMouseDown) {
                            model->setStbdEngine(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/-100.0);
                        }
                  }
              if (id == GUIMain::GUI_ID_RUDDER_SCROLL_BAR)
                  {
                        model->setRudder(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos());
                  }
              if (id == GUIMain::GUI_ID_RADAR_GAIN_SCROLL_BAR)
                  {
                        model->setRadarGain(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos());
                  }
              if (id == GUIMain::GUI_ID_RADAR_CLUTTER_SCROLL_BAR)
                  {
                        model->setRadarClutter(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos());
                  }
              if (id == GUIMain::GUI_ID_RADAR_RAIN_SCROLL_BAR)
                  {
                        model->setRadarRain(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos());
                  }
              if (id == GUIMain::GUI_ID_WEATHER_SCROLL_BAR)
                  {
                        model->setWeather(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/10.0); //Scroll bar 0-120, weather 0-12
                  }
              if (id == GUIMain::GUI_ID_RAIN_SCROLL_BAR)
                  {
                        model->setRain(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/10.0); //Scroll bar 0-100, rain 0-10
                  }
              if (id == GUIMain::GUI_ID_VISIBILITY_SCROLL_BAR)
                  {
                        model->setVisibility(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/10.0); //Scroll bar 1-101, vis 0.1-10.1
                  }
            }

            if (event.GUIEvent.EventType==gui::EGET_BUTTON_CLICKED) {
                if (id == GUIMain::GUI_ID_START_BUTTON)
                {
                    model->setAccelerator(1.0);
                }

                if (id == GUIMain::GUI_ID_RADAR_INCREASE_BUTTON)
                {
                    model->increaseRadarRange();
                }

                if (id == GUIMain::GUI_ID_RADAR_DECREASE_BUTTON)
                {
                    model->decreaseRadarRange();
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
                    model->setZoom(((gui::IGUIButton*)event.GUIEvent.Caller)->isPressed());
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

            } //Button clicked

        } //GUI Event


        //From keyboard
        if (event.EventType == EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown)
		{

            if (event.KeyInput.Shift) {
                //Shift down

            } else if (event.KeyInput.Control) {
                //Ctrl down

                switch(event.KeyInput.Key)
                {
                    //Camera look
                    case KEY_UP:
                        model->lookAhead();
                        break;
                    case KEY_DOWN:
                        model->lookAstern();
                        break;
                    case KEY_LEFT:
                        model->lookPort();
                        break;
                    case KEY_RIGHT:
                        model->lookStbd();
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
                    case KEY_KEY_0:
                        model->setAccelerator(0.0);
                        break;
                    case KEY_RETURN:
                        model->setAccelerator(1.0);
                        break;
                    case KEY_KEY_1:
                        model->setAccelerator(1.0);
                        break;
                    case KEY_KEY_2:
                        model->setAccelerator(2.0);
                        break;
                    case KEY_KEY_3:
                        model->setAccelerator(5.0);
                        break;
                    case KEY_KEY_4:
                        model->setAccelerator(15.0);
                        break;
                    case KEY_KEY_5:
                        model->setAccelerator(30.0);
                        break;
                    case KEY_KEY_6:
                        model->setAccelerator(60.0);
                        break;
                    case KEY_KEY_7:
                        model->setAccelerator(3600.0);
                        break;

                    //Camera look
                    case KEY_UP:
                        model->lookUp();
                        break;
                    case KEY_DOWN:
                        model->lookDown();
                        break;
                    case KEY_LEFT:
                        model->lookLeft();
                        break;
                    case KEY_RIGHT:
                        model->lookRight();
                        break;
                    case KEY_SPACE:
                        model->changeView();
                        break;

                    //toggle full screen 3d
                    case KEY_KEY_F:
                        gui->toggleShow2dInterface();
                        break;

                    default:
                        //don't do anything
                        break;
                }
            }
		}

		//From joystick (actually polled, once per run():
        if (event.EventType == EET_JOYSTICK_INPUT_EVENT) {


            irr::f32 newJoystickPort = previousJoystickPort;
            irr::f32 newJoystickStbd = previousJoystickStbd;
            irr::f32 newJoystickRudder = previousJoystickRudder;

            u8 thisJoystick = event.JoystickEvent.Joystick;
            s16 thisAxis = *event.JoystickEvent.Axis;

            //Check which type we correspond to
            if (thisJoystick == portJoystickNo && thisAxis == portJoystickAxis) {
                newJoystickPort = event.JoystickEvent.Axis[portJoystickAxis]/32768.0;
                //If previous value is NAN, store current value in previous and current, otherwise only in current
                if (previousJoystickPort==INFINITY) {
                    previousJoystickPort = newJoystickPort;
                }
            }
            if (thisJoystick == stbdJoystickNo && thisAxis == stbdJoystickAxis) {
                newJoystickStbd = event.JoystickEvent.Axis[stbdJoystickAxis]/32768.0;
                //If previous value is NAN, store current value in previous and current, otherwise only in current
                if (previousJoystickStbd==INFINITY) {
                    previousJoystickStbd = newJoystickStbd;
                }
            }
            if (thisJoystick == rudderJoystickNo && thisAxis == rudderJoystickAxis) {
                newJoystickRudder = event.JoystickEvent.Axis[rudderJoystickAxis]/32768.0;
                //If previous value is NAN, store current value in previous and current, otherwise only in current
                if (previousJoystickRudder==INFINITY) {
                    previousJoystickRudder = newJoystickRudder;
                }
            }


            //Do joystick stuff here
            //Todo: track joystick changes, so if not changing, the GUI inputs are used - partially implemented but need to check for jitter etc
            //Todo: Also implement multiplier/offset and joystick map.
            //FIXME: Note that Irrlicht does not have joystick handling on MacOS

            //check if any have changed
            bool joystickChanged = false;
            f32 portChange = fabs(newJoystickPort - previousJoystickPort);
            f32 stbdChange = fabs(newJoystickStbd - previousJoystickStbd);
            f32 rudderChange = fabs(newJoystickRudder - previousJoystickRudder);
            if (portChange > 0.01 || stbdChange > 0.01 || rudderChange > 0.01)
            {
                joystickChanged = true;
            }

            //If any have changed, use all (iff non-infinite)
            if (joystickChanged) {
                if (newJoystickPort<INFINITY) {
                    model->setPortEngine(newJoystickPort);
                    previousJoystickPort=newJoystickPort;
                }

                if (newJoystickStbd<INFINITY) {
                    model->setStbdEngine(newJoystickStbd);
                    previousJoystickStbd=newJoystickStbd;
                }

                if (newJoystickRudder<INFINITY) {
                    model->setRudder(newJoystickRudder);
                    previousJoystickRudder=newJoystickRudder;
                }

            }


        }

        return false;

    }


/*
	s32 MyEventReceiver::GetScrollBarPosSpeed() const
	{
		return scrollBarPosSpeed;
	}

	s32 MyEventReceiver::GetScrollBarPosHeading() const
	{
		return scrollBarPosHeading;
	}
*/
