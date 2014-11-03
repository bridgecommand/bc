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
#include "Constants.hpp"

#include <iostream>

using namespace irr;

ScenarioChoice::ScenarioChoice(irr::IrrlichtDevice* device, Lang* language)
{
    this->language = language;
    this->device = device;
    gui = device->getGUIEnvironment();
}

std::string ScenarioChoice::chooseScenario()
{
    video::IVideoDriver* driver = device->getVideoDriver();

    //Get list of scenarios, stored in scenarioList
    std::vector<std::string> scenarioList;
    getScenarioList(scenarioList,"Scenarios/"); //Populate list //Fixme: Scenarios path duplicated here and in SimulationModel

    //Get screen width
    u32 su = driver->getScreenSize().Width;
    u32 sh = driver->getScreenSize().Height;

    //Make gui elements
    core::stringw titleText(LONGNAME.c_str());
    titleText.append(L"\nCopyright 2014 James Packer");
    core::dimension2d<u32> titleDimensions = gui->getSkin()->getFont()->getDimension(titleText.c_str());
    gui::IGUIListBox* scenarioListBox = gui->addListBox(core::rect<s32>(0.02*su,0.07*sh,0.18*su,0.40*sh),0,GUI_ID_SCENARIO_LISTBOX);
    gui::IGUIButton* okButton = gui->addButton(core::rect<s32>(0.02*su,0.41*sh,0.18*su,0.48*sh),0,GUI_ID_OK_BUTTON,language->translate("ok").c_str());
    gui::IGUIStaticText* instruction = gui->addStaticText(language->translate("scnChoose").c_str(),core::rect<s32>(0.02*su,0.03*sh,0.18*su, 0.07*sh));
    gui::IGUIStaticText* title = gui->addStaticText(titleText.c_str(),core::rect<s32>((su-titleDimensions.Width)/2, 0.017*sh, (su+titleDimensions.Width)/2, 0.05*sh));
    //Add scenarios to list box
    for (std::vector<std::string>::iterator it = scenarioList.begin(); it != scenarioList.end(); ++it) {
        scenarioListBox->addItem(core::stringw(it->c_str()).c_str()); //Fixme - odd conversion from char* to wchar*!
    }
    //select first one if possible
    if (scenarioListBox->getItemCount()>0) {
        scenarioListBox->setSelected(0);
    }
    //select list box as active, so user can use up/down arrows without needing to click
    gui->setFocus(scenarioListBox);

    //Link to our event receiver
    StartupEventReceiver startupReceiver(scenarioListBox,GUI_ID_SCENARIO_LISTBOX,GUI_ID_OK_BUTTON);
    device->setEventReceiver(&startupReceiver);

    while(device->run() && startupReceiver.getScenarioSelected()==-1) {
        if (device->isWindowActive())
        {
            //Event receiver will set Scenario Selected, so we just loop here until that happens
            driver->beginScene(true, true, video::SColor(0,200,200,200));
            gui->drawAll();
            driver->endScene();
        }
    }

    //Get name of selected scenario
    if (startupReceiver.getScenarioSelected()<0 || startupReceiver.getScenarioSelected() >= (s32)scenarioList.size()) {
        exit(EXIT_FAILURE); //No scenario loaded
    }
    std::string scenarioName = scenarioList[startupReceiver.getScenarioSelected()];

    //Clean up
    scenarioListBox->remove(); scenarioListBox = 0;
    okButton->remove(); okButton = 0;
    title->remove(); title = 0;
    instruction->remove(); instruction=0;
    device->setEventReceiver(0); //Remove link to startup event receiver, as this will be destroyed.

    //Show loading message
    gui::IGUIStaticText* loadingMessage = gui->addStaticText(language->translate("loadingmsg").c_str(), core::rect<s32>(0.0125*su,0.017*sh,0.27*su,0.033*sh));
    device->run();
    driver->beginScene(true, true, video::SColor(0,200,200,200));
    gui->drawAll();
    driver->endScene();
    loadingMessage->remove(); loadingMessage = 0;

    return scenarioName;
}

void ScenarioChoice::getScenarioList(std::vector<std::string>&scenarioList, std::string scenarioPath) {

    io::IFileSystem* fileSystem = device->getFileSystem();
    if (fileSystem==0) {
        exit(EXIT_FAILURE); //Could not get file system TODO: Message for user
    }
    //store current dir
    io::path cwd = fileSystem->getWorkingDirectory();

    //change to scenario dir
    if (!fileSystem->changeWorkingDirectoryTo(scenarioPath.c_str())) {
        exit(EXIT_FAILURE); //Couldn't change to scenario dir
    }

    io::IFileList* fileList = fileSystem->createFileList();
    if (fileList==0) {
        exit(EXIT_FAILURE); //Could not get file list for scenarios TODO: Message for user
    }

    //List here
    for (u32 i=0;i<fileList->getFileCount();i++) {
        if (fileList->isDirectory(i)) {
            const io::path& fileName = fileList->getFileName(i);
            if (fileName.findFirst('.')!=0) { //Check it doesn't start with '.' (., .., or hidden)
                //std::cout << fileName.c_str() << std::endl;
                scenarioList.push_back(fileName.c_str());
            }
        }
    }

    //change back
    if (!fileSystem->changeWorkingDirectoryTo(cwd)) {
        exit(EXIT_FAILURE); //Couldn't change dir back
    }
    fileList->drop();
}
