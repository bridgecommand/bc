#include "irrlicht.h"

#include "MyEventReceiver.hpp"

using namespace irr;

// Define some values that we'll use to identify individual GUI controls.
enum //FIXME: This is now defined twice
{
	GUI_ID_HEADING_SCROLL_BAR = 101,
	GUI_ID_SPEED_SCROLL_BAR
};

    MyEventReceiver::MyEventReceiver(SimulationModel* mdl, GUIMain* gui) //Constructor
	{
		model = mdl; //Link to the model
		guiMain = gui; //Link to GUI (Not currently used!)
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

               if (id == GUI_ID_HEADING_SCROLL_BAR)
                  {
                      scrollBarPosHeading = ((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                      model->setHeading(scrollBarPosHeading);
                  }

              if (id == GUI_ID_SPEED_SCROLL_BAR)
                  {
                        scrollBarPosSpeed = ((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                        model->setSpeed(scrollBarPosSpeed);
                  }
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
