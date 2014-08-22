#include "StartupEventReceiver.hpp"

#include <iostream>

using namespace irr;

    StartupEventReceiver::StartupEventReceiver(irr::gui::IGUIListBox* scenarioListBox, s32 listBoxID, s32 okButtonID) //Constructor
	{
		this->scenarioListBox = scenarioListBox;
		this->listBoxID = listBoxID;
		this->okButtonID = okButtonID;
		scenarioSelected = -1; //Set as initially invalid
	}

    bool StartupEventReceiver::OnEvent(const SEvent& event)
	{
        if (event.EventType == EET_GUI_EVENT)
		{
			s32 id = event.GUIEvent.Caller->getID();
			//If OK button, or double click on list
            if ( (event.GUIEvent.EventType==gui::EGET_BUTTON_CLICKED && id == okButtonID ) || event.GUIEvent.EventType==gui::EGET_LISTBOX_SELECTED_AGAIN  )
            {
                if (scenarioListBox->getSelected() > -1 ) {
                    scenarioSelected = scenarioListBox->getSelected();
                }
            }
		}
        return false;
    }

    irr::s32 StartupEventReceiver::getScenarioSelected() const
    {
        return scenarioSelected;
    }
