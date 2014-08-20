#include "ScenarioChoice.hpp"
#include "StartupEventReceiver.hpp"

using namespace irr;

ScenarioChoice::ScenarioChoice(irr::IrrlichtDevice* device)
{
    this->device = device;
}

std::string ScenarioChoice::chooseScenario()
{
    video::IVideoDriver* driver = device->getVideoDriver();
    std::vector<std::string> scenarioList;
    getScenarioList(scenarioList,"Scenarios/"); //Populate list //Fixme: Scenarios path duplicated here and in SimulationModel
    const s32 LISTBOXID = 10; //Fixme - should be an enum?
    const s32 OKBUTTONID = 11;
    gui::IGUIListBox* scenarioListBox = device->getGUIEnvironment()->addListBox(core::rect<s32>(10,10,110,210),0,LISTBOXID);
    gui::IGUIButton* okButton = device->getGUIEnvironment()->addButton(core::rect<s32>(10,220,110,240),0,OKBUTTONID,L"OK"); //i18n?
    for (std::vector<std::string>::iterator it = scenarioList.begin(); it != scenarioList.end(); ++it) {
        scenarioListBox->addItem(core::stringw(it->c_str()).c_str()); //Fixme!
    }
    StartupEventReceiver startupReceiver(scenarioListBox,LISTBOXID,OKBUTTONID);
    device->setEventReceiver(&startupReceiver);
    while(device->run() && startupReceiver.getScenarioSelected()==-1) { //Event receiver will set Scenario Selected, so we just loop here until that happens
        if (device->isWindowActive())
        {
            driver->beginScene(true, true, video::SColor(0,200,200,200));
            device->getGUIEnvironment()->drawAll();
            driver->endScene();
        }
    }
    scenarioListBox->remove(); scenarioListBox = 0;
    okButton->remove(); okButton = 0;
    device->setEventReceiver(0); //Remove link to startup event receiver, as this will be destroyed.
    std::string scenarioName = scenarioList[startupReceiver.getScenarioSelected()];

    //Show loading message
    gui::IGUIStaticText* loadingMessage = device->getGUIEnvironment()->addStaticText(L"Loading...", core::rect<s32>(10,10,210,20)); //i18n
    device->run();
    driver->beginScene(true, true, video::SColor(0,200,200,200));
    device->getGUIEnvironment()->drawAll();
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
