//Common launcher program
//This just launches Bridge Command or
//Map Controller executable depending
//on which button the user presses

#include "irrlicht.h"
#include <iostream>
#include <fstream>
#include "../Lang.hpp"
#include "../Utilities.hpp"

// Irrlicht Namespaces
using namespace irr;

const irr::s32 SAVE_BUTTON = 1;
//const irr::s32 MC_BUTTON = 2;
//const irr::s32 ED_BUTTON = 3;

//Structures to hold ini file contents
struct IniFileEntry {
    std::string settingName;
    std::string settingValue;
    std::string description;
    std::vector<std::string> settingOption;
};

struct IniFileTab {
    std::string tabName;
    std::vector<IniFileEntry> settings;
};

//Event receiver: This does the actual launching
class Receiver : public IEventReceiver
{
public:
    Receiver(gui::IGUIEnvironment* environment)
    {
        this->environment = environment;
    }

private:
    gui::IGUIEnvironment* environment;
    core::position2di mousePos;
    irr::gui::IGUIEditBox* valueEntryBox = 0;
    irr::gui::IGUITable* selectedTable = 0; //Keep track of which table was last selected
    s32 selectedRow = 0; //In the selected table, which row was last selected?

    virtual bool OnEvent(const SEvent& event)
    {
        if (event.EventType == EET_MOUSE_INPUT_EVENT) {
            if (event.MouseInput.Event == EMIE_MOUSE_MOVED) {
                mousePos.X = event.MouseInput.X;
                mousePos.Y = event.MouseInput.Y;
            }
        }

        if (event.EventType == EET_GUI_EVENT) {

            if (event.GUIEvent.EventType == gui::EGET_EDITBOX_ENTER || (event.GUIEvent.EventType == gui::EGET_ELEMENT_FOCUS_LOST && event.GUIEvent.Caller == valueEntryBox ) ) {
                //Enter or edit box has lost focus
                if (valueEntryBox!=0) {

                    //Update the table
                    selectedTable->setCellText(selectedRow,1,core::stringw(valueEntryBox->getText()),video::SColor (255, 255, 255, 255));

                    //Remove the edit box
                    valueEntryBox->remove();
                    valueEntryBox = 0;
                }
            }

            if (event.GUIEvent.EventType == gui::EGET_BUTTON_CLICKED ) {
                s32 id = event.GUIEvent.Caller->getID();
                if (id == SAVE_BUTTON) {

                }

            }

            if (event.GUIEvent.EventType == gui::EGET_TABLE_SELECTED_AGAIN  ) {
                selectedTable = ((gui::IGUITable*)event.GUIEvent.Caller);
                s32 id = event.GUIEvent.Caller->getID();
                selectedRow = selectedTable->getSelected();
                core::stringw selectedValue = core::stringw(selectedTable->getCellText(selectedRow,1));

                if (valueEntryBox==0) {
                    valueEntryBox = environment->addEditBox(selectedValue.c_str(),core::rect<s32>(mousePos.X, mousePos.Y, mousePos.X + 100, mousePos.Y + 30 )); //FIXME: Hardcoding of size
                    environment->setFocus(valueEntryBox);
                }

            }

            //When edit box looses focus or enter pressed (selected again?), save this and use

        }
        return false;
    }
};

int findCharOccurrences(std::string inputString, std::string findStr)
{
    int occurrences = 0;
    std::string::size_type charFound = -1; //FIXME: Valid?

    while (true) {
        charFound = inputString.find(findStr,charFound+1);
        if (charFound != std::string::npos) {
            occurrences++;
        } else {
            return occurrences;
        }
    }

}

int main (int argc, char ** argv)
{

    //Mac OS:
    //Find starting folder
	#ifdef __APPLE__
    char exePath[1024];
    uint32_t pathSize = sizeof(exePath);
    std::string exeFolderPath = "";
    if (_NSGetExecutablePath(exePath, &pathSize) == 0) {
        std::string exePathString(exePath);
        size_t pos = exePathString.find_last_of("\\/");
        if (std::string::npos != pos) {
            exeFolderPath = exePathString.substr(0, pos);
        }
    }
    //change up from BridgeCommand.app/Contents/MacOS to ../Resources
    exeFolderPath.append("/../Resources");
    //change to this path now
    chdir(exeFolderPath.c_str());
    //Note, we use this again after the createDevice call
	#endif

    u32 graphicsWidth = 800;
    u32 graphicsHeight = 600;
    u32 graphicsDepth = 32;
    bool fullScreen = false;

    Lang language("languageIniEditor.txt");

    IrrlichtDevice* device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(graphicsWidth,graphicsHeight),graphicsDepth,fullScreen,false,false,0);
    video::IVideoDriver* driver = device->getVideoDriver();

    gui::IGUIEnvironment* environment = device->getGUIEnvironment();

    core::dimension2d<u32> screenSize = driver->getScreenSize();
    u32 width = screenSize.Width;
    u32 height = screenSize.Height;

    #ifdef __APPLE__
    //Mac OS - cd back to original dir - seems to be changed during createDevice
    io::IFileSystem* fileSystem = device->getFileSystem();
    if (fileSystem==0) {
        exit(EXIT_FAILURE); //Could not get file system TODO: Message for user
        std::cout << "Could not get filesystem" << std::endl;
    }
    fileSystem->changeWorkingDirectoryTo(exeFolderPath.c_str());
    #endif

    //User read/write location - look in here first and the exe folder second for files
    std::string userFolder = Utilities::getUserDir();
    std::cout << "User folder is " << userFolder << std::endl;
    //Read basic ini settings
    std::string iniFilename = "bc5.ini";
    //Use local ini file if it exists
    if (Utilities::pathExists(userFolder + iniFilename)) {
        iniFilename = userFolder + iniFilename;
    }

    //Set font : Todo - make this configurable
    gui::IGUIFont *font = environment->getFont("media/lucida.xml");
    if (font == 0) {
        std::cout << "Could not load font, using default" << std::endl;
    } else {
        //set skin default font
        environment->getSkin()->setFont(font);
    }

    //Vector to hold values read into file
    std::vector<IniFileTab> iniFileStructure;

    IniFileTab* thisSection = 0;

    //open the ini file:
    std::ifstream file (iniFilename.c_str());
    if (file.is_open())
    {
        std::string line;
        while ( std::getline (file,line) )
        {
            line = Utilities::trim(line);

            if (findCharOccurrences(line,"[") == 1 && findCharOccurrences(line,"]") == 1 ) {
                //A section heading

                //Store the previous tab if it exists, and create a new one
                if (thisSection!=0) {
                    iniFileStructure.push_back(*thisSection);
                    delete thisSection; //TODO: Check if this is appropriate
                }
                thisSection = new IniFileTab;
                thisSection->tabName=line;

            } else if (findCharOccurrences(line,"=") == 1 && findCharOccurrences(line,"_DESC") == 0 && findCharOccurrences(line,"_OPTION") == 0 ) {
                //A normal entry

                //Create a section if it doesn't exist
                if (thisSection==0) {
                    thisSection = new IniFileTab;
                    thisSection->tabName="[General]";
                }

                //Store this line
                std::vector<std::string> splitLine = Utilities::split(line,'=');
                if (splitLine.size() == 2) {
                    IniFileEntry thisEntry;
                    thisEntry.settingName = splitLine.at(0);
                    thisEntry.settingValue = splitLine.at(1);

                    thisSection->settings.push_back(thisEntry);
                }
            }
        }
        file.close();
        //Store the final tab if it exists
        if (thisSection!=0) {
            iniFileStructure.push_back(*thisSection);
            delete thisSection; //TODO: Check if this is appropriate
        }
    }

    //Do set-up here
    gui::IGUITabControl* tabbedPane = environment->addTabControl( core::rect<s32>(10,10,width-10,height-10));


    //Add tab entry here
    for(int i = 0; i<iniFileStructure.size(); i++) {
        gui::IGUITab* thisTab = tabbedPane->addTab((core::stringw(iniFileStructure.at(i).tabName.c_str())).c_str());

        //Add tab contents in a table
        gui::IGUITable* thisTable = environment->addTable(core::rect<s32>(10,10,width-30,height-170),thisTab);

        thisTable->addColumn(language.translate("name").c_str());
        thisTable->addColumn(language.translate("value").c_str());
        thisTable->addColumn(language.translate("description").c_str());

        for (int j = 0; j<iniFileStructure.at(i).settings.size(); j++) {
            thisTable->addRow(j);
            thisTable->setCellText(j,0,core::stringw(iniFileStructure.at(i).settings.at(j).settingName.c_str()).c_str(),video::SColor (255, 255, 255, 255) );
            thisTable->setCellText(j,1,core::stringw(iniFileStructure.at(i).settings.at(j).settingValue.c_str()).c_str(),video::SColor (255, 255, 255, 255) );
        }



    }

    Receiver receiver(environment);
    device->setEventReceiver(&receiver);

    while (device->run()) {
        driver->beginScene();
        device->getGUIEnvironment()->drawAll();
        driver->endScene();
    }
    return(0);
}
