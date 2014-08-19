// main.cpp
// Include the Irrlicht header
#include "irrlicht.h"

#include "GUIMain.hpp"
#include "SimulationModel.hpp"
#include "StartupEventReceiver.hpp"
#include "MyEventReceiver.hpp"
#include "Network.hpp"

#include <cstdlib> //For rand(), srand()
#include <vector>
#include <sstream>

// Irrlicht Namespaces
using namespace irr;

void getScenarioList(std::vector<std::string>&scenarioList, std::string scenarioPath, IrrlichtDevice* device) {

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

int main()
{

    //create device
    IrrlichtDevice* device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(800,600),32,false,false,false,0);
    device->setWindowCaption(L"Bridge Command 5.0 Alpha");

    video::IVideoDriver* driver = device->getVideoDriver();
    scene::ISceneManager* smgr = device->getSceneManager();

    //Choose scenario
    std::vector<std::string> scenarioList;
    getScenarioList(scenarioList,"Scenarios/", device); //Populate list //Fixme: Scenarios path duplicated here and in SimulationModel
    const s32 LISTBOXID = 10; //Fixme
    const s32 OKBUTTONID = 11;
    gui::IGUIListBox* scenarioListBox = device->getGUIEnvironment()->addListBox(core::rect<s32>(10,10,110,210),0,LISTBOXID);
    gui::IGUIButton* okButton = device->getGUIEnvironment()->addButton(core::rect<s32>(10,220,110,240),0,OKBUTTONID,L"OK");
    for (std::vector<std::string>::iterator it = scenarioList.begin(); it != scenarioList.end(); ++it) {
        scenarioListBox->addItem(core::stringw(it->c_str()).c_str()); //Fixme!
    }
    StartupEventReceiver startupReceiver(scenarioListBox,LISTBOXID,OKBUTTONID);
    device->setEventReceiver(&startupReceiver);
    while(device->run() && startupReceiver.getScenarioSelected()==-1) {
        if (device->isWindowActive())
        {
            driver->beginScene(true, true, video::SColor(0,200,200,200));
            device->getGUIEnvironment()->drawAll();
            driver->endScene();
        }
    }
    scenarioListBox->remove();
    okButton->remove();
    std::string scenarioName = scenarioList[startupReceiver.getScenarioSelected()];

    //seed random number generator
    std::srand(device->getTimer()->getTime());

    //create GUI
    GUIMain guiMain(device);

    //Create simulation model
    SimulationModel model (device, smgr, &guiMain, scenarioName);

    //create event receiver, linked to model
    MyEventReceiver receiver(&model, &guiMain);
    device->setEventReceiver(&receiver);

    //Create networking, linked to model
    Network network(&model);
    network.connectToServer();

    //main loop
    while(device->run())
    {

        network.update();
        model.update();

        //Render
        driver->beginScene(true,true,video::SColor(255,100,101,140));
        smgr->drawAll();
        guiMain.drawGUI();
        driver->endScene();

    }

    device->drop();
    //networking should be stopped (presumably with destructor when it goes out of scope?)


    return(0);
}
