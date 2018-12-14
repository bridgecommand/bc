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

#include "StartupEventReceiver.hpp"

#include <iostream>


StartupEventReceiver::StartupEventReceiver(irr::gui::IGUIListBox* scenarioListBox, irr::gui::IGUIStaticText* scenarioText, irr::gui::IGUIStaticText* hostnameText, irr::gui::IGUIEditBox* hostnameBox, irr::gui::IGUICheckBox* secondaryBox, irr::gui::IGUICheckBox* multiplayerBox, irr::s32 listBoxID, irr::s32 okButtonID, irr::s32 secondaryBoxID, irr::s32 multiplayerBoxID, irr::IrrlichtDevice* dev)
    : device(dev)
    , scenarioListBox(scenarioListBox)
    , hostnameText(hostnameText)
    , scenarioText(scenarioText)
    , hostnameBox(hostnameBox)
    , secondaryBox(secondaryBox)
    , multiplayerBox(multiplayerBox)
    , listBoxID(listBoxID)
    , okButtonID(okButtonID)
    , secondaryBoxID(secondaryBoxID)
    , multiplayerBoxID(multiplayerBoxID)
    , scenarioSelected(-1)  // Set as initially invalid
{
}

bool StartupEventReceiver::OnEvent(const irr::SEvent& event)
{
    if (event.EventType == irr::EET_GUI_EVENT) {
        irr::s32 id = event.GUIEvent.Caller->getID();
        //If OK button, or double click on list
        if ( (event.GUIEvent.EventType==irr::gui::EGET_BUTTON_CLICKED && id == okButtonID) || event.GUIEvent.EventType==irr::gui::EGET_LISTBOX_SELECTED_AGAIN) {
            if (scenarioListBox->getSelected() > -1) {
                scenarioSelected = scenarioListBox->getSelected();
            }
        }

        if (event.GUIEvent.EventType==irr::gui::EGET_CHECKBOX_CHANGED) {
            if (id == secondaryBoxID || id == multiplayerBoxID) {
                //Check state, and set hostname box and text visible
                if ( ((irr::gui::IGUICheckBox*)event.GUIEvent.Caller)->isChecked()) {
                    scenarioListBox->setVisible(false);
                    scenarioText->setVisible(false);
                    hostnameBox->setVisible(false);
                    hostnameText->setVisible(false);
                } else {
                    scenarioListBox->setVisible(true);
                    scenarioText->setVisible(true);
                    hostnameBox->setVisible(true);
                    hostnameText->setVisible(true);
                }
            }

            //Only one check box should be on
            if (id == secondaryBoxID) {
                multiplayerBox->setChecked(false);
            }
            if (id == multiplayerBoxID) {
                secondaryBox->setChecked(false);
            }
        }
    }

    if (event.EventType == irr::EET_KEY_INPUT_EVENT)
    {
        if (event.KeyInput.Key==irr::KEY_RETURN) {
            if (scenarioListBox->getSelected() > -1 ) {
                scenarioSelected = scenarioListBox->getSelected();
            }
        }

        if (event.KeyInput.Key == irr::KEY_ESCAPE || event.KeyInput.Key == irr::KEY_F4) {
            device->closeDevice(); //Shutdown.
        }
    }
    return false;
}

irr::s32 StartupEventReceiver::getScenarioSelected() const
{
    return scenarioSelected;
}
