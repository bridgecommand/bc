// main.cpp
// Include the Irrlicht header
#include "irrlicht.h"

#include "GUIMain.hpp"
#include "SimulationModel.hpp"
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

std::string getScenarioName(std::string scenarioPath, IrrlichtDevice* device) {
    std::string scenarioName = "";
    std::vector<std::string> scenarioList;
    getScenarioList(scenarioList,scenarioPath, device);
    u32 scenarioInList = 1;
    for(std::vector<std::string>::iterator it = scenarioList.begin(); it != scenarioList.end(); ++it) {
        std::cout << "Scenario " << scenarioInList << ": " << *it << std::endl;
        scenarioInList++;
    }
    //Ask user to choose a scenario.
    u32 chosenScenario = 0;
    std::string tempInput = "";
    if (scenarioList.size()>0) {
        while (chosenScenario==0 || chosenScenario > scenarioList.size()) {
            std::cout << "Please choose a scenario: ";
            getline(std::cin,tempInput); //Read response into string
            std::stringstream myStream(tempInput);
            if (!(myStream >> chosenScenario)) {chosenScenario=0;} //Convert to int, or make sure it's 0 if failed.
        }
    scenarioName = scenarioList[chosenScenario-1];
    }
    return scenarioName;

}

int main()
{

    //create device
    IrrlichtDevice* device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(800,600),32,false,false,false,0);
    device->setWindowCaption(L"Bridge Command 5.0 Alpha");

    video::IVideoDriver* driver = device->getVideoDriver();
    scene::ISceneManager* smgr = device->getSceneManager();

    //Choose scenario
    std::string scenarioName = getScenarioName("Scenarios/",device); //Fixme: Scenarios path duplicated here and in SimulationModel
    //std::cout << "Chosen scenario \"" << scenarioName << "\""<<std::endl;

    //create GUI
    GUIMain guiMain(device);

    //seed random number generator
    std::srand(device->getTimer()->getTime());

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
