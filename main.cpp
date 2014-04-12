// main.cpp
// Include the Irrlicht header
#include "irrlicht.h"

#include "GUIMain.hpp"
#include "SimulationModel.hpp"
#include "MyEventReceiver.hpp"
#include "Network.hpp"

#include <iostream>
#include <fstream> //for ini loading
#include <string> //for ini loading
#include <boost/algorithm/string.hpp> //for ini loading

// Irrlicht Namespaces
using namespace irr;

//Utility functions
namespace IniFile
{
    std::string iniFileToString(std::string fileName, std::string command)
    {
        std::ifstream file (fileName.c_str());
        std::string valuePart = "";
        if (file.is_open())
        {
            std::string line;
            while ( std::getline (file,line) )
            {
                boost::to_lower(command);
                boost::to_lower(line); //Make lowercase //FIXME - Probably don't want to do this
                std::size_t commandFound = line.find(command); //Check if the 'command' is found (returns std::string::npos if not)
                std::size_t equalsFound = line.find("="); //Check if the '=' is found
                if (commandFound!=std::string::npos && equalsFound!=std::string::npos && equalsFound>commandFound)
                {
                    //get the value
                    try {
                        valuePart = line.substr(equalsFound+1,std::string::npos);//from equals to end, not including the equals
                    }
                    catch (const std::out_of_range& oor) {
                        std::cerr << "Could not get value for: " << command << " " << oor.what() << '\n';
                    }
                }

            }
            file.close();
        }
        else std::cout << "Unable to open file " << fileName << std::endl;

        return valuePart;
    }

    //Load unsigned integer from an ini file
    u32 iniFileToul(std::string fileName, std::string command)
    {
        u32 result = 0;
        std::string valueString = iniFileToString(fileName, command); //Get the value as a string
        result = core::strtoul10(valueString.c_str()); //Convert this into an unsigned int
        return result;
    }
}

int main()
{

    //create device
    IrrlichtDevice* device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(700,525),32,false,false,false,0);

    device->setWindowCaption(L"Bridge Command 5.Alpha - Irrlicht test example");

    video::IVideoDriver* driver = device->getVideoDriver();
    scene::ISceneManager* smgr = device->getSceneManager();

    //create GUI
    GUIMain guiMain(device);

    //Create simulation model
    SimulationModel model (device, driver, smgr, &guiMain); //Add link to gui

    //create event receiver, linked to model
    MyEventReceiver receiver(&model, &guiMain);
    device->setEventReceiver(&receiver);

    //Create networking, linked to model
    Network network(&model);
    network.connectToServer();

    //main loop
    while(device->run())
    {

        network.updateNetwork();
        model.updateModel();

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
