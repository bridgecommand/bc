//Common launcher program
//This just launches Bridge Command or
//Map Controller executable depending
//on which button the user presses

//TODO:
//Description text
//Copy bc5.ini into user dir if needed
//(Later - drop downs for _OPTION)

#include "irrlicht.h"
#include <iostream>
#include <fstream>
#include "../Lang.hpp"
#include "../IniFile.hpp"
#include "../Utilities.hpp"
#ifdef _WIN32
#include <direct.h> //for windows _mkdir
#else
#include <sys/stat.h>
#endif // _WIN32

//Mac OS:
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif //__APPLE__

// Irrlicht Namespaces
using namespace irr;

const irr::s32 SAVE_BUTTON = 1;

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

void saveFile(IrrlichtDevice* device, std::string iniFilename, gui::IGUITabControl* tabbedPane) {

    bool successSaving = false;
    std::ofstream file (iniFilename.c_str());
    if (file.is_open())
    {
        //For each tab, get tab name and child table
        for (int i = 0; i<tabbedPane->getTabCount(); i++) {

            //Get tab name - convert from stringw to std::string, loosing any extended character info.
            std::string sectionName(core::stringc(tabbedPane->getTab(i)->getText()).c_str());
            file << sectionName << std::endl;

            //For each table, get contents, including description
            core::list<gui::IGUIElement*> tabChildren = tabbedPane->getTab(i)->getChildren();
            //There should just be one child, but iterate through all, and do stuff with tables
            for (core::list<gui::IGUIElement*>::Iterator it = tabChildren.begin(); it != tabChildren.end(); ++it) {

                if ((*it)->getType() == gui::EGUIET_TABLE ) {
                    gui::IGUITable* thisTable = (gui::IGUITable*)(*it);
                    u32 numberOfRows = thisTable->getRowCount();
                    for (int j = 0; j<numberOfRows; j++) {
                        std::string varName(core::stringc(thisTable->getCellText(j,0)).c_str());
                        std::string varValue(core::stringc(thisTable->getCellText(j,1)).c_str());
                        std::string desc(core::stringc(thisTable->getCellText(j,2)).c_str());
                        file << varName << "=" << "\"" << varValue << "\"" << std::endl;
                        file <<varName << "_DESC=\"" << desc << "\"" << std::endl;
                    }
                }
            }



            //Collate these
            //Save (or warn if not possible)
        }
    if (file.good()) {successSaving=true;}
    file.close();
    }

    if (successSaving) {
        device->closeDevice();
    }
}

//Event receiver: This does the actual launching
class Receiver : public IEventReceiver
{
public:
    Receiver(IrrlichtDevice* device, gui::IGUIEnvironment* environment, gui::IGUITabControl* tabbedPane, std::string iniFilename)
    {
        this->environment = environment;
        this->tabbedPane = tabbedPane;
        this->iniFilename = iniFilename;
        this->device = device;
    }

private:
    IrrlichtDevice* device;
    gui::IGUIEnvironment* environment;
    core::position2di mousePos;
    irr::gui::IGUIEditBox* valueEntryBox = 0;
    irr::gui::IGUITable* selectedTable = 0; //Keep track of which table was last selected
    s32 selectedRow = 0; //In the selected table, which row was last selected?
    gui::IGUITabControl* tabbedPane;
    std::string iniFilename;

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
                    selectedTable->setCellText(selectedRow,1,core::stringw(valueEntryBox->getText())/*,video::SColor (255, 255, 255, 255)*/);

                    //Remove the edit box
                    valueEntryBox->remove();
                    valueEntryBox = 0;
                }
            }

            if (event.GUIEvent.EventType == gui::EGET_BUTTON_CLICKED ) {
                s32 id = event.GUIEvent.Caller->getID();
                if (id == SAVE_BUTTON) {

                    //Save to the ini file
                    saveFile(device,iniFilename,tabbedPane);

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

    //Choose the file to edit, with default of bc5.ini, change to map.ini if '-M' is used as first argument.
    std::string iniFilename = "bc5.ini";
    if ((argc>1)&&(strcmp(argv[1],"-M")==0)) {
        iniFilename = "map.ini";
    }

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
    //change up from BridgeCommand.app/Contents/MacOS/ini.app/Contents/MacOS to BridgeCommand.app/Contents/Resources
    exeFolderPath.append("/../../../../Resources");
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


    //Copy into userdir if not already there
    if (!Utilities::pathExists(userFolder + iniFilename)) {
        if (!Utilities::pathExists(Utilities::getUserDirBase())) {
            std::string pathToMake = Utilities::getUserDirBase();
            if (pathToMake.size() > 1) {pathToMake.erase(pathToMake.size()-1);} //Remove trailing slash
            #ifdef _WIN32
            mkdir(pathToMake.c_str());
            #else
            mkdir(pathToMake.c_str(),0755);
            #endif // _WIN32
        }
        if (!Utilities::pathExists(Utilities::getUserDir())) {
            std::string pathToMake = Utilities::getUserDir();
            if (pathToMake.size() > 1) {pathToMake.erase(pathToMake.size()-1);} //Remove trailing slash
            #ifdef _WIN32
            mkdir(pathToMake.c_str());
            #else
            mkdir(pathToMake.c_str(),0755);
            #endif // _WIN32
        }

        //Copy ini file from main into user dir
        std::ifstream iniFileIn (iniFilename.c_str());
        std::ofstream iniFileOut ((userFolder + iniFilename).c_str());
        if (iniFileIn.is_open()) {
            if (iniFileOut.is_open()) {
                //Copy line by line
                std::string line;
                while ( std::getline (iniFileIn,line) ) {
                    iniFileOut << line << std::endl ;
                }
                iniFileOut.close();
            }
            iniFileIn.close();
        }
    }
    //end copy

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
                    thisEntry.settingValue = Utilities::trim(splitLine.at(1),"\"");
                    //check if a '_DESC' setting is available
                    thisEntry.description = IniFile::iniFileToString(iniFilename,thisEntry.settingName+"_DESC");

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

    gui::IGUIButton* saveButton = environment->addButton(core::rect<s32>(10,height-90, 150, height - 10),0,SAVE_BUTTON,language.translate("save").c_str());

    gui::IGUITabControl* tabbedPane = environment->addTabControl( core::rect<s32>(10,10,width-10,height-100),0,true);

    //Add tab entry here
    for(int i = 0; i<iniFileStructure.size(); i++) {
        gui::IGUITab* thisTab = tabbedPane->addTab((core::stringw(iniFileStructure.at(i).tabName.c_str())).c_str());

        //Add tab contents in a table
        gui::IGUITable* thisTable = environment->addTable(core::rect<s32>(10,10,width-30,height-170),thisTab);

        thisTable->addColumn(language.translate("name").c_str());
        thisTable->addColumn(language.translate("value").c_str());
        thisTable->addColumn(language.translate("description").c_str());
        thisTable->setColumnWidth(0,150);
        thisTable->setColumnWidth(2,2000);
        thisTable->setToolTipText(language.translate("doubleClick").c_str());

        for (int j = 0; j<iniFileStructure.at(i).settings.size(); j++) {
            thisTable->addRow(j);
            thisTable->setCellText(j,0,core::stringw(iniFileStructure.at(i).settings.at(j).settingName.c_str()).c_str()/*,video::SColor (255, 255, 255, 255)*/ );
            thisTable->setCellText(j,1,core::stringw(iniFileStructure.at(i).settings.at(j).settingValue.c_str()).c_str()/*,video::SColor (255, 255, 255, 255)*/ );
            thisTable->setCellText(j,2,core::stringw(iniFileStructure.at(i).settings.at(j).description.c_str()).c_str()/*,video::SColor (255, 255, 255, 255)*/ );
        }



    }

    Receiver receiver(device, environment, tabbedPane, iniFilename);
    device->setEventReceiver(&receiver);

    while (device->run()) {
        driver->beginScene();
        device->getGUIEnvironment()->drawAll();
        driver->endScene();
    }
    return(0);
}
