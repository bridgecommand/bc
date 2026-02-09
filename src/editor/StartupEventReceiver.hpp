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

#ifndef __STARTUPEVENTRECEIVER_HPP_INCLUDED__
#define __STARTUPEVENTRECEIVER_HPP_INCLUDED__

#include "irrlicht.h"

//Forward declarations
class GUIImportExport;
class ScenarioData;

class StartupEventReceiver : public irr::IEventReceiver
{
public:

    StartupEventReceiver(
        irr::gui::IGUIListBox* scenarioListBox, 
        irr::gui::IGUIListBox* worldListBox,
        irr::gui::IGUIWindow* selectWindow,
        int32_t scenarioListBoxID, 
        int32_t worldListBoxID, 
        int32_t okScenarioButtonID, 
        int32_t okWorldButtonID, 
        int32_t importScenarioButtonID, 
        int32_t exportScenarioButtonID, 
        int32_t importExportOKButtonID,
        GUIImportExport* guiImportExport,
        ScenarioData* scenarioData);
    bool OnEvent(const irr::SEvent& event);

    int32_t getScenarioSelected() const;
    int32_t getWorldSelected() const;

private:

    irr::gui::IGUIListBox* scenarioListBox;
    irr::gui::IGUIListBox* worldListBox;
    irr::gui::IGUIWindow* selectWindow;
    GUIImportExport* guiImportExport;
    ScenarioData* scenarioData;
    int32_t scenarioListBoxID;
    int32_t worldListBoxID;
    int32_t okScenarioButtonID;
    int32_t okWorldButtonID;
    int32_t importScenarioButtonID;
    int32_t exportScenarioButtonID;
    int32_t importExportOKButtonID;
    int32_t scenarioSelected;
    int32_t worldSelected;

};

#endif

