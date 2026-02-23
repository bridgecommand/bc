#include "JoyStick.hpp"
#include "GUIMain.hpp"
#include "SimulationModel.hpp"
#include <SDL.h>
#include <SDL_main.h>

JoyStick::JoyStick(void)
{


}

JoyStick::JoyStick(sJsMapping aJsMapping)
{
   mJsMapping = aJsMapping;
}


JoyStick::~JoyStick(void)
{

}

bool JoyStick::Init(void *aModel, void *aGuiMain)
{
    SDL_Joystick* js;

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        SDL_JoystickEventState(SDL_ENABLE);
        std::cerr << "SDL_Init error : " << SDL_GetError() << std::endl;
        return false;
    }

    mNumJoysticks = SDL_NumJoysticks();

    if(mNumJoysticks < 1 || mNumJoysticks > MAX_JS)
    {
        SDL_Quit();
        return false;
    }

    std::cout << "::::::JoyStick Parameters::::::" << std::endl;
    for (unsigned char i = 0; i < mNumJoysticks; i++)
    {
        js = SDL_JoystickOpen(i);

        if (!js)
        {
            std::cerr << "Unable to open js " << SDL_GetError() << std::endl;
            SDL_Quit();
            return false;
        }

        std::cout << "Name : " << SDL_JoystickName(js) << std::endl;
        std::cout << "Axes : " << SDL_JoystickNumAxes(js) << std::endl;
        std::cout << "Buttons : " << SDL_JoystickNumButtons(js) << std::endl;
        std::cout << "POV : " << SDL_JoystickNumHats(js) << std::endl << std::endl;

        SDL_JoystickClose(js);
    }
    std::cout << "::::::::::::" << std::endl;

    std::thread pollingTask(Process, mNumJoysticks, mJsMapping, aModel, aGuiMain);
    pollingTask.detach();

    return true;
}

void JoyStick::Process(int aNumJoysticks, sJsMapping& aJsMapping, void* aModel, void* aGuiMain)
{
    SDL_Joystick* js;
    SDL_Event event;
    SimulationModel* pModel = (SimulationModel*)aModel;
    GUIMain* pGuiMain = (GUIMain*)aGuiMain;
    float axisValue = 0;

    while(true)
    {
        for (unsigned char i = 0; i < aNumJoysticks; i++)
        {
            js = SDL_JoystickOpen(i);

            if (!js)
            {
                std::cerr << "Unable to open js " << SDL_GetError() << std::endl;
                SDL_Quit();
                return;
            }

            while (SDL_PollEvent(&event))
            {
                //std::cout << "Event type: " << event.type << std::endl;

                switch (event.type) 
                {
                case SDL_QUIT:
                    break;

                case SDL_JOYAXISMOTION:
                    axisValue = event.jaxis.value / 32767.0f;
                    //std::cout << "Axe " << (int)event.jaxis.axis << " = " << axisValue << std::endl;
                    for (unsigned char j = 0; j < MAX_JS_AXIS; j++)
                    {
                        if (aJsMapping.entry[j].jsNumber == i) //Right JS ?
                        {
                            if (aJsMapping.entry[j].type == AXIS) //Right Type ?
                            { 
                                if ((int)event.jaxis.axis == aJsMapping.entry[j].channel) //Right Channel ?
                                {
                                    if(0 == j)//Port
                                     pModel->getOwnShip()->setPortEngine(axisValue);
                                    else if(1 == j) //Stbd
                                     pModel->getOwnShip()->setPortEngine(axisValue);
                                    else if (2 == j) //Rudder
                                     pModel->getOwnShip()->setWheel(axisValue*(pModel->getOwnShip()->getRudder().getDeltaMax()*180/M_PI));
                                }
                            }
                        }
                    }
                    break;

                case SDL_JOYBUTTONDOWN:
                    std::cout << "Button " << (int)event.jbutton.button << " push" << std::endl;
                    for (unsigned char j = MAX_JS_AXIS + MAX_JS_POV; j < MAX_JS_ENTRY; j++)
                    {
                        if (aJsMapping.entry[j].jsNumber == i) //Right JS ?
                        {
                            if (aJsMapping.entry[j].type == BUTTON) //Right Type ?
                            {
                                if ((int)event.jbutton.button == aJsMapping.entry[j].channel) //Right Button ?
                                {
                                    if (3 == j)//Horn
                                        pModel->getSound()->startHorn();
                                    else if (4 == j)//change view
                                    {
                                        pModel->getCamera()->changeView();
                                        pModel->setMoveViewWithPrimary(true);
                                    }
                                    else if (5 == j)//change and lock view
                                    {
                                        pModel->getCamera()->changeView();
                                        pModel->setMoveViewWithPrimary(false);
                                    }
                                    else if (6 == j)//look step left
                                        pModel->getCamera()->lookStepLeft();
                                    else if (7 == j)//look step right
                                        pModel->getCamera()->lookStepRight();
                                    else if (8 == j)//bearing on
                                        pGuiMain->showBearings();
                                    else if (9 == j)//bearing off
                                        pGuiMain->hideBearings();
                                    else if (10 == j)//zoom on
                                    {
                                        pGuiMain->zoomOn();
                                        pModel->setZoom(true);
                                    }
                                    else if (11 == j)//zoom off
                                    {
                                        pGuiMain->zoomOff();
                                        pModel->setZoom(false);
                                    }
                                    else if (12 == j)
                                        pModel->getCamera()->setPanSpeed(-5);
                                    else if (13 == j)
                                        pModel->getCamera()->setPanSpeed(5);
                                    else if (14 == j)
                                        pModel->getCamera()->setVerticalPanSpeed(5);
                                    else if (15 == j)
                                        pModel->getCamera()->setVerticalPanSpeed(-5);
                                }
                            }
                        }
                    }
                    break;

                case SDL_JOYBUTTONUP:
                    //std::cout << "Button " << (int)event.jbutton.button << " release" << std::endl;
                    for (unsigned char j = MAX_JS_AXIS + MAX_JS_POV; j < MAX_JS_ENTRY; j++)
                    {
                        if (aJsMapping.entry[j].jsNumber == i) //Right JS ?
                        {
                            if (aJsMapping.entry[j].type == BUTTON) //Right Type ?
                            {
                                if ((int)event.jbutton.button == aJsMapping.entry[j].channel) //Right Button ?
                                {
                                    if (3 == j)//Horn
                                        pModel->getSound()->endHorn();
                                    else if (12 == j)
                                        pModel->getCamera()->setPanSpeed(0);
                                    else if (13 == j)
                                        pModel->getCamera()->setPanSpeed(0);
                                    else if (14 == j)
                                        pModel->getCamera()->setVerticalPanSpeed(0);
                                    else if (15 == j)
                                        pModel->getCamera()->setVerticalPanSpeed(0);
                                }
                            }
                        }
                    }
                    break;

                case SDL_JOYHATMOTION:
                    //std::cout << "Pov " << (int)event.jhat.hat << " = " << (int)event.jhat.value << std::endl;
                    break;
                }
            }
            SDL_JoystickClose(js);
        }
    }
}
