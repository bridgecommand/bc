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

    MyEventReceiver::MyEventReceiver(irr::IrrlichtDevice* dev, SimulationModel* model, GUIMain* gui, irr::u32 portJoystickAxis, irr::u32 stbdJoystickAxis, irr::u32 rudderJoystickAxis) //Constructor
	{
		this->model = model; //Link to the model
		this->gui = gui; //Link to GUI (Not currently used!)
		scrollBarPosSpeed = 0;
		scrollBarPosHeading = 0;

		//set up joystick if present
		dev->activateJoysticks(joystickInfo);
		previousJoystickLoaded = false;
		this->portJoystickAxis=portJoystickAxis;
		this->stbdJoystickAxis=stbdJoystickAxis;
		this->rudderJoystickAxis=rudderJoystickAxis;
	}

    bool MyEventReceiver::OnEvent(const SEvent& event)
	{

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
                  }
              if (id == GUIMain::GUI_ID_PORT_SCROLL_BAR)
                  {
                        model->setPortEngine(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/-100.0); //Convert to from +-100 to +-1, and invert up/down
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
                    model->toggleZoom();
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

            }


        }

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
                    default:
                        //don't do anything
                        break;

                    //Camera look
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
                }
            }
		}

		//From joystick (actually polled, once per run():
        if (event.EventType == EET_JOYSTICK_INPUT_EVENT) {
            u32 numberOfAxes = event.JoystickEvent.NUMBER_OF_AXES;
            //Do joystick stuff here
            //Todo: track joystick changes, so if not changing, the GUI inputs are used - partially implemented but need to check for jitter etc
            //Todo: Also implement multiplier/offset and joystick map.
            //FIXME: Note that Irrlicht does not have joystick handling on MacOS
            if (numberOfAxes>portJoystickAxis && numberOfAxes>stbdJoystickAxis && numberOfAxes>rudderJoystickAxis ) { //check required axes exist on this joystick
                if (!previousJoystickLoaded) { //Load initial joystick values, so we can detect a change
                    previousJoystickPort = event.JoystickEvent.Axis[portJoystickAxis]/32768.0;
                    previousJoystickStbd = event.JoystickEvent.Axis[stbdJoystickAxis]/32768.0;
                    previousJoystickRudder = 30.0*event.JoystickEvent.Axis[rudderJoystickAxis]/32768.0;
                    previousJoystickLoaded=true;
                } else { //Normal running
                    //Get current joystick inputs
                    f32 newJoystickPort = event.JoystickEvent.Axis[portJoystickAxis]/32768.0;//+-1 from axis range -32768 to 32767
                    f32 newJoystickStbd = event.JoystickEvent.Axis[stbdJoystickAxis]/32768.0;//+-1
                    f32 newJoystickRudder = 30.0*event.JoystickEvent.Axis[rudderJoystickAxis]/32768.0;//+-30 from axis range -32768 to 32767

                    //check if any have changed
                    bool joystickChanged = false;
                    f32 portChange = fabs(newJoystickPort - previousJoystickPort);
                    f32 stbdChange = fabs(newJoystickStbd - previousJoystickStbd);
                    f32 rudderChange = fabs(newJoystickRudder - previousJoystickRudder);
                    if (portChange > 0.01 || stbdChange > 0.01 || rudderChange > 0.01)
                    {
                        joystickChanged = true;
                    }

                    //If any have changed, use all
                    if (joystickChanged) {
                        model->setPortEngine(newJoystickPort);
                        model->setStbdEngine(newJoystickStbd);
                        model->setRudder(newJoystickRudder);
                        previousJoystickPort=newJoystickPort;
                        previousJoystickStbd=newJoystickStbd;
                        previousJoystickRudder=newJoystickRudder;
                    }
                }
            } else {
                std::cout << "Trying to use non-existent joystick axis." << std::endl;
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
