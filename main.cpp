// main.cpp
// Include the Irrlicht header
#include "irrlicht.h"

#include "GUIMain.hpp"
#include "SimulationModel.hpp"
//#include "MyEventReceiver.hpp"

// Irrlicht Namespaces
using namespace irr;

// Define some values that we'll use to identify individual GUI controls.
enum //FIXME: This is now defined twice
{
	GUI_ID_HEADING_SCROLL_BAR = 101,
	GUI_ID_SPEED_SCROLL_BAR
};

class MyEventReceiver : public IEventReceiver
{
public:

    MyEventReceiver(SimulationModel* mdl, GUIMain* gui) //Constructor
	{
		model = mdl; //Link to the model
		guiMain = gui; //Link to GUI (Not currently used!)
		scrollBarPosSpeed = 0;
		scrollBarPosHeading = 0;
	}

    virtual bool OnEvent(const SEvent& event)
	{

        if (event.EventType == EET_GUI_EVENT)
		{
			s32 id = event.GUIEvent.Caller->getID();
            if (event.GUIEvent.EventType==gui::EGET_SCROLL_BAR_CHANGED)
            {

               if (id == GUI_ID_HEADING_SCROLL_BAR)
                  {
                      scrollBarPosHeading = ((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                      model->setHeading(scrollBarPosHeading);
                  }

              if (id == GUI_ID_SPEED_SCROLL_BAR)
                  {
                        scrollBarPosSpeed = ((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                        model->setSpeed(scrollBarPosSpeed);
                  }
            }
        }

        return false;

    }


	virtual s32 GetScrollBarPosSpeed() const
	{
		return scrollBarPosSpeed;
	}

	virtual s32 GetScrollBarPosHeading() const
	{
		return scrollBarPosHeading;
	}

private:

	s32 scrollBarPosSpeed, scrollBarPosHeading;
	SimulationModel* model; //Link to model so we can set model options
	GUIMain* guiMain;
};

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

    //main loop
    while(device->run())
    {

        model.updateModel();

        //Render
        driver->beginScene(true,true,video::SColor(255,100,101,140));
        smgr->drawAll();
        guiMain.drawGUI();
        driver->endScene();
    }

    device->drop();

    return(0);
}
