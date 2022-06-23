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

#include "ScenarioChoice.hpp"
#include "StartupEventReceiver.hpp"
#include "../Constants.hpp"

#include <iostream>

//using namespace irr;

ScenarioChoice::ScenarioChoice(irr::IrrlichtDevice* device, Lang* language)
{
    this->language = language;
    this->device = device;
    gui = device->getGUIEnvironment();
}

void ScenarioChoice::chooseScenario(std::string& scenarioName, std::string& hostname, std::string scenarioPath)
{
	irr::video::IVideoDriver* driver = device->getVideoDriver();

    //Get list of scenarios, stored in scenarioList
    std::vector<std::string> scenarioList;
    getScenarioList(scenarioList,scenarioPath); //Populate list

    //Get screen width
    irr::u32 su = driver->getScreenSize().Width;
    irr::u32 sh = driver->getScreenSize().Height;

    //Make gui elements
    irr::core::stringw titleText(LONGNAME.c_str());
    titleText.append(L"\nCopyright 2022 James Packer");
    irr::core::dimension2d<irr::u32> titleDimensions = gui->getSkin()->getFont()->getDimension(titleText.c_str());
    irr::gui::IGUIStaticText* title = gui->addStaticText(titleText.c_str(),irr::core::rect<irr::s32>((su-titleDimensions.Width)/2, 0.017*sh, (su+titleDimensions.Width)/2, 0.09*sh));

    irr::gui::IGUIStaticText* instruction = gui->addStaticText(language->translate("scnChoose").c_str(),irr::core::rect<irr::s32>(0.02*su,0.13*sh,0.30*su, 0.17*sh));
    irr::gui::IGUIListBox* scenarioListBox = gui->addListBox(irr::core::rect<irr::s32>(0.02*su,0.20*sh,0.30*su,0.50*sh),0,GUI_ID_SCENARIO_LISTBOX);
    irr::gui::IGUIButton* okButton = gui->addButton(irr::core::rect<irr::s32>(0.32*su,0.40*sh,0.98*su,0.50*sh),0,GUI_ID_OK_BUTTON,language->translate("ok").c_str());

    irr::gui::IGUIStaticText* hostnameText = gui->addStaticText(language->translate("hostname").c_str(),irr::core::rect<irr::s32>(0.32*su,0.13*sh,0.98*su, 0.22*sh));
    irr::gui::IGUIEditBox* hostnameBox = gui->addEditBox(L"",irr::core::rect<irr::s32>(0.32*su,0.20*sh,0.98*su,0.23*sh));

    irr::gui::IGUIStaticText* instructionText = gui->addStaticText(language->translate("startupInstructions").c_str(),irr::core::rect<irr::s32>(0.32*su,0.26*sh,0.98*su, 0.32*sh));

    //add credits text
    //irr::gui::IGUIStaticText* creditsText = gui->addStaticText((getCredits()).c_str(),irr::core::rect<irr::s32>(0.35*su,0.35*sh,0.95*su, 0.95*sh),true);

    //Add scenarios to list box
    for (std::vector<std::string>::iterator it = scenarioList.begin(); it != scenarioList.end(); ++it) {
        scenarioListBox->addItem(irr::core::stringw(it->c_str()).c_str()); //Fixme - odd conversion from char* to wchar*!
    }
    //select first one if possible
    if (scenarioListBox->getItemCount()>0) {
        scenarioListBox->setSelected(0);
    }
    //select list box as active, so user can use up/down arrows without needing to click
    gui->setFocus(scenarioListBox);

    //Flush old key/clicks etc, with a 0.2s pause
    device->sleep(200);
    device->clearSystemMessages();

    //Link to our event receiver
    StartupEventReceiver startupReceiver(scenarioListBox,GUI_ID_SCENARIO_LISTBOX,GUI_ID_OK_BUTTON);
    device->setEventReceiver(&startupReceiver);

    while(device->run() && startupReceiver.getScenarioSelected()==-1) {
        if (device->isWindowActive())
        {
            //Event receiver will set Scenario Selected, so we just loop here until that happens
            driver->beginScene(true, false, irr::video::SColor(0,200,200,200)); //Don't need to clear z buffer for 2d
            gui->drawAll();
            driver->endScene();
        }
    }

    //Get name of selected scenario
    if (startupReceiver.getScenarioSelected()<0 || startupReceiver.getScenarioSelected() >= (irr::s32)scenarioList.size()) {
        exit(EXIT_FAILURE); //No scenario loaded
    }

    //Get hostname, and convert from wchar_t* to wstring to string
    std::wstring wHostname = std::wstring(hostnameBox->getText());
    std::string sHostname(wHostname.begin(), wHostname.end());
    hostname = sHostname; //hostname is a pass by reference return value

    scenarioName = scenarioList[startupReceiver.getScenarioSelected()]; //scenarioName is a pass by reference return value

    //Clean up
    scenarioListBox->remove(); scenarioListBox = 0;
    okButton->remove(); okButton = 0;
    title->remove(); title = 0;
    instruction->remove(); instruction=0;
    hostnameBox->remove(); hostnameBox=0;
    hostnameText->remove();hostnameText=0;
    instructionText->remove();instructionText=0;
    //creditsText->remove(); creditsText=0;
    device->setEventReceiver(0); //Remove link to startup event receiver, as this will be destroyed.

}

void ScenarioChoice::getScenarioList(std::vector<std::string>&scenarioList, std::string scenarioPath) {

	irr::io::IFileSystem* fileSystem = device->getFileSystem();
    if (fileSystem==0) {
        exit(EXIT_FAILURE); //Could not get file system TODO: Message for user
    }
    //store current dir
    irr::io::path cwd = fileSystem->getWorkingDirectory();

    //change to scenario dir
    if (!fileSystem->changeWorkingDirectoryTo(scenarioPath.c_str())) {
        exit(EXIT_FAILURE); //Couldn't change to scenario dir
    }

    irr::io::IFileList* fileList = fileSystem->createFileList();
    if (fileList==0) {
        exit(EXIT_FAILURE); //Could not get file list for scenarios TODO: Message for user
    }

    //List here
    for (irr::u32 i=0;i<fileList->getFileCount();i++) {
        if (fileList->isDirectory(i)) {
            const irr::io::path& fileName = fileList->getFileName(i);
            if (fileName.findFirst('.')!=0) { //Check it doesn't start with '.' (., .., or hidden)
                //std::cout << fileName.c_str() << std::endl;

                //Check if name ends with "_mp" for multiplayer:
                if (fileName.size() >= 3) {
                    const irr::io::path endChars = fileName.subString(fileName.size()-3,3,true);
                    if (endChars == irr::io::path("_mp")) {
                        scenarioList.push_back(fileName.c_str());
                    }
                }

            }
        }
    }

    //change back
    if (!fileSystem->changeWorkingDirectoryTo(cwd)) {
        exit(EXIT_FAILURE); //Couldn't change dir back
    }
    fileList->drop();
}
