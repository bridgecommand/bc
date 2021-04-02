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
#include <unistd.h>
#endif //__APPLE__

#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

#ifdef __linux__
    #include <unistd.h>
#endif

// Irrlicht Namespaces
//using namespace irr;

const irr::s32 SAVE_BUTTON = 1;

//Set up global for ini reader to have access to irrlicht logger if needed.
namespace IniFile {
    irr::ILogger* irrlichtLogger = 0;
}

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

void saveFile(irr::IrrlichtDevice* device, std::string iniFilename, irr::gui::IGUITabControl* tabbedPane) {

    bool successSaving = false;
    std::ofstream file (iniFilename.c_str());
    if (file.is_open())
    {
        //For each tab, get tab name and child table
        for (int i = 0; i<tabbedPane->getTabCount(); i++) {

            //Get tab name - convert from stringw to std::string, loosing any extended character info.
            std::string sectionName(irr::core::stringc(tabbedPane->getTab(i)->getText()).c_str());
            file << "[" + sectionName + "]" << std::endl;

            //For each table, get contents, including description
            irr::core::list<irr::gui::IGUIElement*> tabChildren = tabbedPane->getTab(i)->getChildren();
            //There should just be one child, but iterate through all, and do stuff with tables
            for (irr::core::list<irr::gui::IGUIElement*>::Iterator it = tabChildren.begin(); it != tabChildren.end(); ++it) {

                if ((*it)->getType() == irr::gui::EGUIET_TABLE ) {
                    irr::gui::IGUITable* thisTable = (irr::gui::IGUITable*)(*it);
                    irr::u32 numberOfRows = thisTable->getRowCount();
                    for (int j = 0; j<numberOfRows; j++) {
                        std::string varName(irr::core::stringc(thisTable->getCellText(j,0)).c_str());
                        std::string varValue(irr::core::stringc(thisTable->getCellText(j,1)).c_str());
                        std::string desc(irr::core::stringc(thisTable->getCellText(j,2)).c_str());
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
class Receiver : public irr::IEventReceiver
{
public:
    Receiver(irr::IrrlichtDevice* device, irr::gui::IGUIEnvironment* environment, irr::gui::IGUITabControl* tabbedPane, std::string iniFilename)
    {
        this->environment = environment;
        this->tabbedPane = tabbedPane;
        this->iniFilename = iniFilename;
        this->device = device;
    }

private:
    irr::IrrlichtDevice* device;
    irr::gui::IGUIEnvironment* environment;
    irr::core::position2di mousePos;
    irr::gui::IGUIEditBox* valueEntryBox = 0;
    irr::gui::IGUITable* selectedTable = 0; //Keep track of which table was last selected
    irr::s32 selectedRow = 0; //In the selected table, which row was last selected?
    irr::gui::IGUITabControl* tabbedPane;
    std::string iniFilename;

    virtual bool OnEvent(const irr::SEvent& event)
    {
        if (event.EventType == irr::EET_MOUSE_INPUT_EVENT) {
            if (event.MouseInput.Event == irr::EMIE_MOUSE_MOVED) {
                mousePos.X = event.MouseInput.X;
                mousePos.Y = event.MouseInput.Y;
            }
        }

        if (event.EventType == irr::EET_GUI_EVENT) {

            if (event.GUIEvent.EventType == irr::gui::EGET_EDITBOX_ENTER || (event.GUIEvent.EventType == irr::gui::EGET_ELEMENT_FOCUS_LOST && event.GUIEvent.Caller == valueEntryBox ) ) {
                //Enter or edit box has lost focus
                if (valueEntryBox!=0) {

                    //Update the table
                    selectedTable->setCellText(selectedRow,1,irr::core::stringw(valueEntryBox->getText())/*,video::SColor (255, 255, 255, 255)*/);

                    //Remove the edit box
                    valueEntryBox->remove();
                    valueEntryBox = 0;
                }
            }

            if (event.GUIEvent.EventType == irr::gui::EGET_BUTTON_CLICKED ) {
                irr::s32 id = event.GUIEvent.Caller->getID();
                if (id == SAVE_BUTTON) {

                    //Save to the ini file
                    saveFile(device,iniFilename,tabbedPane);

                }

            }

            if (event.GUIEvent.EventType == irr::gui::EGET_TABLE_SELECTED_AGAIN  ) {
                selectedTable = ((irr::gui::IGUITable*)event.GUIEvent.Caller);
                irr::s32 id = event.GUIEvent.Caller->getID();
                selectedRow = selectedTable->getSelected();
                irr::core::stringw selectedValue = irr::core::stringw(selectedTable->getCellText(selectedRow,1));

                if (valueEntryBox==0) {
                    valueEntryBox = environment->addEditBox(selectedValue.c_str(),irr::core::rect<irr::s32>(mousePos.X, mousePos.Y, mousePos.X + 100, mousePos.Y + 30 )); //FIXME: Hardcoding of size
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

    #ifdef FOR_DEB
    chdir("/usr/share/bridgecommand");
    #endif // FOR_DEB

    //Choose the file to edit, with default of bc5.ini, change to map.ini if '-M' is used as first argument, or mph.ini if -H, or repeater.ini -f -R
    std::string iniFilename = "bc5.ini";
    if ((argc>1)&&(strcmp(argv[1],"-M")==0)) {
        iniFilename = "map.ini";
    }
    if ((argc>1)&&(strcmp(argv[1],"-H")==0)) {
        iniFilename = "mph.ini";
    }
    if ((argc>1)&&(strcmp(argv[1],"-R")==0)) {
        iniFilename = "repeater.ini";
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

    //User read/write location - look in here first and the exe folder second for files
    std::string userFolder = Utilities::getUserDir();
    std::cout << "User folder is " << userFolder << std::endl;


    //Copy into userdir if not already there
    if (!Utilities::pathExists(userFolder + iniFilename)) {
        if (!Utilities::pathExists(Utilities::getUserDirBase())) {
            std::string pathToMake = Utilities::getUserDirBase();
            if (pathToMake.size() > 1) {pathToMake.erase(pathToMake.size()-1);} //Remove trailing slash
            #ifdef _WIN32
            _mkdir(pathToMake.c_str());
            #else
            mkdir(pathToMake.c_str(),0755);
            #endif // _WIN32
        }
        if (!Utilities::pathExists(Utilities::getUserDir())) {
            std::string pathToMake = Utilities::getUserDir();
            if (pathToMake.size() > 1) {pathToMake.erase(pathToMake.size()-1);} //Remove trailing slash
            #ifdef _WIN32
            _mkdir(pathToMake.c_str());
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

    //Store a copy of the global ini filename
    std::string globalIniFilename = iniFilename;

    //Use local ini file if it exists
    if (Utilities::pathExists(userFolder + iniFilename)) {
        iniFilename = userFolder + iniFilename;
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

    //Read the global ini file. If there are any lines that don't exist in the user's ini file, copy in
    std::ifstream globalFile (globalIniFilename.c_str());
    if (globalFile.is_open())
    {
		//std::cout << "Opened global ini file " << globalIniFilename << std::endl;
        std::string line;
		std::string currentTabName = "[General]";
		while ( std::getline (globalFile,line) )
		{
			line = Utilities::trim(line);
			if (findCharOccurrences(line,"[") == 1 && findCharOccurrences(line,"]") == 1 ) {
				//A section heading
				currentTabName=line;
			} else if (findCharOccurrences(line,"=") == 1 && findCharOccurrences(line,"_DESC") == 0 && findCharOccurrences(line,"_OPTION") == 0 ) {
				//A normal entry
				std::vector<std::string> splitLine = Utilities::split(line,'=');
				if (splitLine.size() == 2) {
					IniFileEntry thisEntry;
					thisEntry.settingName = splitLine.at(0);
					thisEntry.settingValue = Utilities::trim(splitLine.at(1),"\"");
					//check if a '_DESC' setting is available
					thisEntry.description = IniFile::iniFileToString(globalIniFilename,thisEntry.settingName+"_DESC");
					//Check if this exists in the main iniFileStructure
					bool found = false;

                    //std::cout << "Entry: " << thisEntry.settingName << ":" << thisEntry.settingValue << std::endl;

					//TODO: Ignore 'joystick_map*' cases, ie force these to return found = true;

					for (int i = 0; i < iniFileStructure.size(); i++) {
						for (int j = 0; j < iniFileStructure.at(i).settings.size(); j++) {
							std::string compare1 = iniFileStructure.at(i).settings.at(j).settingName;
							std::string compare2 = thisEntry.settingName;
							Utilities::to_lower(compare1);
							Utilities::to_lower(compare2);
							//Ignore 'joystick_map*' cases, as we don't want to copy these across even if different
							if (compare2.find("joystick_map") != std::string::npos  || compare1.compare(compare2)==0 ) {
								found = true;
								break;
							}
						}
						if (found) {
							break;
						}
					}

					//If not, find the corresponding tab, or fall back to the first tab
					if (!found) {
						//Add to corresponding tab
                        int whichTab = 0;
						for (int i = 0; i < iniFileStructure.size(); i++) {
							if (currentTabName.compare(iniFileStructure.at(i).tabName) == 0) {
								whichTab = i;
							}
						}
                        iniFileStructure.at(whichTab).settings.push_back(thisEntry);
					}

				}
			}
		}
		globalFile.close();
    }

    std::string modifier = IniFile::iniFileToString(iniFilename, "lang");
    if (modifier.length()==0) {
        modifier = "en"; //Default
    }
    std::string languageFile = "languageIniEditor-";
    languageFile.append(modifier);
    languageFile.append(".txt");
    if (Utilities::pathExists(userFolder + languageFile)) {
        languageFile = userFolder + languageFile;
    }

    Lang language(languageFile);

    int fontSize = 12;
    float fontScale = IniFile::iniFileTof32(iniFilename, "font_scale");
    if (fontScale > 1) {
        fontSize = (int)(fontSize * fontScale + 0.5);
    } else {
	    fontScale = 1.0;
    }
    
    irr::u32 graphicsWidth;
    irr::u32 graphicsHeight;
    irr::IrrlichtDevice *nulldevice = irr::createDevice(irr::video::EDT_NULL);
	irr::core::dimension2d<irr::u32> deskres = nulldevice->getVideoModeList()->getDesktopResolution();
	nulldevice->drop();
    graphicsWidth = 1200 * fontScale;
    if (graphicsWidth > deskres.Width*0.9) {
        graphicsWidth = deskres.Width*0.9;
    }
    graphicsHeight = 900 * fontScale;
    if (graphicsHeight > deskres.Height*0.9) {
        graphicsHeight = deskres.Height*0.9;
    }
    
    irr::u32 graphicsDepth = 32;
    bool fullScreen = false;

    irr::IrrlichtDevice* device = irr::createDevice(irr::video::EDT_OPENGL, irr::core::dimension2d<irr::u32>(graphicsWidth,graphicsHeight),graphicsDepth,fullScreen,false,false,0);
    irr::video::IVideoDriver* driver = device->getVideoDriver();

    irr::gui::IGUIEnvironment* environment = device->getGUIEnvironment();

    #ifdef __APPLE__
    //Mac OS - cd back to original dir - seems to be changed during createDevice
    irr::io::IFileSystem* fileSystem = device->getFileSystem();
    if (fileSystem==0) {
        exit(EXIT_FAILURE); //Could not get file system TODO: Message for user
        std::cout << "Could not get filesystem" << std::endl;
    }
    fileSystem->changeWorkingDirectoryTo(exeFolderPath.c_str());
    #endif

    std::string fontName = IniFile::iniFileToString(iniFilename, "font");
    std::string fontPath = "media/fonts/" + fontName + "/" + fontName + "-" + std::to_string(fontSize) + ".xml";
    irr::gui::IGUIFont *font = environment->getFont(fontPath.c_str());
    if (font == NULL) {
        std::cout << "Could not load font, using fallback" << std::endl;
    } else {
        //set skin default font
        environment->getSkin()->setFont(font);
    }

    //Set gui colours
    irr::gui::IGUISkin* skin = environment->getSkin();

    skin->setColor(irr::gui::EGDC_3D_DARK_SHADOW ,irr::video::SColor(255,128,128,128));
    skin->setColor(irr::gui::EGDC_3D_SHADOW ,irr::video::SColor(255,190,190,190));
    skin->setColor(irr::gui::EGDC_3D_FACE ,irr::video::SColor(255,235,235,235));
    skin->setColor(irr::gui::EGDC_3D_LIGHT ,irr::video::SColor(255,255,255,255));

    irr::core::dimension2d<irr::u32> screenSize = driver->getScreenSize();
    irr::u32 width = screenSize.Width;
    irr::u32 height = screenSize.Height;

    //Do set-up here

    int pad = 10;

    irr::gui::IGUIButton* saveButton = environment->addButton(irr::core::rect<irr::s32>(pad, height-(50*fontScale), 150*fontScale, height-pad),0,SAVE_BUTTON,language.translate("save").c_str());

    irr::gui::IGUITabControl* tabbedPane = environment->addTabControl( irr::core::rect<irr::s32>(pad,pad,width-pad,height-(50*fontScale)-pad),0,true);

    //Add tab entry here
    for(int i = 0; i<iniFileStructure.size(); i++) {
        std::string tabName = iniFileStructure.at(i).tabName;
        irr::gui::IGUITab* thisTab = tabbedPane->addTab((irr::core::stringw(tabName.substr(1,tabName.length()-2).c_str())).c_str());

        //Add tab contents in a table
        irr::gui::IGUITable* thisTable = environment->addTable(irr::core::rect<irr::s32>(pad,pad,width-(3*pad),height-(50*fontScale)-2*(3*pad)),thisTab);

        thisTable->addColumn(language.translate("name").c_str());
        thisTable->addColumn(language.translate("value").c_str());
        thisTable->addColumn(language.translate("description").c_str());
        thisTable->setColumnWidth(0,300*fontScale);
        thisTable->setColumnWidth(2,2*width);
        thisTable->setToolTipText(language.translate("doubleClick").c_str());

        for (int j = 0; j<iniFileStructure.at(i).settings.size(); j++) {
            thisTable->addRow(j);
            thisTable->setCellText(j,0,irr::core::stringw(iniFileStructure.at(i).settings.at(j).settingName.c_str()).c_str()/*,video::SColor (255, 255, 255, 255)*/ );
            thisTable->setCellText(j,1,irr::core::stringw(iniFileStructure.at(i).settings.at(j).settingValue.c_str()).c_str()/*,video::SColor (255, 255, 255, 255)*/ );
            thisTable->setCellText(j,2,irr::core::stringw(iniFileStructure.at(i).settings.at(j).description.c_str()).c_str()/*,video::SColor (255, 255, 255, 255)*/ );
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
