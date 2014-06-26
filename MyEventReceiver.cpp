#include "irrlicht.h"

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
            }
        }

        //key events
        if (event.EventType == EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown)
		{
            //Accelerator from keyboard
            switch(event.KeyInput.Key)
            {
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
            }
		}

        return false;

    }


	s32 MyEventReceiver::GetScrollBarPosSpeed() const
	{
		return scrollBarPosSpeed;
	}

	s32 MyEventReceiver::GetScrollBarPosHeading() const
	{
		return scrollBarPosHeading;
	}
