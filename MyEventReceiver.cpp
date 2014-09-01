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

    MyEventReceiver::MyEventReceiver(SimulationModel* model, GUIMain* gui) //Constructor
	{
		this->model = model; //Link to the model
		this->gui = gui; //Link to GUI (Not currently used!)
		scrollBarPosSpeed = 0;
		scrollBarPosHeading = 0;
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
                    case KEY_LEFT:
                        model->lookLeft();
                        break;
                    case KEY_RIGHT:
                        model->lookRight();
                        break;
                    case KEY_SPACE:
                        model->changeView();
                        break;
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
