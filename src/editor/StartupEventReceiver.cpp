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
#include <string>
#include "../ScenarioDataStructure.hpp"
#include "../Utilities.hpp"
#include "ImportExportGUI.hpp"

//using namespace irr;

    StartupEventReceiver::StartupEventReceiver(
        irr::gui::IGUIListBox* scenarioListBox, 
        irr::gui::IGUIListBox* worldListBox, 
        irr::gui::IGUIWindow* selectWindow,
        irr::s32 scenarioListBoxID, 
        irr::s32 worldListBoxID, 
        irr::s32 okScenarioButtonID, 
        irr::s32 okWorldButtonID, 
        irr::s32 importScenarioButtonID, 
        irr::s32 exportScenarioButtonID, 
        irr::s32 importExportOKButtonID,
        GUIImportExport* guiImportExport,
        ScenarioData* scenarioData) //Constructor
	{
		this->scenarioListBox = scenarioListBox;
		this->worldListBox = worldListBox;
        this->selectWindow = selectWindow;
		this->scenarioListBoxID = scenarioListBoxID;
		this->worldListBoxID = worldListBoxID;
		this->okScenarioButtonID = okScenarioButtonID;
		this->okWorldButtonID = okWorldButtonID;
        this->importScenarioButtonID = importScenarioButtonID;
        this->exportScenarioButtonID = exportScenarioButtonID;
        this->importExportOKButtonID = importExportOKButtonID;
        this->guiImportExport = guiImportExport;
        this->scenarioData = scenarioData;
		scenarioSelected = -1; //Set as initially invalid
		worldSelected = -1; //Set as initially invalid
	}

    bool StartupEventReceiver::OnEvent(const irr::SEvent& event)
	{
        if (event.EventType == irr::EET_GUI_EVENT)
		{
			irr::s32 id = event.GUIEvent.Caller->getID();
			//If OK button, or double click on list, for scenario
            if ( (event.GUIEvent.EventType==irr::gui::EGET_BUTTON_CLICKED && id == okScenarioButtonID ) || (event.GUIEvent.EventType==irr::gui::EGET_LISTBOX_SELECTED_AGAIN  && id == scenarioListBoxID ) )
            {
                if (scenarioListBox->getSelected() > -1 ) {
                    scenarioSelected = scenarioListBox->getSelected();
                }
            }

            //If OK button, or double click on list, for world
            if ( (event.GUIEvent.EventType==irr::gui::EGET_BUTTON_CLICKED && id == okWorldButtonID ) || (event.GUIEvent.EventType==irr::gui::EGET_LISTBOX_SELECTED_AGAIN  && id == worldListBoxID ) )
            {
                if (worldListBox->getSelected() > -1 ) {
                    worldSelected = worldListBox->getSelected();
                }
            }

            // Other buttons
            if (event.GUIEvent.EventType==irr::gui::EGET_BUTTON_CLICKED) 
            {
                if (id == importScenarioButtonID) {
                    guiImportExport->setText("");
                    selectWindow->setVisible(false);
                    guiImportExport->setVisible(true, 1); //importExportMode: 0 = export, 1 = import
                }

                if (id == exportScenarioButtonID) {
                    if (scenarioListBox->getSelected() > -1 ) {
                        
                        std::wstring scenarioWName = std::wstring(scenarioListBox->getListItem(scenarioListBox->getSelected()));
                        std::string scenarioName(scenarioWName.begin(), scenarioWName.end());
                        
                        std::string userFolder = Utilities::getUserDir();
                        std::string scenarioPath = "Scenarios/";
                        if (Utilities::pathExists(userFolder + scenarioPath)) {
                            scenarioPath = userFolder + scenarioPath;
                        }
                        // Load scenario data, and show in window
                        ScenarioData scenarioData = Utilities::getScenarioDataFromFile(scenarioPath + scenarioName, scenarioName); //Read a scenario from ini files
                        guiImportExport->setText(scenarioData.serialise(true));
                    }
                    selectWindow->setVisible(false);
                    guiImportExport->setVisible(true, 0);
                }

                if (id == importExportOKButtonID) {
                    // Mode: 0 = export, 1 = import
                    if (guiImportExport->getMode() == 1) {
                        // Import mode
                        scenarioData->deserialise(guiImportExport->getText());
                    }
                    guiImportExport->setVisible(false, 0);
                    selectWindow->setVisible(true);
                }
            }


		}

        return false;
    }

    irr::s32 StartupEventReceiver::getScenarioSelected() const
    {
        return scenarioSelected;
    }

    irr::s32 StartupEventReceiver::getWorldSelected() const
    {
        return worldSelected;
    }
