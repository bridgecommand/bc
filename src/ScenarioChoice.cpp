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

#include "ExitMessage.hpp"

#include <iostream>
#include <fstream>

//using namespace irr;

ScenarioChoice::ScenarioChoice(irr::IrrlichtDevice* device, Lang* language)
{
    this->language = language;
    this->device = device;
    gui = device->getGUIEnvironment();
}

void ScenarioChoice::chooseScenario(std::string& scenarioName, std::string& hostname, irr::u32& udpPort, OperatingMode::Mode& mode, std::string scenarioPath)
{
    irr::video::IVideoDriver* driver = device->getVideoDriver();

    //Get list of scenarios, stored in scenarioList
    std::vector<std::string> scenarioList;
    std::vector<std::string> scenarioDescription;
    getScenarioList(scenarioList, scenarioDescription, scenarioPath); //Populate list

    //Get screen width
    irr::u32 su = driver->getScreenSize().Width;
    irr::u32 sh = driver->getScreenSize().Height;

    //Make gui elements
    irr::core::stringw titleText(LONGNAME.c_str());
    titleText.append(L"\nCopyright 2022 James Packer\n\n");
    titleText.append(L"Build: ");
    titleText.append(irr::core::stringw(__DATE__));
    titleText.append(L" ");
    titleText.append(irr::core::stringw(__TIME__));
    irr::core::dimension2d<irr::u32> titleDimensions = gui->getSkin()->getFont()->getDimension(titleText.c_str());
    irr::gui::IGUIStaticText* title = gui->addStaticText(titleText.c_str(),irr::core::rect<irr::s32>((su-titleDimensions.Width)/2, 0.017*sh, (su+titleDimensions.Width)/2, 0.09*sh));
    title->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);

    irr::gui::IGUIStaticText* instruction = gui->addStaticText(language->translate("scnChoose").c_str(),irr::core::rect<irr::s32>(0.02*su,0.13*sh,0.30*su, 0.17*sh));
    irr::gui::IGUIListBox* scenarioListBox = gui->addListBox(irr::core::rect<irr::s32>(0.02*su,0.17*sh,0.30*su,0.50*sh),0,GUI_ID_SCENARIO_LISTBOX);
    irr::gui::IGUIStaticText* description = gui->addStaticText(L"",irr::core::rect<irr::s32>(0.02*su,0.59*sh,0.30*su,0.99*sh));
    irr::gui::IGUIButton* okButton = gui->addButton(irr::core::rect<irr::s32>(0.02*su,0.51*sh,0.30*su,0.58*sh),0,GUI_ID_OK_BUTTON,language->translate("ok").c_str());

    irr::gui::IGUIStaticText* secondaryText = gui->addStaticText(language->translate("secondary").c_str(),irr::core::rect<irr::s32>(0.52*su,0.13*sh,1.00*su, 0.17*sh));
    irr::gui::IGUICheckBox* secondaryCheckbox = gui->addCheckBox(false,irr::core::rect<irr::s32>(0.52*su,0.18*sh,0.54*su,0.20*sh),0,GUI_ID_SECONDARY_CHECKBOX);

    irr::gui::IGUIStaticText* multiplayerText = gui->addStaticText(language->translate("multiplayer").c_str(),irr::core::rect<irr::s32>(0.52*su,0.23*sh,1.00*su, 0.27*sh));
    irr::gui::IGUICheckBox* multiplayerCheckbox = gui->addCheckBox(false,irr::core::rect<irr::s32>(0.52*su,0.28*sh,0.54*su,0.30*sh),0,GUI_ID_MULTIPLAYER_CHECKBOX);

    irr::gui::IGUIStaticText* hostnameText = gui->addStaticText(language->translate("hostname").c_str(),irr::core::rect<irr::s32>(0.52*su,0.33*sh,1.00*su, 0.41*sh));
    irr::gui::IGUIEditBox* hostnameBox = gui->addEditBox(irr::core::stringw(hostname.c_str()).c_str(),irr::core::rect<irr::s32>(0.52*su,0.42*sh,0.80*su,0.45*sh));
    hostnameBox->setToolTipText(language->translate("hostnameHelp").c_str());

    //For secondary only, allow the user to change the UDP port to listen on
    irr::gui::IGUIStaticText* portText = gui->addStaticText(language->translate("udpListenPort").c_str(),irr::core::rect<irr::s32>(0.52*su,0.33*sh,1.00*su, 0.41*sh));
    irr::gui::IGUIEditBox* portBox = gui->addEditBox(irr::core::stringw(udpPort).c_str(),irr::core::rect<irr::s32>(0.52*su,0.42*sh,0.80*su,0.45*sh));
    portBox->setToolTipText(language->translate("udpListenPortHelp").c_str());
    portText->setVisible(false);
    portBox->setVisible(false);

    //add credits text
    //irr::gui::IGUIStaticText* creditsText = gui->addStaticText((getCredits()).c_str(),irr::core::rect<irr::s32>(0.35*su,0.35*sh,0.95*su, 0.95*sh),true);

    //Add scenarios to list box
    for (std::vector<std::string>::iterator it = scenarioList.begin(); it != scenarioList.end(); ++it) {
        scenarioListBox->addItem(irr::core::stringw(it->c_str()).c_str()); //Note - odd conversion from char* to wchar*!
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
    StartupEventReceiver startupReceiver(scenarioListBox,instruction,hostnameText,hostnameBox,secondaryCheckbox,multiplayerCheckbox,portText,portBox,description,GUI_ID_SCENARIO_LISTBOX,GUI_ID_OK_BUTTON,GUI_ID_SECONDARY_CHECKBOX,GUI_ID_MULTIPLAYER_CHECKBOX, device);
    irr::IEventReceiver* oldReceiver = device->getEventReceiver();
    device->setEventReceiver(&startupReceiver);

    irr::s32 descriptionScenario = -1; //Which scenario we are showing the description text for

    while(device->run() && startupReceiver.getScenarioSelected()==-1) {
        if (device->isWindowActive())
        {
            //Event receiver will set Scenario Selected, so we just loop here until that happens
            driver->beginScene(irr::video::ECBF_COLOR|irr::video::ECBF_DEPTH, irr::video::SColor(0,200,200,200));
            
            //Set the 'description' text here
            irr::s32 currentSelection = scenarioListBox->getSelected();
            if (currentSelection!=descriptionScenario) {
                //Update the description text
                description->setText(L"");
                if (scenarioDescription.size() > currentSelection && currentSelection>=0) {
                    description->setText(irr::core::stringw(scenarioDescription.at(currentSelection).c_str()).c_str());
                }
                currentSelection = descriptionScenario;
            }
            
            gui->drawAll();
            driver->endScene();
        }
    }



    //Get name of selected scenario
    if (startupReceiver.getScenarioSelected()<0 || startupReceiver.getScenarioSelected() >= (irr::s32)scenarioList.size()) {
		ExitMessage::exitWithMessage("No scenario selected.");
    }

    //Get hostname, and convert from wchar_t* to wstring to string
    std::wstring wHostname = std::wstring(hostnameBox->getText());
    std::string sHostname(wHostname.begin(), wHostname.end());
    hostname = sHostname; //hostname is a pass by reference return value

    //Get UDP port, if the box is visible
    if (portBox->isVisible()) {
        udpPort = irr::core::strtoul10(irr::core::stringc(portBox->getText()).c_str());
    }

    //Check if 'secondary' mode is selected
    if(secondaryCheckbox->isChecked()) {
        mode = OperatingMode::Secondary;
    } else if (multiplayerCheckbox->isChecked()) {
        mode = OperatingMode::Multiplayer;
    } else {
        mode = OperatingMode::Normal;
    }

    if (mode == OperatingMode::Normal) {
        //Use selected scenario - don't need this in secondary mode
        scenarioName = scenarioList[startupReceiver.getScenarioSelected()]; //scenarioName is a pass by reference return value
    }

    //Clean up
    scenarioListBox->remove(); scenarioListBox = 0;
    okButton->remove(); okButton = 0;
    title->remove(); title = 0;
    instruction->remove(); instruction=0;
    secondaryText->remove(); secondaryText=0;
    secondaryCheckbox->remove(); secondaryCheckbox=0;
    multiplayerCheckbox->remove();multiplayerCheckbox=0;
    multiplayerText->remove();multiplayerText=0;
    hostnameBox->remove(); hostnameBox=0;
    hostnameText->remove();hostnameText=0;
    portBox->remove();portBox=0;
    portText->remove();portText=0;
    description->remove();description=0;
    //creditsText->remove(); creditsText=0;
    device->setEventReceiver(oldReceiver); //Remove link to startup event receiver, as this will be destroyed, and return to what we were using

    return;
}

void ScenarioChoice::getScenarioList(std::vector<std::string>&scenarioList, std::vector<std::string>&scenarioDescription, std::string scenarioPath) {

	irr::io::IFileSystem* fileSystem = device->getFileSystem();
	if (fileSystem==0) {
        ExitMessage::exitWithMessage("Could not get file system access.");
    }
    //store current dir
	irr::io::path cwd = fileSystem->getWorkingDirectory();

    //change to scenario dir
    if (!fileSystem->changeWorkingDirectoryTo(scenarioPath.c_str())) {
        ExitMessage::exitWithMessage("Could not get change working directory to scenario directory.");
    }

    irr::io::IFileList* fileList = fileSystem->createFileList();
    if (fileList==0) {
		ExitMessage::exitWithMessage("Could not get file list for secenarios.");
    }

    //List here
    for (irr::u32 i=0;i<fileList->getFileCount();i++) {
        if (fileList->isDirectory(i)) {
            const irr::io::path& fileName = fileList->getFileName(i);
            if (fileName.findFirst('.')!=0) { //Check it doesn't start with '.' (., .., or hidden)

                //Don't include scenarios ending in _mp (Multiplayer)
                //Check if name ends with "_mp" for multiplayer:
                bool multiplayerScenario = false;
                if (fileName.size() >= 3) {
                    const irr::io::path endChars = fileName.subString(fileName.size()-3,3,true);
                    if (endChars == irr::io::path("_mp")) {
                        multiplayerScenario = true;
                    }
                }

                //Add scenario to the list
                if (!multiplayerScenario) {
                    scenarioList.push_back(fileName.c_str());

                    //Try reading description.ini if it exists
                    irr::io::path descriptionFilePath = fileName;
                    irr::io::path descriptionFilename = descriptionFilePath.append("/description.ini");
                    std::ifstream descriptionStream (descriptionFilename.c_str());
                    //Set UTF-8 on Linux/OSX etc
                    #ifndef _WIN32
                        try {
                    #  ifdef __APPLE__
                            char* thisLocale = setlocale(LC_ALL, "");
                            if (thisLocale) {
                                descriptionStream.imbue(std::locale(thisLocale));
                            }
                    #  else
                            descriptionStream.imbue(std::locale("en_US.UTF8"));
                    #  endif
                        } catch (const std::runtime_error& runtimeError) {
                            descriptionStream.imbue(std::locale(""));
                        }
                    #endif

                    std::string descriptionLines="";
                    if (descriptionStream.is_open()) {
                        std::string descriptionLine;
                        while ( std::getline (descriptionStream,descriptionLine) )
                        {
                            descriptionLines.append(descriptionLine);
                            descriptionLines.append("\n");

                        }
                        descriptionStream.close();
                    }
                    scenarioDescription.push_back(descriptionLines); //Add even if empty

                }

            }
        }
    }

    //change back
    if (!fileSystem->changeWorkingDirectoryTo(cwd)) {
        ExitMessage::exitWithMessage("Can't return to normal working directory.");
    }
    fileList->drop();
}
