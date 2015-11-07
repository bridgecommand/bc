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

void ScenarioChoice::chooseScenario(std::string& scenarioName, std::string& hostname, bool& secondary, std::string scenarioPath)
{
    video::IVideoDriver* driver = device->getVideoDriver();

    //Get list of scenarios, stored in scenarioList
    std::vector<std::string> scenarioList;
    getScenarioList(scenarioList,scenarioPath); //Populate list

    //Get screen width
    u32 su = driver->getScreenSize().Width;
    u32 sh = driver->getScreenSize().Height;

    //Make gui elements
    core::stringw titleText(LONGNAME.c_str());
    titleText.append(L"\nCopyright 2015 James Packer");
    core::dimension2d<u32> titleDimensions = gui->getSkin()->getFont()->getDimension(titleText.c_str());
    gui::IGUIStaticText* title = gui->addStaticText(titleText.c_str(),core::rect<s32>((su-titleDimensions.Width)/2, 0.017*sh, (su+titleDimensions.Width)/2, 0.09*sh));

    gui::IGUIStaticText* instruction = gui->addStaticText(language->translate("scnChoose").c_str(),core::rect<s32>(0.02*su,0.13*sh,0.30*su, 0.17*sh));
    gui::IGUIListBox* scenarioListBox = gui->addListBox(core::rect<s32>(0.02*su,0.17*sh,0.30*su,0.50*sh),0,GUI_ID_SCENARIO_LISTBOX);
    gui::IGUIButton* okButton = gui->addButton(core::rect<s32>(0.02*su,0.51*sh,0.30*su,0.58*sh),0,GUI_ID_OK_BUTTON,language->translate("ok").c_str());

    gui::IGUIStaticText* secondaryText = gui->addStaticText(language->translate("secondary").c_str(),core::rect<s32>(0.52*su,0.13*sh,1.00*su, 0.17*sh));
    gui::IGUICheckBox* secondaryCheckbox = gui->addCheckBox(false,core::rect<s32>(0.52*su,0.18*sh,0.54*su,0.20*sh),0,GUI_ID_SECONDARY_CHECKBOX);

    gui::IGUIStaticText* hostnameText = gui->addStaticText(language->translate("hostname").c_str(),core::rect<s32>(0.52*su,0.23*sh,1.00*su, 0.32*sh));
    gui::IGUIEditBox* hostnameBox = gui->addEditBox(L"",core::rect<s32>(0.52*su,0.30*sh,0.80*su,0.33*sh));

    //add credits text
    //gui::IGUIStaticText* creditsText = gui->addStaticText((getCredits()).c_str(),core::rect<s32>(0.35*su,0.35*sh,0.95*su, 0.95*sh),true);

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
    StartupEventReceiver startupReceiver(scenarioListBox,hostnameText,hostnameBox,GUI_ID_SCENARIO_LISTBOX,GUI_ID_OK_BUTTON,GUI_ID_SECONDARY_CHECKBOX);
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

    //Use selected scenario
    scenarioName = scenarioList[startupReceiver.getScenarioSelected()]; //scenarioName is a pass by reference return value
    //Get hostname, and convert from wchar_t* to wstring to string
    std::wstring wHostname = std::wstring(hostnameBox->getText());
    std::string sHostname(wHostname.begin(), wHostname.end());
    hostname = sHostname; //hostname is a pass by reference return value

    //Check if 'secondary' mode is selected
    secondary = secondaryCheckbox->isChecked(); //secondary is a pass by reference return value

    //Clean up
    scenarioListBox->remove(); scenarioListBox = 0;
    okButton->remove(); okButton = 0;
    title->remove(); title = 0;
    instruction->remove(); instruction=0;
    secondaryText->remove(); secondaryText=0;
    secondaryCheckbox->remove(); secondaryCheckbox=0;
    hostnameBox->remove(); hostnameBox=0;
    hostnameText->remove();hostnameText=0;
    //creditsText->remove(); creditsText=0;
    device->setEventReceiver(0); //Remove link to startup event receiver, as this will be destroyed.

    //Show loading message
    irr::core::stringw creditsText = language->translate("loadingmsg");
    creditsText.append(L"\n\n");
    creditsText.append(getCredits());
    gui::IGUIStaticText* loadingMessage = gui->addStaticText(creditsText.c_str(), core::rect<s32>(0.05*su,0.05*sh,0.95*su,0.95*sh),true);
    device->run();
    driver->beginScene(true, true, video::SColor(0,200,200,200));
    gui->drawAll();
    driver->endScene();
    loadingMessage->remove(); loadingMessage = 0;

    return;
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

irr::core::stringw ScenarioChoice::getCredits(){

    irr::core::stringw creditsString(L"NO DATA SUPPLIED WITH THIS PROGRAM, OR DERIVED FROM IT IS TO BE USED FOR NAVIGATION.\n\n");
    creditsString.append(L"Bridge Command is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 2 as published by the Free Software Foundation.\n\n");
    creditsString.append(L"Bridge Command  is distributed  in the  hope that  it will  be useful, but WITHOUT ANY WARRANTY; without even the implied  warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.\n\n");
    creditsString.append(L"In memory of Sergio Fuentes, who provided many useful suggestions for the program's development.\n\n");
    creditsString.append(L"Many thanks to those who have made their models available for use in Bridge Command:\n");
    creditsString.append(L"> Juergen Klemp\n");
    creditsString.append(L"> Simon D Richardson\n");
    creditsString.append(L"> Jason Simpson\n");
    creditsString.append(L"> Ragnar\n");
    creditsString.append(L"> Thierry Videlaine\n");
    creditsString.append(L"> NETC (Naval Education and Training Command)\n\n");
    creditsString.append(L"Many thanks to Ken Trethewey for making his images of the Eddystone lighthouse available.\n\n");
    creditsString.append(L"Bridge Command uses the Irrlicht Engine, the ENet networking library, the RealisticWaterSceneNode by elvman, and the Serial library by William Woodall.\n\n");
    creditsString.append(L"The Irrlicht Engine is based in part on the work of the Independent JPEG Group, the zlib, and libpng.");


    return creditsString;


}
