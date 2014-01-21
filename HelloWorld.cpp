// HelloUniverse.cpp
// Include the Irrlicht header
#include "irrlicht.h"

// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

// Define some values that we'll use to identify individual GUI controls.
enum
{
	GUI_ID_HEADING_SCROLL_BAR = 101,
	GUI_ID_SPEED_SCROLL_BAR
};

class MyEventReceiver : public IEventReceiver
{
public:
	virtual bool OnEvent(const SEvent& event)
	{ 
        // Remember whether each key is down or up
		if (event.EventType == irr::EET_KEY_INPUT_EVENT)
			KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
        
        if (event.EventType == EET_GUI_EVENT)
		{
			s32 id = event.GUIEvent.Caller->getID();
            if (event.GUIEvent.EventType==EGET_SCROLL_BAR_CHANGED) 
            {
               
               if (id == GUI_ID_HEADING_SCROLL_BAR) 
                  scrollBarPosHeading = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                  
              if (id == GUI_ID_SPEED_SCROLL_BAR) 
                  scrollBarPosSpeed = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();    
            }
        }
        
        return false;
           
    }
    
    // This is used to check whether a key is being held down
	virtual bool IsKeyDown(EKEY_CODE keyCode) const
	{
		return KeyIsDown[keyCode];
	}
	
	virtual s32 GetScrollBarPosSpeed() const
	{
		return scrollBarPosSpeed;
	}
	
	virtual s32 GetScrollBarPosHeading() const
	{
		return scrollBarPosHeading;
	}
	
	MyEventReceiver() //Constructor?
	{
		for (u32 i=0; i<KEY_KEY_CODES_COUNT; ++i)
			KeyIsDown[i] = false;
		scrollBarPosSpeed = 0;
		scrollBarPosHeading = 0;
	}

private:
	// We use this array to store the current state of each key
	bool KeyIsDown[KEY_KEY_CODES_COUNT];
	s32 scrollBarPosSpeed, scrollBarPosHeading;
};



int main()
{
    
    //create event receiver
    MyEventReceiver receiver;
    
    //create device
    IrrlichtDevice* device = createDevice(EDT_OPENGL, dimension2d<s32>(640,480),32,false,false,false,&receiver);

    device->setWindowCaption(L"Irrlicht test example");
    
    IVideoDriver* driver = device->getVideoDriver();
    ISceneManager* smgr = device->getSceneManager();
    IGUIEnvironment* guienv = device->getGUIEnvironment();
    
    //gui
    IGUIScrollBar* hdgScrollbar = guienv->addScrollBar(false,rect<s32>(10, 240, 30, 470), 0, GUI_ID_HEADING_SCROLL_BAR);
    hdgScrollbar->setMax(360);
    
    IGUIScrollBar* spdScrollbar = guienv->addScrollBar(false,rect<s32>(40, 240, 60, 470), 0, GUI_ID_SPEED_SCROLL_BAR);
    spdScrollbar->setMax(100);
    
    //Load a ship model
    IMesh* shipMesh = smgr->getMesh("Models/Ownship/Atlantic85/Hull.3ds");
    IMeshSceneNode* shipNode = smgr->addMeshSceneNode(shipMesh);
    if (shipNode) {shipNode->setMaterialFlag(EMF_LIGHTING, false);}
    
    //make a camera, child of ship model
    //ICameraSceneNode* camera = smgr->addCameraSceneNode(shipNode, vector3df(0,0.9,0.6), vector3df(0,0.9,1));    
    ICameraSceneNode* camera = smgr->addCameraSceneNode(0, vector3df(0,0,0), vector3df(0,0,1));    
    
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
    waterNode = smgr->addWaterSurfaceSceneNode(waterMesh->getMesh(0), 1.0f, 300.0f, 10.0f);
    waterNode->setPosition(core::vector3df(0,-1.0f,0));

    waterNode->setMaterialTexture(0, driver->getTexture("media/water.bmp"));
    waterNode->setMaterialFlag(EMF_LIGHTING, false);
    
    //sky box/dome
    // create skydome
	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
    scene::ISceneNode* skydome=smgr->addSkyDomeSceneNode(driver->getTexture("media/sky.bmp"),16,8,0.95f,2.0f);
	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);
    
    //Ship movement
    irr::f32 heading = 70;
    irr::f32 xPos = 0;
    irr::f32 yPos = 0; 
    irr::f32 zPos = 0; 
    irr::f32 speed = 0.1; //Need to make FPS independent

    //main loop
    while(device->run())
    {
        //check for keys
        /*
        if(receiver.IsKeyDown(irr::KEY_LEFT))
                               heading--;             
                               
        if(receiver.IsKeyDown(irr::KEY_RIGHT))
                               heading++;
        
        if(receiver.IsKeyDown(irr::KEY_UP))
                               speed+=0.01f;             
                               
        if(receiver.IsKeyDown(irr::KEY_DOWN))
                               speed+=-0.01f;
        */
        
        heading = receiver.GetScrollBarPosHeading(); //Should probably work the other way, ie event receiver calls setSomething() on model
        speed = receiver.GetScrollBarPosSpeed()/100.f;
        
        //move
        xPos = xPos + sin(heading*irr::core::DEGTORAD)*speed;
        zPos = zPos + cos(heading*irr::core::DEGTORAD)*speed;
        
        //Set position
        shipNode->setPosition(vector3df(xPos,yPos,zPos));
        shipNode->setRotation(vector3df(0, heading, 0)); //Global vectors
        
        //link camera rotation to shipNode
        // get transformation matrix of node
        irr::core::matrix4 m;
        m.setRotationDegrees(shipNode->getRotation());
        
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
        camera->setPosition(shipNode->getPosition() + offset); //position camera behind the ship
        camera->setUpVector(upv); //set up vector of camera
        camera->setTarget(shipNode->getPosition() + offset + frv); //set target of camera (look at point)
        camera->updateAbsolutePosition();
        
        //Render
        driver->beginScene(true,true,SColor(255,100,101,140));
        smgr->drawAll();
        guienv->drawAll();
        driver->endScene();
    }
           
    device->drop();    
    
    return(0);
}
