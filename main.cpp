// main.cpp
// Include the Irrlicht header
#include "irrlicht.h"

// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

//forward declaration of all classes (should these be in .h files?)
class SimulationModel;
class GUIMain;
class MyEventReceiver;

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
        hdgScrollbar = guienv->addScrollBar(false,rect<s32>(10, 240, 30, 470), 0, GUI_ID_HEADING_SCROLL_BAR);
        hdgScrollbar->setMax(360);

        spdScrollbar = guienv->addScrollBar(false,rect<s32>(40, 240, 60, 470), 0, GUI_ID_SPEED_SCROLL_BAR);
        spdScrollbar->setMax(100);

        //add heading indicator:
        hdgDisplay = guienv->addStaticText(L"Heading:", rect<s32>(20,20,120,40), true);
        guiHeading = 0;
    }

    void updateGuiHeading(irr::f32 hdg)
    {
        guiHeading = hdg;
    }

    void drawGUI()
    {
        //update heading display element
        hdgDisplay->setText(core::stringw(guiHeading).c_str());

        guienv->drawAll();
    }

private:
    IrrlichtDevice* device;
    IGUIEnvironment* guienv;

    IGUIScrollBar* spdScrollbar;
    IGUIScrollBar* hdgScrollbar;
    IGUIStaticText* hdgDisplay;

    irr::f32 guiHeading;

};

class SimulationModel //Start of the 'Model' part of MVC
{
private:
        IMeshSceneNode* ownShipNode;
        IrrlichtDevice* device;
        IVideoDriver* driver;
        ISceneManager* smgr;
        ICameraSceneNode* camera;
        GUIMain* guiMain;

        //Ship movement
        irr::f32 heading;
        irr::f32 xPos;
        irr::f32 yPos;
        irr::f32 zPos;
        irr::f32 speed;

        irr::u32 currentTime;
        irr::u32 previousTime;
        irr::f32 deltaTime;

    void setPosition(irr::f32 x, irr::f32 y, irr::f32 z)
    {
         ownShipNode->setPosition(vector3df(x,y,z));
    }

    void setRotation(irr::f32 rx, irr::f32 ry, irr::f32 rz)
    {
         ownShipNode->setRotation(vector3df(rx,ry,rz));
    }

public:

    void setSpeed(irr::f32 spd)
    {
         speed = spd;
    }

    void setHeading(irr::f32 hdg)
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
        xPos = xPos + sin(heading*irr::core::DEGTORAD)*speed*deltaTime;
        zPos = zPos + cos(heading*irr::core::DEGTORAD)*speed*deltaTime;

        //Set position & speed by calling our private methods
        setPosition(xPos,yPos,zPos);
        setRotation(0, heading, 0); //Global vectors

         //link camera rotation to shipNode
        // get transformation matrix of node
        irr::core::matrix4 m;
        m.setRotationDegrees(ownShipNode->getRotation());

        // transform forward vector of camera
        irr::core::vector3df frv(0.0f, 0.0f, 1.0f);
        m.transformVect(frv);

        // transform upvector of camera
        irr::core::vector3df upv(0.0f, 1.0f, 0.0f);
        m.transformVect(upv);

        // transform camera offset (thanks to Zeuss for finding it was missing)
        irr::core::vector3df offset(0.0f,0.9f,0.6f);
        m.transformVect(offset);

        //move camera and angle
        camera->setPosition(ownShipNode->getPosition() + offset); //position camera behind the ship
        camera->setUpVector(upv); //set up vector of camera
        camera->setTarget(ownShipNode->getPosition() + offset + frv); //set target of camera (look at point)
        camera->updateAbsolutePosition();

        //send data to gui
        guiMain->updateGuiHeading(heading);
    }

    SimulationModel(IrrlichtDevice* dev, IVideoDriver* drv, ISceneManager* scene, GUIMain* gui) //constructor, including own ship model
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
        IMesh* shipMesh = smgr->getMesh("Models/Ownship/Atlantic85/Hull.3ds");
        ownShipNode = smgr->addMeshSceneNode(shipMesh);
        if (ownShipNode) {ownShipNode->setMaterialFlag(EMF_LIGHTING, false);}

        //make a camera
        camera = smgr->addCameraSceneNode(0, vector3df(0,0,0), vector3df(0,0,1));

        //Add terrain
        ITerrainSceneNode* terrain = smgr->addTerrainSceneNode(
                       "World/SimpleEstuary/height.bmp",
                       0,					// parent node
                       -1,					// node id
		               core::vector3df(0.f, -44.07f, 0.f),		// position
		               core::vector3df(0.f, 180.f, 0.f),		// rotation (NOTE 180 deg rotation)
		               core::vector3df(6.97705f, 0.56498f, 8.6871f),	// scale
		               video::SColor ( 255, 255, 255, 255 ),	// vertexColor
		               5,					// maxLOD
		               scene::ETPS_17,		// patchSize
		               4					// smoothFactor
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
        waterNode->setMaterialFlag(EMF_LIGHTING, false);

        //sky box/dome
        // create skydome
	    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
        scene::ISceneNode* skydome=smgr->addSkyDomeSceneNode(driver->getTexture("media/sky.bmp"),16,8,0.95f,2.0f);
	    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

    } //end of SimulationModel constructor

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
            if (event.GUIEvent.EventType==EGET_SCROLL_BAR_CHANGED)
            {

               if (id == GUI_ID_HEADING_SCROLL_BAR)
                  {
                      scrollBarPosHeading = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                      model->setHeading(scrollBarPosHeading);
                  }

              if (id == GUI_ID_SPEED_SCROLL_BAR)
                  {
                        scrollBarPosSpeed = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                        model->setSpeed(scrollBarPosSpeed/10.0);
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
    IrrlichtDevice* device = createDevice(EDT_OPENGL, dimension2d<u32>(700,525),32,false,false,false,0);

    device->setWindowCaption(L"Bridge Command 5.Alpha - Irrlicht test example");

    IVideoDriver* driver = device->getVideoDriver();
    ISceneManager* smgr = device->getSceneManager();

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
        driver->beginScene(true,true,SColor(255,100,101,140));
        smgr->drawAll();
        guiMain.drawGUI();
        driver->endScene();
    }

    device->drop();

    return(0);
}
