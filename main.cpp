// main.cpp
// Include the Irrlicht header
#include "irrlicht.h"

// Irrlicht Namespaces
using namespace irr;

// Define some values that we'll use to identify individual GUI controls.
enum
{
	GUI_ID_HEADING_SCROLL_BAR = 101,
	GUI_ID_SPEED_SCROLL_BAR
};

class GUIMain //Create, build and update GUI
{
public:
    GUIMain(IrrlichtDevice* dev)
    {
        device = dev;
        guienv = device->getGUIEnvironment();

        //gui - add scroll bars
        hdgScrollbar = guienv->addScrollBar(false,core::rect<s32>(10, 240, 30, 470), 0, GUI_ID_HEADING_SCROLL_BAR);
        hdgScrollbar->setMax(360);

        spdScrollbar = guienv->addScrollBar(false,core::rect<s32>(40, 240, 60, 470), 0, GUI_ID_SPEED_SCROLL_BAR);
        spdScrollbar->setMax(20.f*1852.f/3600.f); //20 knots in m/s

        //add heading indicator:
        dataDisplay = guienv->addStaticText(L"", core::rect<s32>(20,20,120,40), true); //Actual text set later
        guiHeading = 0;
        guiSpeed = 0;
    }

    void updateGuiData(f32 hdg, f32 spd)
    {
        guiHeading = hdg; //Heading in degrees
        guiSpeed = spd; //Speed in knots
    }

    void drawGUI()
    {
        //update heading display element
        core::stringw displayText = L"Heading: "; //Do we need to destroy this when done?
        displayText.append(core::stringw(guiHeading));
        displayText.append(L"\nSpeed: ");
        displayText.append(core::stringw(guiSpeed));
        dataDisplay->setText(displayText.c_str());

        guienv->drawAll();
    }

private:
    IrrlichtDevice* device;
    gui::IGUIEnvironment* guienv;

    gui::IGUIScrollBar* spdScrollbar;
    gui::IGUIScrollBar* hdgScrollbar;
    gui::IGUIStaticText* dataDisplay;

    f32 guiHeading;
    f32 guiSpeed;

};

class SimulationModel //Start of the 'Model' part of MVC
{

public:

    SimulationModel(IrrlichtDevice* dev, video::IVideoDriver* drv, scene::ISceneManager* scene, GUIMain* gui) //constructor, including own ship model
    {
        //get reference to scene manager
        device = dev;
        driver = drv;
        smgr = scene;
        guiMain = gui;

        //initialise variables - Start with a hardcoded initial position
        heading = 0;
        xPos = 864.34f;
        yPos = 0;
        zPos = 619.317f;
        speed = 0; //Need to make FPS independent

        //store time
        previousTime = device->getTimer()->getTime();

        //Load a ship model
        scene::IMesh* shipMesh = smgr->getMesh("Models/Ownship/Atlantic85/Hull.3ds");
        ownShipNode = smgr->addMeshSceneNode(shipMesh);
        if (ownShipNode) {ownShipNode->setMaterialFlag(video::EMF_LIGHTING, false);}

        //make a camera
        camera = smgr->addCameraSceneNode(0, core::vector3df(0,0,0), core::vector3df(0,0,1));

        //Add terrain
        scene::ITerrainSceneNode* terrain = smgr->addTerrainSceneNode(
                       "World/SimpleEstuary/height.bmp",
                       0,					// parent node
                       -1,					// node id
		               core::vector3df(0.f, -44.07f, 0.f),		// position
		               core::vector3df(0.f, 180.f, 0.f),		// rotation (NOTE 180 deg rotation)
		               core::vector3df(6.97705f, 0.56498f, 8.6871f),	// scale
		               video::SColor ( 255, 255, 255, 255 ),	// vertexColor
		               5,					// maxLOD
		               scene::ETPS_17,		// patchSize
		               4					// smoothFactoespr
                       );
        terrain->setMaterialFlag(video::EMF_LIGHTING, false);
        terrain->setMaterialTexture(0, driver->getTexture("World/SimpleEstuary/texture.bmp"));

        //add some water (from demo 8)
        scene::IAnimatedMesh* waterMesh = smgr->addHillPlaneMesh( "myHill",
                       core::dimension2d<f32>(50,50),
                       core::dimension2d<u32>(100,100), 0, 0,
                       core::dimension2d<f32>(0,0),
                       core::dimension2d<f32>(10,10));

        scene::ISceneNode* waterNode = 0;
        waterNode = smgr->addWaterSurfaceSceneNode(waterMesh->getMesh(0), 0.5f, 300.0f, 10.0f);
        waterNode->setPosition(core::vector3df(0,-2*0.5f,0));

        waterNode->setMaterialTexture(0, driver->getTexture("media/water.bmp"));
        waterNode->setMaterialFlag(video::EMF_LIGHTING, false);

        //sky box/dome
        // create skydome
	    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
        scene::ISceneNode* skydome=smgr->addSkyDomeSceneNode(driver->getTexture("media/sky.bmp"),16,8,0.95f,2.0f);
	    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

        //make fog
        driver->setFog(video::SColor(128,128,128,128), video::EFT_FOG_LINEAR, 250, 1000, .003f, true, true);
        waterNode->setMaterialFlag(video::EMF_FOG_ENABLE, true);
        terrain->setMaterialFlag(video::EMF_FOG_ENABLE, true);
        skydome->setMaterialFlag(video::EMF_FOG_ENABLE, true);

    } //end of SimulationModel constructor

    void setSpeed(f32 spd)
    {
         speed = spd;
    }

    void setHeading(f32 hdg)
    {
         heading = hdg;
    }

    void updateModel()
    {

        //get delta time
        currentTime = device->getTimer()->getTime();
        deltaTime = (currentTime - previousTime)/1000.f; //Time in seconds
        previousTime = currentTime;

        //move, according to heading and speed
        xPos = xPos + sin(heading*core::DEGTORAD)*speed*deltaTime;
        zPos = zPos + cos(heading*core::DEGTORAD)*speed*deltaTime;

        //Set position & speed by calling our private methods
        setPosition(xPos,yPos,zPos);
        setRotation(0, heading, 0); //Global vectors

         //link camera rotation to shipNode
        // get transformation matrix of node
        core::matrix4 m;
        m.setRotationDegrees(ownShipNode->getRotation());

        // transform forward vector of camera
        core::vector3df frv(0.0f, 0.0f, 1.0f);
        m.transformVect(frv);

        // transform upvector of camera
        core::vector3df upv(0.0f, 1.0f, 0.0f);
        m.transformVect(upv);

        // transform camera offset (thanks to Zeuss for finding it was missing)
        core::vector3df offset(0.0f,0.9f,0.6f);
        m.transformVect(offset);

        //move camera and angle
        camera->setPosition(ownShipNode->getPosition() + offset); //position camera behind the ship
        camera->setUpVector(upv); //set up vector of camera
        camera->setTarget(ownShipNode->getPosition() + offset + frv); //set target of camera (look at point)
        camera->updateAbsolutePosition();

        //send data to gui
        guiMain->updateGuiData(heading, speed*3600.f/1852.f); //Set GUI heading in degrees and speed in knots
    }


private:
        IrrlichtDevice* device;
        scene::IMeshSceneNode* ownShipNode;
        video::IVideoDriver* driver;
        scene::ISceneManager* smgr;
        scene::ICameraSceneNode* camera;
        GUIMain* guiMain;

        //Ship movement
        f32 heading;
        f32 xPos;
        f32 yPos;
        f32 zPos;
        f32 speed;

        u32 currentTime;
        u32 previousTime;
        f32 deltaTime;

    void setPosition(f32 x, f32 y, f32 z)
    {
         ownShipNode->setPosition(core::vector3df(x,y,z));
    }

    void setRotation(f32 rx, f32 ry, f32 rz)
    {
         ownShipNode->setRotation(core::vector3df(rx,ry,rz));
    }

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
